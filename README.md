# slackwiz — Slackware Post-Installation Wizard

![Platform](https://img.shields.io/badge/OS-Slackware%2015.0%20%2F%20Current-2e3440?style=flat-square&logo=linux)
![Language](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus)
![Framework](https://img.shields.io/badge/GUI-Qt5%20(Widgets%20%7C%20Network)-41CD52?style=flat-square&logo=qt)
![Init System](https://img.shields.io/badge/Init-BSD%20Style%20(%2Fetc%2Frc.d)-1b5e20?style=flat-square)
![Version](https://img.shields.io/badge/Version-0.1.0-0d6efd?style=flat-square)

**slackwiz** (*Slackware Wizard*) es un asistente gráfico de post-instalación nativo para **Slackware Linux**, desarrollado en **C++17** y **Qt5**. Su objetivo es guiar al usuario paso a paso en la configuración inicial del sistema operativo tras una instalación limpia, evitando la edición manual de archivos en `/etc` sin alterar ni abstraer la arquitectura tradicional de Slackware.

---

## Tabla de Contenidos

1. [¿De qué se trata el proyecto?](#1-de-qué-se-trata-el-proyecto)
2. [Público objetivo y cometido](#2-público-objetivo-y-cometido)
3. [Módulos incluidos (0 al 8)](#3-módulos-incluidos-0-al-8)
4. [Arquitectura técnica (Guía para desarrolladores)](#4-arquitectura-técnica-guía-para-desarrolladores)
5. [Estructura del proyecto](#5-estructura-del-proyecto)
6. [Requisitos y dependencias](#6-requisitos-y-dependencias)
7. [Compilación y uso del script `build.sh`](#7-compilación-y-uso-del-script-buildsh)
8. [Seguridad e integridad del sistema](#8-seguridad-e-integridad-del-sistema)
9. [Guía para añadir nuevos módulos](#9-guía-para-añadir-nuevos-módulos)

---

## 1. ¿De qué se trata el proyecto?

A diferencia de otras distribuciones Linux que incorporan asistentes de primer arranque o herramientas centralizadas de administración, Slackware delega la configuración post-instalación en la edición directa de scripts y archivos planos (`/etc/profile.d/lang.sh`, `/etc/rc.d/rc.keymap`, `/etc/sudoers`, `/etc/httpd/httpd.conf`, `/etc/rc.d/rc.mysqld`, `/etc/localtime`, entre otros).

**slackwiz** encapsula estas tareas administrativas en una interfaz gráfica unificada, ligera y sin demonios residentes en segundo plano. La aplicación lee el estado real del sistema en tiempo real, valida las entradas del usuario, aplica los cambios directamente sobre los archivos nativos de Slackware y verifica los resultados (por ejemplo, probando peticiones HTTP locales o sesiones SQL autenticadas).

---

## 2. Público objetivo y cometido

### ¿A quién está dirigido?
* **Usuarios provenientes de distribuciones como Ubuntu, Linux Mint, Fedora o Debian:** Quienes desean dar el salto a Slackware y buscan una experiencia de configuración inicial familiar, guiada y visual para tareas esenciales (idioma, teclado, usuarios, permisos `sudo` y hora).
* **Desarrolladores y administradores de sistemas (SysAdmins):** Quienes despliegan estaciones de trabajo o servidores LAMP/LEMP/PostgreSQL sobre Slackware y desean automatizar la puesta a punto inicial (servicios `rc.d`, ajustes de `php.ini`, hardening de bases de datos) en segundos.
* **Entornos educativos y de laboratorio:** Instalaciones de prueba o máquinas virtuales donde se requiere dejar un entorno funcional de forma rápida y reproducible.

### Cometido principal
* **Reducir la curva de entrada** sin sacrificar la transparencia ni el control que caracterizan a Slackware.
* **Respetar al 100 % las convenciones de Slackware:** No introduce capas de abstracción propietarias ni archivos de estado paralelos; todo cambio realizado por `slackwiz` es equivalente a haber configurado el sistema manualmente desde consola y puede seguir manteniéndose a mano en el futuro.
* **Prevenir errores de sintaxis críticos:** Mediante validaciones previas y comprobaciones automatizadas antes de persistir cambios en archivos sensibles del sistema.

---

## 3. Módulos incluidos (0 al 8)

El asistente organiza el flujo de configuración en **9 secciones secuenciales** accesibles desde un panel lateral de navegación o mediante los botones *Anterior / Siguiente*:

| Índice | Módulo | Funciones principales | Archivos / Servicios afectados |
| :--- | :--- | :--- | :--- |
| **0** | **Bienvenida** | Detección de versión de Slackware, verificación de privilegios `root` y resumen general del entorno. | `/etc/slackware-version` |
| **1** | **Idioma del sistema** | Selección de locale UTF-8 para consola y entorno gráfico. | `/etc/profile.d/lang.sh` |
| **2** | **Teclado** | Configuración de mapa de teclas para X11 y consolas virtuales, con campo de prueba en vivo. | `setxkbmap`, `/etc/rc.d/rc.keymap`, `/etc/X11/xorg.conf.d/` |
| **3** | **Añadir usuario** | Creación de cuentas estándar con carpeta `/home`, shell por defecto y asignación de grupos de hardware/sistema. | `useradd`, `chpasswd`, `/etc/passwd`, `/etc/shadow`, `/etc/group` |
| **4** | **Grupos de usuario** | Administración granular de grupos secundarios (`wheel`, `audio`, `video`, `plugdev`, `netdev`, etc.) para usuarios existentes. | `usermod`, `/etc/group` |
| **5** | **Configurar sudo** | Habilitación de `sudo` por usuario (con contraseña o `NOPASSWD`) y sincronización opcional con el grupo `wheel`. | `/etc/sudoers`, `/etc/sudoers.d/`, `visudo` |
| **6** | **Servidor web** | Detección de Apache/Nginx y PHP, ajuste de `php.ini` (`date.timezone`, límites de memoria/subida), arranque en boot y **navegador embebido** para verificar `http://localhost/` y `php-info.php`. | `/etc/httpd/httpd.conf`, `/etc/nginx/nginx.conf`, `/etc/php.ini`, `/etc/rc.d/rc.httpd`, `/etc/rc.d/rc.nginx` |
| **7** | **Base de datos** | Inicialización de clústeres de datos, hardening de seguridad (equivalente a `mariadb-secure-installation` o `scram-sha-256` en PostgreSQL), **prueba interactiva de sesión** y arranque automático. | `/var/lib/mysql`, `/var/lib/pgsql`, `pg_hba.conf`, `postgresql.conf`, `/etc/rc.d/rc.mysqld`, `/etc/rc.d/rc.postgresql` |
| **8** | **Hora del sistema** | Selección de zona horaria mediante **mapa mundial interactivo** o buscador, control de horario de verano (DST), reloj de hardware (UTC/localtime) y sincronización NTP. | `/etc/localtime`, `/usr/share/zoneinfo/zone1970.tab`, `/etc/hardwareclock`, `/etc/ntp.conf`, `/etc/rc.d/rc.ntpd` |

---

## 4. Arquitectura técnica (Guía para desarrolladores)

El diseño de **slackwiz** sigue una arquitectura **estrictamente desacoplada** en dos capas: **Capa de Presentación (UI)** y **Capa de Gestión del Sistema (Managers)**.

### 4.1. Desacoplamiento UI / Lógica de Sistema
* **`MainWindow` (`mainWindow.h` / `mainWindow.cpp`)**: Actúa exclusivamente como orquestador visual. Construye dinámicamente cada página dentro de un `QStackedWidget` (usando `QScrollArea` en formularios extensos para garantizar adaptabilidad en resoluciones desde `900x620`), gestiona las señales de Qt y renderiza la retroalimentación visual. **No contiene llamadas directas a archivos de sistema ni comandos de shell.**
* **Clases `*Manager` (`localeManager`, `keyboardManager`, `userManager`, `groupManager`, `sudoManager`, `webServerManager`, `databaseManager`, `timeManager`)**: Son módulos autocontenidos que exponen métodos estáticos y estructuras de transferencia de datos (DTOs) fuertemente tipadas:
  * **Estructuras de consulta:** `KeyboardCurrentConfig`, `UserGroupsInfo`, `UserSudoStatus`, `WebServerDetectionInfo`, `DatabaseDetectionInfo`, `TimeSystemState`.
  * **Estructuras de petición y resultado:** `NewUserRequest`, `WebServerApplyRequest`, `DatabaseConfigRequest`, `TimeConfigRequest`, y sus respectivos `*Result` (`bool success`, `QString message`, `QString output`).

### 4.2. Ejecución segura de procesos y control de privilegios (`CommandRunner`)
Para interactuar con las utilidades del sistema operativo (`useradd`, `usermod`, `visudo`, `apachectl`, `nginx`, `mariadb`, `psql`, `hwclock`, `ntpdate`, scripts `/etc/rc.d/rc.*`), la aplicación centraliza toda ejecución externa en `CommandRunner`:
* **Prevención de inyección de comandos (Command Injection):** Los binarios se invocan mediante `QProcess` pasando el ejecutable y sus argumentos como un `QStringList` explícito, evitando el uso inseguro de `system()` o interpretaciones intermedias de `/bin/sh -c`.
* **Modo Administrador vs. Modo Vista Previa (Sin Root):** Al iniciar, `CommandRunner::isRoot()` evalúa `geteuid() == 0`. Si el proceso se ejecuta sin privilegios de superusuario, la interfaz lo informa en la barra de estado y permite navegar e inspeccionar la configuración actual del sistema en modo lectura sin comprometer la integridad de `/etc`.

### 4.3. Trazabilidad y depuración (`Logger`)
El subsistema `Logger` provee macros globales (`LOG_INFO`, `LOG_SUCCESS`, `LOG_WARN`, `LOG_ERROR`) que registran cada transición de módulo, lectura de configuración y operación ejecutada.
Cuando se compila con la directiva de preprocesador `SLACKWIZ_DEBUG`, la ventana principal habilita el indicador `[COMPILACIÓN DEBUG]` en la barra de estado y emite trazas detalladas para diagnóstico durante el desarrollo.

### 4.4. Subsistema de verificación HTTP embebido (Módulo 6)
Tras aplicar cambios en Apache o Nginx y reiniciar el servicio, `MainWindow` utiliza una instancia de `QNetworkAccessManager` (`m_networkManager`) junto con un `QTextBrowser` (`m_webPreviewBrowser`) para realizar peticiones reales `GET` contra `http://localhost/` y `http://localhost/php-info.php`. Esto permite validar dentro de la misma ventana:
* El código de estado HTTP devuelto (`200 OK`, `403`, `500`, o fallo de conexión).
* El renderizado HTML real y la interpretación efectiva del módulo/FPM de PHP sin depender de un navegador externo.

### 4.5. Proyección cartográfica y gestión horaria (Módulo 8)
El Módulo 8 combina tres componentes especializados:
1. **`TimeManager::loadSystemTimezones()`**: Analiza directamente la base de datos oficial `tzdata` instalada en Slackware (`/usr/share/zoneinfo/zone1970.tab`), extrayendo el identificador IANA (`America/Argentina/Buenos_Aires`, `Europe/Madrid`, etc.) y parseando las coordenadas geográficas en formato **ISO 6709** (`±DDMM±DDDMM` o `±DDMMSS±DDDMMSS`) a grados decimales (`lat`, `lon`).
2. **`TimezoneMapWidget` (`timezoneMapWidget.h` / `timezoneMapWidget.cpp`)**: Widget personalizado basado en `QPainter` que proyecta las coordenadas geográficas sobre un mapamundi equirectangular, dibuja los husos horarios y ciudades disponibles, calcula la distancia euclidiana al cursor en eventos `mousePressEvent` y emite la señal `timezoneSelected(const QString &)`.
3. **Sincronización bidireccional y vista previa en vivo**: La selección en el mapa actualiza automáticamente el `QComboBox` (equipado con `QCompleter` con filtrado insensible a mayúsculas) y viceversa. Un `QTimer` de 1000 ms consulta `QTimeZone` para calcular el desplazamiento UTC resultante (`standardTimeOffset` + ajuste DST opcional) y mostrar la hora exacta antes de escribir `/etc/localtime`, `/etc/hardwareclock` y `/etc/rc.d/rc.ntpd`.

---

## 5. Estructura del proyecto

```text
slackwiz/
├── build.sh                            # Script automatizado de compilación, limpieza e instalación
├── CMakeLists.txt                      # Configuración de compilación con CMake (Qt5::Widgets, Qt5::Network)
├── slackwiz.pro                        # Archivo de proyecto alternativo para qmake
├── README.md                           # Documentación técnica y de usuario
└── src/
    ├── main.cpp                        # Punto de entrada de la aplicación Qt (QApplication)
    ├── ui/mainWindow.h                 # Declaración de la ventana principal y slots de los 9 módulos
    ├── ui/mainWindow.cpp               # Construcción de UI, navegación y enlace con los Managers
    ├── core/commandRunner.h            # Interfaz de ejecución segura de comandos y detección de entorno
    ├── core/commandRunner.cpp          # Implementación basada en QProcess y verificación geteuid()
    ├── core/logger.h                   # Macros y niveles de registro de eventos (LOG_INFO, etc.)
    ├── core/logger.cpp                 # Salida formateada de logs de ejecución y depuración
    ├── modules/localeManager.h         # Estructuras y API del Módulo 1 (Idioma)
    ├── modules/localeManager.cpp       # Lectura/escritura de /etc/profile.d/lang.sh
    ├── modules/keyboardManager.h       # Estructuras y API del Módulo 2 (Teclado)
    ├── modules/keyboardManager.cpp     # Gestión de setxkbmap, rc.keymap y xorg.conf.d
    ├── modules/userManager.h           # Estructuras y API del Módulo 3 (Creación de usuarios)
    ├── modules/userManager.cpp         # Validación y ejecución de useradd / chpasswd
    ├── modules/groupManager.h          # Estructuras y API del Módulo 4 (Grupos secundarios)
    ├── modules/groupManager.cpp        # Consulta de /etc/group y actualización vía usermod
    ├── modules/sudoManager.h           # Estructuras y API del Módulo 5 (Privilegios sudo)
    ├── modules/sudoManager.cpp         # Gestión segura de /etc/sudoers.d/ y grupo wheel
    ├── modules/webServerManager.h      # Estructuras y API del Módulo 6 (Apache/Nginx + PHP)
    ├── modules/webServerManager.cpp    # Configuración de virtualhosts, php.ini y control de rc.d
    ├── modules/databaseManager.h       # Estructuras y API del Módulo 7 (MariaDB / PostgreSQL)
    ├── modules/databaseManager.cpp     # Inicialización, hardening, prueba SQL y control de rc.d
    ├── modules/timeManager.h           # Estructuras y API del Módulo 8 (Fecha, Hora y Zona Horaria)
    ├── modules/timeManager.cpp         # Parser de zone1970.tab, /etc/localtime, hwclock y rc.ntpd
    ├── ui/timezoneMapWidget.h          # Widget gráfico interactivo del mapamundi
    └── ui/timezoneMapWidget.cpp        # Renderizado QPainter y selección por proximidad geográfica
    ├── ui/timeConfigWidget.h           # Widget gráfico soporte para timezoneMapWidget
    └── ui/timeConfigWidget.cpp         # Widget gráfico soporte para timezoneMapWidget
```

---

## 6. Requisitos y dependencias

En una **instalación completa (Full Install) de Slackware 15.0 o Slackware -current**, todas las dependencias de compilación y ejecución ya vienen incluidas de fábrica:

* **Compilador:** `gcc` / `g++` con soporte para **C++17** (GCC 11.2+ en Slackware 15.0).
* **Herramientas de construcción:** `cmake` (>= 3.16) o `qmake` (Qt5), y `make`.
* **Bibliotecas Qt5:**
  * `Qt5Core`
  * `Qt5Gui`
  * `Qt5Widgets`
  * `Qt5Network`
* **Utilidades base del sistema:** `shadow` (`useradd`, `usermod`, `chpasswd`), `sudo`, `glibc-zoneinfo` (`/usr/share/zoneinfo`), `util-linux` (`hwclock`), `ntp` (`ntpdate`, `rc.ntpd`).
* **Servicios opcionales gestionados por los módulos 6 y 7:** `httpd` (Apache), `nginx` (SlackBuilds), `php`, `mariadb`, `postgresql` (SlackBuilds).

---

## 7. Compilación y uso del script `build.sh`

El proyecto incluye el script **`build.sh`** en la raíz del repositorio para estandarizar y simplificar todo el ciclo de compilación sin tener que invocar manualmente `cmake` o `qmake`.

### ¿Qué hace internamente `build.sh`?
1. **Verificación de entorno:** Comprueba la disponibilidad de `g++`, `cmake` (o `qmake-qt5`) y los encabezados de desarrollo de Qt5.
2. **Aislamiento de artefactos (`out-of-source build`):** Crea y gestiona el directorio `build/` para mantener el árbol de código fuente (`src/`) completamente limpio.
3. **Configuración de perfil:** Inyecta los flags de optimización (`-O2` en modo *Release*) o activa la macro `-DSLACKWIZ_DEBUG` junto con símbolos de depuración (`-g`) cuando se solicita una compilación de desarrollo.
4. **Compilación paralela:** Detecta automáticamente el número de núcleos lógicos del procesador mediante `nproc` (`make -j$(nproc)`) para acelerar la construcción.
5. **Instalación opcional en el sistema:** Permite copiar el binario resultante en `/usr/local/bin/slackwiz` con los permisos adecuados (`0755`).

### Modos de ejecución de `build.sh`

Primero, asegúrate de que el script tenga permisos de ejecución:

```bash
chmod +x build.sh
```

#### 0. Ayuda de build.sh
Antes de lanzar el compilador revise las opciones de ejcución de `build.sh`:
```bash
./build.sh --help
```

#### 1. Compilación estándar (Release)
Genera el binario optimizado en `build/slackwiz`:
```bash
./build.sh
```

#### 2. Compilación en modo depuración (Debug)
Habilita logs extendidos y muestra la etiqueta `[COMPILACIÓN DEBUG]` en la interfaz:
```bash
./build.sh --debug
```

#### 3. Compilación en modo depuración (Debug) con permisos de root
Habilita logs extendidos y muestra la etiqueta `[COMPILACIÓN DEBUG]` en la interfaz con permisos de root (PRECAUCION):
```bash
./build.sh --debug --root
```

#### 4. Limpiar archivos de compilación previos y recompilar desde cero
Elimina el directorio `build/` antes de iniciar una nueva compilación limpia:
```bash
./build.sh --clean
```

#### 5. Compilación en modo depuración (Debug) con permisos de root y recompilación desde cero (CLEAN)
Habilita logs extendidos y muestra la etiqueta `[COMPILACIÓN DEBUG]` en la interfaz con permisos de root (PRECAUCION):
```bash
./build.sh --clean --debug --root
```

#### 6. Instalar el binario en el sistema (`/usr/local/bin/slackwiz`)
Compila (si es necesario) e instala el ejecutable globalmente (requiere permisos de `root`):
```bash
su -c "./build.sh --install"
```

### Ejecución de la aplicación

* **Modo completo (recomendado — aplica cambios en el sistema):**
  ```bash
  su -
  ./build/slackwiz
  ```
  *(O bien, si tienes sesión gráfica iniciada como usuario estándar y deseas conservar el acceso al servidor X11/Wayland)*:
  ```bash
  sudo -E ./build/slackwiz
  ```
  *(En entornos X11 tradicionales sin `sudo` configurado aún, puedes autorizar temporalmente al usuario `root` local con `xhost +SI:localuser:root` antes de ejecutar `su -c "./build/slackwiz"`).*

* **Modo vista previa (sin privilegios de `root`):**
  ```bash
  ./build/slackwiz
  ```
  Permite inspeccionar el estado actual de todos los módulos sin modificar archivos del sistema.

---

## 8. Seguridad e integridad del sistema

* **Respaldos automáticos:** Antes de modificar archivos críticos como `/etc/sudoers`, `/etc/httpd/httpd.conf`, `/etc/nginx/nginx.conf`, `/etc/php.ini` o `pg_hba.conf`, los managers generan copias de respaldo con marca temporal o extensión `.bak`.
* **Validación de `sudoers`:** Toda regla generada por `SudoManager` es verificada sintácticamente con `visudo -c` antes de hacerse efectiva, evitando bloquear el acceso administrativo por un error de formato.
* **Control de arranque al estilo BSD:** La habilitación o deshabilitación de servicios en el arranque de Slackware se realiza exclusivamente gestionando el bit de ejecución (`chmod +x` / `chmod -x`) sobre los scripts oficiales en `/etc/rc.d/rc.*`, respetando la documentación oficial de la distribución.

---

## 9. Guía para añadir nuevos módulos

Si deseas contribuir añadiendo un nuevo módulo (por ejemplo, **Módulo 9: Configuración de Red** o **Módulo 10: Mirrors de slackpkg**):

1. **Crea el par `src/<nombre>Manager.h` y `src/<nombre>Manager.cpp`:** Define tus estructuras de datos (`*Info`, `*Request`, `*Result`) e implementa toda la lógica de lectura/escritura apoyándote en `CommandRunner` y `Logger`.
2. **Registra los nuevos archivos fuente** en `CMakeLists.txt` (y `slackwiz.pro`).
3. **Integra la interfaz en `MainWindow`:**
   * Añade la entrada correspondiente en `MainWindow::setupNavigationMenu()`.
   * Crea el método constructor `QWidget* MainWindow::create<Nombre>Page()` siguiendo el diseño de tarjetas (`QFrame` con bordes `#d0d7de` / `#b6d4fe` y botones de acción `#2e7d32`).
   * Añade el widget en `MainWindow::setupPages()` y su actualización de estado en `MainWindow::onNavigationChanged(int index)`.
