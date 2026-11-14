#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <QString>
#include <QStringList>
#include <QList>

struct ManagedGroupItem {
    QString name;
    QString description;
    bool recommended = false;
    bool existsOnSystem = true;
};

struct UserGroupsInfo {
    QString username;
    QString primaryGroup;
    QStringList secondaryGroups;
    bool valid = false;
};

struct GroupUpdateResult {
    bool success = false;
    QString message;
};

class GroupManager
{
public:
    static QStringList getManageableUsers();
    static UserGroupsInfo getUserGroups(const QString &username);
    static QList<ManagedGroupItem> getManageableGroups(const QStringList &extraUserGroups = QStringList());
    static GroupUpdateResult updateUserGroups(const QString &username,
                                              const QStringList &selectedSecondaryGroups);
};

#endif // GROUPMANAGER_H
