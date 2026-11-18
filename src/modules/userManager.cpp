#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QProcess>
#include <pwd.h>
#include <grp.h>
#include "userManager.h"
#include "commandRunner.h"
#include "logger.h"

QList<SystemGroupOption> UserManager::getAvailableGroups()
{
    QList<SystemGroupOption> groups = {
        { "wheel",   "Administrador del sistema (permite usar sudo)",          true,  true },
        { "audio",   "Dispositivos de sonido y multimedia (ALSA / PipeWire)",  true,  true },
        { "video",   "Aceleración gráfica 3D, cámaras web y pantallas",        true,  true },
        { "plugdev", "Montaje automático de memorias USB y discos externos",   true,  true },
        { "power",   "Apagar, reiniciar y suspender desde el entorno gráfico", true,  true },
        { "netdev",  "Gestión de conexiones Wi-Fi y red (NetworkManager)",     true,  true },
        { "cdrom",   "Lectura y grabación en unidades ópticas CD/DVD",         true,  true },
        { "scanner", "Acceso a escáneres de documentos",                       true,  true },
        { "lp",      "Impresoras y administración de trabajos (CUPS)",         true,  true },
        { "floppy",  "Unidades de disquete y medios extraíbles heredados",     false, true }
    };

    for (auto &g : groups) {
        g.existsOnSystem = groupExists(g.name);
        if (!g.existsOnSystem) {
            LOG_WARN("UserManager", QString("El grupo '%1' no existe en /etc/group de este sistema.").arg(g.name));
        }
    }

    return groups;
}

QStringList UserManager::getAvailableShells()
{
    QStringList shells;
    const QStringList candidates = { "/bin/bash", "/bin/zsh", "/bin/sh", "/bin/tcsh" };

    for (const QString &sh : candidates) {
        if (QFile::exists(sh)) {
            shells.append(sh);
        }
    }

    if (shells.isEmpty()) {
        shells.append("/bin/bash");
    }

    return shells;
}

QStringList UserManager::getExistingHumanUsers()
{
    QStringList users;
    QFile passwdFile("/etc/passwd");

    if (!passwdFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_WARN("UserManager", "No se pudo leer /etc/passwd para listar usuarios.");
        return users;
    }

    QTextStream in(&passwdFile);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        const QStringList parts = line.split(':');
        if (parts.size() >= 7) {
            const QString uname = parts[0];
            bool ok = false;
            const int uid = parts[2].toInt(&ok);
            const QString shell = parts[6];

            if (ok && uid >= 1000 && uid < 65534 && uname != "nobody" && !shell.endsWith("nologin") && !shell.endsWith("false")) {
                users.append(uname);
            }
        }
    }
    passwdFile.close();

    LOG_INFO("UserManager", QString("Usuarios humanos detectados en el sistema: %1")
                                .arg(users.isEmpty() ? "ninguno" : users.join(", ")));
    return users;
}

bool UserManager::userExists(const QString &username)
{
    const QByteArray utf8Name = username.toUtf8();
    struct passwd *pw = getpwnam(utf8Name.constData());
    return (pw != nullptr);
}

bool UserManager::groupExists(const QString &groupName)
{
    const QByteArray utf8Group = groupName.toUtf8();
    struct group *gr = getgrnam(utf8Group.constData());
    return (gr != nullptr);
}

UserOperationResult UserManager::createUser(const NewUserRequest &request)
{
    UserOperationResult result;
    const QString cleanUser = request.username.trimmed();
    const QString cleanFullName = request.fullName.trimmed();
    const QString cleanShell = request.shell.trimmed().isEmpty() ? "/bin/bash" : request.shell.trimmed();

    LOG_INFO("UserManager", QString("Iniciando creación de usuario: '%1'").arg(cleanUser));

    // 1. Verificar permisos root
    if (!CommandRunner::isRoot()) {
        result.success = false;
        result.message = "No tienes permisos de administrador (root). Ejecuta slackwiz como root para crear usuarios.";
        LOG_ERROR("UserManager", result.message);
        return result;
    }

    // 2. Validar nombre de usuario
    QRegularExpression validUserRegex(R"(^[a-z_][a-z0-9_-]{0,31}$)");
    if (cleanUser.isEmpty() || !validUserRegex.match(cleanUser).hasMatch()) {
        result.success = false;
        result.message = "El nombre de usuario no es válido. Usa solo letras minúsculas, números, guiones (-) o guiones bajos (_), comenzando por una letra.";
        LOG_ERROR("UserManager", result.message);
        return result;
    }

    if (userExists(cleanUser)) {
        result.success = false;
        result.message = QString("El usuario '%1' ya existe en el sistema. Elige otro nombre de usuario.").arg(cleanUser);
        LOG_ERROR("UserManager", result.message);
        return result;
    }

    // 3. Validar nombre completo (evitar ':' que rompe el formato de /etc/passwd)
    if (cleanFullName.contains(':')) {
        result.success = false;
        result.message = "El nombre completo no puede contener el carácter dos puntos (:).";
        LOG_ERROR("UserManager", result.message);
        return result;
    }

    // 4. Validar contraseñas
    if (request.password.isEmpty()) {
        result.success = false;
        result.message = "Debes ingresar una contraseña para el nuevo usuario.";
        LOG_ERROR("UserManager", result.message);
        return result;
    }

    if (request.password != request.confirmPassword) {
        result.success = false;
        result.message = "Las contraseñas ingresadas no coinciden. Verifica ambos campos.";
        LOG_ERROR("UserManager", result.message);
        return result;
    }

    if (request.password.contains('\n') || request.password.contains('\r')) {
        result.success = false;
        result.message = "La contraseña contiene caracteres de salto de línea no permitidos.";
        LOG_ERROR("UserManager", result.message);
        return result;
    }

    // 5. Filtrar y validar los grupos seleccionados contra /etc/group
    QStringList validSecondaryGroups;
    for (const QString &grp : request.selectedGroups) {
        const QString cleanGrp = grp.trimmed();
        if (cleanGrp.isEmpty() || cleanGrp == "users") {
            continue;
        }
        if (groupExists(cleanGrp)) {
            if (!validSecondaryGroups.contains(cleanGrp)) {
                validSecondaryGroups.append(cleanGrp);
            }
        } else {
            LOG_WARN("UserManager", QString("Omitiendo grupo inexistente en el sistema: %1").arg(cleanGrp));
        }
    }

    // 6. Construir argumentos de useradd (estándar Slackware: grupo primario 'users', directorio /home creado)
    QStringList useraddArgs;
    useraddArgs << "-m"
                << "-d" << QString("/home/%1").arg(cleanUser)
                << "-g" << "users"
                << "-s" << cleanShell;

    if (!cleanFullName.isEmpty()) {
        useraddArgs << "-c" << cleanFullName;
    }

    if (!validSecondaryGroups.isEmpty()) {
        useraddArgs << "-G" << validSecondaryGroups.join(',');
    }

    useraddArgs << cleanUser;

    const CommandResult addRes = CommandRunner::execute("/usr/sbin/useradd", useraddArgs);
    if (!addRes.success) {
        result.success = false;
        result.message = QString("Error al crear el usuario '%1' con useradd (código %2):\n%3")
                             .arg(cleanUser)
                             .arg(addRes.exitCode)
                             .arg(addRes.stdErr);
        LOG_ERROR("UserManager", result.message);
        return result;
    }

    // 7. Establecer la contraseña de forma segura vía chpasswd (sin exponerla en argumentos CLI)
    if (!setUserPassword(cleanUser, request.password)) {
        result.success = false;
        result.message = QString("El usuario '%1' fue creado, pero ocurrió un error al establecer su contraseña mediante chpasswd.")
                             .arg(cleanUser);
        LOG_ERROR("UserManager", result.message);
        return result;
    }

    const QString groupsSummary = validSecondaryGroups.isEmpty()
                                      ? "users (solo grupo primario)"
                                      : QString("users, %1").arg(validSecondaryGroups.join(", "));

    const bool hasWheel = validSecondaryGroups.contains("wheel");
    const QString sudoTip = hasWheel
                                ? "\n• Nota: Al pertenecer al grupo 'wheel', recuerda habilitar sudo en el paso '5. Configurar sudo'."
                                : "";

    result.success = true;
    result.message = QString(
        "Usuario '%1' creado correctamente:\n"
        "• Directorio personal: /home/%1\n"
        "• Intérprete (Shell): %2\n"
        "• Grupos asignados: %3%4"
    ).arg(cleanUser, cleanShell, groupsSummary, sudoTip);

    LOG_SUCCESS("UserManager", QString("Usuario '%1' creado y configurado con éxito.").arg(cleanUser));
    return result;
}

bool UserManager::setUserPassword(const QString &username, const QString &password)
{
    LOG_CMD("UserManager", QString("Asignando contraseña vía /usr/sbin/chpasswd para usuario: %1").arg(username));

    QProcess process;
    process.start("/usr/sbin/chpasswd", QStringList());

    if (!process.waitForStarted(5000)) {
        LOG_ERROR("UserManager", "No se pudo iniciar /usr/sbin/chpasswd");
        return false;
    }

    const QByteArray payload = QString("%1:%2\n").arg(username, password).toUtf8();
    process.write(payload);
    process.closeWriteChannel();

    if (!process.waitForFinished(10000)) {
        process.kill();
        LOG_ERROR("UserManager", "Tiempo de espera agotado al ejecutar /usr/sbin/chpasswd");
        return false;
    }

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        LOG_SUCCESS("UserManager", QString("Contraseña actualizada correctamente para '%1'").arg(username));
        return true;
    }

    const QString err = QString::fromUtf8(process.readAllStandardError()).trimmed();
    LOG_ERROR("UserManager", QString("chpasswd falló (exit=%1): %2").arg(process.exitCode()).arg(err));
    return false;
}
