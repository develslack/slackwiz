#ifndef LOCALEMANAGER_H
#define LOCALEMANAGER_H

#include <QString>
#include <QList>

struct LocaleOption {
    QString code;
    QString displayName;
};

struct LocaleApplyResult {
    bool success = false;
    QString message;
};

class LocaleManager
{
public:
    static QString getCurrentSystemLocale();
    static QString getLocaleDisplayName(const QString &localeCode);
    static QList<LocaleOption> getAvailableLocales();
    static LocaleApplyResult applySystemLocale(const QString &localeCode);
};

#endif // LOCALEMANAGER_H
