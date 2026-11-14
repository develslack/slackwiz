#ifndef SUDOMANAGER_H
#define SUDOMANAGER_H

#include <QString>
#include <QStringList>
#include <QList>

enum class SudoAccessMode {
    Disabled,
    WithPassword,
    NoPassword
};

struct UserSudoStatus {
    QString username;
    bool inWheelGroup = false;
    SudoAccessMode accessMode = SudoAccessMode::Disabled;
    QString sourceDetail;
};

struct SudoOperationResult {
    bool success = false;
    QString message;
};

class SudoManager
{
public:
    static QList<UserSudoStatus> getAllUsersSudoStatus();
    static UserSudoStatus getUserSudoStatus(const QString &username);
    static SudoOperationResult applyUserSudo(const QString &username,
                                             SudoAccessMode mode,
                                             bool syncWheelGroup = true);

private:
    static bool ensureSudoersIncludeDir();
    static bool validateSudoersSyntax(const QString &filePath);
    static bool secureSudoersFile(const QString &filePath);
    static bool isUserInGroup(const QString &username, const QString &groupName);
    static SudoAccessMode parseRuleFromFile(const QString &filePath, const QString &username);
};

#endif // SUDOMANAGER_H
