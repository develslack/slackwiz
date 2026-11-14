#include "groupManager.h"
#include "commandRunner.h"
#include "logger.h"

#include <QFile>
#include <QTextStream>
#include <vector>
#include <pwd.h>
#include <grp.h>

QStringList GroupManager::getManageableUsers()
{
    QStringList users;
    QFile passwdFile("/etc/passwd");

    if (!passwdFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR("GroupManager", "No se pudo abrir /etc/passwd para obtener usuarios.");
        return users;
    }

    QTextStream in(&passwdFile);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        const QStringList parts = line.split(':');
        if (parts.size() >= 7) {
            const QString uname = parts[0];
            bool ok = false;
            const int uid = parts[2].toInt(&ok);
            const QString shell = parts[6];

            if (ok && uid >= 1000 && uid < 65534 && uname != "nobody" &&
                !shell.endsWith("nologin") && !shell.endsWith("false")) {
                users.append(uname);
            }
        }
    }
    passwdFile.close();

    LOG_INFO("GroupManager", QString("Usuarios disponibles para gestionar grupos: %1")
                                 .arg(users.isEmpty() ? "ninguno" : users.join(", ")));
    return users;
}

UserGroupsInfo GroupManager::getUserGroups(const QString &username)
{
    UserGroupsInfo info;
    info.username = username.trimmed();

    if (info.username.isEmpty()) {
        return info;
    }

    const QByteArray utf8User = info.username.toUtf8();
    struct passwd *pw = getpwnam(utf8User.constData());
    if (!pw) {
        LOG_WARN("GroupManager", QString("Usuario '%1' no encontrado en /etc/passwd.").arg(info.username));
        return info;
    }

    struct group *primaryGr = getgrgid(pw->pw_gid);
    info.primaryGroup = primaryGr ? QString::fromUtf8(primaryGr->gr_name) : QString::number(pw->pw_gid);

    int ngroups = 32;
    std::vector<gid_t> gids(static_cast<size_t>(ngroups));

    if (getgrouplist(utf8User.constData(), pw->pw_gid, gids.data(), &ngroups) == -1) {
        gids.resize(static_cast<size_t>(ngroups));
        getgrouplist(utf8User.constData(), pw->pw_gid, gids.data(), &ngroups);
    }

    for (int i = 0; i < ngroups; ++i) {
        const gid_t gid = gids[static_cast<size_t>(i)];
        if (gid == pw->pw_gid) {
            continue; // El grupo primario se muestra por separado
        }

        struct group *gr = getgrgid(gid);
        if (gr) {
            const QString gname = QString::fromUtf8(gr->gr_name);
            if (!info.secondaryGroups.contains(gname)) {
                info.secondaryGroups.append(gname);
            }
        }
    }

    info.secondaryGroups.sort();
    info.valid = true;

    LOG_INFO("GroupManager", QString("Grupos de '%1' -> Primario: %2 | Secundarios: [%3]")
                                 .arg(info.username,
                                      info.primaryGroup,
                                      info.secondaryGroups.isEmpty() ? "ninguno" : info.secondaryGroups.join(", ")));
    return info;
}

QList<ManagedGroupItem> GroupManager::getManageableGroups(const QStringList &extraUserGroups)
{
    QList<ManagedGroupItem> items = {
        { "wheel",     "Administrador del sistema (permite usar sudo)",          true,  true },
        { "audio",     "Dispositivos de sonido y multimedia (ALSA / PipeWire)",  true,  true },
        { "video",     "Aceleración gráfica 3D, cámaras web y pantallas",        true,  true },
        { "plugdev",   "Montaje automático de memorias USB y discos externos",   true,  true },
        { "power",     "Apagar, reiniciar y suspender desde el entorno gráfico", true,  true },
        { "netdev",    "Gestión de conexiones Wi-Fi y red (NetworkManager)",     true,  true },
        { "cdrom",     "Lectura y grabación en unidades ópticas CD/DVD",         true,  true },
        { "scanner",   "Acceso a escáneres de documentos",                       true,  true },
        { "lp",        "Impresoras y administración de trabajos (CUPS)",         true,  true },
        { "floppy",    "Unidades de disquete y medios extraíbles heredados",     false, true },
        { "dialout",   "Acceso a puertos serie, modems y placas Arduino/ESP32",  false, true },
        { "games",     "Acceso a binarios y puntuaciones de juegos del sistema", false, true }
    };

    // Grupos opcionales comunes en sistemas con virtualización/contenedores si están instalados
    const QList<ManagedGroupItem> optionalSystemGroups = {
        { "docker",    "Gestión de contenedores Docker sin usar root",           false, true },
        { "kvm",       "Aceleración por hardware para máquinas virtuales (KVM)", false, true },
        { "libvirt",   "Administración de máquinas virtuales con libvirt/QEMU",  false, true },
        { "vboxusers", "Acceso a dispositivos USB en VirtualBox",                false, true }
    };

    for (const auto &opt : optionalSystemGroups) {
        if (getgrnam(opt.name.toUtf8().constData()) != nullptr) {
            items.append(opt);
        }
    }

    QStringList knownNames;
    for (auto &item : items) {
        item.existsOnSystem = (getgrnam(item.name.toUtf8().constData()) != nullptr);
        knownNames.append(item.name);
    }

    // Añadir cualquier otro grupo secundario que el usuario ya posea para que sea visible y editable
    for (const QString &extra : extraUserGroups) {
        const QString cleanExtra = extra.trimmed();
        if (!cleanExtra.isEmpty() && cleanExtra != "users" && !knownNames.contains(cleanExtra)) {
            const bool exists = (getgrnam(cleanExtra.toUtf8().constData()) != nullptr);
            items.append({ cleanExtra, "Grupo adicional asignado actualmente al usuario", false, exists });
            knownNames.append(cleanExtra);
        }
    }

    return items;
}

GroupUpdateResult GroupManager::updateUserGroups(const QString &username,
                                                 const QStringList &selectedSecondaryGroups)
{
    GroupUpdateResult result;
    const QString cleanUser = username.trimmed();

    LOG_INFO("GroupManager", QString("Actualizando grupos para usuario '%1'...").arg(cleanUser));

    if (!CommandRunner::isRoot()) {
        result.success = false;
        result.message = "No tienes permisos de administrador (root). Ejecuta slackwiz como root para modificar grupos.";
        LOG_ERROR("GroupManager", result.message);
        return result;
    }

    if (cleanUser.isEmpty() || cleanUser == "root") {
        result.success = false;
        result.message = "Selecciona un usuario estándar válido para modificar sus grupos.";
        LOG_ERROR("GroupManager", result.message);
        return result;
    }

    const UserGroupsInfo currentInfo = getUserGroups(cleanUser);
    if (!currentInfo.valid) {
        result.success = false;
        result.message = QString("El usuario '%1' no existe en el sistema.").arg(cleanUser);
        LOG_ERROR("GroupManager", result.message);
        return result;
    }

    QStringList validGroups;
    for (const QString &grp : selectedSecondaryGroups) {
        const QString cleanGrp = grp.trimmed();
        if (cleanGrp.isEmpty() || cleanGrp == currentInfo.primaryGroup) {
            continue;
        }
        if (getgrnam(cleanGrp.toUtf8().constData()) != nullptr) {
            if (!validGroups.contains(cleanGrp)) {
                validGroups.append(cleanGrp);
            }
        } else {
            LOG_WARN("GroupManager", QString("Omitiendo grupo inexistente en /etc/group: %1").arg(cleanGrp));
        }
    }

    validGroups.sort();

    QStringList usermodArgs;
    usermodArgs << "-G" << validGroups.join(',') << cleanUser;

    const CommandResult cmdRes = CommandRunner::execute("/usr/sbin/usermod", usermodArgs);
    if (!cmdRes.success) {
        result.success = false;
        result.message = QString("Error al actualizar grupos de '%1' con usermod (código %2):\n%3")
                             .arg(cleanUser)
                             .arg(cmdRes.exitCode)
                             .arg(cmdRes.stdErr);
        LOG_ERROR("GroupManager", result.message);
        return result;
    }

    const QString groupsSummary = validGroups.isEmpty()
                                      ? QString("%1 (sin grupos secundarios)").arg(currentInfo.primaryGroup)
                                      : QString("%1, %2").arg(currentInfo.primaryGroup, validGroups.join(", "));

    const bool hasWheel = validGroups.contains("wheel");
    const QString wheelHint = hasWheel
                                  ? "\n• El usuario pertenece a 'wheel': asegúrate de activar sudo en el paso '5. Configurar sudo'."
                                  : "";

    result.success = true;
    result.message = QString(
        "Grupos actualizados correctamente para '%1':\n"
        "• Membresía actual: %2%3\n\n"
        "Nota: Para que el usuario aplique los nuevos permisos en su sesión gráfica, debe cerrar sesión y volver a entrar."
    ).arg(cleanUser, groupsSummary, wheelHint);

    LOG_SUCCESS("GroupManager", QString("Grupos de '%1' guardados correctamente: [%2]")
                                    .arg(cleanUser, validGroups.join(", ")));
    return result;
}
