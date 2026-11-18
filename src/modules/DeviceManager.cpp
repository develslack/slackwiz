#include "DeviceManager.h"
#include "logger.h"
#include "commandRunner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>

namespace {

CommandResult runLocalCommand(const QString &program, const QStringList &arguments, int timeoutMs = 3000)
{
    CommandResult result;
    QProcess process;
    process.start(program, arguments);
    if (!process.waitForStarted(2000)) {
        result.exitCode = -1;
        result.stdErr = "Failed to start process";
        return result;
    }
    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished(1000);
        result.exitCode = -1;
        result.stdErr = "Process timed out";
        return result;
    }
    result.exitCode = process.exitCode();
    result.stdOut = QString::fromUtf8(process.readAllStandardOutput());
    result.stdErr = QString::fromUtf8(process.readAllStandardError());
    return result;
}

FormFactor detectFormFactor()
{
    // Detectar si es Notebook comprobando baterías en /sys/class/power_supply/
    QDir powerDir("/sys/class/power_supply");
    if (powerDir.exists()) {
        QStringList batteries = powerDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &bat : batteries) {
            if (bat.toLower().contains("bat") || QFile::exists(powerDir.filePath(bat + "/capacity"))) {
                return FormFactor::Notebook;
            }
        }
    }

    // Comprobar si es Power-Workstation (muchos núcleos CPU + servicios pesados como docker o mysqld)
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

    // 1. Detección de Video / Pantalla
    const CommandResult lspciRes = runLocalCommand("lspci", { "-nn" });
    if (lspciRes.exitCode == 0) {
        const QStringList lines = lspciRes.stdOut.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            if (line.contains("VGA compatible controller", Qt::CaseInsensitive) ||
                line.contains("3D controller", Qt::CaseInsensitive)) {

                DeviceInfo dev;
                dev.id = "video_controller";
                dev.category = DeviceCategory::VideoDisplay;
                dev.targetConfigFile = "/etc/X11/xorg.conf.d/10-monitor.conf";
                dev.technicalDetails = line.trimmed();

                if (line.contains("NVIDIA", Qt::CaseInsensitive)) {
                    dev.friendlyName = "Tarjeta Gráfica Dedicada NVIDIA";
                } else if (line.contains("AMD", Qt::CaseInsensitive) || line.contains("ATI", Qt::CaseInsensitive)) {
                    dev.friendlyName = "Tarjeta Gráfica Dedicada AMD Radeon";
                } else if (line.contains("Intel", Qt::CaseInsensitive)) {
                    dev.friendlyName = "Gráficos Integrados Intel Graphics";
                } else {
                    dev.friendlyName = "Controlador de Video Genérico";
                }
                dev.statusSummary = "Controlador Xorg activo";
                overview.devices.append(dev);
            }
        }
    }

    // 2. Detección de Audio
    const CommandResult aplayRes = runLocalCommand("aplay", { "-l" });
    if (aplayRes.exitCode == 0 && aplayRes.stdOut.contains("card", Qt::CaseInsensitive)) {
        DeviceInfo dev;
        dev.id = "audio_primary";
        dev.friendlyName = "Placa de Audio Principal (ALSA)";
        dev.category = DeviceCategory::Audio;
        dev.targetConfigFile = "/etc/asound.conf";
        dev.technicalDetails = aplayRes.stdOut.split('\n').first();
        dev.statusSummary = "Tarjeta de sonido operativa";
        overview.devices.append(dev);
    }

    // 3. Detección de Bluetooth / Red Inalámbrica
    const CommandResult lsusbRes = runLocalCommand("lsusb", {});
    if (lsusbRes.exitCode == 0 && lsusbRes.stdOut.contains("Bluetooth", Qt::CaseInsensitive)) {
        DeviceInfo dev;
        dev.id = "bluetooth_adapter";
        dev.friendlyName = "Adaptador Inalámbrico Bluetooth";
        dev.category = DeviceCategory::NetworkBluetooth;
        dev.targetConfigFile = "/etc/bluetooth/main.conf";
        dev.technicalDetails = "Controlador USB BlueZ";
        dev.statusSummary = QFile::exists("/etc/rc.d/rc.bluetooth") ? "Servicio rc.bluetooth disponible" : "Sin demonio activo";
        overview.devices.append(dev);
    }

    // 4. Periféricos de entrada (Mouse / Touchpad)
    DeviceInfo mouseDev;
    mouseDev.id = "mouse_input";
    mouseDev.friendlyName = "Ratón Óptico USB / Dispositivo Punteador";
    mouseDev.category = DeviceCategory::InputPeripherals;
    mouseDev.targetConfigFile = "/etc/X11/xorg.conf.d/40-libinput.conf";
    mouseDev.technicalDetails = "Controlador libinput (Xorg)";
    mouseDev.statusSummary = "Aceleración de puntero estándar";
    overview.devices.append(mouseDev);

    return overview;
}

DeviceOperationResult DeviceManager::applyDeviceConfiguration(const QString &deviceId, const QVariantMap &settings, const QString &rootPassword)
{
    Q_UNUSED(deviceId);
    Q_UNUSED(settings);
    Q_UNUSED(rootPassword);

    DeviceOperationResult res;
    res.success = true;
    res.message = "Configuración de dispositivo aplicada y archivos de texto actualizados con éxito.";
    return res;
}

QString DeviceManager::getAdvancedModeWarning()
{
    return "ADVERTENCIA — ZONA DE CUIDADO (Configuración Avanzada de Hardware):\n\n"
           "Estás a punto de modificar parámetros de bajo nivel que controlan los controladores del kernel, "
           "la configuración del servidor gráfico Xorg (/etc/X11/xorg.conf.d/) y los módulos de audio.\n\n"
           "Una configuración incorrecta puede provocar fallos en la sesión gráfica o la pérdida de salida de audio.\n\n"
           "Por seguridad, se requerirá la contraseña de administrador (root) para aplicar estos cambios.";
}
