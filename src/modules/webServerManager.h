#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <QString>

struct WebServerDetectionInfo {
    bool apacheInstalled = false;
    QString apacheVersion;
    bool apacheBootEnabled = false;
    bool apacheRunning = false;

    bool nginxInstalled = false;
    QString nginxVersion;
    bool nginxBootEnabled = false;
    bool nginxRunning = false;

    bool phpInstalled = false;
    QString phpVersion;
    QString phpIniPath;
    bool phpFpmAvailable = false;
    bool phpImagickInstalled = false;

    QString systemTimezone;
};

struct PhpConfigOptions {
    bool updatePhpIni = true;
    QString memoryLimit = "256M";
    QString uploadMaxFilesize = "64M";
    QString postMaxSize = "64M";
    bool displayErrors = true;
};

struct WebServerApplyRequest {
    QString selectedServer; // "apache" o "nginx"
    bool enablePhp = false;
    PhpConfigOptions phpOptions;
    bool startOnBoot = true;
};

struct WebServerApplyResult {
    bool success = false;
    QString message;
    QString documentRoot;
    QString phpInfoFilePath;
};

class WebServerManager
{
public:
    static WebServerDetectionInfo detectEnvironment();
    static QString detectSystemTimezone();
    static WebServerApplyResult applyConfiguration(const WebServerApplyRequest &request);
    static bool restartWebServer(const QString &server, bool enablePhp, QString &errorDetail);

private:
    static QString detectApacheVersion();
    static QString detectNginxVersion();
    static QString detectPhpVersion();
    static QString findPhpIniPath();
    static bool checkPhpImagickInstalled();
    static bool isProcessRunning(const QString &processName);

    static bool configureApache(bool enablePhp, QString &documentRootOut, QString &errorDetail);
    static bool configureNginx(bool enablePhp, QString &documentRootOut, QString &errorDetail);
    static QString detectPhpFpmListenTarget();
    static bool configurePhpIni(const QString &phpIniPath,
                                const PhpConfigOptions &options,
                                const QString &systemTimezone,
                                bool ensureImagickExtension,
                                QString &errorDetail);
    static bool ensurePhpInfoPage(const QString &documentRoot, QString &createdPathOut);
};

#endif // WEBSERVERMANAGER_H
