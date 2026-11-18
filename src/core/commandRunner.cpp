#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <unistd.h>

#include "commandRunner.h"
#include "logger.h"

bool CommandRunner::isRoot()
{
    return (geteuid() == 0);
}

QString CommandRunner::detectSlackwareVersion()
{
    QFile file("/etc/slackware-version");
    if (!file.exists()) {
        LOG_WARN("CommandRunner", "No se encontró /etc/slackware-version");
        return "Desconocida (no es Slackware)";
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString version = QString::fromUtf8(file.readAll()).trimmed();
        file.close();
        LOG_INFO("CommandRunner", QString("Versión detectada: %1").arg(version));
        return version;
    }

    LOG_ERROR("CommandRunner", "No se pudo leer /etc/slackware-version");
    return "Error al leer versión";
}

CommandResult CommandRunner::execute(const QString &program,
                                     const QStringList &arguments,
                                     int timeoutMs)
{
    CommandResult result;
    const QString fullCmd = QString("%1 %2").arg(program, arguments.join(' ')).trimmed();
    LOG_CMD("CommandRunner", QString("Ejecutando: %1").arg(fullCmd));

    QProcess process;
    process.start(program, arguments);

    if (!process.waitForStarted(5000)) {
        result.exitCode = -1;
        result.stdErr = QString("No se pudo iniciar el proceso: %1").arg(program);
        result.success = false;
        LOG_ERROR("CommandRunner", result.stdErr);
        return result;
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        result.exitCode = -2;
        result.stdErr = QString("Tiempo de espera agotado (%1 ms) para: %2").arg(timeoutMs).arg(fullCmd);
        result.success = false;
        LOG_ERROR("CommandRunner", result.stdErr);
        return result;
    }

    result.exitCode = process.exitCode();
    result.stdOut = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    result.stdErr = QString::fromUtf8(process.readAllStandardError()).trimmed();
    result.success = (process.exitStatus() == QProcess::NormalExit && result.exitCode == 0);

    if (result.success) {
        LOG_SUCCESS("CommandRunner", QString("Comando OK (exit=0): %1").arg(fullCmd));
        if (!result.stdOut.isEmpty()) {
            LOG_INFO("CommandRunner", QString("stdout: %1").arg(result.stdOut));
        }
    } else {
        LOG_ERROR("CommandRunner", QString("Comando falló (exit=%1): %2 | stderr: %3")
                                       .arg(result.exitCode)
                                       .arg(fullCmd, result.stdErr));
    }

    return result;
}

CommandResult CommandRunner::executeShell(const QString &shellCommand, int timeoutMs)
{
    return execute("/bin/sh", QStringList() << "-c" << shellCommand, timeoutMs);
}

bool CommandRunner::writeFileWithBackup(const QString &filePath,
                                        const QString &content,
                                        bool createBackup)
{
    LOG_INFO("CommandRunner", QString("Preparando escritura en: %1").arg(filePath));

    QFile targetFile(filePath);
    if (createBackup && targetFile.exists()) {
        const QString backupPath = QString("%1.bak.%2")
                                       .arg(filePath, QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
        if (QFile::copy(filePath, backupPath)) {
            LOG_SUCCESS("CommandRunner", QString("Respaldo creado en: %1").arg(backupPath));
        } else {
            LOG_WARN("CommandRunner", QString("No se pudo crear respaldo en: %1").arg(backupPath));
        }
    }

    if (!targetFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        LOG_ERROR("CommandRunner", QString("Error al abrir archivo para escritura: %1 (%2)")
                                       .arg(filePath, targetFile.errorString()));
        return false;
    }

    QTextStream out(&targetFile);
    out.setCodec("UTF-8");
    out << content;
    targetFile.close();

    LOG_SUCCESS("CommandRunner", QString("Archivo guardado correctamente: %1").arg(filePath));
    return true;
}

bool CommandRunner::setExecutablePermission(const QString &filePath, bool executable)
{
    QFile file(filePath);
    if (!file.exists()) {
        LOG_ERROR("CommandRunner", QString("Archivo inexistente para cambiar permisos: %1").arg(filePath));
        return false;
    }

    QFileDevice::Permissions perms = file.permissions();
    if (executable) {
        perms |= (QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther);
    } else {
        perms &= ~(QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther);
    }

    if (file.setPermissions(perms)) {
        LOG_SUCCESS("CommandRunner", QString("Permiso ejecutable (%1) aplicado en: %2")
                                         .arg(executable ? "+x" : "-x", filePath));
        return true;
    }

    LOG_ERROR("CommandRunner", QString("No se pudieron cambiar permisos en: %1").arg(filePath));
    return false;
}
