#ifndef TIMECONFIGWIDGET_H
#define TIMECONFIGWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QDateEdit>
#include <QTimeEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

#include "timezoneMapWidget.h"
#include "timeManager.h"

class TimeConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimeConfigWidget(QWidget *parent = nullptr);
    void refreshFromSystem();

private slots:
    void onMapTimezoneSelected(const QString &timezoneId);
    void onComboTimezoneChanged(int index);
    void onDstToggled(bool checked);
    void onSyncModeChanged();
    void updateLiveClockPreview();
    void onApplyClicked();

private:
    void setupUi();
    void showStatusBanner(bool success, const QString &message);

    TimezoneMapWidget *m_mapWidget = nullptr;
    QComboBox *m_zoneCombo = nullptr;
    QLabel *m_zoneInfoLabel = nullptr;
    QLabel *m_liveClockLabel = nullptr;
    QCheckBox *m_dstCheck = nullptr;

    QRadioButton *m_autoNtpRadio = nullptr;
    QRadioButton *m_manualTimeRadio = nullptr;
    QLineEdit *m_ntpServerEdit = nullptr;
    QDateEdit *m_dateEdit = nullptr;
    QTimeEdit *m_timeEdit = nullptr;

    QCheckBox *m_hwClockUtcCheck = nullptr;
    QPushButton *m_applyBtn = nullptr;
    QLabel *m_statusLabel = nullptr;

    QTimer *m_clockTimer = nullptr;
    QList<TimezoneEntry> m_zones;
    bool m_syncingSelection = false;
};

#endif // TIMECONFIGWIDGET_H
