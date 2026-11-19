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

enum class DeviceKind {
    VideoDisplay,
    Audio,
    Network,
    Bluetooth,
    Mouse,
    Touchpad,
    Keyboard,
    Storage
};

enum class FormFactor {
    Notebook,
    Workstation,
    PowerWorkstation,
    Unknown
};

struct DeviceInfo {
    QString id;                 // Identificador único (ej. "mouse_input", "video_controller_0")
    DeviceKind kind = DeviceKind::Mouse;
    QString friendlyName;       // Nombre comercial limpio para novatos
    QString technicalDetails;   // Detalles técnicos para modo avanzado
    DeviceCategory category = DeviceCategory::InputPeripherals;
    bool isConfigurable = true;
    QString statusSummary;      // Estado actual legible
    QString targetConfigFile;   // Archivo de texto subyacente que modifica
    QVariantMap currentSettings;// Valores actuales leídos del sistema para precargar el diálogo
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
    static DeviceOperationResult applyDeviceConfiguration(const QString &deviceId,
                                                            const QVariantMap &settings,
                                                            const QString &rootPassword = QString());
    static QString previewConfigContent(const QString &deviceId, const QVariantMap &settings);
    static QString getAdvancedModeWarning();
};

#endif // DEVICEMANAGER_H
