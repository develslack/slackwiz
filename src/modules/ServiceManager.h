#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <QString>
#include <QStringList>
#include <QList>

enum class ServiceCategory {
    SystemBase,
    NetworkSecurity,
    DesktopHardware,
    ServersDatabases,
    ThirdPartySlackBuild
};

enum class ServiceAction {
    Start,
    Stop,
    Restart
};

enum class ServicePreset {
    DesktopLaptop,
    ServerDevelopment
};

enum class ServiceRunState {
    Running,
    Stopped,
    Unconfigured
};

enum class ServiceImportance {
    Essential,
    RecommendedByUse,
    NonEssential
};

struct SystemServiceInfo {
    QString scriptName;          // Ej: "rc.httpd", "rc.sshd"
    QString scriptPath;          // Ej: "/etc/rc.d/rc.httpd"
    QString displayName;         // Nombre amigable en español
    QString description;         // Explicación clara para principiantes
    ServiceCategory category = ServiceCategory::ServersDatabases;
    bool isCriticalBase = false; // Scripts críticos de arranque (rc.S, rc.M, rc.udev...)
    bool isThirdParty = false;   // Proveniente de SlackBuilds / terceros
    bool bootEnabled = false;    // Permiso ejecutable (+x)
    bool isRunning = false;      // Proceso activo actualmente
    ServiceRunState runState = ServiceRunState::Stopped;
    QString unconfiguredReason;  // Motivo si runState == Unconfigured
    ServiceImportance importance = ServiceImportance::RecommendedByUse;
    QString importanceNote;      // Recomendación explícita (esencial / opcional / no esencial)
    QStringList supportedActions;// start, stop, restart, status
    QString conflictWarning;     // Aviso si entra en conflicto con otro servicio activo
};

struct ServiceOperationResult {
    bool success = false;
    QString message;
    QString commandOutput;
};

struct ServicePresetChange {
    QString scriptName;
    QString displayName;
    bool enableOnBoot = false;
    bool startNow = false;
    QString reason;
};

struct SambaDetectedResources {
    QStringList detectedPrinters;      // Impresoras detectadas en CUPS o USB
    QStringList detectedStorageMounts; // Discos/particiones montadas candidatas a compartir
    bool cupsAvailable = false;
    bool cupsRunning = false;
    bool smbConfExists = false;
};

struct SambaShareItem {
    QString shareName;
    QString path;
    bool readOnly = false;
    bool guestOk = false;
};

class ServiceManager
{
public:
    // Lista todos los servicios en /etc/rc.d/rc.* (filtrando críticos si advancedMode == false)
    static QList<SystemServiceInfo> listServices(bool advancedMode);

    // Obtiene el detalle actualizado de un servicio puntual
    static SystemServiceInfo inspectService(const QString &scriptName);

    // Valida la contraseña de root sin ejecutar cambios (ej. al entrar a Modo Avanzado)
    static bool verifyRootPasswordOnly(const QString &rootPassword, QString &errorOut);

    // Activa o desactiva el arranque automático (chmod +x / -x) exigiendo contraseña de root
    static ServiceOperationResult setStartOnBoot(const QString &scriptName,
                                                 bool enable,
                                                 const QString &rootPassword,
                                                 bool allowCriticalOverride = false);

    // Ejecuta start / stop / restart exigiendo contraseña de root
    static ServiceOperationResult executeServiceAction(const QString &scriptName,
                                                       ServiceAction action,
                                                       const QString &rootPassword,
                                                       bool allowCriticalOverride = false);

    // Calcula los cambios que aplicaría un perfil recomendado (Laptop/Escritorio o Servidor)
    static QList<ServicePresetChange> previewPreset(ServicePreset preset);

    // Aplica un perfil recomendado exigiendo contraseña de root
    static ServiceOperationResult applyPreset(ServicePreset preset, const QString &rootPassword);

    // Lee el contenido de un script /etc/rc.d/rc.* para el visor en solo lectura (Modo Avanzado)
    static QString readServiceScript(const QString &scriptName);

    // Lee /etc/rc.d/rc.local o /etc/rc.d/rc.local_shutdown
    static QString readLocalBootScript(bool shutdownScript);

    // Guarda /etc/rc.d/rc.local o /etc/rc.d/rc.local_shutdown con respaldo previo y contraseña root
    static ServiceOperationResult saveLocalBootScript(const QString &content,
                                                      bool shutdownScript,
                                                      const QString &rootPassword);

    // Detecta impresoras conectadas y discos/particiones montadas para proponer en Samba
    static SambaDetectedResources detectSambaResources();

    // Genera un /etc/samba/smb.conf limpio con las impresoras y carpetas seleccionadas
    static ServiceOperationResult generateBasicSambaConfig(const QString &workgroup,
                                                           bool sharePrinters,
                                                           const QList<SambaShareItem> &shares,
                                                           const QString &rootPassword);

    // Texto de advertencia para el Modo Avanzado ("Zona de Cuidado")
    static QString getAdvancedModeWarning();

private:
    struct KnownServiceMetadata {
        QString displayName;
        QString description;
        ServiceCategory category;
        bool isCriticalBase;
        ServiceImportance importance;
        QString importanceNote;
        QStringList processNames;
    };

    static KnownServiceMetadata lookupMetadata(const QString &scriptName);
    static bool isCriticalSystemScript(const QString &scriptName);
    static bool isIgnoredHelperFile(const QString &fileName);
    static bool checkServiceRunning(const QString &scriptName,
                                    const QString &scriptPath,
                                    const QStringList &processNames);
    static ServiceRunState detectServiceRunState(const QString &scriptName,
                                                 bool isRunning,
                                                 QString &outReason);
    static QStringList detectSupportedActions(const QString &scriptPath);
    static QString detectServiceConflict(const QString &scriptName,
                                         bool bootEnabled,
                                         bool isRunning);
    static bool verifyRootPassword(const QString &rootPassword, QString &errorOut);
    static bool runAsRootWithPassword(const QString &shellCommand,
                                      const QString &rootPassword,
                                      QString &stdOut,
                                      QString &stdErr);
    static void secureWipe(QByteArray &buffer);
};

#endif // SERVICEMANAGER_H
