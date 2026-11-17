#include "ServiceManager.h"
#include "logger.h"
#include "commandRunner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QProcess>
#include <QDateTime>
#include <QSet>
#include <QMap>
#include <unistd.h>

namespace {

void secureWipe(QByteArray &buf)
{
    if (!buf.isEmpty()) {
        volatile char *ptr = buf.data();
        for (int i = 0; i < buf.size(); ++i) {
            ptr[i] = '\0';
        }
        buf.clear();
    }
}

QString shellQuote(const QString &arg)
{
    QString escaped = arg;
    escaped.replace('\'', "'\\''");
    return "'" + escaped + "'";
}

CommandResult runCommand(const QString &program, const QStringList &arguments, int timeoutMs = 30000)
{
    CommandResult result;
    QProcess process;
    process.start(program, arguments);
    if (!process.waitForStarted(3000)) {
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

} // namespace

void ServiceManager::secureWipe(QByteArray &buffer)
{
    ::secureWipe(buffer);
}

bool ServiceManager::isIgnoredHelperFile(const QString &fileName)
{
    static const QSet<QString> ignoredFiles = {
        "rc.0",
        "rc.4",
        "rc.6",
        "rc.K",
        "rc.M",
        "rc.S",
        "rc.inet1.conf",
        "rc.local",
        "rc.local_shutdown",
        "rc.sysvinit"
    };

    if (ignoredFiles.contains(fileName)) {
        return true;
    }

    if (fileName.endsWith(".conf") ||
        fileName.endsWith(".new") ||
        fileName.endsWith(".orig") ||
        fileName.endsWith(".bak") ||
        fileName.endsWith("~") ||
        fileName.contains(".bak.")) {
        return true;
    }

    return false;
}

bool ServiceManager::isCriticalSystemScript(const QString &scriptName)
{
    static const QSet<QString> criticalScripts = {
        "rc.udev",
        "rc.modules",
        "rc.modules.local",
        "rc.loop",
        "rc.inet1",
        "rc.inet2",
        "rc.acpid",
        "rc.cpufreq",
        "rc.font",
        "rc.keymap",
        "rc.console",
        "rc.gpm",
        "rc.setterm",
        "rc.syslog",
        "rc.messagebus",
        "rc.elogind",
        "rc.udev_retry"
    };

    return criticalScripts.contains(scriptName);
}

ServiceManager::KnownServiceMetadata ServiceManager::lookupMetadata(const QString &scriptName)
{
    static const QMap<QString, KnownServiceMetadata> catalog = {
        // --- Red y Seguridad ---
        { "rc.networkmanager", {
            "NetworkManager (Gestor de Red)",
            "Administra conexiones Wi-Fi, Ethernet, VPN y bandas anchas móviles de forma automática desde el escritorio.",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::Essential,
            "Esencial en laptops y equipos de escritorio con Wi-Fi o DHCP automático.",
            { "NetworkManager" }
        }},
        { "rc.sshd", {
            "Servidor SSH (OpenSSH)",
            "Permite conectarse de forma segura y cifrada al equipo desde la red mediante terminal o SFTP.",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Muy útil para administración remota y desarrollo; innecesario si no accedes por red.",
            { "sshd" }
        }},
        { "rc.firewall", {
            "Cortafuegos del Sistema (iptables/nftables)",
            "Aplica las reglas de filtrado de paquetes y protección de puertos de red definidas por el administrador.",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::RecommendedByUse,
            "Recomendado si el equipo está expuesto a redes públicas o actúa como servidor.",
            { "iptables", "nft" }
        }},
        { "rc.ntpd", {
            "Sincronización Horaria NTP",
            "Mantiene la fecha y hora del sistema sincronizadas con servidores horarios de Internet.",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::Essential,
            "Recomendado para mantener certificados SSL, registros y tareas programadas con hora exacta.",
            { "ntpd" }
        }},
        { "rc.dnsmasq", {
            "Dnsmasq (Caché DNS y DHCP local)",
            "Provee resolución DNS en caché local y servidor DHCP ligero para redes pequeñas o máquinas virtuales.",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::NonEssential,
            "No esencial: Solo necesario si compartes conexión o usas virtualización con red local.",
            { "dnsmasq" }
        }},
        { "rc.bind", {
            "Servidor DNS BIND (named)",
            "Servidor autoritativo y recursivo de nombres de dominio (DNS).",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::NonEssential,
            "No esencial: Servicio avanzado exclusivo para servidores DNS dedicados.",
            { "named" }
        }},
        { "rc.openvpn", {
            "Cliente/Servidor OpenVPN",
            "Establece túneles privados virtuales (VPN) cifrados al iniciar el sistema.",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Actívalo únicamente si utilizas túneles OpenVPN persistentes.",
            { "openvpn" }
        }},
        { "rc.wireguard", {
            "Túneles VPN WireGuard",
            "Levanta interfaces de red privada virtual WireGuard configuradas en /etc/wireguard/.",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Actívalo únicamente si utilizas conexiones WireGuard.",
            { "wg-crypt" }
        }},
        { "rc.rpc", {
            "Portmapper RPC (rpcbind)",
            "Servicio requerido únicamente si montas o compartes carpetas mediante NFS.",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::NonEssential,
            "No esencial: Puedes dejarlo desactivado salvo que uses carpetas compartidas NFS.",
            { "rpcbind" }
        }},
        { "rc.nfsd", {
            "Servidor de Archivos NFS",
            "Comparte directorios por red con otros sistemas Linux/Unix usando el protocolo NFS.",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::NonEssential,
            "No esencial: Puedes dejarlo desactivado si no compartes carpetas por NFS.",
            { "nfsd", "rpc.mountd" }
        }},
        { "rc.yp", {
            "Servicios NIS / Yellow Pages",
            "Autenticación centralizada heredada para redes Unix antiguas.",
            ServiceCategory::NetworkSecurity,
            false,
            ServiceImportance::NonEssential,
            "No esencial (Legado): No se utiliza en instalaciones modernas; déjalo desactivado.",
            { "ypbind", "ypserv" }
        }},

        // --- Escritorio y Hardware ---
        { "rc.bluetooth", {
            "Soporte Bluetooth (BlueZ)",
            "Habilita el adaptador Bluetooth para conectar auriculares, ratones, teclados o teléfonos.",
            ServiceCategory::DesktopHardware,
            false,
            ServiceImportance::RecommendedByUse,
            "Recomendado en laptops y equipos con adaptador Bluetooth; innecesario en PC sin Bluetooth.",
            { "bluetoothd" }
        }},
        { "rc.cups", {
            "Sistema de Impresión CUPS",
            "Gestiona impresoras locales USB e impresoras en red, así como las colas de impresión.",
            ServiceCategory::DesktopHardware,
            false,
            ServiceImportance::RecommendedByUse,
            "Recomendado si tienes impresoras físicas o virtuales (PDF) configuradas.",
            { "cupsd" }
        }},
        { "rc.cups-browsed", {
            "Descubrimiento de Impresoras CUPS",
            "Detecta automáticamente impresoras compartidas en la red local mediante Bonjour/Avahi.",
            ServiceCategory::DesktopHardware,
            false,
            ServiceImportance::NonEssential,
            "Opcional: Útil solo si imprimes en impresoras compartidas por red.",
            { "cups-browsed" }
        }},
        { "rc.alsa", {
            "Restauración de Audio ALSA",
            "Guarda y restaura los niveles de volumen de las tarjetas de sonido al iniciar y apagar.",
            ServiceCategory::DesktopHardware,
            false,
            ServiceImportance::Essential,
            "Esencial para conservar el volumen de sonido entre reinicios.",
            { "alsactl" }
        }},
        { "rc.pulseaudio", {
            "Servidor de Sonido PulseAudio (Modo Sistema)",
            "Ejecuta PulseAudio como demonio global (en entornos de escritorio suele iniciarse por sesión de usuario).",
            ServiceCategory::DesktopHardware,
            false,
            ServiceImportance::NonEssential,
            "No esencial: En escritorios gráficos (XFCE/KDE) PulseAudio/PipeWire arranca con tu sesión.",
            { "pulseaudio" }
        }},
        { "rc.avahidaemon", {
            "Avahi (Descubrimiento de Red mDNS / ZeroConf)",
            "Permite encontrar impresoras, dispositivos y nombres '.local' en la red doméstica sin configuración.",
            ServiceCategory::DesktopHardware,
            false,
            ServiceImportance::RecommendedByUse,
            "Recomendado en escritorio para detectar impresoras en red y compartir archivos fácilmente.",
            { "avahi-daemon" }
        }},
        { "rc.smartd", {
            "Monitor de Salud de Discos (SMART)",
            "Vigila el estado de salud S.M.A.R.T. de discos SSD y HDD para alertar de fallos inminentes.",
            ServiceCategory::DesktopHardware,
            false,
            ServiceImportance::RecommendedByUse,
            "Recomendado para prevenir pérdida de datos por desgaste de discos físicos.",
            { "smartd" }
        }},
        { "rc.atd", {
            "Programador de Tareas Diferidas (atd)",
            "Ejecuta comandos programados una sola vez mediante la herramienta 'at'.",
            ServiceCategory::DesktopHardware,
            false,
            ServiceImportance::NonEssential,
            "No esencial: Puedes dejarlo desactivado si no usas el comando 'at'.",
            { "atd" }
        }},
        { "rc.crond", {
            "Programador de Tareas Periódicas (crond)",
            "Ejecuta tareas de mantenimiento automático del sistema (rotación de logs, limpieza temporal, cron).",
            ServiceCategory::DesktopHardware,
            false,
            ServiceImportance::Essential,
            "Esencial: Mantiene limpias las carpetas temporales y ejecuta tareas de mantenimiento.",
            { "crond" }
        }},

        // --- Servidores y Bases de Datos ---
        { "rc.httpd", {
            "Servidor Web Apache (httpd)",
            "Sirve páginas web estáticas y aplicaciones PHP en el puerto 80/443.",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Actívalo si desarrollas sitios web locales o alojas servicios HTTP.",
            { "httpd" }
        }},
        { "rc.nginx", {
            "Servidor Web Nginx",
            "Servidor web de alto rendimiento y proxy inverso.",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Actívalo si utilizas Nginx como servidor web o proxy inverso.",
            { "nginx" }
        }},
        { "rc.php-fpm", {
            "Gestor de Procesos PHP-FPM",
            "Procesa código PHP mediante FastCGI (necesario si usas PHP con Nginx).",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Necesario si ejecutas aplicaciones PHP sobre Nginx.",
            { "php-fpm" }
        }},
        { "rc.mysqld", {
            "Base de Datos MariaDB / MySQL",
            "Motor de bases de datos relacionales compatible con MySQL para aplicaciones web y desarrollo.",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Actívalo si tus aplicaciones o sitios web requieren bases de datos MySQL/MariaDB.",
            { "mariadbd", "mysqld" }
        }},
        { "rc.postgresql", {
            "Base de Datos PostgreSQL",
            "Sistema avanzado de gestión de bases de datos relacionales objeto-relacional.",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Actívalo si desarrollas o ejecutas aplicaciones que usen PostgreSQL.",
            { "postgres", "postmaster" }
        }},
        { "rc.samba", {
            "Compartición de Archivos e Impresoras Samba (SMB)",
            "Permite compartir carpetas e impresoras en red con equipos Windows, macOS y Linux.",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Actívalo solo si deseas compartir carpetas o impresoras en tu red local.",
            { "smbd", "nmbd" }
        }},
        { "rc.docker", {
            "Motor de Contenedores Docker",
            "Ejecuta contenedores aislados de aplicaciones y microservicios.",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Actívalo si utilizas contenedores Docker para desarrollo o despliegue.",
            { "dockerd" }
        }},
        { "rc.libvirt", {
            "Virtualización Libvirt / KVM-QEMU",
            "Administra máquinas virtuales KVM/QEMU y redes virtuales (virt-manager).",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::RecommendedByUse,
            "Opcional: Actívalo si ejecutas máquinas virtuales con QEMU/KVM.",
            { "libvirtd" }
        }},
        { "rc.postfix", {
            "Servidor de Correo Postfix",
            "Agente de transferencia de correo electrónico (MTA) local o en red.",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::NonEssential,
            "No esencial: En equipos de escritorio no se requiere servidor de correo local.",
            { "master" }
        }},
        { "rc.sendmail", {
            "Servidor de Correo Sendmail",
            "Agente clásico de envío de correo electrónico del sistema.",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::NonEssential,
            "No esencial: Puedes dejarlo desactivado en instalaciones de escritorio.",
            { "sendmail" }
        }},
        { "rc.proftpd", {
            "Servidor FTP ProFTPD",
            "Permite transferencias de archivos mediante el protocolo FTP.",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::NonEssential,
            "No esencial: Para transferir archivos de forma segura es preferible usar SSH/SFTP.",
            { "proftpd" }
        }},
        { "rc.vsftpd", {
            "Servidor FTP vsftpd",
            "Servidor FTP ligero y seguro.",
            ServiceCategory::ServersDatabases,
            false,
            ServiceImportance::NonEssential,
            "No esencial: Puedes dejarlo desactivado salvo que necesites un servidor FTP dedicado.",
            { "vsftpd" }
        }},

        // --- Sistema Base (Críticos) ---
        { "rc.messagebus", {
            "Bus de Mensajes del Sistema (D-Bus)",
            "Canal de comunicación interno esencial entre aplicaciones de escritorio y servicios del hardware.",
            ServiceCategory::SystemBase,
            true,
            ServiceImportance::Essential,
            "ESENCIAL: No desactivar; el entorno gráfico y NetworkManager dependen de D-Bus.",
            { "dbus-daemon" }
        }},
        { "rc.elogind", {
            "Gestor de Sesiones y Energía (elogind)",
            "Controla el inicio de sesión gráfico, suspensión, apagado y permisos de dispositivos del usuario.",
            ServiceCategory::SystemBase,
            true,
            ServiceImportance::Essential,
            "ESENCIAL: No desactivar; gestiona sesiones de escritorio y permisos de hardware.",
            { "elogind" }
        }},
        { "rc.syslog", {
            "Registro de Eventos del Sistema (syslogd/klogd)",
            "Guarda los mensajes del kernel y servicios en /var/log/messages y /var/log/syslog.",
            ServiceCategory::SystemBase,
            true,
            ServiceImportance::Essential,
            "ESENCIAL: Mantiene el historial de diagnósticos del sistema.",
            { "syslogd", "klogd" }
        }},
        { "rc.acpid", {
            "Eventos de Energía ACPI (acpid)",
            "Detecta pulsaciones del botón de encendido, cierre de tapa de laptop y eventos de batería.",
            ServiceCategory::SystemBase,
            true,
            ServiceImportance::Essential,
            "ESENCIAL: Necesario para gestión correcta de energía y botón de apagado.",
            { "acpid" }
        }},
        { "rc.udev", {
            "Gestor Dinámico de Dispositivos (udev)",
            "Detecta y crea los nodos de hardware conectado (USB, discos, tarjetas gráficas y de red).",
            ServiceCategory::SystemBase,
            true,
            ServiceImportance::Essential,
            "CRÍTICO: Nunca desactivar; el sistema no detectaría hardware al arrancar.",
            { "udevd" }
        }},
        { "rc.inet1", {
            "Configuración de Interfaces de Red Base (rc.inet1)",
            "Inicializa la interfaz loopback (lo) y las tarjetas configuradas en /etc/rc.d/rc.inet1.conf.",
            ServiceCategory::SystemBase,
            true,
            ServiceImportance::Essential,
            "CRÍTICO: Necesario para la red loopback y conectividad básica.",
            {}
        }},
        { "rc.inet2", {
            "Servicios de Red Base (rc.inet2)",
            "Orquesta el arranque de demonios de red habilitados (sshd, rpc, firewall, etc.).",
            ServiceCategory::SystemBase,
            true,
            ServiceImportance::Essential,
            "CRÍTICO: Lanza los servicios de red del sistema durante el inicio.",
            {}
        }}
    };

    if (catalog.contains(scriptName)) {
        return catalog.value(scriptName);
    }

    KnownServiceMetadata fallback;
    QString cleanName = scriptName;
    if (cleanName.startsWith("rc.")) {
        cleanName = cleanName.mid(3);
    }
    fallback.displayName = QString("Servicio %1").arg(cleanName);

    if (isCriticalSystemScript(scriptName)) {
        fallback.category = ServiceCategory::SystemBase;
        fallback.isCriticalBase = true;
        fallback.importance = ServiceImportance::Essential;
        fallback.importanceNote = "Script base del sistema Slackware; modificarlo solo si sabes exactamente qué hace.";
        fallback.description = QString("Script de inicialización base del sistema Slackware (%1).").arg(scriptName);
    } else {
        fallback.category = ServiceCategory::ThirdPartySlackBuild;
        fallback.isCriticalBase = false;
        fallback.importance = ServiceImportance::RecommendedByUse;
        fallback.importanceNote = "Servicio adicional / SlackBuild: actívalo solo si utilizas esta aplicación.";
        fallback.description = QString("Servicio adicional instalado en /etc/rc.d/%1 (paquete extra o SlackBuild).").arg(scriptName);
    }
    fallback.processNames = QStringList{ cleanName };
    return fallback;
}

bool ServiceManager::checkServiceRunning(const QString &scriptName,
                                         const QString &scriptPath,
                                         const QStringList &processNames)
{
    if (scriptName == "rc.inet1") {
        return QFile::exists("/sys/class/net/lo");
    }

    const QFileInfo fi(scriptPath);
    if (fi.isExecutable()) {
        const CommandResult statusRes = runCommand(scriptPath, { "status" }, 2500);
        const QString combined = (statusRes.stdOut + "\n" + statusRes.stdErr).toLower();
        if (statusRes.exitCode == 0 &&
            (combined.contains("running") || combined.contains("active") || combined.contains("ejecución") || combined.contains("pid"))) {
            if (!combined.contains("not running") && !combined.contains("stopped") && !combined.contains("inactive")) {
                return true;
            }
        }
    }

    for (const QString &proc : processNames) {
        if (proc.trimmed().isEmpty()) {
            continue;
        }
        const CommandResult pgrepRes = runCommand("pgrep", { "-x", proc }, 1500);
        if (pgrepRes.exitCode == 0 && !pgrepRes.stdOut.trimmed().isEmpty()) {
            return true;
        }
    }

    return false;
}

ServiceRunState ServiceManager::detectServiceRunState(const QString &scriptName,
                                                      bool isRunning,
                                                      QString &outReason)
{
    outReason.clear();

    if (isRunning) {
        return ServiceRunState::Running;
    }

    // 1. MariaDB sin tablas de sistema inicializadas
    if (scriptName == "rc.mysqld") {
        if (!QDir("/var/lib/mysql/mysql").exists()) {
            outReason = "MariaDB aún no tiene sus tablas del sistema inicializadas (ejecuta mysql_install_db o configúralo en el Módulo 7).";
            return ServiceRunState::Unconfigured;
        }
    }

    // 2. PostgreSQL sin clúster de datos inicializado
    if (scriptName == "rc.postgresql") {
        bool hasPgData = false;
        const QDir pgsqlBase("/var/lib/pgsql");
        if (pgsqlBase.exists()) {
            const QStringList subDirs = pgsqlBase.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString &d : subDirs) {
                if (QFile::exists(pgsqlBase.filePath(d + "/data/PG_VERSION"))) {
                    hasPgData = true;
                    break;
                }
            }
        }
        if (!hasPgData) {
            outReason = "PostgreSQL aún no tiene su directorio de datos inicializado (initdb). Puedes inicializarlo en el Módulo 7.";
            return ServiceRunState::Unconfigured;
        }
    }

    // 3. Samba sin archivo /etc/samba/smb.conf (Slackware solo trae smb.conf-sample por defecto)
    if (scriptName == "rc.samba") {
        if (!QFile::exists("/etc/samba/smb.conf")) {
            outReason = "Falta /etc/samba/smb.conf (Slackware incluye únicamente smb.conf-sample hasta que lo configures).";
            return ServiceRunState::Unconfigured;
        }
    }

    // 4. BIND DNS sin /etc/named.conf
    if (scriptName == "rc.bind") {
        if (!QFile::exists("/etc/named.conf")) {
            outReason = "Falta el archivo de configuración /etc/named.conf.";
            return ServiceRunState::Unconfigured;
        }
    }

    // 5. OpenVPN sin archivos de configuración .conf
    if (scriptName == "rc.openvpn") {
        const QDir ovpnDir("/etc/openvpn");
        if (!ovpnDir.exists() || ovpnDir.entryList({ "*.conf" }, QDir::Files).isEmpty()) {
            outReason = "No se encontraron perfiles *.conf en /etc/openvpn/.";
            return ServiceRunState::Unconfigured;
        }
    }

    // 6. WireGuard sin interfaces .conf
    if (scriptName == "rc.wireguard") {
        const QDir wgDir("/etc/wireguard");
        if (!wgDir.exists() || wgDir.entryList({ "*.conf" }, QDir::Files).isEmpty()) {
            outReason = "No se encontraron interfaces *.conf en /etc/wireguard/.";
            return ServiceRunState::Unconfigured;
        }
    }

    // 7. NTPd sin servidores configurados en /etc/ntp.conf
    if (scriptName == "rc.ntpd") {
        QFile ntpConf("/etc/ntp.conf");
        bool hasServerLine = false;
        if (ntpConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString content = QString::fromUtf8(ntpConf.readAll());
            const QRegularExpression srvRe(R"regex(^\s*(?:server|pool)\s+\S+)regex", QRegularExpression::MultilineOption);
            hasServerLine = srvRe.match(content).hasMatch();
        }
        if (!hasServerLine) {
            outReason = "No hay ningún servidor 'server' o 'pool' activo en /etc/ntp.conf.";
            return ServiceRunState::Unconfigured;
        }
    }

    return ServiceRunState::Stopped;
}

QStringList ServiceManager::detectSupportedActions(const QString &scriptPath)
{
    QStringList actions = { "start", "stop", "restart" };
    QFile f(scriptPath);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString content = QString::fromUtf8(f.readAll());
        if (content.contains("status")) {
            actions << "status";
        }
        if (content.contains("reload")) {
            actions << "reload";
        }
    }
    return actions;
}

QString ServiceManager::detectServiceConflict(const QString &scriptName,
                                              bool bootEnabled,
                                              bool isRunning)
{
    // Conflicto 1: Apache y Nginx ambos activos o habilitados en el puerto 80
    if (scriptName == "rc.httpd" && (bootEnabled || isRunning)) {
        const QFileInfo nginxFi("/etc/rc.d/rc.nginx");
        if (nginxFi.exists() && nginxFi.isExecutable()) {
            return "Conflicto potencial: Nginx (rc.nginx) también está habilitado y ambos suelen usar el puerto 80.";
        }
    }
    if (scriptName == "rc.nginx" && (bootEnabled || isRunning)) {
        const QFileInfo httpdFi("/etc/rc.d/rc.httpd");
        if (httpdFi.exists() && httpdFi.isExecutable()) {
            return "Conflicto potencial: Apache (rc.httpd) también está habilitado y ambos suelen usar el puerto 80.";
        }
    }

    // Conflicto 2: NetworkManager vs interfaces estáticas/DHCP en rc.inet1.conf
    if (scriptName == "rc.networkmanager") {
        QFile inetConf("/etc/rc.d/rc.inet1.conf");
        if (inetConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString content = QString::fromUtf8(inetConf.readAll());
            const QRegularExpression activeIfRe(
                R"regex(^\s*(?:USE_DHCP|IPADDR)\[\d+\]\s*=\s*"(yes|[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)")regex",
                QRegularExpression::MultilineOption
            );
            if (activeIfRe.match(content).hasMatch()) {
                return "Aviso: /etc/rc.d/rc.inet1.conf tiene interfaces configuradas manualmente; NetworkManager suele requerir rc.inet1.conf vacío.";
            }
        }
    }

    return QString();
}

QList<SystemServiceInfo> ServiceManager::listServices(bool advancedMode)
{
    QList<SystemServiceInfo> result;
    const QDir rcdDir("/etc/rc.d");
    if (!rcdDir.exists()) {
        LOG_ERROR("ServiceManager", "El directorio /etc/rc.d no existe en el sistema.");
        return result;
    }

    const QFileInfoList entries = rcdDir.entryInfoList(
        { "rc.*" },
        QDir::Files | QDir::NoSymLinks,
        QDir::Name
    );

    for (const QFileInfo &fi : entries) {
        const QString fileName = fi.fileName();
        if (isIgnoredHelperFile(fileName)) {
            continue;
        }

        const KnownServiceMetadata meta = lookupMetadata(fileName);
        if (!advancedMode && meta.isCriticalBase) {
            continue;
        }

        SystemServiceInfo info;
        info.scriptName = fileName;
        info.scriptPath = fi.absoluteFilePath();
        info.displayName = meta.displayName;
        info.description = meta.description;
        info.category = meta.category;
        info.isCriticalBase = meta.isCriticalBase;
        info.isThirdParty = (meta.category == ServiceCategory::ThirdPartySlackBuild);
        info.bootEnabled = fi.isExecutable();
        info.isRunning = checkServiceRunning(fileName, info.scriptPath, meta.processNames);
        info.runState = detectServiceRunState(fileName, info.isRunning, info.unconfiguredReason);
        info.importance = meta.importance;
        info.importanceNote = meta.importanceNote;
        info.supportedActions = detectSupportedActions(info.scriptPath);
        info.conflictWarning = detectServiceConflict(fileName, info.bootEnabled, info.isRunning);

        result.append(info);
    }

    return result;
}

SystemServiceInfo ServiceManager::inspectService(const QString &scriptName)
{
    const QString cleanName = QFileInfo(scriptName).fileName();
    const QString path = "/etc/rc.d/" + cleanName;
    const QFileInfo fi(path);
    const KnownServiceMetadata meta = lookupMetadata(cleanName);

    SystemServiceInfo info;
    info.scriptName = cleanName;
    info.scriptPath = path;
    info.displayName = meta.displayName;
    info.description = meta.description;
    info.category = meta.category;
    info.isCriticalBase = meta.isCriticalBase;
    info.isThirdParty = (meta.category == ServiceCategory::ThirdPartySlackBuild);
    info.bootEnabled = fi.exists() && fi.isExecutable();
    info.isRunning = fi.exists() && checkServiceRunning(cleanName, path, meta.processNames);
    info.runState = detectServiceRunState(cleanName, info.isRunning, info.unconfiguredReason);
    info.importance = meta.importance;
    info.importanceNote = meta.importanceNote;
    info.supportedActions = fi.exists() ? detectSupportedActions(path) : QStringList();
    info.conflictWarning = detectServiceConflict(cleanName, info.bootEnabled, info.isRunning);

    return info;
}

bool ServiceManager::verifyRootPasswordOnly(const QString &rootPassword, QString &errorOut)
{
    return verifyRootPassword(rootPassword, errorOut);
}

bool ServiceManager::verifyRootPassword(const QString &rootPassword, QString &errorOut)
{
    if (rootPassword.isEmpty()) {
        errorOut = "Debes ingresar la contraseña de administrador (root) para autorizar esta operación.";
        return false;
    }

    QProcess proc;
    proc.start("su", { "-c", "true", "root" });
    if (!proc.waitForStarted(3000)) {
        errorOut = "No se pudo iniciar la verificación de credenciales con 'su'.";
        return false;
    }

    QByteArray passBytes = rootPassword.toUtf8();
    passBytes.append('\n');
    proc.write(passBytes);
    proc.closeWriteChannel();
    ::secureWipe(passBytes);

    if (!proc.waitForFinished(8000)) {
        proc.kill();
        errorOut = "Tiempo de espera agotado al verificar la contraseña de root.";
        return false;
    }

    if (proc.exitCode() != 0) {
        errorOut = "Contraseña de administrador (root) incorrecta. Operación cancelada por seguridad.";
        return false;
    }

    return true;
}

bool ServiceManager::runAsRootWithPassword(const QString &shellCommand,
                                           const QString &rootPassword,
                                           QString &stdOut,
                                           QString &stdErr)
{
    if (::geteuid() == 0) {
        QString authErr;
        if (!verifyRootPassword(rootPassword, authErr)) {
            stdErr = authErr;
            return false;
        }
        const CommandResult res = runCommand("/bin/sh", { "-c", shellCommand }, 20000);
        stdOut = res.stdOut.trimmed();
        stdErr = res.stdErr.trimmed();
        return (res.exitCode == 0);
    }

    QProcess proc;
    proc.start("su", { "-c", shellCommand, "root" });
    if (!proc.waitForStarted(3000)) {
        stdErr = "No se pudo iniciar el proceso de elevación de privilegios (su).";
        return false;
    }

    QByteArray passBytes = rootPassword.toUtf8();
    passBytes.append('\n');
    proc.write(passBytes);
    proc.closeWriteChannel();
    ::secureWipe(passBytes);

    if (!proc.waitForFinished(25000)) {
        proc.kill();
        stdErr = "Tiempo de espera agotado al ejecutar la orden como root.";
        return false;
    }

    stdOut = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    stdErr = QString::fromUtf8(proc.readAllStandardError()).trimmed();

    if (proc.exitCode() != 0) {
        if (stdErr.contains("Authentication failure", Qt::CaseInsensitive) ||
            stdErr.contains("incorrect password", Qt::CaseInsensitive) ||
            stdErr.contains("fallo de autenticación", Qt::CaseInsensitive)) {
            stdErr = "Contraseña de administrador (root) incorrecta. Operación cancelada.";
        }
        return false;
    }

    return true;
}

ServiceOperationResult ServiceManager::setStartOnBoot(const QString &scriptName,
                                                      bool enable,
                                                      const QString &rootPassword,
                                                      bool allowCriticalOverride)
{
    ServiceOperationResult res;
    const QString cleanName = QFileInfo(scriptName).fileName();
    if (!cleanName.startsWith("rc.") || isIgnoredHelperFile(cleanName)) {
        res.message = "Nombre de script de servicio inválido.";
        return res;
    }

    if (isCriticalSystemScript(cleanName) && !allowCriticalOverride) {
        res.message = QString("El script '%1' pertenece al núcleo de arranque de Slackware. "
                              "Activa 'Configuración Avanzada' si realmente necesitas modificarlo.")
                          .arg(cleanName);
        return res;
    }

    const QString fullPath = "/etc/rc.d/" + cleanName;
    if (!QFile::exists(fullPath)) {
        res.message = QString("No existe el archivo %1 en el sistema.").arg(fullPath);
        return res;
    }

    const QString modeFlag = enable ? "+x" : "-x";
    const QString cmd = QString("chmod %1 %2").arg(modeFlag, shellQuote(fullPath));

    QString out, err;
    if (!runAsRootWithPassword(cmd, rootPassword, out, err)) {
        res.message = err.isEmpty()
                          ? QString("No se pudo cambiar el permiso de arranque de %1.").arg(cleanName)
                          : err;
        return res;
    }

    LOG_SUCCESS("ServiceManager", QString("Permiso de arranque actualizado: chmod %1 %2").arg(modeFlag, fullPath));
    res.success = true;
    res.commandOutput = out;
    res.message = enable
                      ? QString("Servicio '%1' habilitado para iniciar automáticamente con el sistema (chmod +x).").arg(cleanName)
                      : QString("Servicio '%1' deshabilitado del arranque automático (chmod -x).").arg(cleanName);
    return res;
}

ServiceOperationResult ServiceManager::executeServiceAction(const QString &scriptName,
                                                            ServiceAction action,
                                                            const QString &rootPassword,
                                                            bool allowCriticalOverride)
{
    ServiceOperationResult res;
    const QString cleanName = QFileInfo(scriptName).fileName();
    if (!cleanName.startsWith("rc.") || isIgnoredHelperFile(cleanName)) {
        res.message = "Nombre de script de servicio inválido.";
        return res;
    }

    if (isCriticalSystemScript(cleanName) && !allowCriticalOverride) {
        res.message = QString("El script '%1' es un componente crítico del sistema base. "
                              "Activa 'Configuración Avanzada' si estás seguro de querer manipularlo.")
                          .arg(cleanName);
        return res;
    }

    const QString fullPath = "/etc/rc.d/" + cleanName;
    if (!QFile::exists(fullPath)) {
        res.message = QString("No existe el script %1.").arg(fullPath);
        return res;
    }

    QString actionWord = "restart";
    QString friendlyAction = "reiniciado";
    if (action == ServiceAction::Start) {
        actionWord = "start";
        friendlyAction = "iniciado";
    } else if (action == ServiceAction::Stop) {
        actionWord = "stop";
        friendlyAction = "detenido";
    }

    const QFileInfo fi(fullPath);
    const bool wasExecutable = fi.isExecutable();

    // En Slackware muchos scripts rc.* requieren +x incluso para ejecutarse manualmente
    QString cmd;
    if (!wasExecutable) {
        cmd = QString("chmod +x %1 && %1 %2; rc=$?; chmod -x %1; exit $rc")
                  .arg(shellQuote(fullPath), actionWord);
    } else {
        cmd = QString("%1 %2").arg(shellQuote(fullPath), actionWord);
    }

    QString out, err;
    if (!runAsRootWithPassword(cmd, rootPassword, out, err)) {
        res.message = err.isEmpty()
                          ? QString("Error al ejecutar '%1 %2'.").arg(cleanName, actionWord)
                          : err;
        res.commandOutput = out;
        return res;
    }

    LOG_SUCCESS("ServiceManager", QString("Acción ejecutada: %1 %2").arg(fullPath, actionWord));
    res.success = true;
    res.commandOutput = out;
    res.message = QString("El servicio '%1' ha sido %2 correctamente.").arg(cleanName, friendlyAction);
    return res;
}

QList<ServicePresetChange> ServiceManager::previewPreset(ServicePreset preset)
{
    QList<ServicePresetChange> changes;

    struct Rule {
        QString scriptName;
        bool enableOnBoot;
        bool startNow;
        QString reason;
    };

    QList<Rule> rules;
    if (preset == ServicePreset::DesktopLaptop) {
        rules = {
            { "rc.networkmanager", true,  true,  "Gestión automática de Wi-Fi y Ethernet en escritorio" },
            { "rc.bluetooth",      true,  false, "Soporte para periféricos y audio Bluetooth" },
            { "rc.cups",           true,  false, "Soporte para impresoras locales y en red" },
            { "rc.avahidaemon",    true,  false, "Descubrimiento automático de impresoras y dispositivos locales" },
            { "rc.ntpd",           true,  true,  "Sincronización automática de fecha y hora" },
            { "rc.httpd",          false, false, "Desactivar servidor web en arranque de escritorio" },
            { "rc.mysqld",         false, false, "Desactivar base de datos en arranque de escritorio" },
            { "rc.samba",          false, false, "Desactivar compartición SMB innecesaria en arranque" }
        };
    } else {
        rules = {
            { "rc.sshd",           true,  true,  "Acceso remoto seguro mediante SSH" },
            { "rc.ntpd",           true,  true,  "Hora precisa para certificados, logs y bases de datos" },
            { "rc.httpd",          true,  false, "Servidor web Apache listo para proyectos locales" },
            { "rc.mysqld",         true,  false, "Motor de base de datos MariaDB para desarrollo" },
            { "rc.bluetooth",      false, false, "Ahorrar recursos desactivando Bluetooth en servidor" }
        };
    }

    for (const Rule &r : rules) {
        const QString fullPath = "/etc/rc.d/" + r.scriptName;
        const QFileInfo fi(fullPath);
        if (!fi.exists()) {
            continue;
        }
        const bool currentlyBoot = fi.isExecutable();
        if (currentlyBoot != r.enableOnBoot) {
            const KnownServiceMetadata meta = lookupMetadata(r.scriptName);
            changes.append({
                r.scriptName,
                meta.displayName,
                r.enableOnBoot,
                r.startNow,
                r.reason
            });
        }
    }

    return changes;
}

ServiceOperationResult ServiceManager::applyPreset(ServicePreset preset, const QString &rootPassword)
{
    ServiceOperationResult res;
    const QList<ServicePresetChange> changes = previewPreset(preset);
    if (changes.isEmpty()) {
        res.success = true;
        res.message = "Todos los servicios del perfil seleccionado ya estaban configurados exactamente según lo recomendado.";
        return res;
    }

    QStringList commands;
    for (const ServicePresetChange &chg : changes) {
        const QString path = "/etc/rc.d/" + chg.scriptName;
        commands << QString("chmod %1 %2").arg(chg.enableOnBoot ? "+x" : "-x", shellQuote(path));
        if (chg.enableOnBoot && chg.startNow) {
            commands << QString("%1 start || true").arg(shellQuote(path));
        }
    }

    QString out, err;
    if (!runAsRootWithPassword(commands.join(" && "), rootPassword, out, err)) {
        res.message = err.isEmpty() ? "No se pudo aplicar el perfil seleccionado." : err;
        return res;
    }

    LOG_SUCCESS("ServiceManager", QString("Perfil de servicios aplicado (%1 cambios).").arg(changes.size()));
    res.success = true;
    res.commandOutput = out;
    res.message = QString("Perfil aplicado correctamente (%1 servicios actualizados).").arg(changes.size());
    return res;
}

QString ServiceManager::readServiceScript(const QString &scriptName)
{
    const QString cleanName = QFileInfo(scriptName).fileName();
    if (!cleanName.startsWith("rc.") || cleanName.contains("..")) {
        return "# Script inválido.";
    }

    QFile file("/etc/rc.d/" + cleanName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString("# No se pudo leer /etc/rc.d/%1").arg(cleanName);
    }

    return QString::fromUtf8(file.readAll());
}

QString ServiceManager::readLocalBootScript(bool shutdownScript)
{
    const QString path = shutdownScript ? "/etc/rc.d/rc.local_shutdown" : "/etc/rc.d/rc.local";
    QFile file(path);
    if (!file.exists()) {
        return QString("#!/bin/sh\n# %1 - Comandos personalizados del administrador\n\n").arg(path);
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString("# No se pudo abrir %1").arg(path);
    }
    return QString::fromUtf8(file.readAll());
}

ServiceOperationResult ServiceManager::saveLocalBootScript(const QString &content,
                                                           bool shutdownScript,
                                                           const QString &rootPassword)
{
    ServiceOperationResult res;
    const QString targetPath = shutdownScript ? "/etc/rc.d/rc.local_shutdown" : "/etc/rc.d/rc.local";
    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    const QString backupPath = QString("%1.bak.%2").arg(targetPath, stamp);

    QString normalized = content;
    if (!normalized.startsWith("#!")) {
        normalized.prepend("#!/bin/sh\n");
    }
    if (!normalized.endsWith('\n')) {
        normalized.append('\n');
    }

    const QString tempFile = QString("/tmp/slackwiz_rclocal_%1.tmp").arg(::getpid());
    QFile tf(tempFile);
    if (!tf.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        res.message = "No se pudo crear el archivo temporal de guardado.";
        return res;
    }
    tf.write(normalized.toUtf8());
    tf.close();

    const QString cmd = QString(
        "if [ -f %1 ]; then cp -p %1 %2; fi && "
        "install -m 0755 -o root -g root %3 %1 && "
        "rm -f %3"
    ).arg(shellQuote(targetPath), shellQuote(backupPath), shellQuote(tempFile));

    QString out, err;
    if (!runAsRootWithPassword(cmd, rootPassword, out, err)) {
        QFile::remove(tempFile);
        res.message = err.isEmpty()
                          ? QString("No se pudo guardar %1.").arg(targetPath)
                          : err;
        return res;
    }

    LOG_SUCCESS("ServiceManager", QString("Script local guardado con respaldo: %1 -> %2").arg(targetPath, backupPath));
    res.success = true;
    res.message = QString("Archivo %1 guardado y marcado como ejecutable (+x).\nRespaldo creado en: %2")
                      .arg(targetPath, backupPath);
    return res;
}

SambaDetectedResources ServiceManager::detectSambaResources()
{
    SambaDetectedResources info;
    info.smbConfExists = QFile::exists("/etc/samba/smb.conf");

    const QFileInfo cupsFi("/etc/rc.d/rc.cups");
    info.cupsAvailable = cupsFi.exists();
    if (info.cupsAvailable) {
        info.cupsRunning = checkServiceRunning("rc.cups", "/etc/rc.d/rc.cups", { "cupsd" });
    }

    // 1. Detectar impresoras configuradas en CUPS (lpstat -p)
    const CommandResult lpRes = runCommand("lpstat", { "-p" }, 2500);
    if (lpRes.exitCode == 0 && !lpRes.stdOut.trimmed().isEmpty()) {
        const QStringList lines = lpRes.stdOut.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            // Formato habitual: "printer HP_LaserJet is idle..." o "impresora HP_LaserJet..."
            const QStringList parts = line.trimmed().split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 2 && (parts[0] == "printer" || parts[0] == "impresora")) {
                info.detectedPrinters.append(parts[1]);
            }
        }
    }

    // 2. Detectar también impresoras USB conectadas directamente (/dev/usb/lp*)
    const QDir usbDevDir("/dev/usb");
    if (usbDevDir.exists()) {
        const QStringList usbLps = usbDevDir.entryList({ "lp*" }, QDir::System | QDir::Files);
        for (const QString &lp : usbLps) {
            const QString label = QString("Impresora USB directa (/dev/usb/%1)").arg(lp);
            if (!info.detectedPrinters.contains(label)) {
                info.detectedPrinters.append(label);
            }
        }
    }

    // 3. Detectar discos/particiones montadas no-sistema (/proc/mounts)
    QFile mountsFile("/proc/mounts");
    if (mountsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QStringList mountLines = QString::fromUtf8(mountsFile.readAll()).split('\n', Qt::SkipEmptyParts);
        for (const QString &line : mountLines) {
            const QStringList cols = line.split(' ', Qt::SkipEmptyParts);
            if (cols.size() < 3) {
                continue;
            }
            const QString dev = cols[0];
            QString mountPoint = cols[1];
            mountPoint.replace("\\040", " ");
            const QString fsType = cols[2];

            if (!dev.startsWith("/dev/")) {
                continue;
            }
            if (mountPoint == "/" || mountPoint.startsWith("/boot") ||
                mountPoint.startsWith("/sys") || mountPoint.startsWith("/proc") ||
                mountPoint.startsWith("/dev") || mountPoint.startsWith("/run/udev")) {
                continue;
            }

            info.detectedStorageMounts.append(
                QString("%1|%2 (%3, %4)").arg(mountPoint, mountPoint, dev, fsType)
            );
        }
    }

    return info;
}

ServiceOperationResult ServiceManager::generateBasicSambaConfig(const QString &workgroup,
                                                                bool sharePrinters,
                                                                const QList<SambaShareItem> &shares,
                                                                const QString &rootPassword)
{
    ServiceOperationResult res;
    const QString wg = workgroup.trimmed().isEmpty() ? "WORKGROUP" : workgroup.trimmed().toUpper();

    QStringList smbLines;
    smbLines << "# Generado por Slackware Wizard (slackwiz)";
    smbLines << "[global]";
    smbLines << QString("   workgroup = %1").arg(wg);
    smbLines << "   server string = Slackware Samba Server";
    smbLines << "   security = user";
    smbLines << "   map to guest = Bad User";
    smbLines << "   log file = /var/log/samba/log.%m";
    smbLines << "   max log size = 50";
    smbLines << "   dns proxy = no";

    if (sharePrinters) {
        smbLines << "   load printers = yes";
        smbLines << "   printing = cups";
        smbLines << "   printcap name = cups";
        smbLines << "";
        smbLines << "[printers]";
        smbLines << "   comment = Impresoras compartidas del sistema";
        smbLines << "   path = /var/spool/samba";
        smbLines << "   browseable = no";
        smbLines << "   guest ok = yes";
        smbLines << "   writable = no";
        smbLines << "   printable = yes";
    } else {
        smbLines << "   load printers = no";
    }

    for (const SambaShareItem &item : shares) {
        if (item.shareName.trimmed().isEmpty() || item.path.trimmed().isEmpty()) {
            continue;
        }
        smbLines << "";
        smbLines << QString("[%1]").arg(item.shareName.trimmed());
        smbLines << QString("   comment = Recurso compartido (%1)").arg(item.path.trimmed());
        smbLines << QString("   path = %1").arg(item.path.trimmed());
        smbLines << "   browseable = yes";
        smbLines << QString("   read only = %1").arg(item.readOnly ? "yes" : "no");
        smbLines << QString("   guest ok = %1").arg(item.guestOk ? "yes" : "no");
    }

    smbLines << "";

    const QString tempFile = QString("/tmp/slackwiz_smbconf_%1.tmp").arg(::getpid());
    QFile tf(tempFile);
    if (!tf.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        res.message = "No se pudo crear el archivo temporal para smb.conf.";
        return res;
    }
    tf.write(smbLines.join('\n').toUtf8());
    tf.close();

    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    const QString backupPath = QString("/etc/samba/smb.conf.bak.%1").arg(stamp);

    QString cmd = QString(
        "mkdir -p /etc/samba /var/spool/samba && chmod 1777 /var/spool/samba && "
        "if [ -f /etc/samba/smb.conf ]; then cp -p /etc/samba/smb.conf %1; fi && "
        "install -m 0644 -o root -g root %2 /etc/samba/smb.conf && "
        "rm -f %2"
    ).arg(shellQuote(backupPath), shellQuote(tempFile));

    if (sharePrinters && QFile::exists("/etc/rc.d/rc.cups")) {
        cmd += " && chmod +x /etc/rc.d/rc.cups && /etc/rc.d/rc.cups start || true";
    }

    QString out, err;
    if (!runAsRootWithPassword(cmd, rootPassword, out, err)) {
        QFile::remove(tempFile);
        res.message = err.isEmpty() ? "No se pudo guardar /etc/samba/smb.conf." : err;
        return res;
    }

    LOG_SUCCESS("ServiceManager", "Configuración de Samba guardada en /etc/samba/smb.conf");
    res.success = true;
    res.message = "Configuración /etc/samba/smb.conf generada correctamente. Recuerda crear una contraseña SMB para tu usuario con 'smbpasswd -a <usuario>'.";
    return res;
}

QString ServiceManager::getAdvancedModeWarning()
{
    return "ADVERTENCIA — ZONA DE CUIDADO (Modo Avanzado):\n\n"
           "Estás a punto de desbloquear todos los scripts de inicialización de Slackware (/etc/rc.d/rc.*), "
           "incluidos servicios críticos del sistema base (como rc.udev, rc.messagebus, rc.elogind o rc.inet1) "
           "y la edición directa de rc.local.\n\n"
           "Desactivar un script crítico del núcleo puede impedir que el entorno gráfico o la red arranquen "
           "en el próximo inicio.\n\n"
           "Por seguridad, se te solicitará tu contraseña de administrador (root) ahora mismo.";
}
