QT += core gui widgets network

CONFIG += c++17
TARGET = slackwiz
TEMPLATE = app

# Directorios de salida para mantener limpio el directorio raíz
DESTDIR = build
OBJECTS_DIR = build/.obj
MOC_DIR = build/.moc
UI_DIR = build/.ui
RCC_DIR = build/.rcc

# Diferenciación entre compilación --debug y --release
CONFIG(debug, debug|release) {
    DEFINES += SLACKWIZ_DEBUG
} else {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# Rutas de cabeceras
INCLUDEPATH += \
    src \
    src/core \
    src/ui \
    src/modules

# Archivos fuente
SOURCES += \
    src/main.cpp \
    src/core/logger.cpp \
    src/core/commandRunner.cpp \
    src/modules/localemanager.cpp \
    src/modules/keyboardManager.cpp \
    src/modules/userManager.cpp \
    src/modules/groupManager.cpp \
    src/modules/sudoManager.cpp \
    src/modules/webServerManager.cpp \
    src/modules/databaseManager.cpp \
    src/modules/timeManager.cpp \
    src/modules/ServiceManager.cpp \
    src/modules/DeviceManager.cpp \
    src/ui/timezoneMapWidget.cpp \
    src/ui/timeConfigWidget.cpp \
    src/ui/mainWindow.cpp

# Archivos de cabecera
HEADERS += \
    src/core/logger.h \
    src/core/commandRunner.h \
    src/modules/localeManager.h \
    src/modules/keyboardManager.h \
    src/modules/userManager.h \
    src/modules/groupManager.h \
    src/modules/sudoManager.h \
    src/modules/webServerManager.h \
    src/modules/databaseManager.h \
    src/modules/timeManager.h \
    src/modules/ServiceManager.h \
    src/modules/DeviceManager.h \
    src/ui/timezoneMapWidget.h \
    src/ui/timeConfigWidget.h \
    src/ui/mainWindow.h

# Reglas de instalación (make install)
target.path = /usr/local/bin/slackwiz
desktop.path = /usr/local/share/applications
desktop.files = resources/slackwiz.desktop

INSTALLS += target desktop
