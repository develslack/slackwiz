#include <QApplication>
#include <QtGlobal>

#include "mainWindow.h"
#include "logger.h"
#include "commandRunner.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("slackwiz");
    app.setApplicationDisplayName("Slackware Wizard");
    app.setApplicationVersion("0.1.0");

    LOG_INFO("Main", "=== Iniciando Slackware Wizard (slackwiz) ===");
    LOG_INFO("Main", QString("Versión de Qt en tiempo de ejecución: %1").arg(qVersion()));
    LOG_INFO("Main", QString("Ejecutando como root: %1").arg(CommandRunner::isRoot() ? "SÍ" : "NO"));

    if (!CommandRunner::isRoot()) {
        LOG_WARN("Main", "La aplicación no se inició como root. Las escrituras en /etc fallarán hasta ejecutarla con privilegios.");
    }

    MainWindow window;
    window.show();

    int exitCode = app.exec();
    LOG_INFO("Main", QString("Finalizando aplicación con código de salida: %1").arg(exitCode));
    return exitCode;
}
