#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <QString>
#include <QList>
#include <QDateTime>

struct TimezoneEntry {
    QString id;           // Ej: "America/Argentina/Buenos_Aires"
    QString countryCode;  // Ej: "AR"
    QString comment;      // Ej: "Buenos Aires (BA, CF)"
    double latitude = 0.0;
    double longitude = 0.0;
};

struct TimeSystemState {
    QString currentTimezone = "America/Argentina/Buenos_Aires";
    bool ntpEnabled = false;
    bool ntpRunning = false;
    bool dstActive = false;
    bool manualDstOverride = false;
    bool hardwareClockUtc = true;
};

struct TimeConfigRequest {
    QString timezoneId;
    bool enableDst = false;
    bool automaticNtp = true;
    QDateTime manualDateTime;
    bool hardwareClockUtc = true;
    QString ntpServer = "pool.ntp.org";
};

struct TimeConfigResult {
    bool success = false;
    QString message;
};

class TimeManager
{
public:
    static QList<TimezoneEntry> loadSystemTimezones();
    static TimeSystemState detectCurrentState();
    static TimeConfigResult applyConfiguration(const TimeConfigRequest &req);
    static QString formatUtcOffset(int offsetSeconds);

private:
    static bool parseIso6709(const QString &coord, double &latOut, double &lonOut);
    static QString formatZicOffset(int offsetSeconds);
    static bool applyCustomOffsetZone(const QString &zoneName, int offsetSeconds);
    static void ensureNtpPoolConfigured(const QString &ntpServer);
};

#endif // TIMEMANAGER_H
