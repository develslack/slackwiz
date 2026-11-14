#include "webServerManager.h"
#include "commandRunner.h"
#include "logger.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>
#include <QTimeZone>

WebServerDetectionInfo WebServerManager::detectEnvironment()
{
    LOG_INFO("WebServerManager", "Iniciando verificación previa de servidores web, PHP, php-imagick y zona horaria...");
    WebServerDetectionInfo info;
    info.systemTimezone = detectSystemTimezone();

    // 1. Verificar Apache
    if (QFile::exists("/usr/sbin/httpd") || QFile::exists("/etc/rc.d/rc.httpd")) {
        info.apacheInstalled = true;
        info.apacheVersion = detectApacheVersion();
        info.apacheBootEnabled = QFileInfo("/etc/rc.d/rc.httpd").isExecutable();
        info.apacheRunning = isProcessRunning("httpd");
        LOG_SUCCESS("WebServerManager", QString("Apache detectado: %1 (boot=%2, activo=%3)")
                                            .arg(info.apacheVersion)
                                            .arg(info.apacheBootEnabled ? "SÍ" : "NO")
                                            .arg(info.apacheRunning ? "SÍ" : "NO"));
    } else {
        LOG_INFO("WebServerManager", "Apache no se encuentra instalado.");
    }

    // 2. Verificar Nginx
    if (QFile::exists("/usr/sbin/nginx") || QFile::exists("/etc/rc.d/rc.nginx")) {
        info.nginxInstalled = true;
        info.nginxVersion = detectNginxVersion();
        info.nginxBootEnabled = QFileInfo("/etc/rc.d/rc.nginx").isExecutable();
        info.nginxRunning = isProcessRunning("nginx");
        LOG_SUCCESS("WebServerManager", QString("Nginx detectado: %1 (boot=%2, activo=%3)")
                                            .arg(info.nginxVersion)
                                            .arg(info.nginxBootEnabled ? "SÍ" : "NO")
                                            .arg(info.nginxRunning ? "SÍ" : "NO"));
    } else {
        LOG_INFO("WebServerManager", "Nginx no se encuentra instalado.");
    }

    // 3. Verificar PHP
    if (QFile::exists("/usr/bin/php")) {
        info.phpInstalled = true;
        info.phpVersion = detectPhpVersion();
        info.phpIniPath = findPhpIniPath();
        info.phpFpmAvailable = (QFile::exists("/usr/sbin/php-fpm") || QFile::exists("/etc/rc.d/rc.php-fpm"));
        info.phpImagickInstalled = checkPhpImagickInstalled();

        LOG_SUCCESS("WebServerManager", QString("PHP detectado: %1 | php.ini: %2 | php-fpm: %3 | php-imagick: %4 | TZ: %5")
                                            .arg(info.phpVersion,
                                                 info.phpIniPath,
                                                 info.phpFpmAvailable ? "SÍ" : "NO",
                                                 info.phpImagickInstalled ? "SÍ" : "NO",
                                                 info.systemTimezone));
    } else {
        LOG_INFO("WebServerManager", "PHP no se encuentra instalado en /usr/bin/php.");
    }

    return info;
}

QString WebServerManager::detectSystemTimezone()
{
    // En Slackware, timeconfig crea enlaces simbólicos en /etc/localtime-copied-from y/o /etc/localtime
    const QStringList candidates = {
        "/etc/localtime",
        "/etc/localtime-copied-from"
    };

    for (const QString &path : candidates) {
        QFileInfo fi(path);
        if (!fi.exists()) {
            continue;
        }

        QString target = fi.isSymLink() ? fi.symLinkTarget() : fi.canonicalFilePath();
        if (target.isEmpty()) {
            const CommandResult rl = CommandRunner::execute("readlink", QStringList() << "-f" << path);
            if (rl.success) {
                target = rl.stdOut.trimmed();
            }
        }

        const int idx = target.indexOf("zoneinfo/");
        if (idx != -1) {
            const QString tz = target.mid(idx + 9).trimmed();
            if (!tz.isEmpty() && tz != "posixrules" && tz != "localtime") {
                LOG_INFO("WebServerManager", QString("Zona horaria detectada desde %1: %2").arg(path, tz));
                return tz;
            }
        }
    }

    // Respaldo: archivo /etc/timezone si existiera
    QFile tzFile("/etc/timezone");
    if (tzFile.exists() && tzFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString tz = QTextStream(&tzFile).readLine().trimmed();
        tzFile.close();
        if (!tz.isEmpty()) {
            return tz;
        }
    }

    // Respaldo final mediante QTimeZone del sistema
    const QByteArray sysId = QTimeZone::systemTimeZoneId();
    if (!sysId.isEmpty() && sysId != "UTC") {
        return QString::fromUtf8(sysId);
    }

    return "UTC";
}

QString WebServerManager::detectApacheVersion()
{
    const CommandResult res = CommandRunner::execute("/usr/sbin/httpd", QStringList() << "-v");
    const QString combined = res.stdOut + "\n" + res.stdErr;
    QRegularExpression regex(R"(Apache/([0-9]+\.[0-9]+\.[0-9]+))");
    QRegularExpressionMatch match = regex.match(combined);
    if (match.hasMatch()) {
        return QString("Apache %1").arg(match.captured(1));
    }
    return "Apache HTTP Server (versión detectada en sistema)";
}

QString WebServerManager::detectNginxVersion()
{
    const CommandResult res = CommandRunner::execute("/usr/sbin/nginx", QStringList() << "-v");
    const QString combined = res.stdOut + "\n" + res.stdErr;
    QRegularExpression regex(R"(nginx/([0-9]+\.[0-9]+\.[0-9]+))");
    QRegularExpressionMatch match = regex.match(combined);
    if (match.hasMatch()) {
        return QString("Nginx %1").arg(match.captured(1));
    }
    return "Nginx (versión detectada en sistema)";
}

QString WebServerManager::detectPhpVersion()
{
    const CommandResult res = CommandRunner::execute("/usr/bin/php", QStringList() << "-v");
    QRegularExpression regex(R"(PHP\s+([0-9]+\.[0-9]+\.[0-9]+))");
    QRegularExpressionMatch match = regex.match(res.stdOut);
    if (match.hasMatch()) {
        return QString("PHP %1").arg(match.captured(1));
    }
    return "PHP (instalado en /usr/bin/php)";
}

QString WebServerManager::findPhpIniPath()
{
    const QStringList candidates = {
        "/etc/php.ini",
        "/etc/php/php.ini"
    };

    for (const QString &path : candidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    return "/etc/php.ini";
}

bool WebServerManager::checkPhpImagickInstalled()
{
    QDir pkgDir("/var/log/packages");
    if (pkgDir.exists()) {
        const QStringList matches = pkgDir.entryList(QStringList() << "php-imagick*", QDir::Files);
        if (!matches.isEmpty()) {
            LOG_INFO("WebServerManager", QString("Paquete encontrado en /var/log/packages: %1").arg(matches.first()));
            return true;
        }
    }

    const QStringList extPaths = {
        "/usr/lib64/php/extensions/imagick.so",
        "/usr/lib/php/extensions/imagick.so"
    };
    for (const QString &ext : extPaths) {
        if (QFile::exists(ext)) {
            LOG_INFO("WebServerManager", QString("Extensión encontrada en: %1").arg(ext));
            return true;
        }
    }

    const CommandResult modRes = CommandRunner::execute("/usr/bin/php", QStringList() << "-m");
    if (modRes.success) {
        const QStringList modules = modRes.stdOut.split('\n', Qt::SkipEmptyParts);
        for (const QString &mod : modules) {
            if (mod.trimmed().compare("imagick", Qt::CaseInsensitive) == 0) {
                LOG_INFO("WebServerManager", "Módulo 'imagick' reportado por php -m");
                return true;
            }
        }
    }

    LOG_WARN("WebServerManager", "php-imagick no está instalado en el sistema.");
    return false;
}

bool WebServerManager::isProcessRunning(const QString &processName)
{
    const CommandResult res = CommandRunner::execute("pgrep", QStringList() << "-x" << processName);
    return res.success;
}

WebServerApplyResult WebServerManager::applyConfiguration(const WebServerApplyRequest &request)
{
    WebServerApplyResult result;
    const QString server = request.selectedServer.trimmed().toLower();

    LOG_INFO("WebServerManager", QString("Guardando configuración web -> servidor='%1', php=%2, boot=%3")
                                     .arg(server)
                                     .arg(request.enablePhp ? "SÍ" : "NO")
                                     .arg(request.startOnBoot ? "SÍ" : "NO"));

    if (!CommandRunner::isRoot()) {
        result.success = false;
        result.message = "No tienes permisos de administrador (root). Ejecuta slackwiz como root para configurar el servidor web.";
        LOG_ERROR("WebServerManager", result.message);
        return result;
    }

    const WebServerDetectionInfo env = detectEnvironment();

    if (server != "apache" && server != "nginx") {
        result.success = false;
        result.message = "Debes seleccionar un servidor web válido (Apache o Nginx).";
        LOG_ERROR("WebServerManager", result.message);
        return result;
    }

    if (server == "apache" && !env.apacheInstalled) {
        result.success = false;
        result.message = "Apache no se encuentra instalado en el sistema.";
        LOG_ERROR("WebServerManager", result.message);
        return result;
    }

    if (server == "nginx" && !env.nginxInstalled) {
        result.success = false;
        result.message = "Nginx no se encuentra instalado en el sistema.";
        LOG_ERROR("WebServerManager", result.message);
        return result;
    }

    if (request.enablePhp && !env.phpInstalled) {
        result.success = false;
        result.message = "Se solicitó habilitar PHP, pero PHP no está instalado en el sistema.";
        LOG_ERROR("WebServerManager", result.message);
        return result;
    }

    QStringList summarySteps;
    QString errorDetail;

    // 1. Configurar php.ini (siempre incluye date.timezone con la zona horaria del sistema)
    if (request.enablePhp) {
        if (!configurePhpIni(env.phpIniPath, request.phpOptions, env.systemTimezone, env.phpImagickInstalled, errorDetail)) {
            result.success = false;
            result.message = QString("Error al configurar PHP (%1):\n%2").arg(env.phpIniPath, errorDetail);
            return result;
        }

        summarySteps << QString("• Zona horaria de PHP configurada: date.timezone = \"%1\" en %2")
                            .arg(env.systemTimezone, env.phpIniPath);

        if (request.phpOptions.updatePhpIni) {
            summarySteps << QString("• Límites de PHP actualizados (memory_limit=%1, upload_max_filesize=%2, post_max_size=%3, display_errors=%4)")
                                .arg(request.phpOptions.memoryLimit,
                                     request.phpOptions.uploadMaxFilesize,
                                     request.phpOptions.postMaxSize,
                                     request.phpOptions.displayErrors ? "On" : "Off");
        }
    }

    // 2. Configurar el servidor web elegido (Apache o Nginx)
    QString detectedDocRoot;
    if (server == "apache") {
        if (!configureApache(request.enablePhp, detectedDocRoot, errorDetail)) {
            result.success = false;
            result.message = QString("Error al configurar Apache:\n%1").arg(errorDetail);
            return result;
        }

        summarySteps << "• Apache configurado: ServerName localhost:80 establecido en /etc/httpd/httpd.conf";
        summarySteps << (request.enablePhp
                              ? "• Soporte PHP habilitado en Apache (Include /etc/httpd/mod_php.conf)"
                              : "• Soporte PHP deshabilitado en Apache");

        const QString rcHttpd = "/etc/rc.d/rc.httpd";
        if (QFile::exists(rcHttpd)) {
            CommandRunner::setExecutablePermission(rcHttpd, request.startOnBoot);
            summarySteps << (request.startOnBoot
                                  ? "• Arranque automático de Apache habilitado (chmod +x /etc/rc.d/rc.httpd)"
                                  : "• Arranque automático de Apache deshabilitado (chmod -x /etc/rc.d/rc.httpd)");
        }
    } else if (server == "nginx") {
        if (!configureNginx(request.enablePhp, detectedDocRoot, errorDetail)) {
            result.success = false;
            result.message = QString("Error al configurar Nginx:\n%1").arg(errorDetail);
            return result;
        }

        summarySteps << (request.enablePhp
                              ? "• Nginx configurado con soporte PHP-FPM (/etc/nginx/nginx.conf)"
                              : "• Nginx configurado (sin bloque PHP-FPM activo)");

        const QString rcPhpFpm = "/etc/rc.d/rc.php-fpm";
        if (request.enablePhp && QFile::exists(rcPhpFpm)) {
            CommandRunner::setExecutablePermission(rcPhpFpm, request.startOnBoot);
            summarySteps << (request.startOnBoot
                                  ? "• Arranque automático de PHP-FPM habilitado (chmod +x /etc/rc.d/rc.php-fpm)"
                                  : "• Arranque automático de PHP-FPM deshabilitado");
        }

        const QString rcNginx = "/etc/rc.d/rc.nginx";
        if (QFile::exists(rcNginx)) {
            CommandRunner::setExecutablePermission(rcNginx, request.startOnBoot);
            summarySteps << (request.startOnBoot
                                  ? "• Arranque automático de Nginx habilitado (chmod +x /etc/rc.d/rc.nginx)"
                                  : "• Arranque automático de Nginx deshabilitado (chmod -x /etc/rc.d/rc.nginx)");
        }
    }

    result.documentRoot = detectedDocRoot;

    // 3. Crear/verificar el archivo de prueba php-info.php si PHP está habilitado
    if (request.enablePhp && !detectedDocRoot.isEmpty()) {
        QString createdPhpInfo;
        if (ensurePhpInfoPage(detectedDocRoot, createdPhpInfo)) {
            result.phpInfoFilePath = createdPhpInfo;
            summarySteps << QString("• Archivo de verificación PHP listo en %1").arg(createdPhpInfo);
        }
    }

    if (request.enablePhp && !env.phpImagickInstalled) {
        summarySteps << "• Recordatorio: php-imagick no está instalado; si lo necesitas, puedes instalarlo desde SlackBuilds.";
    }

    result.success = true;
    result.message = QString("Configuración guardada correctamente:\n%1")
                         .arg(summarySteps.join('\n'));

    LOG_SUCCESS("WebServerManager", "Archivos de configuración del servidor web guardados correctamente.");
    return result;
}

bool WebServerManager::restartWebServer(const QString &server, bool enablePhp, QString &errorDetail)
{
    const QString cleanServer = server.trimmed().toLower();
    LOG_INFO("WebServerManager", QString("Reiniciando servidor web '%1' (php=%2)...").arg(cleanServer).arg(enablePhp));

    if (cleanServer == "apache") {
        const QString rcHttpd = "/etc/rc.d/rc.httpd";
        if (!QFile::exists(rcHttpd)) {
            errorDetail = "No se encontró el script /etc/rc.d/rc.httpd.";
            return false;
        }
        const CommandResult res = CommandRunner::execute("/bin/sh", QStringList() << rcHttpd << "restart");
        if (!res.success) {
            errorDetail = res.stdErr.isEmpty() ? res.stdOut : res.stdErr;
            LOG_ERROR("WebServerManager", QString("Fallo al reiniciar Apache: %1").arg(errorDetail));
            return false;
        }
        LOG_SUCCESS("WebServerManager", "Apache reiniciado correctamente.");
        return true;
    }

    if (cleanServer == "nginx") {
        if (enablePhp) {
            const QString rcPhpFpm = "/etc/rc.d/rc.php-fpm";
            if (QFile::exists(rcPhpFpm)) {
                const CommandResult fpmRes = CommandRunner::execute("/bin/sh", QStringList() << rcPhpFpm << "restart");
                if (!fpmRes.success) {
                    errorDetail = QString("Error al reiniciar PHP-FPM: %1")
                                      .arg(fpmRes.stdErr.isEmpty() ? fpmRes.stdOut : fpmRes.stdErr);
                    LOG_ERROR("WebServerManager", errorDetail);
                    return false;
                }
            }
        }

        const QString rcNginx = "/etc/rc.d/rc.nginx";
        if (!QFile::exists(rcNginx)) {
            errorDetail = "No se encontró el script /etc/rc.d/rc.nginx.";
            return false;
        }
        const CommandResult nginxRes = CommandRunner::execute("/bin/sh", QStringList() << rcNginx << "restart");
        if (!nginxRes.success) {
            errorDetail = nginxRes.stdErr.isEmpty() ? nginxRes.stdOut : nginxRes.stdErr;
            LOG_ERROR("WebServerManager", QString("Fallo al reiniciar Nginx: %1").arg(errorDetail));
            return false;
        }
        LOG_SUCCESS("WebServerManager", "Nginx reiniciado correctamente.");
        return true;
    }

    errorDetail = "Servidor web no reconocido.";
    return false;
}

bool WebServerManager::configureApache(bool enablePhp, QString &documentRootOut, QString &errorDetail)
{
    const QString httpdConfPath = "/etc/httpd/httpd.conf";
    QFile file(httpdConfPath);

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorDetail = QString("No se encontró o no se pudo leer %1.").arg(httpdConfPath);
        LOG_ERROR("WebServerManager", errorDetail);
        return false;
    }

    documentRootOut = "/srv/httpd/htdocs";
    QStringList lines;
    QTextStream in(&file);
    bool foundModPhpInclude = false;
    bool updatedServerName = false;

    QRegularExpression serverNameRegex(R"(^\s*#?\s*ServerName\s+.*$)");
    QRegularExpression docRootRegex(R"(^\s*DocumentRoot\s+\"?([^\"]+)\"?)");

    while (!in.atEnd()) {
        QString line = in.readLine();
        const QString trimmed = line.trimmed();

        // Detectar DocumentRoot activo
        QRegularExpressionMatch docMatch = docRootRegex.match(trimmed);
        if (docMatch.hasMatch()) {
            documentRootOut = docMatch.captured(1).trimmed();
        }

        // Descomentar #ServerName www.example.com:80 y cambiar a ServerName localhost:80
        if (!updatedServerName && serverNameRegex.match(line).hasMatch()) {
            line = "ServerName localhost:80";
            updatedServerName = true;
            LOG_INFO("WebServerManager", "Directiva establecida en httpd.conf: ServerName localhost:80");
        }

        // Gestionar Include /etc/httpd/mod_php.conf
        if (trimmed.contains("mod_php.conf")) {
            foundModPhpInclude = true;
            if (enablePhp) {
                line = "Include /etc/httpd/mod_php.conf";
            } else {
                line = "#Include /etc/httpd/mod_php.conf";
            }
        }

        // Asegurar index.php en DirectoryIndex cuando PHP está activo
        if (enablePhp && trimmed.startsWith("DirectoryIndex ") && !trimmed.contains("index.php")) {
            line.replace("DirectoryIndex ", "DirectoryIndex index.php ");
        }

        lines.append(line);
    }
    file.close();

    if (!updatedServerName) {
        lines.append("");
        lines.append("ServerName localhost:80");
    }

    if (enablePhp && !foundModPhpInclude) {
        lines.append("");
        lines.append("# Habilitado por Slackware Wizard (slackwiz)");
        lines.append("Include /etc/httpd/mod_php.conf");
    }

    // Verificar que /etc/httpd/mod_php.conf exista si se habilitó PHP
    if (enablePhp && !QFile::exists("/etc/httpd/mod_php.conf")) {
        const QString libDir = QFile::exists("/usr/lib64/httpd/modules/libphp.so")
                                   ? "lib64/httpd/modules/libphp.so"
                                   : (QFile::exists("/usr/lib64/httpd/modules/libphp7.so")
                                          ? "lib64/httpd/modules/libphp7.so"
                                          : "lib/httpd/modules/libphp.so");

        const QString modPhpContent = QString(
            "# Configuración mod_php generada por Slackware Wizard (slackwiz)\n"
            "LoadModule php_module %1\n"
            "<FilesMatch \\.php$>\n"
            "    SetHandler application/x-httpd-php\n"
            "</FilesMatch>\n"
        ).arg(libDir);

        if (!CommandRunner::writeFileWithBackup("/etc/httpd/mod_php.conf", modPhpContent, true)) {
            errorDetail = "No se pudo crear /etc/httpd/mod_php.conf.";
            return false;
        }
    }

    if (!CommandRunner::writeFileWithBackup(httpdConfPath, lines.join('\n') + "\n", true)) {
        errorDetail = QString("No se pudo escribir %1.").arg(httpdConfPath);
        return false;
    }

    // Validar sintaxis de Apache con apachectl configtest
    const CommandResult testRes = CommandRunner::execute("/usr/sbin/apachectl", QStringList() << "configtest");
    if (!testRes.success) {
        errorDetail = QString("La validación de sintaxis de Apache falló (apachectl configtest):\n%1")
                          .arg(testRes.stdErr.isEmpty() ? testRes.stdOut : testRes.stdErr);
        LOG_ERROR("WebServerManager", errorDetail);
        return false;
    }

    LOG_SUCCESS("WebServerManager", "Sintaxis de Apache verificada correctamente (Syntax OK).");
    return true;
}

QString WebServerManager::detectPhpFpmListenTarget()
{
    const QString wwwConfPath = "/etc/php-fpm.d/www.conf";
    QFile file(wwwConfPath);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QRegularExpression listenRegex(R"(^\s*listen\s*=\s*([^\s;#]+))");
        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith(';')) {
                continue;
            }
            QRegularExpressionMatch match = listenRegex.match(line);
            if (match.hasMatch()) {
                const QString target = match.captured(1).trimmed();
                file.close();
                if (target.startsWith('/')) {
                    return QString("unix:%1").arg(target);
                }
                return target;
            }
        }
        file.close();
    }
    return "127.0.0.1:9000";
}

bool WebServerManager::configureNginx(bool enablePhp, QString &documentRootOut, QString &errorDetail)
{
    const QString nginxConfPath = "/etc/nginx/nginx.conf";
    QFile file(nginxConfPath);

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorDetail = QString("No se encontró o no se pudo leer %1.").arg(nginxConfPath);
        LOG_ERROR("WebServerManager", errorDetail);
        return false;
    }

    documentRootOut = QDir("/var/www/htdocs").exists() ? "/var/www/htdocs" : "/srv/httpd/htdocs";
    QStringList cleanedLines;
    QTextStream in(&file);
    bool insideManagedBlock = false;
    bool foundDocRoot = false;

    QRegularExpression rootRegex(R"(^\s*root\s+([^;]+);)");

    while (!in.atEnd()) {
        QString line = in.readLine();
        const QString trimmed = line.trimmed();

        if (!foundDocRoot && !trimmed.startsWith('#')) {
            QRegularExpressionMatch rootMatch = rootRegex.match(trimmed);
            if (rootMatch.hasMatch()) {
                const QString candidate = rootMatch.captured(1).trimmed();
                if (QDir(candidate).exists()) {
                    documentRootOut = candidate;
                    foundDocRoot = true;
                }
            }
        }

        if (trimmed == "# slackwiz-php-start") {
            insideManagedBlock = true;
            continue;
        }
        if (trimmed == "# slackwiz-php-end") {
            insideManagedBlock = false;
            continue;
        }
        if (insideManagedBlock) {
            continue;
        }

        if (enablePhp && trimmed.startsWith("index ") && !trimmed.contains("index.php")) {
            line.replace("index ", "index index.php ");
        }

        cleanedLines.append(line);
    }
    file.close();

    if (enablePhp) {
        const QString fastcgiPass = detectPhpFpmListenTarget();
        LOG_INFO("WebServerManager", QString("Destino PHP-FPM detectado para Nginx: %1").arg(fastcgiPass));

        int serverDepth = 0;
        bool insideServer = false;
        int insertIndex = -1;

        for (int i = 0; i < cleanedLines.size(); ++i) {
            const QString trimmed = cleanedLines[i].trimmed();
            if (trimmed.startsWith('#')) {
                continue;
            }

            if (!insideServer && (trimmed.startsWith("server {") || trimmed == "server")) {
                insideServer = true;
            }

            if (insideServer) {
                serverDepth += trimmed.count('{');
                serverDepth -= trimmed.count('}');
                if (serverDepth <= 0 && trimmed.contains('}')) {
                    insertIndex = i;
                    break;
                }
            }
        }

        if (insertIndex == -1) {
            errorDetail = "No se encontró un bloque 'server { ... }' activo dentro de /etc/nginx/nginx.conf.";
            LOG_ERROR("WebServerManager", errorDetail);
            return false;
        }

        QStringList phpBlock;
        phpBlock << "        # slackwiz-php-start"
                 << "        location ~ \\.php$ {"
                 << "            try_files $uri =404;"
                 << QString("            fastcgi_pass   %1;").arg(fastcgiPass)
                 << "            fastcgi_index  index.php;"
                 << "            fastcgi_param  SCRIPT_FILENAME  $document_root$fastcgi_script_name;"
                 << "            include        fastcgi_params;"
                 << "        }"
                 << "        # slackwiz-php-end";

        for (int k = 0; k < phpBlock.size(); ++k) {
            cleanedLines.insert(insertIndex + k, phpBlock[k]);
        }
    }

    if (!CommandRunner::writeFileWithBackup(nginxConfPath, cleanedLines.join('\n') + "\n", true)) {
        errorDetail = QString("No se pudo escribir %1.").arg(nginxConfPath);
        return false;
    }

    const CommandResult testRes = CommandRunner::execute("/usr/sbin/nginx", QStringList() << "-t");
    if (!testRes.success) {
        errorDetail = QString("La validación de sintaxis de Nginx falló (nginx -t):\n%1")
                          .arg(testRes.stdErr.isEmpty() ? testRes.stdOut : testRes.stdErr);
        LOG_ERROR("WebServerManager", errorDetail);
        return false;
    }

    LOG_SUCCESS("WebServerManager", "Sintaxis de Nginx verificada correctamente (nginx -t OK).");
    return true;
}

bool WebServerManager::configurePhpIni(const QString &phpIniPath,
                                       const PhpConfigOptions &options,
                                       const QString &systemTimezone,
                                       bool ensureImagickExtension,
                                       QString &errorDetail)
{
    QFile file(phpIniPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorDetail = QString("No se encontró o no se pudo leer %1.").arg(phpIniPath);
        LOG_ERROR("WebServerManager", errorDetail);
        return false;
    }

    QStringList lines;
    QTextStream in(&file);

    bool setTimezone = false;
    bool setMemory = false;
    bool setUpload = false;
    bool setPost = false;
    bool setDisplay = false;
    bool hasImagickExt = false;

    const QString displayVal = options.displayErrors ? "On" : "Off";
    const QString tzDirective = QString("date.timezone = \"%1\"").arg(systemTimezone);
    QRegularExpression tzRegex(R"(^\s*;?\s*date\.timezone\s*=)");

    while (!in.atEnd()) {
        QString line = in.readLine();
        const QString trimmed = line.trimmed();

        // Siempre configurar date.timezone con la zona geográfica del sistema
        if (!setTimezone && tzRegex.match(line).hasMatch()) {
            line = tzDirective;
            setTimezone = true;
            LOG_INFO("WebServerManager", QString("Zona horaria establecida en php.ini: %1").arg(tzDirective));
        } else if (options.updatePhpIni && trimmed.startsWith("memory_limit") && trimmed.contains('=')) {
            line = QString("memory_limit = %1").arg(options.memoryLimit);
            setMemory = true;
        } else if (options.updatePhpIni && trimmed.startsWith("upload_max_filesize") && trimmed.contains('=')) {
            line = QString("upload_max_filesize = %1").arg(options.uploadMaxFilesize);
            setUpload = true;
        } else if (options.updatePhpIni && trimmed.startsWith("post_max_size") && trimmed.contains('=')) {
            line = QString("post_max_size = %1").arg(options.postMaxSize);
            setPost = true;
        } else if (options.updatePhpIni && trimmed.startsWith("display_errors") && trimmed.contains('=')) {
            line = QString("display_errors = %1").arg(displayVal);
            setDisplay = true;
        } else if (trimmed == "extension=imagick.so" || trimmed == "extension=imagick") {
            hasImagickExt = true;
        } else if (ensureImagickExtension && (trimmed == ";extension=imagick.so" || trimmed == ";extension=imagick")) {
            line = "extension=imagick.so";
            hasImagickExt = true;
        }

        lines.append(line);
    }
    file.close();

    if (!setTimezone) {
        lines.append("");
        lines.append("[Date]");
        lines.append(tzDirective);
    }

    if (options.updatePhpIni) {
        if (!setMemory) {
            lines.append(QString("memory_limit = %1").arg(options.memoryLimit));
        }
        if (!setUpload) {
            lines.append(QString("upload_max_filesize = %1").arg(options.uploadMaxFilesize));
        }
        if (!setPost) {
            lines.append(QString("post_max_size = %1").arg(options.postMaxSize));
        }
        if (!setDisplay) {
            lines.append(QString("display_errors = %1").arg(displayVal));
        }
    }

    if (ensureImagickExtension && !hasImagickExt && !QFile::exists("/etc/php.d/imagick.ini")) {
        lines.append("extension=imagick.so");
    }

    if (!CommandRunner::writeFileWithBackup(phpIniPath, lines.join('\n') + "\n", true)) {
        errorDetail = QString("No se pudo guardar %1.").arg(phpIniPath);
        return false;
    }

    LOG_SUCCESS("WebServerManager", QString("Archivo %1 actualizado correctamente.").arg(phpIniPath));
    return true;
}

bool WebServerManager::ensurePhpInfoPage(const QString &documentRoot, QString &createdPathOut)
{
    QDir rootDir(documentRoot);
    if (!rootDir.exists()) {
        LOG_WARN("WebServerManager", QString("DocumentRoot '%1' no existe; omitiendo creación de php-info.php.").arg(documentRoot));
        return false;
    }

    const QString targetFile = rootDir.filePath("php-info.php");
    const QString content =
        "<?php\n"
        "// Archivo de diagnóstico generado por Slackware Wizard (slackwiz)\n"
        "phpinfo();\n"
        "?>\n";

    if (CommandRunner::writeFileWithBackup(targetFile, content, false)) {
        createdPathOut = targetFile;
        LOG_SUCCESS("WebServerManager", QString("Archivo de prueba PHP creado en: %1").arg(targetFile));
        return true;
    }

    return false;
}
