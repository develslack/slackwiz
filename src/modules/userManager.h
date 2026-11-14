#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QString>
#include <QStringList>
#include <QList>

struct SystemGroupOption {
    QString name;
    QString description;
    bool recommendedByDefault = false;
    bool existsOnSystem = true;
};

struct NewUserRequest {
    QString username;
    QString fullName;
    QString password;
    QString confirmPassword;
    QString shell;
    QStringList selectedGroups;
};

struct UserOperationResult {
    bool success = false;
    QString message;
};

class UserManager
{
public:
    static QList<SystemGroupOption> getAvailableGroups();
    static QStringList getAvailableShells();
    static QStringList getExistingHumanUsers();
    static bool userExists(const QString &username);
    static bool groupExists(const QString &groupName);
    static UserOperationResult createUser(const NewUserRequest &request);

private:
    static bool setUserPassword(const QString &username, const QString &password);
};

#endif // USERMANAGER_H
