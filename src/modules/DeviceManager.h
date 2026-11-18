#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QVariantMap>

enum class DeviceCategory {
    Audio,
    VideoDisplay,
    NetworkBluetooth,
    InputPeripherals,
    Storage
};

enum class FormFactor {
    Notebook,
    Workstation,
    PowerWorkstation,
    Unknown
};

struct DeviceInfo {
    QString id;                 // Identificador único (ej. "audio_primary", "video_nvidia")
    QString friendlyName;       // Nombre comercial limpio para novatos (ej. "Placa de Audio Realtek ALC892")
    QString technicalDetails;   // Detalles técnicos para modo avanzado (ej. "PCI: 8086:a348, Driver: snd_hda_intel")
    DeviceCategory category;
    bool isConfigurable = true;
    QString statusSummary;      // Estado actual (ej. "Activo y listo", "Conectado")
    QString targetConfigFile;   // Archivo de texto subyacente que modifica (ej. "/etc/asound.conf")
};

struct SystemHardwareOverview {
    FormFactor formFactor = FormFactor::Unknown;
    QString formFactorDisplayName;
    QString formFactorDescription;
    QList<DeviceInfo> devices;
};

struct DeviceOperationResult {
    bool success = false;
    QString message;
    QString commandOutput;
};

class DeviceManager
{
public:
    static SystemHardwareOverview detectSystemHardware();
    static DeviceOperationResult applyDeviceConfiguration(const QString &deviceId, const QVariantMap &settings, const QString &rootPassword);
    static QString getAdvancedModeWarning();
};

#endif // DEVICEMANAGER_H
