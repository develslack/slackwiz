#include <QDateTime>
#include <QString>
#include <iostream>
#include "logger.h"

void Logger::log(Level level, const QString &context, const QString &message)
{
#ifdef SLACKWIZ_DEBUG
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");

    QString colorCode;
    QString tag;

    switch (level) {
        case Level::Info:
            colorCode = "\033[0;36m"; // Cian
            tag = "INFO ";
            break;
        case Level::Success:
            colorCode = "\033[0;32m"; // Verde
            tag = "OK   ";
            break;
        case Level::Warning:
            colorCode = "\033[1;33m"; // Amarillo
            tag = "WARN ";
            break;
        case Level::Error:
            colorCode = "\033[0;31m"; // Rojo
            tag = "ERROR";
            break;
        case Level::Command:
            colorCode = "\033[0;35m"; // Magenta
            tag = "CMD  ";
            break;
    }

    const QString resetCode = "\033[0m";
    const QString formatted = QString("[%1] %2[%3]%4 [%5] %6")
                                  .arg(timestamp, colorCode, tag, resetCode, context, message);

    if (level == Level::Error) {
        std::cerr << formatted.toStdString() << std::endl;
    } else {
        std::cout << formatted.toStdString() << std::endl;
    }
#else
    Q_UNUSED(level)
    Q_UNUSED(context)
    Q_UNUSED(message)
#endif
}
