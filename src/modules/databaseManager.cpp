#include "databaseManager.h"
#include "commandRunner.h"
#include "logger.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QProcess>
#include <QProcessEnvironment>
#include <QThread>
#include <QRegularExpression>

QString DatabaseManager::findExecutable(const QStringList &candidates)
{
    for (const QString &path : candidates) {
        QFileInfo fi(path);
        if (fi.exists() && fi.isExecutable()) {
            return path;
        }
    }

    for (const QString &name : candidates) {
        if (!name.contains('/')) {
            const CommandResult res = CommandRunner::execute("which", QStringList() << name);
            if (res.success && !res.stdOut.trimmed().isEmpty()) {
                return res.stdOut.trimmed();
            }
        }
    }

    return QString();
}

QString DatabaseManager::detectPostgresBinDir()
{
    const QString direct = findExecutable({
        "/usr/bin/initdb",
        "/usr/local/bin/initdb"
    });
    if (!direct.isEmpty()) {
        return QFileInfo(direct).absolutePath();
    }

    // En SlackBuilds PostgreSQL suele instalar binarios en /usr/lib64/postgresql/<ver>/bin
    const QStringList baseDirs = { "/usr/lib64/postgresql", "/usr/lib/postgresql" };
    for (const QString &base : baseDirs) {
        QDir dir(base);
        if (!dir.exists()) {
            continue;
        }
        const QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (const QString &ver : subDirs) {
            const QString candidate = QString("%1/%2/bin/initdb").arg(base, ver);
            if (QFileInfo::exists(candidate)) {
                return QString("%1/%2/bin").arg(base, ver);
            }
        }
    }

    return "/usr/bin";
}

QString DatabaseManager::detectPostgresDataDir()
{
    QFile rcFile("/etc/rc.d/rc.postgresql");
    if (rcFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&rcFile);
        const QRegularExpression re(R"(^\s*(?:PGDATA|DATADIR)\s*=\s*["']?([^"'\s#]+)["']?)");
        while (!in.atEnd()) {
            const QString line = in.readLine();
            const QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                return match.captured(1);
            }
        }
    }

    QDir pgsqlDir("/var/lib/pgsql");
    if (pgsqlDir.exists()) {
        const QStringList entries = pgsqlDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (const QString &entry : entries) {
            const QString versionedData = QString("/var/lib/pgsql/%1/data").arg(entry);
            if (QDir(versionedData).exists()) {
                return versionedData;
            }
        }
        if (QDir("/var/lib/pgsql/data").exists()) {
            return "/var/lib/pgsql/data";
        }
    }

    return "/var/lib/pgsql/data";
}

QString DatabaseManager::escapeSqlString(const QString &input)
{
    QString escaped = input;
    escaped.replace("\\", "\\\\");
    escaped.replace("'", "''");
    return escaped;
}

DatabaseDetectionInfo DatabaseManager::detectEngines()
{
    DatabaseDetectionInfo info;

    // 1. Detección de MariaDB
    const QString mysqldBin = findExecutable({
        "/usr/libexec/mysqld",
        "/usr/sbin/mysqld",
        "/usr/bin/mariadbd",
        "mysqld",
        "mariadbd"
    });

    if (!mysqldBin.isEmpty()) {
        info.mariadbInstalled = true;
        const CommandResult verRes = CommandRunner::execute(mysqldBin, QStringList() << "--version");
        if (verRes.success && !verRes.stdOut.trimmed().isEmpty()) {
            info.mariadbVersion = verRes.stdOut.trimmed().split('\n').first();
        } else {
            info.mariadbVersion = "MariaDB Server";
        }
    }

    info.mariadbDataDir = "/var/lib/mysql";
    info.mariadbDataInitialized = QDir("/var/lib/mysql/mysql").exists();

    const QFileInfo rcMysql("/etc/rc.d/rc.mysqld");
    info.mariadbBootEnabled = (rcMysql.exists() && rcMysql.isExecutable());

    const CommandResult pgrepMysql = CommandRunner::execute("pgrep", QStringList() << "-x" << "mysqld|mariadbd");
    info.mariadbRunning = (pgrepMysql.success && !pgrepMysql.stdOut.trimmed().isEmpty());

    // 2. Detección de PostgreSQL
    info.postgresBinDir = detectPostgresBinDir();
    const QString postgresBin = findExecutable({
        info.postgresBinDir + "/postgres",
        "/usr/bin/postgres",
        "postgres"
    });

    if (!postgresBin.isEmpty()) {
        info.postgresInstalled = true;
        const CommandResult verRes = CommandRunner::execute(postgresBin, QStringList() << "--version");
        if (verRes.success && !verRes.stdOut.trimmed().isEmpty()) {
            info.postgresVersion = verRes.stdOut.trimmed().split('\n').first();
        } else {
            info.postgresVersion = "PostgreSQL Server";
        }
    }

    info.postgresDataDir = detectPostgresDataDir();
    info.postgresDataInitialized = QFileInfo::exists(info.postgresDataDir + "/PG_VERSION");

    const QFileInfo rcPg("/etc/rc.d/rc.postgresql");
    info.postgresBootEnabled = (rcPg.exists() && rcPg.isExecutable());

    const CommandResult pgrepPg = CommandRunner::execute("pgrep", QStringList() << "-x" << "postgres");
    info.postgresRunning = (pgrepPg.success && !pgrepPg.stdOut.trimmed().isEmpty());

    return info;
}

bool DatabaseManager::runMariaDbQuery(const QString &clientBin,
                                      const QString &password,
                                      const QString &sql,
                                      QString &stdOut,
                                      QString &stdErr,
                                      bool forceTcp)
{
    QProcess proc;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!password.isEmpty()) {
        env.insert("MYSQL_PWD", password);
    } else {
        env.remove("MYSQL_PWD");
    }
    proc.setProcessEnvironment(env);

    QStringList args;
    args << "-u" << "root";
    if (forceTcp) {
        args << "-h" << "127.0.0.1" << "--protocol=tcp";
    }
    args << "-e" << sql;

    proc.start(clientBin, args);
    if (!proc.waitForFinished(10000)) {
        stdErr = "Tiempo de espera agotado al ejecutar consulta en MariaDB.";
        return false;
    }

    stdOut = QString::fromUtf8(proc.readAllStandardOutput());
    stdErr = QString::fromUtf8(proc.readAllStandardError());
    return (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0);
}

DatabaseConfigResult DatabaseManager::configureEngine(const DatabaseConfigRequest &req)
{
    if (!CommandRunner::isRoot()) {
        return { false, "Permisos insuficientes: ejecuta slackwiz como root para configurar el motor de base de datos." };
    }

    if (req.engine != "mariadb" && req.engine != "postgresql") {
        return { false, "Selecciona un motor de base de datos válido (MariaDB o PostgreSQL)." };
    }

    if (req.setRootPassword) {
        if (req.newRootPassword.isEmpty()) {
            return { false, "La nueva contraseña de administración no puede estar vacía." };
        }
        if (req.newRootPassword != req.confirmRootPassword) {
            return { false, "Las contraseñas ingresadas no coinciden." };
        }
    }

    const DatabaseDetectionInfo env = detectEngines();

    if (req.engine == "mariadb") {
        if (!env.mariadbInstalled) {
            return { false, "MariaDB no está instalado en este sistema." };
        }
        return configureMariaDb(req, env);
    }

    if (!env.postgresInstalled) {
        return { false, "PostgreSQL no está instalado en este sistema." };
    }
    return configurePostgres(req, env);
}

DatabaseConfigResult DatabaseManager::configureMariaDb(const DatabaseConfigRequest &req,
                                                         const DatabaseDetectionInfo &env)
{
    LOG_INFO("DatabaseManager", "Iniciando configuración de MariaDB...");
    QStringList reportSteps;

    // 1. Inicializar tablas del sistema si /var/lib/mysql/mysql no existe
    if (!env.mariadbDataInitialized) {
        const QString installDbBin = findExecutable({
            "/usr/bin/mariadb-install-db",
            "/usr/bin/mysql_install_db",
            "mariadb-install-db",
            "mysql_install_db"
        });

        if (installDbBin.isEmpty()) {
            return { false, "No se encontró 'mysql_install_db' ni 'mariadb-install-db' en el sistema." };
        }

        CommandRunner::execute("mkdir", QStringList() << "-p" << env.mariadbDataDir);
        CommandRunner::execute("chown", QStringList() << "-R" << "mysql:mysql" << env.mariadbDataDir);

        const CommandResult initRes = CommandRunner::execute(
            installDbBin,
            QStringList() << "--user=mysql" << QString("--datadir=%1").arg(env.mariadbDataDir)
        );

        if (!initRes.success) {
            return {
                false,
                QString("Error al inicializar las tablas del sistema de MariaDB:\n%1\n%2")
                    .arg(initRes.stdErr, initRes.stdOut)
            };
        }

        CommandRunner::execute("chown", QStringList() << "-R" << "mysql:mysql" << env.mariadbDataDir);
        reportSteps << "Tablas del sistema inicializadas en /var/lib/mysql.";
    } else {
        reportSteps << "Directorio de datos /var/lib/mysql ya inicializado.";
    }

    // 2. Preparar directorio de runtime /var/run/mysql y arrancar desacoplado de stdout/stderr
    const QString rcScript = "/etc/rc.d/rc.mysqld";
    if (!QFileInfo::exists(rcScript)) {
        return { false, "No se encontró el script /etc/rc.d/rc.mysqld en el sistema." };
    }

    CommandRunner::execute("mkdir", QStringList() << "-p" << "/var/run/mysql");
    CommandRunner::execute("chown", QStringList() << "-R" << "mysql:mysql" << "/var/run/mysql");

    const bool wasExecutable = QFileInfo(rcScript).isExecutable();
    if (!wasExecutable) {
        CommandRunner::execute("chmod", QStringList() << "+x" << rcScript);
    }

    if (!env.mariadbRunning) {
        // Limpiar socket o pid huérfanos si el demonio no está activo
        QFile::remove("/var/run/mysql/mysql.sock");
        QFile::remove("/var/run/mysql/mysql.pid");

        // setsid + redirección a /dev/null evita que mysqld_safe reciba SIGPIPE al cerrarse QProcess
        const CommandResult startRes = CommandRunner::execute(
            "setsid",
            QStringList() << "/bin/sh" << "-c" << "/etc/rc.d/rc.mysqld start >/dev/null 2>&1 </dev/null"
        );

        if (!startRes.success) {
            if (!wasExecutable) {
                CommandRunner::execute("chmod", QStringList() << "-x" << rcScript);
            }
            return {
                false,
                QString("No se pudo iniciar MariaDB (%1 start):\n%2")
                    .arg(rcScript, startRes.stdErr)
            };
        }
        reportSteps << "Servicio MariaDB iniciado correctamente.";
    }

    // Esperar a que el socket/servidor responda (hasta 10 segundos)
    const QString adminBin = findExecutable({ "/usr/bin/mariadb-admin", "/usr/bin/mysqladmin", "mysqladmin" });
    bool ready = false;
    for (int i = 0; i < 20; ++i) {
        if (!adminBin.isEmpty()) {
            const CommandResult pingRes = CommandRunner::execute(adminBin, QStringList() << "ping");
            if (pingRes.success) {
                ready = true;
                break;
            }
        } else if (QFileInfo::exists("/var/run/mysql/mysql.sock")) {
            ready = true;
            break;
        }
        QThread::msleep(500);
    }

    if (!ready) {
        if (!wasExecutable) {
            CommandRunner::execute("chmod", QStringList() << "-x" << rcScript);
        }

        QString errLogTail;
        QDir dataDir(env.mariadbDataDir);
        const QStringList errFiles = dataDir.entryList(QStringList() << "*.err", QDir::Files, QDir::Time);
        if (!errFiles.isEmpty()) {
            QFile errFile(dataDir.filePath(errFiles.first()));
            if (errFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QStringList allLines = QString::fromUtf8(errFile.readAll()).split('\n', Qt::SkipEmptyParts);
                const int startIdx = qMax(0, allLines.size() - 12);
                errLogTail = "\n\nÚltimas líneas del log (" + errFiles.first() + "):\n" +
                             allLines.mid(startIdx).join('\n');
            }
        }

        return {
            false,
            QString("MariaDB tardó demasiado en responder tras iniciar el servicio.%1").arg(errLogTail)
        };
    }

    const QString clientBin = findExecutable({ "/usr/bin/mariadb", "/usr/bin/mysql", "mariadb", "mysql" });
    if (clientBin.isEmpty()) {
        return { false, "No se encontró el cliente CLI 'mariadb' o 'mysql'." };
    }

    // 3. Aplicar opciones de mariadb-secure-installation
    QString out, err;
    QString authPass = req.currentRootPassword;

    if (!runMariaDbQuery(clientBin, authPass, "SELECT 1;", out, err, false)) {
        return {
            false,
            QString("No se pudo autenticar como root en MariaDB. Si ya tenía una contraseña previa, ingrésala en el campo correspondiente.\nDetalle: %1")
                .arg(err.trimmed())
        };
    }

    if (req.removeAnonymousUsers) {
        QString hostsOut, hostsErr;
        if (runMariaDbQuery(clientBin, authPass, "SELECT Host FROM mysql.user WHERE User='';", hostsOut, hostsErr, false)) {
            const QStringList lines = hostsOut.split('\n', Qt::SkipEmptyParts);
            for (int i = 1; i < lines.size(); ++i) {
                const QString host = escapeSqlString(lines[i].trimmed());
                runMariaDbQuery(clientBin, authPass,
                                QString("DROP USER IF EXISTS ''@'%1';").arg(host), out, err, false);
            }
        }
        reportSteps << "Usuarios anónimos eliminados.";
    }

    if (req.disallowRemoteRoot) {
        QString hostsOut, hostsErr;
        if (runMariaDbQuery(clientBin, authPass,
                            "SELECT Host FROM mysql.user WHERE User='root' AND Host NOT IN ('localhost', '127.0.0.1', '::1');",
                            hostsOut, hostsErr, false)) {
            const QStringList lines = hostsOut.split('\n', Qt::SkipEmptyParts);
            for (int i = 1; i < lines.size(); ++i) {
                const QString host = escapeSqlString(lines[i].trimmed());
                runMariaDbQuery(clientBin, authPass,
                                QString("DROP USER IF EXISTS 'root'@'%1';").arg(host), out, err, false);
            }
        }
        reportSteps << "Inicio de sesión remoto para 'root' deshabilitado.";
    }

    if (req.removeTestDatabase) {
        runMariaDbQuery(clientBin, authPass, "DROP DATABASE IF EXISTS test;", out, err, false);
        runMariaDbQuery(clientBin, authPass, "DELETE FROM mysql.db WHERE Db='test' OR Db='test\\_%';", out, err, false);
        reportSteps << "Base de datos 'test' y permisos asociados eliminados.";
    }

    if (req.setRootPassword) {
        const QString escapedNew = escapeSqlString(req.newRootPassword);
        const QString alterSql = QString(
            "ALTER USER 'root'@'localhost' IDENTIFIED BY '%1'; "
            "FLUSH PRIVILEGES;"
        ).arg(escapedNew);

        if (!runMariaDbQuery(clientBin, authPass, alterSql, out, err, false)) {
            return {
                false,
                QString("Error al establecer la contraseña de root en MariaDB:\n%1").arg(err.trimmed())
            };
        }
        authPass = req.newRootPassword;
        reportSteps << "Contraseña de 'root'@'localhost' actualizada correctamente.";
    }

    if (req.reloadPrivileges) {
        runMariaDbQuery(clientBin, authPass, "FLUSH PRIVILEGES;", out, err, false);
        reportSteps << "Tablas de privilegios recargadas (FLUSH PRIVILEGES).";
    }

    if (!wasExecutable) {
        CommandRunner::execute("chmod", QStringList() << "-x" << rcScript);
    }

    LOG_SUCCESS("DatabaseManager", "MariaDB configurado correctamente.");
    return {
        true,
        QString("Configuración de MariaDB completada:\n• %1").arg(reportSteps.join("\n• "))
    };
}

DatabaseConfigResult DatabaseManager::configurePostgres(const DatabaseConfigRequest &req,
                                                          const DatabaseDetectionInfo &env)
{
    LOG_INFO("DatabaseManager", "Iniciando configuración de PostgreSQL...");
    QStringList reportSteps;

    const QString initdbBin = env.postgresBinDir + "/initdb";
    const QString psqlBin = env.postgresBinDir + "/psql";
    const QString pgIsReadyBin = env.postgresBinDir + "/pg_isready";

    if (!env.postgresDataInitialized) {
        if (!QFileInfo::exists(initdbBin)) {
            return { false, QString("No se encontró 'initdb' en %1.").arg(initdbBin) };
        }

        CommandRunner::execute("mkdir", QStringList() << "-p" << env.postgresDataDir);
        CommandRunner::execute("chown", QStringList() << "-R" << "postgres:postgres" << env.postgresDataDir);

        const QString initCmd = QString("\"%1\" -D \"%2\" --encoding=UTF8 --locale=C.UTF-8")
                                    .arg(initdbBin, env.postgresDataDir);
        const CommandResult initRes = CommandRunner::execute(
            "su",
            QStringList() << "-" << "postgres" << "-c" << initCmd
        );

        if (!initRes.success) {
            return {
                false,
                QString("Error al inicializar el clúster de PostgreSQL con initdb:\n%1\n%2")
                    .arg(initRes.stdErr, initRes.stdOut)
            };
        }
        reportSteps << QString("Clúster de PostgreSQL inicializado en %1.").arg(env.postgresDataDir);
    } else {
        reportSteps << QString("Clúster de datos ya inicializado en %1.").arg(env.postgresDataDir);
    }

    const QString rcScript = "/etc/rc.d/rc.postgresql";
    if (!QFileInfo::exists(rcScript)) {
        return { false, "No se encontró el script /etc/rc.d/rc.postgresql en el sistema." };
    }

    const bool wasExecutable = QFileInfo(rcScript).isExecutable();
    if (!wasExecutable) {
        CommandRunner::execute("chmod", QStringList() << "+x" << rcScript);
    }

    if (!env.postgresRunning) {
        const CommandResult startRes = CommandRunner::execute(
            "setsid",
            QStringList() << "/bin/sh" << "-c" << "/etc/rc.d/rc.postgresql start >/dev/null 2>&1 </dev/null"
        );
        if (!startRes.success) {
            if (!wasExecutable) {
                CommandRunner::execute("chmod", QStringList() << "-x" << rcScript);
            }
            return {
                false,
                QString("No se pudo iniciar PostgreSQL (%1 start):\n%2")
                    .arg(rcScript, startRes.stdErr)
            };
        }
        reportSteps << "Servicio PostgreSQL iniciado correctamente.";
    }

    bool ready = false;
    for (int i = 0; i < 20; ++i) {
        if (QFileInfo::exists(pgIsReadyBin)) {
            const CommandResult readyRes = CommandRunner::execute(pgIsReadyBin, QStringList());
            if (readyRes.success) {
                ready = true;
                break;
            }
        } else {
            ready = true;
            break;
        }
        QThread::msleep(500);
    }

    if (!ready) {
        if (!wasExecutable) {
            CommandRunner::execute("chmod", QStringList() << "-x" << rcScript);
        }
        return { false, "PostgreSQL tardó demasiado en responder tras iniciar el servicio." };
    }

    if (req.setRootPassword) {
        const QString escapedPass = escapeSqlString(req.newRootPassword);
        const QString sql = QString("ALTER USER postgres WITH PASSWORD '%1';").arg(escapedPass);
        const QString cmd = QString("\"%1\" -d postgres -c \"%2\"").arg(psqlBin, sql);

        const CommandResult passRes = CommandRunner::execute(
            "su",
            QStringList() << "-" << "postgres" << "-c" << cmd
        );

        if (!passRes.success) {
            return {
                false,
                QString("Error al establecer la contraseña del superusuario 'postgres':\n%1")
                    .arg(passRes.stdErr)
            };
        }
        reportSteps << "Contraseña del superusuario 'postgres' actualizada.";
    }

    if (req.pgRemoveTestDb) {
        const QString dropCmd = QString("\"%1\" -d postgres -c \"DROP DATABASE IF EXISTS test;\"").arg(psqlBin);
        CommandRunner::execute("su", QStringList() << "-" << "postgres" << "-c" << dropCmd);
        reportSteps << "Base de datos de prueba 'test' verificada/eliminada.";
    }

    const QString pgHbaPath = env.postgresDataDir + "/pg_hba.conf";
    if (req.pgUseScramAuth && QFileInfo::exists(pgHbaPath)) {
        QFile hbaFile(pgHbaPath);
        if (hbaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString content = QString::fromUtf8(hbaFile.readAll());
            hbaFile.close();

            content.replace(QRegularExpression(R"((host\s+all\s+all\s+127\.0\.0\.1/32\s+)\S+)"), "\\1scram-sha-256");
            content.replace(QRegularExpression(R"((host\s+all\s+all\s+::1/128\s+)\S+)"), "\\1scram-sha-256");

            if (hbaFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                hbaFile.write(content.toUtf8());
                hbaFile.close();
                CommandRunner::execute("chown", QStringList() << "postgres:postgres" << pgHbaPath);
                reportSteps << "Autenticación scram-sha-256 aplicada en pg_hba.conf.";
            }
        }
    }

    const QString pgConfPath = env.postgresDataDir + "/postgresql.conf";
    if (req.pgListenLocalhostOnly && QFileInfo::exists(pgConfPath)) {
        QFile confFile(pgConfPath);
        if (confFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString content = QString::fromUtf8(confFile.readAll());
            confFile.close();

            const QRegularExpression listenRe(R"(^\s*#?\s*listen_addresses\s*=.*$)", QRegularExpression::MultilineOption);
            if (listenRe.match(content).hasMatch()) {
                content.replace(listenRe, "listen_addresses = 'localhost'");
            } else {
                content.append("\nlisten_addresses = 'localhost'\n");
            }

            if (confFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                confFile.write(content.toUtf8());
                confFile.close();
                CommandRunner::execute("chown", QStringList() << "postgres:postgres" << pgConfPath);
                reportSteps << "Escucha de red restringida a 'localhost' en postgresql.conf.";
            }
        }
    }

    const QString reloadCmd = QString("\"%1\" -d postgres -c \"SELECT pg_reload_conf();\"").arg(psqlBin);
    CommandRunner::execute("su", QStringList() << "-" << "postgres" << "-c" << reloadCmd);

    if (!wasExecutable) {
        CommandRunner::execute("chmod", QStringList() << "-x" << rcScript);
    }

    LOG_SUCCESS("DatabaseManager", "PostgreSQL configurado correctamente.");
    return {
        true,
        QString("Configuración de PostgreSQL completada:\n• %1").arg(reportSteps.join("\n• "))
    };
}

DatabaseSessionTestResult DatabaseManager::testRootSession(const QString &engine, const QString &password)
{
    if (engine == "mariadb") {
        const QString clientBin = findExecutable({ "/usr/bin/mariadb", "/usr/bin/mysql", "mariadb", "mysql" });
        if (clientBin.isEmpty()) {
            return { false, "Cliente no encontrado", "No se encontró el ejecutable mariadb/mysql en el sistema." };
        }

        QString stdOut, stdErr;
        const QString sql =
            "SELECT VERSION() AS Version_MariaDB, CURRENT_USER() AS Usuario_Sesion, @@hostname AS Servidor; "
            "SHOW DATABASES;";

        // forceTcp=false usa el socket local /var/run/mysql/mysql.sock (compatible con skip-networking de Slackware)
        const bool ok = runMariaDbQuery(clientBin, password, sql, stdOut, stdErr, false);
        if (ok) {
            LOG_SUCCESS("DatabaseManager", "Sesión de prueba de MariaDB exitosa.");
            return {
                true,
                "Sesión de MariaDB iniciada correctamente como 'root'.",
                stdOut.trimmed()
            };
        }

        return {
            false,
            "Error al iniciar sesión en MariaDB (verifica la contraseña de root).",
            stdErr.trimmed().isEmpty() ? "Acceso denegado o servicio detenido." : stdErr.trimmed()
        };
    }

    if (engine == "postgresql") {
        const QString binDir = detectPostgresBinDir();
        const QString psqlBin = findExecutable({ binDir + "/psql", "/usr/bin/psql", "psql" });
        if (psqlBin.isEmpty()) {
            return { false, "Cliente no encontrado", "No se encontró el ejecutable psql en el sistema." };
        }

        QProcess proc;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PGPASSWORD", password);
        proc.setProcessEnvironment(env);

        proc.start(psqlBin, QStringList()
                                << "-h" << "127.0.0.1"
                                << "-U" << "postgres"
                                << "-d" << "postgres"
                                << "-c" << "SELECT version() AS version_postgresql, current_user AS usuario_sesion;"
                                << "-c" << "\\l");

        if (!proc.waitForFinished(10000)) {
            return { false, "Tiempo agotado", "PostgreSQL no respondió dentro del tiempo esperado." };
        }

        const QString stdOut = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        const QString stdErr = QString::fromUtf8(proc.readAllStandardError()).trimmed();

        if (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0) {
            LOG_SUCCESS("DatabaseManager", "Sesión de prueba de PostgreSQL exitosa.");
            return {
                true,
                "Sesión de PostgreSQL iniciada correctamente como 'postgres'.",
                stdOut
            };
        }

        return {
            false,
            "Error al iniciar sesión en PostgreSQL (verifica la contraseña de postgres).",
            stdErr.isEmpty() ? "Autenticación fallida o servicio detenido." : stdErr
        };
    }

    return { false, "Motor no soportado", "Selecciona MariaDB o PostgreSQL." };
}

DatabaseConfigResult DatabaseManager::setStartOnBoot(const QString &engine, bool enable)
{
    if (!CommandRunner::isRoot()) {
        return { false, "Permisos insuficientes: ejecuta slackwiz como root." };
    }

    QString rcScript;
    QString engineLabel;
    if (engine == "mariadb") {
        rcScript = "/etc/rc.d/rc.mysqld";
        engineLabel = "MariaDB";
    } else if (engine == "postgresql") {
        rcScript = "/etc/rc.d/rc.postgresql";
        engineLabel = "PostgreSQL";
    } else {
        return { false, "Motor de base de datos no válido." };
    }

    if (!QFileInfo::exists(rcScript)) {
        return { false, QString("No se encontró el script de inicio %1.").arg(rcScript) };
    }

    const QString mode = enable ? "+x" : "-x";
    const CommandResult chmodRes = CommandRunner::execute("chmod", QStringList() << mode << rcScript);
    if (!chmodRes.success) {
        return {
            false,
            QString("Error al cambiar permisos de arranque en %1:\n%2").arg(rcScript, chmodRes.stdErr)
        };
    }

    if (enable) {
        CommandRunner::execute(
            "setsid",
            QStringList() << "/bin/sh" << "-c" << QString("%1 start >/dev/null 2>&1 </dev/null").arg(rcScript)
        );
        LOG_SUCCESS("DatabaseManager", QString("Arranque automático habilitado para %1.").arg(engineLabel));
        return {
            true,
            QString("Arranque automático HABILITADO para %1 (%2 con permiso de ejecución). El servicio se iniciará automáticamente con Slackware.")
                .arg(engineLabel, rcScript)
        };
    }

    LOG_INFO("DatabaseManager", QString("Arranque automático deshabilitado para %1.").arg(engineLabel));
    return {
        true,
        QString("Arranque automático DESHABILITADO para %1 (%2 sin permiso de ejecución). Podrás iniciarlo manualmente cuando lo desees.")
            .arg(engineLabel, rcScript)
    };
}
