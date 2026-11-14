#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

class Logger
{
public:
    enum class Level {
        Info,
        Success,
        Warning,
        Error,
        Command
    };

    static void log(Level level, const QString &context, const QString &message);
};

#ifdef SLACKWIZ_DEBUG
    #define LOG_INFO(ctx, msg)    Logger::log(Logger::Level::Info, ctx, msg)
    #define LOG_SUCCESS(ctx, msg) Logger::log(Logger::Level::Success, ctx, msg)
    #define LOG_WARN(ctx, msg)    Logger::log(Logger::Level::Warning, ctx, msg)
    #define LOG_ERROR(ctx, msg)   Logger::log(Logger::Level::Error, ctx, msg)
    #define LOG_CMD(ctx, msg)     Logger::log(Logger::Level::Command, ctx, msg)
#else
    #define LOG_INFO(ctx, msg)    ((void)0)
    #define LOG_SUCCESS(ctx, msg) ((void)0)
    #define LOG_WARN(ctx, msg)    ((void)0)
    #define LOG_ERROR(ctx, msg)   ((void)0)
    #define LOG_CMD(ctx, msg)     ((void)0)
#endif

#endif // LOGGER_H
