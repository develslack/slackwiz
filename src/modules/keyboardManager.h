#ifndef KEYBOARDMANAGER_H
#define KEYBOARDMANAGER_H

#include <QString>
#include <QList>

struct KeyboardLayoutOption {
    QString id;             // Identificador único interno
    QString displayName;    // Nombre amigable para el usuario
    QString consoleKeymap;  // Mapa para loadkeys en /etc/rc.d/rc.keymap
    QString x11Layout;      // Código para X11 y setxkbmap (ej: "es", "latam", "us", "br")
    QString x11Variant;     // Variante opcional X11 (ej: "intl", "" si no aplica)
};

struct KeyboardCurrentConfig {
    QString consoleKeymap;
    QString x11Layout;
    QString profileSetxkbmap;
};

struct KeyboardApplyResult {
    bool success = false;
    QString message;
};

class KeyboardManager
{
public:
    static QList<KeyboardLayoutOption> getAvailableLayouts();
    static KeyboardCurrentConfig getCurrentKeyboardConfig();
    static QString getFriendlyCurrentSummary();
    static KeyboardApplyResult applyKeyboardLayout(const QString &layoutId);

private:
    static bool updateRcKeymap(const QString &consoleKeymap);
    static bool updateXorgKeyboardConf(const QString &x11Layout, const QString &x11Variant);
    static bool updateEtcProfileSetxkbmap(const QString &x11Layout, const QString &x11Variant);
};

#endif // KEYBOARDMANAGER_H
