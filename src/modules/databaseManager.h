#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QStringList>

struct DatabaseDetectionInfo {
    // MariaDB (incluido por defecto en Slackware 15)
    bool mariadbInstalled = false;
    QString mariadbVersion;
    QString mariadbDataDir = "/var/lib/mysql";
    bool mariadbDataInitialized = false;
    bool mariadbRunning = false;
    bool mariadbBootEnabled = false;

    // PostgreSQL (instalable posteriormente vía SlackBuilds)
    bool postgresInstalled = false;
    QString postgresVersion;
    QString postgresBinDir = "/usr/bin";
    QString postgresDataDir = "/var/lib/pgsql/data";
    bool postgresDataInitialized = false;
    bool postgresRunning = false;
    bool postgresBootEnabled = false;
};

struct DatabaseConfigRequest {
    QString engine;                 // "mariadb" o "postgresql"
    bool setRootPassword = true;
    QString currentRootPassword;    // Opcional si ya existía contraseña previa
    QString newRootPassword;
    QString confirmRootPassword;

    // Opciones nativas de mariadb-secure-installation
    bool removeAnonymousUsers = true;
    bool disallowRemoteRoot = true;
    bool removeTestDatabase = true;
    bool reloadPrivileges = true;

    // Opciones de endurecimiento para PostgreSQL
    bool pgUseScramAuth = true;
    bool pgListenLocalhostOnly = true;
    bool pgRemoveTestDb = true;
};

struct DatabaseConfigResult {
    bool success = false;
    QString message;
};

struct DatabaseSessionTestResult {
    bool success = false;
    QString summary;
    QString output;
};

class DatabaseManager
{
public:
    static DatabaseDetectionInfo detectEngines();
    static DatabaseConfigResult configureEngine(const DatabaseConfigRequest &req);
    static DatabaseSessionTestResult testRootSession(const QString &engine, const QString &password);
    static DatabaseConfigResult setStartOnBoot(const QString &engine, bool enable);

private:
    static QString findExecutable(const QStringList &candidates);
    static QString detectPostgresBinDir();
    static QString detectPostgresDataDir();
    static QString escapeSqlString(const QString &input);

    static DatabaseConfigResult configureMariaDb(const DatabaseConfigRequest &req,
                                                 const DatabaseDetectionInfo &env);
    static DatabaseConfigResult configurePostgres(const DatabaseConfigRequest &req,
                                                  const DatabaseDetectionInfo &env);

    static bool runMariaDbQuery(const QString &clientBin,
                                const QString &password,
                                const QString &sql,
                                QString &stdOut,
                                QString &stdErr,
                                bool forceTcp = false);
};

#endif // DATABASEMANAGER_H
