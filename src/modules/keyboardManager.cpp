#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>

#include "keyboardManager.h"
#include "commandRunner.h"
#include "logger.h"

QList<KeyboardLayoutOption> KeyboardManager::getAvailableLayouts()
{
    return {
        { "es",       "Español (España)",                         "es",         "es",    ""     },
        { "latam",    "Español (Latinoamérica)",                  "la-latin1",  "latam", ""     },
        { "us",       "Inglés (Estados Unidos)",                  "us",         "us",    ""     },
        { "us-intl",  "Inglés (EE.UU. Internacional con acentos)", "us-acentos", "us",    "intl" },
        { "gb",       "Inglés (Reino Unido)",                     "uk",         "gb",    ""     },
        { "br",       "Portugués (Brasil ABNT2)",                 "br-abnt2",   "br",    ""     },
        { "pt",       "Portugués (Portugal)",                     "pt-latin1",  "pt",    ""     },
        { "fr",       "Francés (Francia)",                        "fr-latin1",  "fr",    ""     },
        { "de",       "Alemán (Alemania)",                        "de-latin1",  "de",    ""     },
        { "it",       "Italiano (Italia)",                        "it",         "it",    ""     }
    };
}

KeyboardCurrentConfig KeyboardManager::getCurrentKeyboardConfig()
{
    KeyboardCurrentConfig current;

    // 1. Leer /etc/rc.d/rc.keymap
    const QString rcKeymapPath = "/etc/rc.d/rc.keymap";
    QFile rcFile(rcKeymapPath);
    if (rcFile.exists() && rcFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&rcFile);
        QRegularExpression loadkeysRegex(R"(loadkeys\s+([^\s#]+))");
        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith('#')) {
                continue;
            }
            QRegularExpressionMatch match = loadkeysRegex.match(line);
            if (match.hasMatch()) {
                QString mapName = match.captured(1).trimmed();
                if (mapName.endsWith(".map")) {
                    mapName.chop(4);
                } else if (mapName.endsWith(".map.gz")) {
                    mapName.chop(7);
                }
                current.consoleKeymap = mapName;
                break;
            }
        }
        rcFile.close();
    }

    // 2. Leer /etc/X11/xorg.conf.d/90-keyboard-layout.conf
    const QString xorgConfPath = "/etc/X11/xorg.conf.d/90-keyboard-layout.conf";
    QFile xorgFile(xorgConfPath);
    if (xorgFile.exists() && xorgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&xorgFile);
        QRegularExpression xkbRegex(R"(Option\s+"XkbLayout"\s+"([^"]+)");
        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith('#')) {
                continue;
            }
            QRegularExpressionMatch match = xkbRegex.match(line);
            if (match.hasMatch()) {
                current.x11Layout = match.captured(1).trimmed();
                break;
            }
        }
        xorgFile.close();
    }

    // 3. Leer setxkbmap en /etc/profile
    const QString profilePath = "/etc/profile";
    QFile profileFile(profilePath);
    if (profileFile.exists() && profileFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&profileFile);
        QRegularExpression setxkbRegex(R"(setxkbmap\s+([a-zA-Z0-9_\-]+(?:\s+-variant\s+[a-zA-Z0-9_\-]+)?))");
        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith('#')) {
                continue;
            }
            QRegularExpressionMatch match = setxkbRegex.match(line);
            if (match.hasMatch()) {
                current.profileSetxkbmap = match.captured(1).trimmed();
            }
        }
        profileFile.close();
    }

    LOG_INFO("KeyboardManager", QString("Estado actual -> rc.keymap: '%1' | X11: '%2' | /etc/profile: '%3'")
                                    .arg(current.consoleKeymap, current.x11Layout, current.profileSetxkbmap));
    return current;
}

QString KeyboardManager::getFriendlyCurrentSummary()
{
    const KeyboardCurrentConfig current = getCurrentKeyboardConfig();
    const QList<KeyboardLayoutOption> layouts = getAvailableLayouts();

    QString matchedName;
    for (const auto &opt : layouts) {
        if ((!current.x11Layout.isEmpty() && opt.x11Layout == current.x11Layout) ||
            (!current.consoleKeymap.isEmpty() && opt.consoleKeymap == current.consoleKeymap)) {
            matchedName = QString("%1 [%2]").arg(opt.displayName, opt.x11Layout);
            break;
        }
    }

    const QString ttyText = current.consoleKeymap.isEmpty() ? "No definido (us por defecto)" : current.consoleKeymap;
    const QString x11Text = current.x11Layout.isEmpty() ? "No definido" : current.x11Layout;
    const QString profText = current.profileSetxkbmap.isEmpty() ? "Sin línea setxkbmap" : QString("setxkbmap %1").arg(current.profileSetxkbmap);

    if (!matchedName.isEmpty()) {
        return QString("%1 — (Consola: %2 | X11: %3 | /etc/profile: %4)")
            .arg(matchedName, ttyText, x11Text, profText);
    }

    return QString("Personalizado / Por defecto — (Consola: %1 | X11: %2 | /etc/profile: %3)")
        .arg(ttyText, x11Text, profText);
}

KeyboardApplyResult KeyboardManager::applyKeyboardLayout(const QString &layoutId)
{
    KeyboardApplyResult result;
    LOG_INFO("KeyboardManager", QString("Solicitando configuración de teclado para ID: %1").arg(layoutId));

    const QList<KeyboardLayoutOption> layouts = getAvailableLayouts();
    KeyboardLayoutOption selected;
    bool found = false;

    for (const auto &opt : layouts) {
        if (opt.id == layoutId) {
            selected = opt;
            found = true;
            break;
        }
    }

    if (!found) {
        result.success = false;
        result.message = "La distribución de teclado seleccionada no es válida.";
        LOG_ERROR("KeyboardManager", result.message);
        return result;
    }

    if (!CommandRunner::isRoot()) {
        result.success = false;
        result.message = "No tienes permisos de administrador (root). Ejecuta slackwiz como root para guardar el teclado.";
        LOG_ERROR("KeyboardManager", result.message);
        return result;
    }

    // 1. Configurar /etc/rc.d/rc.keymap (Consola TTY)
    if (!updateRcKeymap(selected.consoleKeymap)) {
        result.success = false;
        result.message = "Error al guardar la configuración de teclado de consola en /etc/rc.d/rc.keymap.";
        return result;
    }

    // 2. Configurar /etc/X11/xorg.conf.d/90-keyboard-layout.conf (X11)
    if (!updateXorgKeyboardConf(selected.x11Layout, selected.x11Variant)) {
        result.success = false;
        result.message = "Error al guardar la configuración de teclado gráfico en /etc/X11/xorg.conf.d/90-keyboard-layout.conf.";
        return result;
    }

    // 3. Añadir/actualizar "setxkbmap <idioma>" al final de /etc/profile
    if (!updateEtcProfileSetxkbmap(selected.x11Layout, selected.x11Variant)) {
        result.success = false;
        result.message = "Error al actualizar setxkbmap en /etc/profile.";
        return result;
    }

    // 4. Aplicar inmediatamente en la sesión actual si es posible
    CommandRunner::execute("/usr/bin/loadkeys", QStringList() << selected.consoleKeymap);

    if (!qEnvironmentVariableIsEmpty("DISPLAY")) {
        QStringList xkbArgs;
        xkbArgs << selected.x11Layout;
        if (!selected.x11Variant.isEmpty()) {
            xkbArgs << "-variant" << selected.x11Variant;
        }
        CommandRunner::execute("setxkbmap", xkbArgs);
    } else {
        LOG_WARN("KeyboardManager", "Variable DISPLAY vacía; setxkbmap se aplicará al iniciar el entorno gráfico.");
    }

    const QString setxkbCommand = selected.x11Variant.isEmpty()
                                      ? QString("setxkbmap %1").arg(selected.x11Layout)
                                      : QString("setxkbmap %1 -variant %2").arg(selected.x11Layout, selected.x11Variant);

    result.success = true;
    result.message = QString(
        "Teclado '%1' guardado correctamente en:\n"
        "• Consola TTY: /etc/rc.d/rc.keymap (mapa: %2)\n"
        "• Entorno gráfico: /etc/X11/xorg.conf.d/90-keyboard-layout.conf\n"
        "• Perfil global: '%3' añadido al final de /etc/profile\n\n"
        "Puedes probar el teclado en el cuadro inferior. Todos los cambios quedarán permanentes tras reiniciar."
    ).arg(selected.displayName, selected.consoleKeymap, setxkbCommand);

    LOG_SUCCESS("KeyboardManager", QString("Teclado %1 aplicado correctamente.").arg(selected.displayName));
    return result;
}

bool KeyboardManager::updateRcKeymap(const QString &consoleKeymap)
{
    const QString rcKeymapPath = "/etc/rc.d/rc.keymap";
    const QString content = QString(
        "#!/bin/sh\n"
        "# Configuración de mapa de teclado de consola generada por Slackware Wizard (slackwiz)\n"
        "if [ -x /usr/bin/loadkeys ]; then\n"
        "  /usr/bin/loadkeys %1.map\n"
        "fi\n"
    ).arg(consoleKeymap);

    if (!CommandRunner::writeFileWithBackup(rcKeymapPath, content, true)) {
        return false;
    }

    return CommandRunner::setExecutablePermission(rcKeymapPath, true);
}

bool KeyboardManager::updateXorgKeyboardConf(const QString &x11Layout, const QString &x11Variant)
{
    const QString xorgDir = "/etc/X11/xorg.conf.d";
    QDir dir;
    if (!dir.exists(xorgDir)) {
        LOG_INFO("KeyboardManager", QString("Creando directorio %1").arg(xorgDir));
        if (!dir.mkpath(xorgDir)) {
            LOG_ERROR("KeyboardManager", QString("No se pudo crear el directorio %1").arg(xorgDir));
            return false;
        }
    }

    QString variantLine;
    if (!x11Variant.isEmpty()) {
        variantLine = QString("        Option \"XkbVariant\" \"%1\"\n").arg(x11Variant);
    }

    const QString confPath = xorgDir + "/90-keyboard-layout.conf";
    const QString content = QString(
        "# Configuración de teclado X11 generada por Slackware Wizard (slackwiz)\n"
        "Section \"InputClass\"\n"
        "        Identifier \"keyboard-all\"\n"
        "        MatchIsKeyboard \"on\"\n"
        "        Driver \"libinput\"\n"
        "        Option \"XkbLayout\" \"%1\"\n"
        "%2"
        "EndSection\n"
    ).arg(x11Layout, variantLine);

    return CommandRunner::writeFileWithBackup(confPath, content, true);
}

bool KeyboardManager::updateEtcProfileSetxkbmap(const QString &x11Layout, const QString &x11Variant)
{
    const QString profilePath = "/etc/profile";
    QFile file(profilePath);
    if (!file.exists()) {
        LOG_ERROR("KeyboardManager", "/etc/profile no existe en el sistema.");
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR("KeyboardManager", QString("No se pudo abrir %1 para lectura.").arg(profilePath));
        return false;
    }

    QStringList cleanedLines;
    QTextStream in(&file);
    bool insideManagedBlock = false;

    while (!in.atEnd()) {
        const QString line = in.readLine();
        const QString trimmed = line.trimmed();

        if (trimmed == "# slackwiz-keyboard-start") {
            insideManagedBlock = true;
            continue;
        }
        if (trimmed == "# slackwiz-keyboard-end") {
            insideManagedBlock = false;
            continue;
        }
        if (insideManagedBlock) {
            continue;
        }

        // Evitar duplicados si existía una línea setxkbmap previa añadida manualmente
        if (trimmed.startsWith("setxkbmap ")) {
            LOG_INFO("KeyboardManager", QString("Reemplazando línea previa en /etc/profile: %1").arg(trimmed));
            continue;
        }

        cleanedLines.append(line);
    }
    file.close();

    // Eliminar líneas en blanco sobrantes al final antes de agregar el bloque
    while (!cleanedLines.isEmpty() && cleanedLines.last().trimmed().isEmpty()) {
        cleanedLines.removeLast();
    }

    const QString setxkbCmd = x11Variant.isEmpty()
                                  ? QString("setxkbmap %1").arg(x11Layout)
                                  : QString("setxkbmap %1 -variant %2").arg(x11Layout, x11Variant);

    cleanedLines.append("");
    cleanedLines.append("# slackwiz-keyboard-start");
    cleanedLines.append("# Configuración de teclado añadida por Slackware Wizard (slackwiz)");
    cleanedLines.append(QString("[ -n \"$DISPLAY\" ] && %1").arg(setxkbCmd));
    cleanedLines.append("# slackwiz-keyboard-end");
    cleanedLines.append("");

    const QString finalContent = cleanedLines.join('\n');
    return CommandRunner::writeFileWithBackup(profilePath, finalContent, true);
}
