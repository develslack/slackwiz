#include "timeManager.h"
#include "commandRunner.h"
#include "logger.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTimeZone>
#include <QtMath>
#include <algorithm>

bool TimeManager::parseIso6709(const QString &coord, double &latOut, double &lonOut)
{
    if (coord.size() < 11) {
        return false;
    }

    int lonIdx = -1;
    for (int i = 1; i < coord.size(); ++i) {
        if (coord[i] == '+' || coord[i] == '-') {
            lonIdx = i;
            break;
        }
    }
    if (lonIdx < 5) {
        return false;
    }

    const QString latStr = coord.left(lonIdx);
    const QString lonStr = coord.mid(lonIdx);

    auto parsePart = [](const QString &s, int degDigits, double &outVal) -> bool {
        if (s.size() < 1 + degDigits + 2) {
            return false;
        }
        const double sign = (s[0] == '-') ? -1.0 : 1.0;
        bool okD = false, okM = false, okS = true;
        const int deg = s.mid(1, degDigits).toInt(&okD);
        const int min = s.mid(1 + degDigits, 2).toInt(&okM);
        int sec = 0;
        if (s.size() >= 1 + degDigits + 4) {
            sec = s.mid(1 + degDigits + 2, 2).toInt(&okS);
        }
        if (!okD || !okM || !okS) {
            return false;
        }
        outVal = sign * (deg + (min / 60.0) + (sec / 3600.0));
        return true;
    };

    return parsePart(latStr, 2, latOut) && parsePart(lonStr, 3, lonOut);
}

QList<TimezoneEntry> TimeManager::loadSystemTimezones()
{
    QList<TimezoneEntry> list;
    const QStringList tabPaths = {
        "/usr/share/zoneinfo/zone1970.tab",
        "/usr/share/zoneinfo/zone.tab"
    };

    QFile file;
    for (const QString &path : tabPaths) {
        if (QFileInfo::exists(path)) {
            file.setFileName(path);
            break;
        }
    }

    if (!file.fileName().isEmpty() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith('#')) {
                continue;
            }
            const QStringList cols = line.split('\t');
            if (cols.size() < 3) {
                continue;
            }

            TimezoneEntry entry;
            entry.countryCode = cols[0].trimmed();
            entry.id = cols[2].trimmed();
            entry.comment = (cols.size() >= 4) ? cols[3].trimmed() : QString();

            if (!parseIso6709(cols[1].trimmed(), entry.latitude, entry.longitude)) {
                continue;
            }

            if (QFileInfo::exists("/usr/share/zoneinfo/" + entry.id)) {
                list.append(entry);
            }
        }
    }

    // Garantizar presencia de zonas clave si el archivo no estuviera disponible
    auto containsZone = [&list](const QString &id) {
        return std::any_of(list.begin(), list.end(), [&id](const TimezoneEntry &e) {
            return e.id == id;
        });
    };

    if (!containsZone("America/Argentina/Buenos_Aires")) {
        list.append({ "America/Argentina/Buenos_Aires", "AR", "Buenos Aires", -34.60, -58.45 });
    }
    if (!containsZone("UTC")) {
        list.append({ "UTC", "UN", "Tiempo Universal Coordinado", 0.0, 0.0 });
    }

    std::sort(list.begin(), list.end(), [](const TimezoneEntry &a, const TimezoneEntry &b) {
        return a.id < b.id;
    });

    return list;
}

TimeSystemState TimeManager::detectCurrentState()
{
    TimeSystemState state;

    // 1. Si existe registro de zona base con override manual de DST
    QFile customTag("/etc/slackwiz-timezone");
    if (customTag.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString savedZone = QString::fromUtf8(customTag.readAll()).trimmed();
        if (!savedZone.isEmpty()) {
            state.currentTimezone = savedZone;
            state.manualDstOverride = true;
        }
    }

    // 2. Resolver symlink /etc/localtime o /etc/localtime-copied-from
    if (!state.manualDstOverride) {
        const QStringList linkCandidates = { "/etc/localtime", "/etc/localtime-copied-from" };
        for (const QString &linkPath : linkCandidates) {
            QFileInfo fi(linkPath);
            if (fi.isSymLink()) {
                QString target = fi.symLinkTarget();
                const QString prefix = "/usr/share/zoneinfo/";
                if (target.startsWith(prefix)) {
                    const QString tzId = target.mid(prefix.size());
                    if (!tzId.startsWith("Custom/")) {
                        state.currentTimezone = tzId;
                        break;
                    }
                }
            }
        }
    }

    // 3. Estado de horario de verano actual
    QTimeZone tz(state.currentTimezone.toUtf8());
    const QDateTime nowUtc = QDateTime::currentDateTimeUtc();
    if (state.manualDstOverride) {
        const QFileInfo ltInfo("/etc/localtime");
        state.dstActive = ltInfo.symLinkTarget().endsWith("Slackwiz_DST");
    } else if (tz.isValid()) {
        state.dstActive = tz.isDaylightTime(nowUtc);
    }

    // 4. Estado de NTP en Slackware (/etc/rc.d/rc.ntpd)
    const QFileInfo rcNtpd("/etc/rc.d/rc.ntpd");
    state.ntpEnabled = (rcNtpd.exists() && rcNtpd.isExecutable());
    const CommandResult pgrepNtp = CommandRunner::execute("pgrep", QStringList() << "-x" << "ntpd");
    state.ntpRunning = (pgrepNtp.success && !pgrepNtp.stdOut.trimmed().isEmpty());

    // 5. Modo de reloj de hardware (/etc/hardwareclock)
    QFile hwFile("/etc/hardwareclock");
    if (hwFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString content = QString::fromUtf8(hwFile.readAll());
        if (content.contains("localtime", Qt::CaseInsensitive) &&
            !content.contains("\nlocaltime") == false) {
            for (const QString &line : content.split('\n')) {
                const QString trimmed = line.trimmed();
                if (!trimmed.startsWith('#') && trimmed == "localtime") {
                    state.hardwareClockUtc = false;
                }
            }
        }
    }

    return state;
}

QString TimeManager::formatUtcOffset(int offsetSeconds)
{
    const char sign = (offsetSeconds >= 0) ? '+' : '-';
    const int absSec = qAbs(offsetSeconds);
    const int hours = absSec / 3600;
    const int mins = (absSec % 3600) / 60;
    if (mins == 0) {
        return QString("UTC%1%2").arg(sign).arg(hours);
    }
    return QString("UTC%1%2:%3").arg(sign).arg(hours).arg(mins, 2, 10, QChar('0'));
}

QString TimeManager::formatZicOffset(int offsetSeconds)
{
    const char sign = (offsetSeconds >= 0) ? '+' : '-';
    const int absSec = qAbs(offsetSeconds);
    const int hours = absSec / 3600;
    const int mins = (absSec % 3600) / 60;
    return QString("%1%2:%3").arg(sign).arg(hours).arg(mins, 2, 10, QChar('0'));
}

bool TimeManager::applyCustomOffsetZone(const QString &zoneName, int offsetSeconds)
{
    const QString zicOffset = formatZicOffset(offsetSeconds);
    const int hours = offsetSeconds / 3600;
    const QString abbr = QString("%1%2")
                             .arg(offsetSeconds >= 0 ? "+" : "-")
                             .arg(qAbs(hours), 2, 10, QChar('0'));

    const QString tmpZicPath = "/tmp/slackwiz_custom_tz.zic";
    QFile zicFile(tmpZicPath);
    if (!zicFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&zicFile);
    out << "Zone Custom/" << zoneName << " " << zicOffset << " - " << abbr << "\n";
    zicFile.close();

    const CommandResult zicRes = CommandRunner::execute(
        "zic",
        QStringList() << "-d" << "/usr/share/zoneinfo" << tmpZicPath
    );
    QFile::remove(tmpZicPath);

    if (!zicRes.success) {
        return false;
    }

    const QString targetZoneFile = QString("/usr/share/zoneinfo/Custom/%1").arg(zoneName);
    CommandRunner::execute("ln", QStringList() << "-sf" << targetZoneFile << "/etc/localtime");
    CommandRunner::execute("ln", QStringList() << "-sf" << targetZoneFile << "/etc/localtime-copied-from");
    return true;
}

void TimeManager::ensureNtpPoolConfigured(const QString &ntpServer)
{
    const QString server = ntpServer.trimmed().isEmpty() ? "pool.ntp.org" : ntpServer.trimmed();
    const QString ntpConfPath = "/etc/ntp.conf";

    QString content;
    QFile confFile(ntpConfPath);
    if (confFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        content = QString::fromUtf8(confFile.readAll());
        confFile.close();
    }

    bool hasActiveServer = false;
    for (const QString &line : content.split('\n')) {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith("server ") || trimmed.startsWith("pool ")) {
            hasActiveServer = true;
            break;
        }
    }

    if (!hasActiveServer) {
        if (confFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            QTextStream out(&confFile);
            out << "\n# Configurado por slackwiz\n";
            out << "server " << server << " iburst\n";
            out << "server 0.south-america.pool.ntp.org iburst\n";
            out << "server 1.south-america.pool.ntp.org iburst\n";
            confFile.close();
        }
    }
}

TimeConfigResult TimeManager::applyConfiguration(const TimeConfigRequest &req)
{
    if (!CommandRunner::isRoot()) {
        return { false, "Permisos insuficientes: ejecuta slackwiz como root para modificar la hora del sistema." };
    }

    const QString zoneFile = "/usr/share/zoneinfo/" + req.timezoneId;
    if (!QFileInfo::exists(zoneFile)) {
        return { false, QString("La zona horaria '%1' no existe en /usr/share/zoneinfo.").arg(req.timezoneId) };
    }

    LOG_INFO("TimeManager", QString("Aplicando zona horaria %1 (DST=%2, NTP=%3)...")
                                .arg(req.timezoneId)
                                .arg(req.enableDst ? "Sí" : "No")
                                .arg(req.automaticNtp ? "Auto" : "Manual"));

    QStringList reportSteps;

    // 1. Aplicar zona horaria y gestión de horario de verano (DST)
    QTimeZone tz(req.timezoneId.toUtf8());
    const QDateTime refUtc = req.automaticNtp
                                 ? QDateTime::currentDateTimeUtc()
                                 : QDateTime(req.manualDateTime.date(), req.manualDateTime.time(), tz).toUTC();

    const bool nativeDst = tz.isValid() ? tz.isDaylightTime(refUtc) : false;
    const int stdOffsetSec = tz.isValid() ? tz.standardTimeOffset(refUtc) : 0;

    if (req.enableDst && !nativeDst) {
        // El usuario activó manualmente el horario de verano (+1h) en una zona sin DST vigente (ej: Buenos Aires)
        if (!applyCustomOffsetZone("Slackwiz_DST", stdOffsetSec + 3600)) {
            return { false, "No se pudo compilar la regla de horario de verano con zic." };
        }
        QFile tag("/etc/slackwiz-timezone");
        if (tag.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            tag.write(req.timezoneId.toUtf8());
            tag.close();
        }
        reportSteps << QString("Zona configurada en %1 con horario de verano activado (%2).")
                           .arg(req.timezoneId, formatUtcOffset(stdOffsetSec + 3600));
    } else if (!req.enableDst && nativeDst) {
        // El usuario desactivó explícitamente el horario de verano en una zona con DST activo
        if (!applyCustomOffsetZone("Slackwiz_STD", stdOffsetSec)) {
            return { false, "No se pudo compilar la regla de hora estándar con zic." };
        }
        QFile tag("/etc/slackwiz-timezone");
        if (tag.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            tag.write(req.timezoneId.toUtf8());
            tag.close();
        }
        reportSteps << QString("Zona configurada en %1 en hora estándar sin horario de verano (%2).")
                           .arg(req.timezoneId, formatUtcOffset(stdOffsetSec));
    } else {
        // Zona estándar IANA directa
        QFile::remove("/etc/slackwiz-timezone");
        CommandRunner::execute("ln", QStringList() << "-sf" << zoneFile << "/etc/localtime");
        CommandRunner::execute("ln", QStringList() << "-sf" << zoneFile << "/etc/localtime-copied-from");
        const int activeOffset = tz.isValid() ? tz.offsetFromUtc(refUtc) : 0;
        reportSteps << QString("Zona horaria establecida en %1 (%2).")
                           .arg(req.timezoneId, formatUtcOffset(activeOffset));
    }

    // 2. Configurar modo de reloj de hardware (/etc/hardwareclock)
    QFile hwFile("/etc/hardwareclock");
    if (hwFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&hwFile);
        out << "# /etc/hardwareclock\n";
        out << "# Configurado por slackwiz\n";
        out << (req.hardwareClockUtc ? "UTC\n" : "localtime\n");
        hwFile.close();
        reportSteps << QString("Reloj de hardware configurado en modo %1.")
                           .arg(req.hardwareClockUtc ? "UTC" : "localtime");
    }

    // 3. Sincronización automática mundial (NTP) vs Fecha y hora manual
    const QString rcNtpd = "/etc/rc.d/rc.ntpd";
    if (req.automaticNtp) {
        ensureNtpPoolConfigured(req.ntpServer);

        if (QFileInfo::exists(rcNtpd)) {
            CommandRunner::execute("chmod", QStringList() << "+x" << rcNtpd);
            CommandRunner::execute(
                "setsid",
                QStringList() << "/bin/sh" << "-c" << "/etc/rc.d/rc.ntpd stop >/dev/null 2>&1 </dev/null"
            );
        }

        // Salto inicial inmediato de hora antes de levantar el demonio continuo
        bool syncedNow = false;
        if (QFileInfo::exists("/usr/sbin/ntpdate")) {
            const QString srv = req.ntpServer.trimmed().isEmpty() ? "pool.ntp.org" : req.ntpServer.trimmed();
            const CommandResult ntpDateRes = CommandRunner::execute("/usr/sbin/ntpdate", QStringList() << "-u" << srv);
            syncedNow = ntpDateRes.success;
        }
        if (!syncedNow && QFileInfo::exists("/usr/sbin/ntpd")) {
            const CommandResult ntpdOnce = CommandRunner::execute("/usr/sbin/ntpd", QStringList() << "-gq");
            syncedNow = ntpdOnce.success;
        }

        if (QFileInfo::exists(rcNtpd)) {
            CommandRunner::execute(
                "setsid",
                QStringList() << "/bin/sh" << "-c" << "/etc/rc.d/rc.ntpd start >/dev/null 2>&1 </dev/null"
            );
        }

        reportSteps << (syncedNow
                            ? "Hora sincronizada automáticamente por Internet (NTP) y servicio ntpd activo."
                            : "Servicio ntpd activado en el arranque (sincronizará en cuanto haya conexión de red).");
    } else {
        // Deshabilitar ntpd para evitar que sobreescriba la hora manual
        if (QFileInfo::exists(rcNtpd)) {
            CommandRunner::execute(
                "setsid",
                QStringList() << "/bin/sh" << "-c" << "/etc/rc.d/rc.ntpd stop >/dev/null 2>&1 </dev/null"
            );
            CommandRunner::execute("chmod", QStringList() << "-x" << rcNtpd);
        }

        if (!req.manualDateTime.isValid()) {
            return { false, "La fecha y hora manual ingresadas no son válidas." };
        }

        const QString formattedDate = req.manualDateTime.toString("yyyy-MM-dd HH:mm:ss");
        const CommandResult dateRes = CommandRunner::execute("date", QStringList() << "-s" << formattedDate);
        if (!dateRes.success) {
            return {
                false,
                QString("No se pudo establecer la hora manual del sistema:\n%1").arg(dateRes.stdErr)
            };
        }

        reportSteps << QString("Fecha y hora manuales aplicadas: %1 (NTP desactivado).").arg(formattedDate);
    }

    // 4. Guardar hora actual del sistema en el reloj RTC de la placa madre
    const QString hwFlag = req.hardwareClockUtc ? "--utc" : "--localtime";
    CommandRunner::execute("hwclock", QStringList() << "--systohc" << hwFlag);

    LOG_SUCCESS("TimeManager", "Configuración horaria aplicada correctamente.");
    return {
        true,
        QString("Configuración de hora del sistema aplicada:\n• %1").arg(reportSteps.join("\n• "))
    };
}
