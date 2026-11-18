#include "sudoManager.h"
#include "groupManager.h"
#include "commandRunner.h"
#include "logger.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <vector>

QList<UserSudoStatus> SudoManager::getAllUsersSudoStatus()
{
    QList<UserSudoStatus> list;
    const QStringList users = GroupManager::getManageableUsers();

    for (const QString &uname : users) {
        list.append(getUserSudoStatus(uname));
    }

    return list;
}

UserSudoStatus SudoManager::getUserSudoStatus(const QString &username)
{
    UserSudoStatus status;
    status.username = username.trimmed();

    if (status.username.isEmpty()) {
        return status;
    }

    status.inWheelGroup = isUserInGroup(status.username, "wheel");

    // 1. Verificar archivo dedicado administrado por slackwiz
    const QString managedFile = QString("/etc/sudoers.d/slackwiz_%1").arg(status.username);
    if (QFile::exists(managedFile)) {
        const SudoAccessMode fileMode = parseRuleFromFile(managedFile, status.username);
        if (fileMode != SudoAccessMode::Disabled) {
            status.accessMode = fileMode;
            status.sourceDetail = QString("Regla activa en %1").arg(managedFile);
            LOG_INFO("SudoManager", QString("Usuario '%1': %2").arg(status.username, status.sourceDetail));
            return status;
        }
    }

    // 2. Verificar si existe regla directa en /etc/sudoers u otros archivos en /etc/sudoers.d/
    QStringList filesToCheck;
    filesToCheck << "/etc/sudoers";

    QDir sudoersDir("/etc/sudoers.d");
    if (sudoersDir.exists()) {
        const QStringList entries = sudoersDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString &entry : entries) {
            if (entry.endsWith('~') || entry.contains('.')) {
                continue; // sudo ignora archivos con '.' o '~' en sudoers.d
            }
            filesToCheck << sudoersDir.filePath(entry);
        }
    }

    for (const QString &path : filesToCheck) {
        const SudoAccessMode directMode = parseRuleFromFile(path, status.username);
        if (directMode != SudoAccessMode::Disabled) {
            status.accessMode = directMode;
            status.sourceDetail = QString("Regla directa en %1").arg(path);
            return status;
        }

        if (status.inWheelGroup) {
            const SudoAccessMode wheelMode = parseRuleFromFile(path, "%wheel");
            if (wheelMode != SudoAccessMode::Disabled) {
                status.accessMode = wheelMode;
                status.sourceDetail = QString("Heredado por grupo 'wheel' en %1").arg(path);
                return status;
            }
        }
    }

    status.accessMode = SudoAccessMode::Disabled;
    status.sourceDetail = "Sin privilegios sudo";
    return status;
}

SudoOperationResult SudoManager::applyUserSudo(const QString &username,
                                               SudoAccessMode mode,
                                               bool syncWheelGroup)
{
    SudoOperationResult result;
    const QString cleanUser = username.trimmed();

    LOG_INFO("SudoManager", QString("Configurando sudo para usuario '%1'...").arg(cleanUser));

    if (!CommandRunner::isRoot()) {
        result.success = false;
        result.message = "No tienes permisos de administrador (root). Ejecuta slackwiz como root para configurar sudo.";
        LOG_ERROR("SudoManager", result.message);
        return result;
    }

    if (cleanUser.isEmpty() || cleanUser == "root") {
        result.success = false;
        result.message = "Selecciona un usuario estándar válido.";
        LOG_ERROR("SudoManager", result.message);
        return result;
    }

    if (getpwnam(cleanUser.toUtf8().constData()) == nullptr) {
        result.success = false;
        result.message = QString("El usuario '%1' no existe en el sistema.").arg(cleanUser);
        LOG_ERROR("SudoManager", result.message);
        return result;
    }

    const QString managedFile = QString("/etc/sudoers.d/slackwiz_%1").arg(cleanUser);

    // Caso 1: Deshabilitar sudo para este usuario
    if (mode == SudoAccessMode::Disabled) {
        if (QFile::exists(managedFile)) {
            if (!QFile::remove(managedFile)) {
                result.success = false;
                result.message = QString("No se pudo eliminar el archivo de configuración %1.").arg(managedFile);
                LOG_ERROR("SudoManager", result.message);
                return result;
            }
            LOG_SUCCESS("SudoManager", QString("Archivo eliminado: %1").arg(managedFile));
        }

        // Comprobar si aún conserva sudo por alguna regla externa (%wheel o /etc/sudoers)
        const UserSudoStatus postStatus = getUserSudoStatus(cleanUser);
        if (postStatus.accessMode != SudoAccessMode::Disabled) {
            result.success = true;
            result.message = QString(
                "Se eliminó la regla individual de slackwiz para '%1'.\n"
                "Aviso: El usuario aún conserva permisos sudo mediante: %2.\n"
                "Si deseas revocarlo por completo, quítalo del grupo 'wheel' en el paso '4. Grupos de usuario'."
            ).arg(cleanUser, postStatus.sourceDetail);
            LOG_WARN("SudoManager", result.message);
            return result;
        }

        result.success = true;
        result.message = QString("Privilegios sudo deshabilitados correctamente para el usuario '%1'.").arg(cleanUser);
        LOG_SUCCESS("SudoManager", result.message);
        return result;
    }

    // Caso 2: Habilitar sudo (Con contraseña o NOPASSWD)
    if (!ensureSudoersIncludeDir()) {
        result.success = false;
        result.message = "No se pudo verificar o habilitar '#includedir /etc/sudoers.d' en /etc/sudoers.";
        LOG_ERROR("SudoManager", result.message);
        return result;
    }

    const QString sudoersDirPath = "/etc/sudoers.d";
    QDir dir;
    if (!dir.exists(sudoersDirPath)) {
        if (!dir.mkpath(sudoersDirPath)) {
            result.success = false;
            result.message = "No se pudo crear el directorio /etc/sudoers.d.";
            LOG_ERROR("SudoManager", result.message);
            return result;
        }
    }
    chmod(sudoersDirPath.toUtf8().constData(), 0750);
    chown(sudoersDirPath.toUtf8().constData(), 0, 0);

    const QString ruleSpec = (mode == SudoAccessMode::NoPassword)
                                 ? "ALL=(ALL:ALL) NOPASSWD: ALL"
                                 : "ALL=(ALL:ALL) ALL";

    const QString content = QString(
        "# Configuración de sudo generada por Slackware Wizard (slackwiz)\n"
        "%1 %2\n"
    ).arg(cleanUser, ruleSpec);

    if (!CommandRunner::writeFileWithBackup(managedFile, content, false)) {
        result.success = false;
        result.message = QString("Error al escribir la regla sudo en %1.").arg(managedFile);
        LOG_ERROR("SudoManager", result.message);
        return result;
    }

    if (!secureSudoersFile(managedFile)) {
        QFile::remove(managedFile);
        result.success = false;
        result.message = QString("No se pudieron aplicar permisos seguros (0440 root:root) en %1.").arg(managedFile);
        LOG_ERROR("SudoManager", result.message);
        return result;
    }

    // Validar sintaxis con visudo -cf antes de dar por válida la configuración
    if (!validateSudoersSyntax(managedFile)) {
        QFile::remove(managedFile);
        result.success = false;
        result.message = "La validación de sintaxis con 'visudo' falló. Se revirtió el cambio por seguridad.";
        LOG_ERROR("SudoManager", result.message);
        return result;
    }

    // Sincronizar grupo wheel si el usuario lo solicitó
    QString wheelNote;
    if (syncWheelGroup && !isUserInGroup(cleanUser, "wheel")) {
        if (getgrnam("wheel") != nullptr) {
            const CommandResult modRes = CommandRunner::execute("/usr/sbin/usermod", QStringList() << "-aG" << "wheel" << cleanUser);
            if (modRes.success) {
                wheelNote = "\n• Se añadió el usuario al grupo 'wheel' (estándar administrativo en Slackware).";
                LOG_SUCCESS("SudoManager", QString("Usuario '%1' añadido al grupo wheel.").arg(cleanUser));
            } else {
                LOG_WARN("SudoManager", QString("No se pudo añadir '%1' al grupo wheel: %2").arg(cleanUser, modRes.stdErr));
            }
        }
    }

    const QString modeDesc = (mode == SudoAccessMode::NoPassword)
                                 ? "Habilitado sin contraseña (NOPASSWD)"
                                 : "Habilitado solicitando contraseña del usuario";

    result.success = true;
    result.message = QString(
        "Configuración de sudo guardada y validada correctamente para '%1':\n"
        "• Modo: %2\n"
        "• Archivo generado: %3 (permisos 0440, validado con visudo)%4\n\n"
        "El usuario '%1' ya puede ejecutar comandos administrativos anteponiendo 'sudo'."
    ).arg(cleanUser, modeDesc, managedFile, wheelNote);

    LOG_SUCCESS("SudoManager", QString("Sudo configurado correctamente para '%1'.").arg(cleanUser));
    return result;
}

bool SudoManager::ensureSudoersIncludeDir()
{
    const QString sudoersPath = "/etc/sudoers";
    QFile file(sudoersPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR("SudoManager", "No se pudo abrir /etc/sudoers para verificar includedir.");
        return false;
    }

    QStringList lines;
    QTextStream in(&file);
    bool hasIncludeDir = false;

    while (!in.atEnd()) {
        const QString line = in.readLine();
        const QString trimmed = line.trimmed();
        // En sudoers, '#includedir /etc/sudoers.d' o '@includedir /etc/sudoers.d' son directivas activas
        if (trimmed == "#includedir /etc/sudoers.d" || trimmed == "@includedir /etc/sudoers.d") {
            hasIncludeDir = true;
        }
        lines.append(line);
    }
    file.close();

    if (hasIncludeDir) {
        LOG_INFO("SudoManager", "/etc/sudoers ya tiene activo includedir /etc/sudoers.d");
        return true;
    }

    LOG_WARN("SudoManager", "Activando '#includedir /etc/sudoers.d' en /etc/sudoers...");
    lines.append("");
    lines.append("#includedir /etc/sudoers.d");
    lines.append("");

    if (!CommandRunner::writeFileWithBackup(sudoersPath, lines.join('\n'), true)) {
        return false;
    }

    secureSudoersFile(sudoersPath);
    return validateSudoersSyntax(sudoersPath);
}

bool SudoManager::validateSudoersSyntax(const QString &filePath)
{
    const QString visudoBin = QFile::exists("/usr/sbin/visudo") ? "/usr/sbin/visudo" : "visudo";
    const CommandResult res = CommandRunner::execute(visudoBin, QStringList() << "-cf" << filePath);
    return res.success;
}

bool SudoManager::secureSudoersFile(const QString &filePath)
{
    const QByteArray pathBytes = filePath.toUtf8();
    if (chown(pathBytes.constData(), 0, 0) != 0) {
        LOG_WARN("SudoManager", QString("No se pudo establecer chown 0:0 en %1").arg(filePath));
    }
    return (chmod(pathBytes.constData(), 0440) == 0);
}

bool SudoManager::isUserInGroup(const QString &username, const QString &groupName)
{
    const QByteArray utf8User = username.toUtf8();
    struct passwd *pw = getpwnam(utf8User.constData());
    if (!pw) {
        return false;
    }

    struct group *targetGr = getgrnam(groupName.toUtf8().constData());
    if (!targetGr) {
        return false;
    }

    if (pw->pw_gid == targetGr->gr_gid) {
        return true;
    }

    int ngroups = 32;
    std::vector<gid_t> gids(static_cast<size_t>(ngroups));
    if (getgrouplist(utf8User.constData(), pw->pw_gid, gids.data(), &ngroups) == -1) {
        gids.resize(static_cast<size_t>(ngroups));
        getgrouplist(utf8User.constData(), pw->pw_gid, gids.data(), &ngroups);
    }

    for (int i = 0; i < ngroups; ++i) {
        if (gids[static_cast<size_t>(i)] == targetGr->gr_gid) {
            return true;
        }
    }

    return false;
}

SudoAccessMode SudoManager::parseRuleFromFile(const QString &filePath, const QString &subject)
{
    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return SudoAccessMode::Disabled;
    }

    QTextStream in(&file);
    const QString escapedSubject = QRegularExpression::escape(subject);
    QRegularExpression ruleRegex(QString(R"(^\s*%1\s+ALL\s*=\s*\([^\)]+\)\s*(NOPASSWD:\s*)?ALL\b)").arg(escapedSubject));

    SudoAccessMode detected = SudoAccessMode::Disabled;
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        QRegularExpressionMatch match = ruleRegex.match(line);
        if (match.hasMatch()) {
            if (!match.captured(1).trimmed().isEmpty()) {
                detected = SudoAccessMode::NoPassword;
            } else {
                detected = SudoAccessMode::WithPassword;
            }
        }
    }
    file.close();
    return detected;
}
