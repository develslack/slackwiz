#ifndef COMMANDRUNNER_H
#define COMMANDRUNNER_H

#include <QString>
#include <QStringList>

struct CommandResult {
    int exitCode = -1;
    QString stdOut;
    QString stdErr;
    bool success = false;
};

class CommandRunner
{
public:
    static bool isRoot();
    static QString detectSlackwareVersion();
    static CommandResult execute(const QString &program,
                                 const QStringList &arguments = QStringList(),
                                 int timeoutMs = 30000);
    static CommandResult executeShell(const QString &shellCommand,
                                      int timeoutMs = 30000);
    static bool writeFileWithBackup(const QString &filePath,
                                    const QString &content,
                                    bool createBackup = true);
    static bool setExecutablePermission(const QString &filePath, bool executable);
};

#endif // COMMANDRUNNER_H
