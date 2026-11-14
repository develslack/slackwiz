#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

#include "commandRunner.h"
#include "logger.h"
#include "localeManager.h"

QString LocaleManager::getCurrentSystemLocale()
{
    const QString langShPath = "/etc/profile.d/lang.sh";
    LOG_INFO("LocaleManager", QString("Leyendo idioma actual desde %1").arg(langShPath));

    QFile file(langShPath);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QRegularExpression regex(R"(^\s*(?:export\s+)?LANG\s*=\s*["']?([^"'\s#]+)["']?)");

        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith('#')) {
                continue;
            }

            QRegularExpressionMatch match = regex.match(line);
            if (match.hasMatch()) {
                const QString detected = match.captured(1).trimmed();
                file.close();
                LOG_SUCCESS("LocaleManager", QString("Idioma detectado en lang.sh: %1").arg(detected));
                return detected;
            }
        }
        file.close();
    } else {
        LOG_WARN("LocaleManager", QString("No se pudo abrir %1, consultando variable de entorno LANG").arg(langShPath));
    }

    const QString envLang = qEnvironmentVariable("LANG").trimmed();
    if (!envLang.isEmpty()) {
        LOG_INFO("LocaleManager", QString("Idioma detectado desde entorno LANG: %1").arg(envLang));
        return envLang;
    }

    LOG_WARN("LocaleManager", "No se detectó LANG explícito; asumiendo en_US.UTF-8 por defecto");
    return "en_US.UTF-8";
}

QString LocaleManager::getLocaleDisplayName(const QString &localeCode)
{
    const QList<LocaleOption> locales = getAvailableLocales();
    for (const auto &item : locales) {
        if (item.code.compare(localeCode, Qt::CaseInsensitive) == 0) {
            return QString("%1 (%2)").arg(item.displayName, item.code);
        }
    }
    return QString("Personalizado (%1)").arg(localeCode);
}

QList<LocaleOption> LocaleManager::getAvailableLocales()
{
    return {
        { "es_AR.UTF-8", "Español (Argentina)" },
        { "es_ES.UTF-8", "Español (España)" },
        { "es_MX.UTF-8", "Español (México)" },
        { "es_CL.UTF-8", "Español (Chile)" },
        { "es_CO.UTF-8", "Español (Colombia)" },
        { "es_UY.UTF-8", "Español (Uruguay)" },
        { "es_PE.UTF-8", "Español (Perú)" },
        { "en_US.UTF-8", "Inglés (Estados Unidos)" },
        { "en_GB.UTF-8", "Inglés (Reino Unido)" },
        { "pt_BR.UTF-8", "Portugués (Brasil)" },
        { "pt_PT.UTF-8", "Portugués (Portugal)" },
        { "fr_FR.UTF-8", "Francés (Francia)" },
        { "de_DE.UTF-8", "Alemán (Alemania)" },
        { "it_IT.UTF-8", "Italiano (Italia)" }
    };
}

LocaleApplyResult LocaleManager::applySystemLocale(const QString &localeCode)
{
    LocaleApplyResult result;
    const QString cleanCode = localeCode.trimmed();

    LOG_INFO("LocaleManager", QString("Solicitando cambio de idioma del sistema a: %1").arg(cleanCode));

    QRegularExpression validLocaleRegex(R"(^[a-zA-Z0-9_.\-@]+$)");
    if (cleanCode.isEmpty() || !validLocaleRegex.match(cleanCode).hasMatch()) {
        result.success = false;
        result.message = "El código de idioma seleccionado no es válido.";
        LOG_ERROR("LocaleManager", result.message);
        return result;
    }

    if (!CommandRunner::isRoot()) {
        result.success = false;
        result.message = "No tienes permisos de administrador (root). Ejecuta slackwiz como root para guardar el idioma.";
        LOG_ERROR("LocaleManager", result.message);
        return result;
    }

    const QString langShPath = "/etc/profile.d/lang.sh";
    const QString langShContent = QString(
        "#!/bin/sh\n"
        "# Configuración de idioma del sistema generada por Slackware Wizard (slackwiz)\n"
        "export LANG=%1\n"
        "export LC_COLLATE=C\n"
    ).arg(cleanCode);

    if (!CommandRunner::writeFileWithBackup(langShPath, langShContent, true)) {
        result.success = false;
        result.message = QString("No se pudo escribir la configuración en %1.").arg(langShPath);
        LOG_ERROR("LocaleManager", result.message);
        return result;
    }

    // En Slackware /etc/profile solo ejecuta scripts en /etc/profile.d/ si tienen permiso +x
    if (!CommandRunner::setExecutablePermission(langShPath, true)) {
        result.success = false;
        result.message = QString("Se escribió %1 pero falló la asignación de permisos de ejecución (+x).").arg(langShPath);
        LOG_ERROR("LocaleManager", result.message);
        return result;
    }

    // Actualizar también lang.csh para mantener coherencia en shells csh/tcsh
    const QString langCshPath = "/etc/profile.d/lang.csh";
    const QString langCshContent = QString(
        "#!/bin/csh\n"
        "# Configuración de idioma del sistema generada por Slackware Wizard (slackwiz)\n"
        "setenv LANG %1\n"
        "setenv LC_COLLATE C\n"
    ).arg(cleanCode);

    if (CommandRunner::writeFileWithBackup(langCshPath, langCshContent, true)) {
        CommandRunner::setExecutablePermission(langCshPath, true);
    } else {
        LOG_WARN("LocaleManager", QString("No se pudo actualizar %1 (no crítico)").arg(langCshPath));
    }

    result.success = true;
    result.message = QString(
        "El idioma del sistema (%1) fue guardado correctamente en %2.\n"
        "Los cambios se verán reflejados cuando reinicies el sistema luego de finalizar todos los pasos."
    ).arg(cleanCode, langShPath);

    LOG_SUCCESS("LocaleManager", QString("Idioma %1 aplicado correctamente.").arg(cleanCode));
    return result;
}
