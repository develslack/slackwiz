#include "DeviceManager.h"
#include "logger.h"
#include "commandRunner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>
#include <QtGlobal>

namespace {

QString resolveCommandPath(const QString &program)
{
    if (program.contains('/')) {
        return program;
    }
    const QStringList searchDirs = { "/sbin", "/usr/sbin", "/usr/local/sbin", "/usr/bin", "/bin" };
    for (const QString &dir : searchDirs) {
        const QString candidate = dir + "/" + program;
        if (QFileInfo(candidate).isExecutable()) {
            return candidate;
        }
    }
    return program;
}

CommandResult runLocalCommand(const QString &program, const QStringList &arguments, int timeoutMs = 3000)
{
    CommandResult result;
    QProcess process;
    process.start(resolveCommandPath(program), arguments);
    if (!process.waitForStarted(2000)) {
        result.exitCode = -1;
        result.stdErr = "Failed to start process";
        result.success = false;
        return result;
    }
    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished(1000);
        result.exitCode = -1;
        result.stdErr = "Process timed out";
        result.success = false;
        return result;
    }
    result.exitCode = process.exitCode();
    result.stdOut = QString::fromUtf8(process.readAllStandardOutput());
    result.stdErr = QString::fromUtf8(process.readAllStandardError());
    result.success = (process.exitStatus() == QProcess::NormalExit && result.exitCode == 0);
    return result;
}

bool executePrivilegedScript(const QString &script, const QString &rootPassword, QString &outError)
{
    if (CommandRunner::isRoot()) {
        const CommandResult res = CommandRunner::executeShell(script, 8000);
        if (!res.success) {
            outError = res.stdErr.isEmpty() ? res.stdOut : res.stdErr;
            return false;
        }
        return true;
    }

    if (rootPassword.isEmpty()) {
        outError = "Se requiere contraseña de administrador (root) para aplicar cambios en el sistema.";
        return false;
    }

    QProcess suProc;
    suProc.start("su", { "-c", script });
    if (!suProc.waitForStarted(3000)) {
        outError = "No se pudo iniciar 'su' para elevar privilegios.";
        return false;
    }

    suProc.write((rootPassword + "\n").toUtf8());
    suProc.closeWriteChannel();

    if (!suProc.waitForFinished(8000)) {
        suProc.kill();
        outError = "Tiempo de espera agotado al autenticar como root.";
        return false;
    }

    if (suProc.exitCode() != 0) {
        const QString err = QString::fromUtf8(suProc.readAllStandardError()).trimmed();
        outError = err.isEmpty() ? "Contraseña de root incorrecta o error al ejecutar la orden." : err;
        return false;
    }

    return true;
}

bool writePrivilegedConfigFile(const QString &filePath,
                               const QString &content,
                               const QString &rootPassword,
                               QString &outError)
{
    if (CommandRunner::isRoot()) {
        const QFileInfo fi(filePath);
        QDir dir;
        if (!dir.mkpath(fi.absolutePath())) {
            outError = QString("No se pudo crear el directorio %1").arg(fi.absolutePath());
            return false;
        }
        if (!CommandRunner::writeFileWithBackup(filePath, content, true)) {
            outError = QString("No se pudo escribir el archivo %1").arg(filePath);
            return false;
        }
        return true;
    }

    QString escaped = content;
    escaped.replace("'", "'\"'\"'");

    const QFileInfo fi(filePath);
    const QString cmd = QString(
        "mkdir -p '%1' && "
        "if [ -f '%2' ]; then cp -p '%2' '%2.bak.$(date +%Y%m%d_%H%M%S)'; fi && "
        "printf '%s' '%3' > '%2'"
    ).arg(fi.absolutePath(), filePath, escaped);

    return executePrivilegedScript(cmd, rootPassword, outError);
}

FormFactor detectFormFactor()
{
    QDir powerDir("/sys/class/power_supply");
    if (powerDir.exists()) {
        const QStringList batteries = powerDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &bat : batteries) {
            if (bat.toLower().contains("bat") || QFile::exists(powerDir.filePath(bat + "/capacity"))) {
                return FormFactor::Notebook;
            }
        }
    }

    QFile cpuInfo("/proc/cpuinfo");
    int cpuCount = 0;
    if (cpuInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&cpuInfo);
        while (!in.atEnd()) {
            if (in.readLine().startsWith("processor")) {
                ++cpuCount;
            }
        }
    }

    if (cpuCount >= 16 && (QFile::exists("/etc/rc.d/rc.docker") || QFile::exists("/etc/rc.d/rc.mysqld"))) {
        return FormFactor::PowerWorkstation;
    }

    return FormFactor::Workstation;
}

QString readTextFileSafe(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromUtf8(f.readAll());
}

QVariantMap loadMouseCurrentSettings()
{
    QVariantMap map;
    map["leftHanded"] = false;
    map["accelSpeed"] = 0.0;
    map["accelProfile"] = "adaptive";
    map["naturalScrolling"] = false;
    map["applyLiveX11"] = true;

    const QString content = readTextFileSafe("/etc/X11/xorg.conf.d/40-libinput.conf");
    if (!content.isEmpty()) {
        if (content.contains(QRegularExpression(R"rx(Option\s+"LeftHanded"\s+"(true|1|on)")rx", QRegularExpression::CaseInsensitiveOption))) {
            map["leftHanded"] = true;
        }
        const QRegularExpression speedRe(R"rx(Option\s+"AccelSpeed"\s+"([-+]?[0-9]*\.?[0-9]+)")rx", QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch m = speedRe.match(content);
        if (m.hasMatch()) {
            map["accelSpeed"] = qBound(-0.85, m.captured(1).toDouble(), 0.85);
        }
        if (content.contains(QRegularExpression(R"(Option\s+"AccelProfile"\s+"flat")", QRegularExpression::CaseInsensitiveOption))) {
            map["accelProfile"] = "flat";
        }
        if (content.contains(QRegularExpression(R"rx(Option\s+"NaturalScrolling"\s+"(true|1|on)")rx", QRegularExpression::CaseInsensitiveOption))) {
            map["naturalScrolling"] = true;
        }
    }
    return map;
}

QVariantMap loadTouchpadCurrentSettings()
{
    QVariantMap map;
    map["tapping"] = true;
    map["scrollMethod"] = "twofinger";
    map["naturalScrolling"] = true;
    map["disableWhileTyping"] = true;
    map["accelSpeed"] = 0.15;
    map["applyLiveX11"] = true;

    const QString content = readTextFileSafe("/etc/X11/xorg.conf.d/41-libinput-touchpad.conf");
    if (!content.isEmpty()) {
        if (content.contains(QRegularExpression(R"rx(Option\s+"Tapping"\s+"(false|0|off)")rx", QRegularExpression::CaseInsensitiveOption))) {
            map["tapping"] = false;
        }
        if (content.contains(QRegularExpression(R"(Option\s+"ScrollMethod"\s+"edge")", QRegularExpression::CaseInsensitiveOption))) {
            map["scrollMethod"] = "edge";
        }
        if (content.contains(QRegularExpression(R"rx(Option\s+"NaturalScrolling"\s+"(false|0|off)")rx", QRegularExpression::CaseInsensitiveOption))) {
            map["naturalScrolling"] = false;
        }
        if (content.contains(QRegularExpression(R"rx(Option\s+"DisableWhileTyping"\s+"(false|0|off)")rx", QRegularExpression::CaseInsensitiveOption))) {
            map["disableWhileTyping"] = false;
        }
        const QRegularExpression speedRe(R"rx(Option\s+"AccelSpeed"\s+"([-+]?[0-9]*\.?[0-9]+)")rx", QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch m = speedRe.match(content);
        if (m.hasMatch()) {
            map["accelSpeed"] = qBound(-0.85, m.captured(1).toDouble(), 0.85);
        }
    }
    return map;
}

QVariantMap loadKeyboardDeviceSettings()
{
    QVariantMap map;
    map["repeatDelay"] = 350;
    map["repeatRate"] = 30;
    map["numLockOn"] = true;
    map["applyLiveX11"] = true;

    const QString content = readTextFileSafe("/etc/X11/xorg.conf.d/90-slackwiz-keyboard.conf");
    const QRegularExpression autoRepRe(R"rx(Option\s+"AutoRepeat"\s+"(\d+)\s+(\d+)")rx", QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch m = autoRepRe.match(content);
    if (m.hasMatch()) {
        map["repeatDelay"] = qBound(150, m.captured(1).toInt(), 900);
        map["repeatRate"] = qBound(10, m.captured(2).toInt(), 80);
    }
    return map;
}

QVariantMap loadVideoCurrentSettings()
{
    QVariantMap map;
    map["outputName"] = "Automática";
    map["resolution"] = "Automática (Nativa)";
    map["refreshRate"] = "Automática";
    map["rotation"] = "normal";
    map["brightness"] = 100;
    map["tearFree"] = true;
    map["applyLiveX11"] = true;

    QStringList outputs;
    outputs << "Automática";
    QStringList resolutions;
    resolutions << "Automática (Nativa)" << "1920x1080" << "1680x1050" << "1600x900" << "1440x900" << "1366x768" << "1280x720" << "1024x768";

    const CommandResult xr = runLocalCommand("xrandr", { "--current" }, 2000);
    if (xr.success) {
        const QStringList lines = xr.stdOut.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            if (line.contains(" connected")) {
                const QString outName = line.section(' ', 0, 0).trimmed();
                if (!outName.isEmpty() && !outputs.contains(outName)) {
                    outputs << outName;
                    if (map["outputName"].toString() == "Automática") {
                        map["outputName"] = outName;
                    }
                }
            } else if (line.startsWith("   ")) {
                const QString modeStr = line.trimmed().section(' ', 0, 0);
                if (modeStr.contains('x') && !resolutions.contains(modeStr)) {
                    resolutions.insert(1, modeStr);
                }
            }
        }
    }

    map["availableOutputs"] = outputs;
    map["availableResolutions"] = resolutions;

    const QString conf = readTextFileSafe("/etc/X11/xorg.conf.d/10-monitor.conf");
    if (!conf.isEmpty()) {
        const QRegularExpression modeRe(R"rx(Option\s+"PreferredMode"\s+"([^"]+)")rx");
        const QRegularExpressionMatch mMode = modeRe.match(conf);
        if (mMode.hasMatch()) {
            map["resolution"] = mMode.captured(1);
        }
        const QRegularExpression rotRe(R"rx(Option\s+"Rotate"\s+"([^"]+)")rx");
        const QRegularExpressionMatch mRot = rotRe.match(conf);
        if (mRot.hasMatch()) {
            map["rotation"] = mRot.captured(1);
        }
        if (conf.contains(QRegularExpression(R"(Option\s+"TearFree"\s+"false")", QRegularExpression::CaseInsensitiveOption))) {
            map["tearFree"] = false;
        }
    }

    return map;
}

QVariantMap loadAudioCurrentSettings(const QStringList &cardsList)
{
    QVariantMap map;
    map["cards"] = cardsList;
    map["cardIndex"] = 0;
    map["volume"] = 80;
    map["muted"] = false;
    map["channelProfile"] = "stereo";
    map["backend"] = "pulse_pipewire";

    const CommandResult amix = runLocalCommand("amixer", { "get", "Master" }, 2000);
    if (amix.success) {
        const QRegularExpression volRe(R"(\[(\d+)%\])");
        const QRegularExpressionMatch m = volRe.match(amix.stdOut);
        if (m.hasMatch()) {
            map["volume"] = qBound(0, m.captured(1).toInt(), 100);
        }
        if (amix.stdOut.contains("[off]")) {
            map["muted"] = true;
        }
    }

    const QString asound = readTextFileSafe("/etc/asound.conf");
    if (!asound.isEmpty()) {
        const QRegularExpression cardRe(R"(defaults\.pcm\.card\s+(\d+))");
        const QRegularExpressionMatch mCard = cardRe.match(asound);
        if (mCard.hasMatch()) {
            map["cardIndex"] = mCard.captured(1).toInt();
        }
        if (asound.contains("channels 6")) {
            map["channelProfile"] = "surround51";
        }
    }

    return map;
}

QVariantMap loadBluetoothCurrentSettings()
{
    QVariantMap map;
    map["adapterName"] = "Slackware-PC";
    map["autoEnable"] = true;
    map["discoverable"] = false;
    map["enableServiceBoot"] = QFileInfo("/etc/rc.d/rc.bluetooth").isExecutable();

    const QString mainConf = readTextFileSafe("/etc/bluetooth/main.conf");
    if (!mainConf.isEmpty()) {
        const QRegularExpression nameRe(R"(^\s*Name\s*=\s*(.+)$)", QRegularExpression::MultilineOption);
        const QRegularExpressionMatch mName = nameRe.match(mainConf);
        if (mName.hasMatch()) {
            map["adapterName"] = mName.captured(1).trimmed();
        }
        if (mainConf.contains(QRegularExpression(R"(^\s*AutoEnable\s*=\s*false)", QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption))) {
            map["autoEnable"] = false;
        }
        const QRegularExpression discRe(R"(^\s*DiscoverableTimeout\s*=\s*(\d+))", QRegularExpression::MultilineOption);
        const QRegularExpressionMatch mDisc = discRe.match(mainConf);
        if (mDisc.hasMatch() && mDisc.captured(1).toInt() == 0) {
            map["discoverable"] = true;
        }
    }
    return map;
}

QString prefixToNetmask(int prefix)
{
    if (prefix <= 0) return "0.0.0.0";
    if (prefix >= 32) return "255.255.255.255";
    const quint32 mask = (~quint32(0)) << (32 - prefix);
    return QString("%1.%2.%3.%4")
        .arg((mask >> 24) & 0xFF)
        .arg((mask >> 16) & 0xFF)
        .arg((mask >> 8) & 0xFF)
        .arg(mask & 0xFF);
}

QVariantMap loadNetworkCurrentSettings(const QString &ifaceName)
{
    QVariantMap map;
    map["interface"] = ifaceName;
    map["mode"] = "dhcp";
    map["ipAddress"] = "";
    map["netmask"] = "";
    map["gateway"] = "";
    map["dns"] = "";
    map["startOnBoot"] = true;

    // 1. Detectar gestores de red instalados de serie en Slackware y cuál está activo
    QStringList availableManagers;
    availableManagers << "rc.inet1"; // Siempre disponible en Slackware

    const bool hasNM = QFile::exists("/etc/rc.d/rc.networkmanager") ||
                       QFile::exists("/usr/sbin/NetworkManager");
    if (hasNM) {
        availableManagers << "networkmanager";
    }

    const bool hasWicd = QFile::exists("/etc/rc.d/rc.wicd") ||
                         QFile::exists("/usr/sbin/wicd");
    if (hasWicd) {
        availableManagers << "wicd";
    }

    QString activeManager = "rc.inet1";
    const bool nmRunning = hasNM && runLocalCommand("pgrep", { "-x", "NetworkManager" }, 1500).success;
    const bool wicdRunning = hasWicd && runLocalCommand("pgrep", { "-x", "wicd" }, 1500).success;

    if (nmRunning) {
        activeManager = "networkmanager";
    } else if (wicdRunning) {
        activeManager = "wicd";
    } else if (hasNM && QFileInfo("/etc/rc.d/rc.networkmanager").isExecutable()) {
        activeManager = "networkmanager";
    }

    map["availableManagers"] = availableManagers;
    map["networkManager"] = activeManager;

    // 2. Leer IP real y máscara real directamente del kernel (ip -4 addr)
    const CommandResult ipRes = runLocalCommand("ip", { "-4", "-o", "addr", "show", "dev", ifaceName }, 2000);
    if (ipRes.success) {
        const QRegularExpression ipCidrRe(R"rx(inet\s+(\d+\.\d+\.\d+\.\d+)/(\d+))rx");
        const QRegularExpressionMatch mIp = ipCidrRe.match(ipRes.stdOut);
        if (mIp.hasMatch()) {
            map["ipAddress"] = mIp.captured(1);
            map["netmask"] = prefixToNetmask(mIp.captured(2).toInt());
        }
    }

    // 3. Leer Gateway real (ip -4 route)
    CommandResult gwRes = runLocalCommand("ip", { "-4", "route", "show", "default", "dev", ifaceName }, 2000);
    if (!gwRes.success || gwRes.stdOut.trimmed().isEmpty()) {
        gwRes = runLocalCommand("ip", { "-4", "route", "show", "default" }, 2000);
    }
    if (gwRes.success) {
        const QRegularExpression gwRe(R"rx(default\s+via\s+(\d+\.\d+\.\d+\.\d+))rx");
        const QRegularExpressionMatch mGw = gwRe.match(gwRes.stdOut);
        if (mGw.hasMatch()) {
            map["gateway"] = mGw.captured(1);
        }
    }

    // 4. Leer DNS reales desde /etc/resolv.conf
    const QString resolv = readTextFileSafe("/etc/resolv.conf");
    if (!resolv.isEmpty()) {
        QStringList dnsList;
        const QRegularExpression dnsRe(R"rx(^\s*nameserver\s+(\S+))rx", QRegularExpression::MultilineOption);
        QRegularExpressionMatchIterator it = dnsRe.globalMatch(resolv);
        while (it.hasNext()) {
            const QString ns = it.next().captured(1).trimmed();
            if (!ns.isEmpty() && !dnsList.contains(ns)) {
                dnsList << ns;
            }
        }
        map["dns"] = dnsList.join(", ");
    }

    // 5. Determinar si el modo real actual es DHCP o IP estática (Manual)
    bool modeResolved = false;

    // 5a. Si NetworkManager está corriendo, preguntar a nmcli
    if (nmRunning) {
        const CommandResult devShow = runLocalCommand("nmcli", { "-t", "-f", "GENERAL.CONNECTION", "device", "show", ifaceName }, 2000);
        if (devShow.success) {
            const QString connName = devShow.stdOut.section(':', 1).trimmed();
            if (!connName.isEmpty() && connName != "--") {
                const CommandResult methRes = runLocalCommand("nmcli", { "-t", "-f", "ipv4.method", "connection", "show", connName }, 2000);
                if (methRes.success) {
                    if (methRes.stdOut.contains("manual", Qt::CaseInsensitive)) {
                        map["mode"] = "static";
                        modeResolved = true;
                    } else if (methRes.stdOut.contains("auto", Qt::CaseInsensitive)) {
                        map["mode"] = "dhcp";
                        modeResolved = true;
                    }
                }
            }
        }
    }

    // 5b. Revisar /etc/rc.d/rc.inet1.conf (ignorando líneas comentadas con #)
    if (!modeResolved) {
        const QString inet1 = readTextFileSafe("/etc/rc.d/rc.inet1.conf");
        if (!inet1.isEmpty()) {
            const QRegularExpression dhcpYesRe(R"rx(^\s*USE_DHCP\[\d+\]\s*=\s*"yes")rx", QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption);
            const QRegularExpression ipConfRe(R"rx(^\s*IPADDR\[\d+\]\s*=\s*"(\d+\.\d+\.\d+\.\d+)")rx", QRegularExpression::MultilineOption);
            const QRegularExpression maskConfRe(R"rx(^\s*NETMASK\[\d+\]\s*=\s*"(\d+\.\d+\.\d+\.\d+)")rx", QRegularExpression::MultilineOption);
            const QRegularExpression gwConfRe(R"rx(^\s*GATEWAY\s*=\s*"(\d+\.\d+\.\d+\.\d+)")rx", QRegularExpression::MultilineOption);

            const QRegularExpressionMatch mIpConf = ipConfRe.match(inet1);
            if (mIpConf.hasMatch()) {
                map["mode"] = "static";
                modeResolved = true;
                if (map["ipAddress"].toString().isEmpty()) map["ipAddress"] = mIpConf.captured(1);
            } else if (dhcpYesRe.match(inet1).hasMatch()) {
                map["mode"] = "dhcp";
                modeResolved = true;
            }

            const QRegularExpressionMatch mMaskConf = maskConfRe.match(inet1);
            if (mMaskConf.hasMatch() && map["netmask"].toString().isEmpty()) {
                map["netmask"] = mMaskConf.captured(1);
            }
            const QRegularExpressionMatch mGwConf = gwConfRe.match(inet1);
            if (mGwConf.hasMatch() && map["gateway"].toString().isEmpty()) {
                map["gateway"] = mGwConf.captured(1);
            }
        }
    }

    // 5c. Respaldo por proceso DHCP en ejecución si aún no se determinó
    if (!modeResolved && !map["ipAddress"].toString().isEmpty()) {
        const bool dhcpRunning = runLocalCommand("pgrep", { "-x", "dhcpcd" }, 1000).success ||
                                 runLocalCommand("pgrep", { "-x", "dhclient" }, 1000).success;
        map["mode"] = dhcpRunning ? "dhcp" : "static";
    }

    return map;
}

QVariantMap loadStorageCurrentSettings(bool isSsd)
{
    QVariantMap map;
    map["isSsd"] = isSsd;
    map["scheduler"] = isSsd ? "mq-deadline" : "bfq";
    map["enableTrim"] = isSsd;
    return map;
}

// Generadores de archivos de configuración reales
QString buildMouseConfigText(const QVariantMap &s)
{
    const bool leftHanded = s.value("leftHanded", false).toBool();
    const double accelSpeed = qBound(-0.85, s.value("accelSpeed", 0.0).toDouble(), 0.85);
    const QString accelProfile = (s.value("accelProfile", "adaptive").toString() == "flat") ? "flat" : "adaptive";
    const bool naturalScroll = s.value("naturalScrolling", false).toBool();

    return QString(
        "# Generado por Slackware Wizard (slackwiz) - Configuración Visual del Ratón\n"
        "Section \"InputClass\"\n"
        "    Identifier \"slackwiz-mouse-config\"\n"
        "    MatchIsPointer \"on\"\n"
        "    Driver \"libinput\"\n"
        "    Option \"LeftHanded\" \"%1\"\n"
        "    Option \"AccelSpeed\" \"%2\"\n"
        "    Option \"AccelProfile\" \"%3\"\n"
        "    Option \"NaturalScrolling\" \"%4\"\n"
        "EndSection\n"
    ).arg(leftHanded ? "true" : "false",
          QString::number(accelSpeed, 'f', 2),
          accelProfile,
          naturalScroll ? "true" : "false");
}

QString buildTouchpadConfigText(const QVariantMap &s)
{
    const bool tapping = s.value("tapping", true).toBool();
    const QString scrollMethod = (s.value("scrollMethod", "twofinger").toString() == "edge") ? "edge" : "twofinger";
    const bool naturalScroll = s.value("naturalScrolling", true).toBool();
    const bool disableTyping = s.value("disableWhileTyping", true).toBool();
    const double accelSpeed = qBound(-0.85, s.value("accelSpeed", 0.15).toDouble(), 0.85);

    return QString(
        "# Generado por Slackware Wizard (slackwiz) - Configuración Visual del Touchpad\n"
        "Section \"InputClass\"\n"
        "    Identifier \"slackwiz-touchpad-config\"\n"
        "    MatchIsTouchpad \"on\"\n"
        "    Driver \"libinput\"\n"
        "    Option \"Tapping\" \"%1\"\n"
        "    Option \"ScrollMethod\" \"%2\"\n"
        "    Option \"NaturalScrolling\" \"%3\"\n"
        "    Option \"DisableWhileTyping\" \"%4\"\n"
        "    Option \"AccelSpeed\" \"%5\"\n"
        "EndSection\n"
    ).arg(tapping ? "true" : "false",
          scrollMethod,
          naturalScroll ? "true" : "false",
          disableTyping ? "true" : "false",
          QString::number(accelSpeed, 'f', 2));
}

QString buildKeyboardDeviceConfigText(const QVariantMap &s)
{
    const int delay = qBound(150, s.value("repeatDelay", 350).toInt(), 900);
    const int rate = qBound(10, s.value("repeatRate", 30).toInt(), 80);

    return QString(
        "# Generado por Slackware Wizard (slackwiz) - Velocidad de Repetición de Teclado\n"
        "Section \"InputClass\"\n"
        "    Identifier \"slackwiz-keyboard-repeat\"\n"
        "    MatchIsKeyboard \"on\"\n"
        "    Option \"AutoRepeat\" \"%1 %2\"\n"
        "EndSection\n"
    ).arg(delay).arg(rate);
}

QString buildVideoConfigText(const QVariantMap &s)
{
    const QString outputName = s.value("outputName", "Automática").toString();
    const QString resolution = s.value("resolution", "Automática (Nativa)").toString();
    const QString rotation = s.value("rotation", "normal").toString();
    const bool tearFree = s.value("tearFree", true).toBool();

    QString monitorLines;
    if (!resolution.startsWith("Automática")) {
        monitorLines += QString("    Option \"PreferredMode\" \"%1\"\n").arg(resolution);
    }
    if (rotation != "normal") {
        monitorLines += QString("    Option \"Rotate\" \"%1\"\n").arg(rotation);
    }

    const QString identifier = (outputName.startsWith("Automática") || outputName.isEmpty())
                                   ? "Monitor0"
                                   : outputName;

    QString text = QString(
        "# Generado por Slackware Wizard (slackwiz) - Configuración de Pantalla y Video\n"
        "Section \"Monitor\"\n"
        "    Identifier \"%1\"\n"
        "%2"
        "EndSection\n\n"
        "Section \"Device\"\n"
        "    Identifier \"Card0\"\n"
        "    Option \"TearFree\" \"%3\"\n"
        "EndSection\n"
    ).arg(identifier, monitorLines, tearFree ? "true" : "false");

    return text;
}

QString buildAudioConfigText(const QVariantMap &s)
{
    const int cardIdx = qMax(0, s.value("cardIndex", 0).toInt());
    const QString profile = s.value("channelProfile", "stereo").toString();
    const int channels = (profile == "surround51") ? 6 : 2;

    return QString(
        "# Generado por Slackware Wizard (slackwiz) - Configuración de Audio ALSA\n"
        "defaults.pcm.card %1\n"
        "defaults.ctl.card %1\n\n"
        "pcm.!default {\n"
        "    type plug\n"
        "    slave {\n"
        "        pcm \"hw:%1,0\"\n"
        "        channels %2\n"
        "    }\n"
        "}\n\n"
        "ctl.!default {\n"
        "    type hw\n"
        "    card %1\n"
        "}\n"
    ).arg(cardIdx).arg(channels);
}

QString buildBluetoothConfigText(const QVariantMap &s)
{
    const QString name = s.value("adapterName", "Slackware-PC").toString().trimmed();
    const bool autoEnable = s.value("autoEnable", true).toBool();
    const bool discoverable = s.value("discoverable", false).toBool();

    return QString(
        "# Generado por Slackware Wizard (slackwiz) - Configuración Bluetooth (BlueZ)\n"
        "[General]\n"
        "Name = %1\n"
        "DiscoverableTimeout = %2\n"
        "PairableTimeout = 0\n\n"
        "[Policy]\n"
        "AutoEnable = %3\n"
    ).arg(name.isEmpty() ? "Slackware-PC" : name,
          discoverable ? "0" : "180",
          autoEnable ? "true" : "false");
}

QString buildNetworkConfigText(const QVariantMap &s)
{
    const QString iface = s.value("interface", "eth0").toString();
    const QString manager = s.value("networkManager", "rc.inet1").toString();
    const QString mode = s.value("mode", "dhcp").toString();
    const QString ip = s.value("ipAddress").toString().trimmed();
    const QString mask = s.value("netmask").toString().trimmed();
    const QString gw = s.value("gateway").toString().trimmed();
    const QString dns = s.value("dns").toString().trimmed();

    if (manager == "networkmanager") {
        return QString(
            "# Gestor seleccionado: NetworkManager (/etc/rc.d/rc.networkmanager)\n"
            "# Interfaz: %1 | Modo: %2\n"
            "# IP: %3 | Máscara: %4 | Gateway: %5 | DNS: %6\n"
            "# Se habilitará /etc/rc.d/rc.networkmanager (+x) y se aplicará con nmcli.\n"
        ).arg(iface, mode == "static" ? "IP Estática" : "DHCP", ip, mask, gw, dns);
    }

    if (manager == "wicd") {
        return QString(
            "# Gestor seleccionado: Wicd (/etc/rc.d/rc.wicd)\n"
            "# Se habilitará /etc/rc.d/rc.wicd (+x) para gestionar la interfaz %1.\n"
        ).arg(iface);
    }

    if (mode == "static") {
        return QString(
            "# Generado por Slackware Wizard (slackwiz) - Configuración de Red (%1)\n"
            "IFNAME[0]=\"%1\"\n"
            "IPADDR[0]=\"%2\"\n"
            "NETMASK[0]=\"%3\"\n"
            "USE_DHCP[0]=\"no\"\n"
            "GATEWAY=\"%4\"\n"
        ).arg(iface, ip, mask, gw);
    }

    return QString(
        "# Generado por Slackware Wizard (slackwiz) - Configuración de Red (%1)\n"
        "IFNAME[0]=\"%1\"\n"
        "IPADDR[0]=\"\"\n"
        "NETMASK[0]=\"\"\n"
        "USE_DHCP[0]=\"yes\"\n"
        "DHCP_HOSTNAME[0]=\"slackware\"\n"
    ).arg(iface);
}

QString buildStorageConfigText(const QVariantMap &s)
{
    const QString scheduler = s.value("scheduler", "mq-deadline").toString();
    return QString(
        "# Generado por Slackware Wizard (slackwiz) - Optimización de Discos\n"
        "ACTION==\"add|change\", KERNEL==\"sd[a-z]|nvme[0-9]n[0-9]\", ATTR{queue/scheduler}=\"%1\"\n"
    ).arg(scheduler);
}

void applyLiveMouseXinput(const QVariantMap &s)
{
    const CommandResult listRes = runLocalCommand("xinput", { "list", "--short" }, 2000);
    if (!listRes.success) {
        return;
    }

    const QString leftHandedVal = s.value("leftHanded", false).toBool() ? "1" : "0";
    const QString speedVal = QString::number(qBound(-0.85, s.value("accelSpeed", 0.0).toDouble(), 0.85), 'f', 2);
    const bool isFlat = (s.value("accelProfile", "adaptive").toString() == "flat");
    const QString natScrollVal = s.value("naturalScrolling", false).toBool() ? "1" : "0";

    const QRegularExpression idRe(R"(id=(\d+))");
    const QStringList lines = listRes.stdOut.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (!line.contains("slave  pointer", Qt::CaseInsensitive)) {
            continue;
        }
        if (line.contains("Virtual core", Qt::CaseInsensitive) ||
            line.contains("XTEST", Qt::CaseInsensitive) ||
            line.contains("Touchpad", Qt::CaseInsensitive)) {
            continue;
        }
        const QRegularExpressionMatch m = idRe.match(line);
        if (m.hasMatch()) {
            const QString id = m.captured(1);
            runLocalCommand("xinput", { "set-prop", id, "libinput Left Handed Enabled", leftHandedVal }, 1000);
            runLocalCommand("xinput", { "set-prop", id, "libinput Accel Speed", speedVal }, 1000);
            runLocalCommand("xinput", { "set-prop", id, "libinput Accel Profile Enabled", isFlat ? "0" : "1", isFlat ? "1" : "0" }, 1000);
            runLocalCommand("xinput", { "set-prop", id, "libinput Natural Scrolling Enabled", natScrollVal }, 1000);
        }
    }
}

void applyLiveTouchpadXinput(const QVariantMap &s)
{
    const CommandResult listRes = runLocalCommand("xinput", { "list", "--short" }, 2000);
    if (!listRes.success) {
        return;
    }

    const QString tapVal = s.value("tapping", true).toBool() ? "1" : "0";
    const bool isEdge = (s.value("scrollMethod", "twofinger").toString() == "edge");
    const QString natVal = s.value("naturalScrolling", true).toBool() ? "1" : "0";
    const QString dwtVal = s.value("disableWhileTyping", true).toBool() ? "1" : "0";
    const QString speedVal = QString::number(qBound(-0.85, s.value("accelSpeed", 0.15).toDouble(), 0.85), 'f', 2);

    const QRegularExpression idRe(R"(id=(\d+))");
    const QStringList lines = listRes.stdOut.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (!line.contains("Touchpad", Qt::CaseInsensitive) &&
            !line.contains("Synaptics", Qt::CaseInsensitive) &&
            !line.contains("Elan", Qt::CaseInsensitive)) {
            continue;
        }
        const QRegularExpressionMatch m = idRe.match(line);
        if (m.hasMatch()) {
            const QString id = m.captured(1);
            runLocalCommand("xinput", { "set-prop", id, "libinput Tapping Enabled", tapVal }, 1000);
            runLocalCommand("xinput", { "set-prop", id, "libinput Scroll Method Enabled", isEdge ? "0" : "1", isEdge ? "1" : "0", "0" }, 1000);
            runLocalCommand("xinput", { "set-prop", id, "libinput Natural Scrolling Enabled", natVal }, 1000);
            runLocalCommand("xinput", { "set-prop", id, "libinput Disable While Typing Enabled", dwtVal }, 1000);
            runLocalCommand("xinput", { "set-prop", id, "libinput Accel Speed", speedVal }, 1000);
        }
    }
}

void applyLiveVideoXrandr(const QVariantMap &s)
{
    const QString outName = s.value("outputName", "Automática").toString();
    if (outName.isEmpty() || outName.startsWith("Automática")) {
        return;
    }

    QStringList args;
    args << "--output" << outName;

    const QString res = s.value("resolution", "Automática (Nativa)").toString();
    if (res.startsWith("Automática")) {
        args << "--auto";
    } else {
        args << "--mode" << res;
    }

    const QString hz = s.value("refreshRate", "Automática").toString();
    if (!hz.startsWith("Automática")) {
        QString numericHz = hz;
        numericHz.remove("Hz", Qt::CaseInsensitive);
        args << "--rate" << numericHz.trimmed();
    }

    const QString rot = s.value("rotation", "normal").toString();
    args << "--rotate" << rot;

    const int bright = qBound(40, s.value("brightness", 100).toInt(), 100);
    args << "--brightness" << QString::number(bright / 100.0, 'f', 2);

    runLocalCommand("xrandr", args, 2500);
}

} // namespace

SystemHardwareOverview DeviceManager::detectSystemHardware()
{
    SystemHardwareOverview overview;
    overview.formFactor = detectFormFactor();

    switch (overview.formFactor) {
        case FormFactor::Notebook:
            overview.formFactorDisplayName = "Notebook / Portátil";
            overview.formFactorDescription = "Equipo móvil detectado con gestión de energía, Wi-Fi integrado y panel táctil.";
            break;
        case FormFactor::PowerWorkstation:
            overview.formFactorDisplayName = "Power-Workstation (Estación de Alto Rendimiento + Servidor)";
            overview.formFactorDescription = "Sistema multiprocesador de alto rendimiento con servicios de servidor activos.";
            break;
        case FormFactor::Workstation:
        default:
            overview.formFactorDisplayName = "Workstation / Escritorio";
            overview.formFactorDescription = "Estación de trabajo estándar de escritorio conectada a la red eléctrica.";
            break;
    }

    // 1. Detección de Video / Placa Gráfica (lspci con respaldo /sys/class/drm)
    const QVariantMap baseVideoSettings = loadVideoCurrentSettings();
    const CommandResult lspciRes = runLocalCommand("lspci", { "-nn" });
    int gpuIndex = 0;
    if (lspciRes.exitCode == 0) {
        const QStringList lines = lspciRes.stdOut.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            if (line.contains("VGA compatible controller", Qt::CaseInsensitive) ||
                line.contains("3D controller", Qt::CaseInsensitive) ||
                line.contains("Display controller", Qt::CaseInsensitive)) {

                DeviceInfo dev;
                dev.id = QString("video_controller_%1").arg(gpuIndex++);
                dev.kind = DeviceKind::VideoDisplay;
                dev.category = DeviceCategory::VideoDisplay;
                dev.targetConfigFile = "/etc/X11/xorg.conf.d/10-monitor.conf";
                dev.technicalDetails = line.trimmed();
                dev.currentSettings = baseVideoSettings;

                if (line.contains("[10de:", Qt::CaseInsensitive) ||
                    line.contains("NVIDIA", Qt::CaseInsensitive)) {
                    dev.friendlyName = "Tarjeta Gráfica Dedicada NVIDIA";
                } else if (line.contains("[8086:", Qt::CaseInsensitive) ||
                           line.contains("Intel Corporation", Qt::CaseInsensitive)) {
                    dev.friendlyName = "Gráficos Integrados Intel HD/Iris Graphics";
                } else if (line.contains("[1002:", Qt::CaseInsensitive) ||
                           line.contains("Radeon", Qt::CaseInsensitive) ||
                           line.contains("AMD/ATI", Qt::CaseInsensitive) ||
                           line.contains("Advanced Micro Devices", Qt::CaseInsensitive)) {
                    dev.friendlyName = "Tarjeta Gráfica Dedicada AMD Radeon";
                } else if (line.contains("VMware", Qt::CaseInsensitive) ||
                           line.contains("VirtualBox", Qt::CaseInsensitive) ||
                           line.contains("QXL", Qt::CaseInsensitive) ||
                           line.contains("Virtio", Qt::CaseInsensitive)) {
                    dev.friendlyName = "Adaptador Gráfico Virtual (VM / Xorg)";
                } else {
                    dev.friendlyName = "Controlador de Video / GPU";
                }

                const QString curRes = dev.currentSettings.value("resolution").toString();
                dev.statusSummary = QString("Controlador Xorg activo (%1)").arg(curRes);
                overview.devices.append(dev);
            }
        }
    }

    // Respaldo por /sys/class/drm si lspci no devolvió ninguna GPU
    if (gpuIndex == 0) {
        QDir drmDir("/sys/class/drm");
        const QStringList cards = drmDir.entryList({ "card[0-9]" }, QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &card : cards) {
            const QString vendorId = readTextFileSafe(drmDir.filePath(card + "/device/vendor")).trimmed().toLower();
            const QString deviceIdHex = readTextFileSafe(drmDir.filePath(card + "/device/device")).trimmed().toLower();

            DeviceInfo dev;
            dev.id = QString("video_controller_%1").arg(gpuIndex++);
            dev.kind = DeviceKind::VideoDisplay;
            dev.category = DeviceCategory::VideoDisplay;
            dev.targetConfigFile = "/etc/X11/xorg.conf.d/10-monitor.conf";
            dev.technicalDetails = QString("DRM: /sys/class/drm/%1 | Vendor: %2 | Device: %3")
                                       .arg(card, vendorId, deviceIdHex);
            dev.currentSettings = baseVideoSettings;

            if (vendorId == "0x10de") {
                dev.friendlyName = "Tarjeta Gráfica Dedicada NVIDIA";
            } else if (vendorId == "0x8086") {
                dev.friendlyName = "Gráficos Integrados Intel Graphics";
            } else if (vendorId == "0x1002") {
                dev.friendlyName = "Tarjeta Gráfica Dedicada AMD Radeon";
            } else {
                dev.friendlyName = QString("Controlador de Video (%1)").arg(card);
            }
            dev.statusSummary = "Controlador DRM/KMS activo";
            overview.devices.append(dev);
        }
    }

    // 1b. Detección de Monitores / Pantallas conectadas (xrandr con respaldo /sys/class/drm)
    int monitorIndex = 0;
    const CommandResult xrandrRes = runLocalCommand("xrandr", { "--current" }, 2000);
    if (xrandrRes.success) {
        const QRegularExpression modeRe(R"rx((\d+x\d+)\+\d+\+\d+)rx");
        const QStringList xLines = xrandrRes.stdOut.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : xLines) {
            if (!line.contains(" connected")) {
                continue;
            }
            const QString outName = line.section(' ', 0, 0).trimmed();
            const bool isPrimary = line.contains("primary", Qt::CaseInsensitive);
            const QRegularExpressionMatch mMode = modeRe.match(line);
            const QString activeMode = mMode.hasMatch() ? mMode.captured(1) : "Nativa";

            DeviceInfo monDev;
            monDev.id = QString("video_controller_monitor_%1").arg(monitorIndex++);
            monDev.kind = DeviceKind::VideoDisplay;
            monDev.category = DeviceCategory::VideoDisplay;
            monDev.friendlyName = QString("Monitor / Pantalla (%1)%2")
                                      .arg(outName, isPrimary ? " — Principal" : "");
            monDev.targetConfigFile = "/etc/X11/xorg.conf.d/10-monitor.conf";
            monDev.technicalDetails = line.trimmed();
            monDev.currentSettings = baseVideoSettings;
            monDev.currentSettings["outputName"] = outName;
            if (mMode.hasMatch()) {
                monDev.currentSettings["resolution"] = activeMode;
            }
            monDev.statusSummary = QString("Conectado (%1)").arg(activeMode);
            overview.devices.append(monDev);
        }
    }

    // Respaldo de monitores por /sys/class/drm si xrandr no devolvió salidas
    if (monitorIndex == 0) {
        QDir drmDir("/sys/class/drm");
        const QStringList connectors = drmDir.entryList({ "card*-*" }, QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &conn : connectors) {
            const QString status = readTextFileSafe(drmDir.filePath(conn + "/status")).trimmed();
            if (status != "connected") {
                continue;
            }
            const QString modes = readTextFileSafe(drmDir.filePath(conn + "/modes")).trimmed();
            const QString firstMode = modes.section('\n', 0, 0).trimmed();

            DeviceInfo monDev;
            monDev.id = QString("video_controller_monitor_%1").arg(monitorIndex++);
            monDev.kind = DeviceKind::VideoDisplay;
            monDev.category = DeviceCategory::VideoDisplay;
            monDev.friendlyName = QString("Monitor / Pantalla (%1)").arg(conn);
            monDev.targetConfigFile = "/etc/X11/xorg.conf.d/10-monitor.conf";
            monDev.technicalDetails = QString("Conector DRM: %1 | Estado: connected | Modo preferido: %2")
                                          .arg(conn, firstMode.isEmpty() ? "Auto" : firstMode);
            monDev.currentSettings = baseVideoSettings;
            monDev.statusSummary = firstMode.isEmpty() ? "Conectado" : QString("Conectado (%1)").arg(firstMode);
            overview.devices.append(monDev);
        }
    }

    // 2. Detección de Audio
    const CommandResult aplayRes = runLocalCommand("aplay", { "-l" });
    if (aplayRes.exitCode == 0 && aplayRes.stdOut.contains("card", Qt::CaseInsensitive)) {
        QStringList cardsList;
        const QStringList aLines = aplayRes.stdOut.split('\n', Qt::SkipEmptyParts);
        for (const QString &al : aLines) {
            if (al.startsWith("card ")) {
                cardsList << al.trimmed();
            }
        }
        if (cardsList.isEmpty()) {
            cardsList << "card 0: Placa de Audio Principal";
        }

        DeviceInfo dev;
        dev.id = "audio_primary";
        dev.kind = DeviceKind::Audio;
        dev.friendlyName = "Placa de Audio Principal (ALSA / PulseAudio)";
        dev.category = DeviceCategory::Audio;
        dev.targetConfigFile = "/etc/asound.conf";
        dev.technicalDetails = cardsList.join("\n");
        dev.currentSettings = loadAudioCurrentSettings(cardsList);

        const int vol = dev.currentSettings.value("volume", 80).toInt();
        const bool muted = dev.currentSettings.value("muted", false).toBool();
        dev.statusSummary = muted ? "Operativa (Silenciada)" : QString("Operativa — Volumen %1%").arg(vol);
        overview.devices.append(dev);
    }

    // 3. Detección de Red (Ethernet / Wi-Fi)
    QDir netDir("/sys/class/net");
    if (netDir.exists()) {
        const QStringList ifaces = netDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &iface : ifaces) {
            if (iface == "lo" || iface.startsWith("docker") || iface.startsWith("br-") || iface.startsWith("veth")) {
                continue;
            }
            const bool isWifi = QDir(netDir.filePath(iface + "/wireless")).exists() || iface.startsWith("wl");
            const QString operState = readTextFileSafe(netDir.filePath(iface + "/operstate")).trimmed();

            DeviceInfo dev;
            dev.id = QString("network_%1").arg(iface);
            dev.kind = DeviceKind::Network;
            dev.friendlyName = isWifi
                                   ? QString("Adaptador de Red Inalámbrica Wi-Fi (%1)").arg(iface)
                                   : QString("Tarjeta de Red Cableada Ethernet (%1)").arg(iface);
            dev.category = DeviceCategory::NetworkBluetooth;
            dev.targetConfigFile = "/etc/rc.d/rc.inet1.conf";
            dev.technicalDetails = QString("Interfaz: %1 | Tipo: %2 | Estado kernel: %3")
                                       .arg(iface, isWifi ? "IEEE 802.11 Wi-Fi" : "Ethernet", operState);
            dev.currentSettings = loadNetworkCurrentSettings(iface);
            dev.statusSummary = (operState == "up") ? "Conectado (Enlace activo)" : "Disponible (Sin enlace activo)";
            overview.devices.append(dev);
        }
    }

    // 4. Detección de Bluetooth
    const CommandResult lsusbRes = runLocalCommand("lsusb", {});
    const bool hasBt = (lsusbRes.exitCode == 0 && lsusbRes.stdOut.contains("Bluetooth", Qt::CaseInsensitive)) ||
                       !QDir("/sys/class/bluetooth").entryList(QDir::Dirs | QDir::NoDotAndDotDot).isEmpty();
    if (hasBt) {
        DeviceInfo dev;
        dev.id = "bluetooth_adapter";
        dev.kind = DeviceKind::Bluetooth;
        dev.friendlyName = "Adaptador Inalámbrico Bluetooth";
        dev.category = DeviceCategory::NetworkBluetooth;
        dev.targetConfigFile = "/etc/bluetooth/main.conf";
        dev.technicalDetails = "Controlador USB BlueZ (/etc/rc.d/rc.bluetooth)";
        dev.currentSettings = loadBluetoothCurrentSettings();
        dev.statusSummary = QFileInfo("/etc/rc.d/rc.bluetooth").isExecutable()
                                ? "Habilitado en el arranque"
                                : "Disponible (Servicio manual)";
        overview.devices.append(dev);
    }

    // 5. Periféricos de entrada: Ratón, Touchpad y Teclado detectados en /proc/bus/input/devices
    QString detectedMouseName = "Ratón Óptico USB / Dispositivo Punteador";
    QString detectedMouseTech = "Controlador libinput (Xorg)";
    bool hasTouchpad = (overview.formFactor == FormFactor::Notebook);
    QString detectedTouchpadName = "Panel Táctil Integrado (Touchpad)";
    QString detectedTouchpadTech = "Controlador libinput Touchpad (Xorg)";
    QString detectedKeyboardName = "Teclado del Sistema (Repetición y Bloq Num)";
    QString detectedKeyboardTech = "Controlador evdev / libinput Keyboard";

    const QString inputDevices = readTextFileSafe("/proc/bus/input/devices");
    if (!inputDevices.isEmpty()) {
        const QStringList blocks = inputDevices.split("\n\n", Qt::SkipEmptyParts);
        for (const QString &block : blocks) {
            QString devName;
            QString handlers;
            const QStringList bLines = block.split('\n', Qt::SkipEmptyParts);
            for (const QString &bl : bLines) {
                if (bl.startsWith("N: Name=")) {
                    devName = bl.mid(8).remove('"').trimmed();
                } else if (bl.startsWith("H: Handlers=")) {
                    handlers = bl.mid(12).trimmed();
                }
            }
            if (devName.isEmpty()) {
                continue;
            }
            if (devName.contains("Touchpad", Qt::CaseInsensitive) ||
                devName.contains("Synaptics", Qt::CaseInsensitive) ||
                devName.contains("Elan", Qt::CaseInsensitive)) {
                hasTouchpad = true;
                detectedTouchpadName = QString("Touchpad — %1").arg(devName);
                detectedTouchpadTech = QString("Dispositivo: %1 | Handlers: %2").arg(devName, handlers);
            } else if (handlers.contains("mouse", Qt::CaseInsensitive) &&
                       !devName.contains("Virtual", Qt::CaseInsensitive) &&
                       !devName.contains("Power Button", Qt::CaseInsensitive)) {
                detectedMouseName = QString("Ratón — %1").arg(devName);
                detectedMouseTech = QString("Dispositivo: %1 | Handlers: %2").arg(devName, handlers);
            } else if (handlers.contains("kbd", Qt::CaseInsensitive) &&
                       !devName.contains("Power Button", Qt::CaseInsensitive) &&
                       !devName.contains("Sleep Button", Qt::CaseInsensitive) &&
                       !devName.contains("Video Bus", Qt::CaseInsensitive)) {
                detectedKeyboardName = QString("Teclado — %1").arg(devName);
                detectedKeyboardTech = QString("Dispositivo: %1 | Handlers: %2").arg(devName, handlers);
            }
        }
    }

    // 5a. Ratón (siempre presente para configurar velocidad y mano izquierda/derecha)
    {
        DeviceInfo mouseDev;
        mouseDev.id = "mouse_input";
        mouseDev.kind = DeviceKind::Mouse;
        mouseDev.friendlyName = detectedMouseName;
        mouseDev.category = DeviceCategory::InputPeripherals;
        mouseDev.targetConfigFile = "/etc/X11/xorg.conf.d/40-libinput.conf";
        mouseDev.technicalDetails = detectedMouseTech;
        mouseDev.currentSettings = loadMouseCurrentSettings();

        const bool leftH = mouseDev.currentSettings.value("leftHanded", false).toBool();
        const double spd = mouseDev.currentSettings.value("accelSpeed", 0.0).toDouble();
        const QString spdTxt = (spd < -0.2) ? "Suave" : ((spd > 0.2) ? "Rápido" : "Estándar");
        mouseDev.statusSummary = QString("%1 | Movimiento %2").arg(leftH ? "Mano izquierda" : "Mano derecha", spdTxt);
        overview.devices.append(mouseDev);
    }

    // 5b. Touchpad (si se detectó hardware o es Notebook)
    if (hasTouchpad) {
        DeviceInfo tpDev;
        tpDev.id = "touchpad_input";
        tpDev.kind = DeviceKind::Touchpad;
        tpDev.friendlyName = detectedTouchpadName;
        tpDev.category = DeviceCategory::InputPeripherals;
        tpDev.targetConfigFile = "/etc/X11/xorg.conf.d/41-libinput-touchpad.conf";
        tpDev.technicalDetails = detectedTouchpadTech;
        tpDev.currentSettings = loadTouchpadCurrentSettings();
        tpDev.statusSummary = tpDev.currentSettings.value("tapping", true).toBool()
                                  ? "Toque para clic activo"
                                  : "Clic físico estándar";
        overview.devices.append(tpDev);
    }

    // 5c. Teclado (Velocidad de repetición y Bloq Num)
    {
        DeviceInfo kbDev;
        kbDev.id = "keyboard_input";
        kbDev.kind = DeviceKind::Keyboard;
        kbDev.friendlyName = detectedKeyboardName;
        kbDev.category = DeviceCategory::InputPeripherals;
        kbDev.targetConfigFile = "/etc/X11/xorg.conf.d/90-slackwiz-keyboard.conf";
        kbDev.technicalDetails = detectedKeyboardTech;
        kbDev.currentSettings = loadKeyboardDeviceSettings();
        kbDev.statusSummary = QString("Repetición %1 cps").arg(kbDev.currentSettings.value("repeatRate", 30).toInt());
        overview.devices.append(kbDev);
    }

    // 6. Almacenamiento principal
    const CommandResult lsblkRes = runLocalCommand("lsblk", { "-d", "-n", "-o", "NAME,MODEL,SIZE,ROTA" }, 2000);
    if (lsblkRes.success) {
        const QStringList dLines = lsblkRes.stdOut.split('\n', Qt::SkipEmptyParts);
        for (const QString &dl : dLines) {
            const QString trimmed = dl.trimmed();
            if (trimmed.startsWith("loop") || trimmed.startsWith("sr") || trimmed.startsWith("ram")) {
                continue;
            }
            const QStringList parts = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                const QString diskName = parts.first();
                const bool isSsd = (parts.last() == "0");
                DeviceInfo stDev;
                stDev.id = "storage_primary";
                stDev.kind = DeviceKind::Storage;
                stDev.friendlyName = isSsd
                                         ? QString("Unidad de Estado Sólido SSD (/dev/%1)").arg(diskName)
                                         : QString("Disco Duro Mecánico HDD (/dev/%1)").arg(diskName);
                stDev.category = DeviceCategory::Storage;
                stDev.targetConfigFile = "/etc/udev/rules.d/60-slackwiz-iosched.rules";
                stDev.technicalDetails = trimmed;
                stDev.currentSettings = loadStorageCurrentSettings(isSsd);
                stDev.statusSummary = isSsd ? "Optimizado para SSD" : "Planificador estándar";
                overview.devices.append(stDev);
                break;
            }
        }
    }

    return overview;
}

QString DeviceManager::previewConfigContent(const QString &deviceId, const QVariantMap &settings)
{
    if (deviceId == "mouse_input") {
        return buildMouseConfigText(settings);
    }
    if (deviceId == "touchpad_input") {
        return buildTouchpadConfigText(settings);
    }
    if (deviceId == "keyboard_input") {
        return buildKeyboardDeviceConfigText(settings);
    }
    if (deviceId.startsWith("video_controller")) {
        return buildVideoConfigText(settings);
    }
    if (deviceId == "audio_primary") {
        return buildAudioConfigText(settings);
    }
    if (deviceId == "bluetooth_adapter") {
        return buildBluetoothConfigText(settings);
    }
    if (deviceId.startsWith("network_")) {
        return buildNetworkConfigText(settings);
    }
    if (deviceId == "storage_primary") {
        return buildStorageConfigText(settings);
    }
    return QString();
}


DeviceOperationResult DeviceManager::applyDeviceConfiguration(const QString &deviceId,
                                                                const QVariantMap &settings,
                                                                const QString &rootPassword)
{
    DeviceOperationResult res;
    QString targetFile;
    const QString content = previewConfigContent(deviceId, settings);

    if (deviceId == "mouse_input") {
        targetFile = "/etc/X11/xorg.conf.d/40-libinput.conf";
    } else if (deviceId == "touchpad_input") {
        targetFile = "/etc/X11/xorg.conf.d/41-libinput-touchpad.conf";
    } else if (deviceId == "keyboard_input") {
        targetFile = "/etc/X11/xorg.conf.d/90-slackwiz-keyboard.conf";
    } else if (deviceId.startsWith("video_controller")) {
        targetFile = "/etc/X11/xorg.conf.d/10-monitor.conf";
    } else if (deviceId == "audio_primary") {
        targetFile = "/etc/asound.conf";
    } else if (deviceId == "bluetooth_adapter") {
        targetFile = "/etc/bluetooth/main.conf";
    } else if (deviceId.startsWith("network_")) {
        targetFile = "/etc/rc.d/rc.inet1.conf";
    } else if (deviceId == "storage_primary") {
        targetFile = "/etc/udev/rules.d/60-slackwiz-iosched.rules";
    } else {
        res.success = false;
        res.message = QString("Dispositivo no reconocido: %1").arg(deviceId);
        return res;
    }

    LOG_INFO("DeviceManager", QString("Aplicando configuración gráfica para '%1' en %2").arg(deviceId, targetFile));

    const bool skipInet1Write = deviceId.startsWith("network_") &&
                                settings.value("networkManager", "rc.inet1").toString() != "rc.inet1";

    if (!skipInet1Write) {
        QString writeErr;
        if (!writePrivilegedConfigFile(targetFile, content, rootPassword, writeErr)) {
            res.success = false;
            res.message = QString("No se pudo guardar la configuración en %1:\n%2").arg(targetFile, writeErr);
            return res;
        }
    }

    // Aplicar cambios en vivo en la sesión gráfica / sistema según dispositivo
    QStringList liveNotes;

    if (deviceId == "mouse_input") {
        if (settings.value("applyLiveX11", true).toBool()) {
            applyLiveMouseXinput(settings);
            liveNotes << "Ajustes aplicados al instante en la sesión gráfica actual (xinput).";
        }
    } else if (deviceId == "touchpad_input") {
        if (settings.value("applyLiveX11", true).toBool()) {
            applyLiveTouchpadXinput(settings);
            liveNotes << "Ajustes del panel táctil aplicados al instante (xinput).";
        }
    } else if (deviceId == "keyboard_input") {
        if (settings.value("applyLiveX11", true).toBool()) {
            const int delay = qBound(150, settings.value("repeatDelay", 350).toInt(), 900);
            const int rate = qBound(10, settings.value("repeatRate", 30).toInt(), 80);
            runLocalCommand("xset", { "r", "rate", QString::number(delay), QString::number(rate) }, 1500);
            if (settings.value("numLockOn", true).toBool()) {
                runLocalCommand("numlockx", { "on" }, 1000);
            }
            liveNotes << "Velocidad de repetición aplicada inmediatamente en X11.";
        }
    } else if (deviceId.startsWith("video_controller")) {
        if (settings.value("applyLiveX11", true).toBool()) {
            applyLiveVideoXrandr(settings);
            liveNotes << "Parámetros de pantalla aplicados inmediatamente mediante xrandr.";
        }
    } else if (deviceId == "audio_primary") {
        const int cardIdx = qMax(0, settings.value("cardIndex", 0).toInt());
        const int vol = qBound(0, settings.value("volume", 80).toInt(), 100);
        const bool muted = settings.value("muted", false).toBool();
        runLocalCommand("amixer", { "-c", QString::number(cardIdx), "sset", "Master",
                                    QString("%1%").arg(vol), muted ? "mute" : "unmute" }, 2000);
        liveNotes << QString("Volumen maestro ajustado al %1%% (%2).").arg(vol).arg(muted ? "silenciado" : "activo");
    } else if (deviceId == "bluetooth_adapter") {
        const bool bootEn = settings.value("enableServiceBoot", true).toBool();
        if (QFile::exists("/etc/rc.d/rc.bluetooth")) {
            QString btErr;
            const QString btCmd = bootEn
                                      ? "chmod +x /etc/rc.d/rc.bluetooth && /etc/rc.d/rc.bluetooth restart"
                                      : "chmod -x /etc/rc.d/rc.bluetooth && /etc/rc.d/rc.bluetooth stop";
            if (executePrivilegedScript(btCmd, rootPassword, btErr)) {
                liveNotes << (bootEn ? "Servicio rc.bluetooth habilitado e iniciado."
                                     : "Servicio rc.bluetooth detenido y deshabilitado en el arranque.");
            }
        }
    } else if (deviceId.startsWith("network_")) {
        const QString manager = settings.value("networkManager", "rc.inet1").toString();
        const QString iface = settings.value("interface", "eth0").toString();
        const QString mode = settings.value("mode", "dhcp").toString();
        const QString dnsRaw = settings.value("dns").toString().trimmed();
        QString netErr;

        if (manager == "networkmanager") {
            if (QFile::exists("/etc/rc.d/rc.networkmanager")) {
                executePrivilegedScript("chmod +x /etc/rc.d/rc.networkmanager && /etc/rc.d/rc.networkmanager restart", rootPassword, netErr);
                liveNotes << "Servicio NetworkManager habilitado e iniciado.";
            }
        } else if (manager == "wicd") {
            if (QFile::exists("/etc/rc.d/rc.wicd")) {
                executePrivilegedScript("chmod +x /etc/rc.d/rc.wicd && /etc/rc.d/rc.wicd restart", rootPassword, netErr);
                liveNotes << "Servicio Wicd habilitado e iniciado.";
            }
        } else {
            if (mode == "static" && !dnsRaw.isEmpty()) {
                QString resolvContent = "# Generado por Slackware Wizard (slackwiz)\n";
                const QStringList servers = dnsRaw.split(QRegularExpression(R"rx([\s,;]+)rx"), Qt::SkipEmptyParts);
                for (const QString &srv : servers) {
                    resolvContent += QString("nameserver %1\n").arg(srv);
                }
                if (writePrivilegedConfigFile("/etc/resolv.conf", resolvContent, rootPassword, netErr)) {
                    liveNotes << "Servidores DNS actualizados en /etc/resolv.conf.";
                }
            }
            if (QFile::exists("/etc/rc.d/rc.inet1")) {
                executePrivilegedScript(QString("/etc/rc.d/rc.inet1 %1_restart").arg(iface), rootPassword, netErr);
                liveNotes << QString("Interfaz %1 reiniciada mediante rc.inet1.").arg(iface);
            }
        }
    } else if (deviceId == "storage_primary") {
        QString udevErr;
        executePrivilegedScript("udevadm control --reload-rules && udevadm trigger --subsystem-match=block", rootPassword, udevErr);
        if (settings.value("enableTrim", false).toBool()) {
            executePrivilegedScript("fstrim -av || true", rootPassword, udevErr);
            liveNotes << "Reglas udev recargadas y optimización TRIM ejecutada.";
        }
    }

    res.success = true;
    res.message = skipInet1Write
                      ? "Configuración de red aplicada correctamente."
                      : QString("Configuración guardada correctamente en %1 (con respaldo automático .bak).").arg(targetFile);
    if (!liveNotes.isEmpty()) {
        res.message += "\n• " + liveNotes.join("\n• ");
    }
    return res;
}

QString DeviceManager::getAdvancedModeWarning()
{
    return "ADVERTENCIA — ZONA DE CUIDADO (Configuración Avanzada de Hardware):\n\n"
           "Estás a punto de inspeccionar parámetros de bajo nivel que controlan los controladores del kernel, "
           "la configuración del servidor gráfico Xorg (/etc/X11/xorg.conf.d/) y los módulos de audio.\n\n"
           "Una configuración incorrecta puede provocar fallos en la sesión gráfica o la pérdida de salida de audio.\n\n"
           "Por seguridad, se requerirá la contraseña de administrador (root) para aplicar cambios en archivos del sistema.";
}
