QT += core gui widgets

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
    src/ui/mainWindow.cpp

# Archivos de cabecera
HEADERS += \
    src/core/logger.h \
    src/core/commandRunner.h \
    src/ui/mainWindow.h

# Reglas de instalación (make install)
target.path = /usr/local/bin
desktop.path = /usr/local/share/applications
desktop.files = resources/slackwiz.desktop

INSTALLS += target desktop
