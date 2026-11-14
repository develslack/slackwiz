#include "timeConfigWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCompleter>
#include <QTimeZone>
#include <QSignalBlocker>

TimeConfigWidget::TimeConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();

    m_zones = TimeManager::loadSystemTimezones();
    m_mapWidget->setTimezones(m_zones);

    m_zoneCombo->clear();
    for (const TimezoneEntry &z : m_zones) {
        m_zoneCombo->addItem(z.id, z.id);
    }

    // Permitir escribir para filtrar zonas rápidamente (ej: "Buenos_Aires")
    if (m_zoneCombo->completer()) {
        m_zoneCombo->completer()->setFilterMode(Qt::MatchContains);
        m_zoneCombo->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    }

    refreshFromSystem();

    m_clockTimer = new QTimer(this);
    connect(m_clockTimer, &QTimer::timeout, this, &TimeConfigWidget::updateLiveClockPreview);
    m_clockTimer->start(1000);
}

void TimeConfigWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    // 1. Mapa mundial interactivo + selector desplegable
    QGroupBox *zoneGroup = new QGroupBox("1. Región y Zona Horaria (haz clic en el mapa o elige en la lista):", this);
    QVBoxLayout *zoneLayout = new QVBoxLayout(zoneGroup);

    m_mapWidget = new TimezoneMapWidget(this);
    zoneLayout->addWidget(m_mapWidget);

    QFormLayout *comboForm = new QFormLayout();
    m_zoneCombo = new QComboBox(this);
    m_zoneCombo->setEditable(true);
    m_zoneCombo->setInsertPolicy(QComboBox::NoInsert);
    m_zoneCombo->setMinimumWidth(340);
    comboForm->addRow("Zona horaria seleccionada:", m_zoneCombo);

    m_dstCheck = new QCheckBox("Activar horario de verano (+1 hora / DST)", this);
    comboForm->addRow("", m_dstCheck);

    m_zoneInfoLabel = new QLabel(this);
    m_zoneInfoLabel->setStyleSheet("color: #1b4965; font-weight: bold;");
    comboForm->addRow("Desplazamiento actual:", m_zoneInfoLabel);

    m_liveClockLabel = new QLabel(this);
    m_liveClockLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #2b2d42;");
    comboForm->addRow("Vista previa de hora local:", m_liveClockLabel);

    zoneLayout->addLayout(comboForm);
    mainLayout->addWidget(zoneGroup);

    // 2. Sincronización automática mundial (NTP) o ajuste manual
    QGroupBox *syncGroup = new QGroupBox("2. Ajuste de Fecha y Hora del Sistema:", this);
    QVBoxLayout *syncLayout = new QVBoxLayout(syncGroup);

    m_autoNtpRadio = new QRadioButton("Hora automática mundial por Internet (NTP - Recomendado)", this);
    m_manualTimeRadio = new QRadioButton("Configurar fecha y hora manualmente", this);
    m_autoNtpRadio->setChecked(true);

    syncLayout->addWidget(m_autoNtpRadio);

    QHBoxLayout *ntpRow = new QHBoxLayout();
    ntpRow->setContentsMargins(24, 0, 0, 4);
    ntpRow->addWidget(new QLabel("Servidor NTP:", this));
    m_ntpServerEdit = new QLineEdit("pool.ntp.org", this);
    m_ntpServerEdit->setMaximumWidth(260);
    ntpRow->addWidget(m_ntpServerEdit);
    ntpRow->addStretch();
    syncLayout->addLayout(ntpRow);

    syncLayout->addWidget(m_manualTimeRadio);

    QHBoxLayout *manualRow = new QHBoxLayout();
    manualRow->setContentsMargins(24, 0, 0, 4);
    manualRow->addWidget(new QLabel("Fecha:", this));
    m_dateEdit = new QDateEdit(QDate::currentDate(), this);
    m_dateEdit->setDisplayFormat("dd/MM/yyyy");
    m_dateEdit->setCalendarPopup(true);
    manualRow->addWidget(m_dateEdit);

    manualRow->addSpacing(16);
    manualRow->addWidget(new QLabel("Hora:", this));
    m_timeEdit = new QTimeEdit(QTime::currentTime(), this);
    m_timeEdit->setDisplayFormat("HH:mm:ss");
    manualRow->addWidget(m_timeEdit);
    manualRow->addStretch();
    syncLayout->addLayout(manualRow);

    m_hwClockUtcCheck = new QCheckBox("Mantener reloj de hardware (BIOS/RTC) en UTC (desmarcar si usas dual-boot con Windows)", this);
    m_hwClockUtcCheck->setChecked(true);
    syncLayout->addWidget(m_hwClockUtcCheck);

    mainLayout->addWidget(syncGroup);

    // 3. Botón de aplicación y banner de estado (mismo estilo que el resto de slackwiz)
    QHBoxLayout *btnRow = new QHBoxLayout();
    m_applyBtn = new QPushButton("Aplicar configuración horaria", this);
    m_applyBtn->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; "
        "padding: 8px 18px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
    );
    btnRow->addWidget(m_applyBtn);
    btnRow->addStretch();
    mainLayout->addLayout(btnRow);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setVisible(false);
    mainLayout->addWidget(m_statusLabel);

    // Conexiones
    connect(m_mapWidget, &TimezoneMapWidget::timezoneSelected, this, &TimeConfigWidget::onMapTimezoneSelected);
    connect(m_zoneCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TimeConfigWidget::onComboTimezoneChanged);
    connect(m_dstCheck, &QCheckBox::toggled, this, &TimeConfigWidget::onDstToggled);
    connect(m_autoNtpRadio, &QRadioButton::toggled, this, &TimeConfigWidget::onSyncModeChanged);
    connect(m_manualTimeRadio, &QRadioButton::toggled, this, &TimeConfigWidget::onSyncModeChanged);
    connect(m_applyBtn, &QPushButton::clicked, this, &TimeConfigWidget::onApplyClicked);

    onSyncModeChanged();
}

void TimeConfigWidget::refreshFromSystem()
{
    const TimeSystemState state = TimeManager::detectCurrentState();

    m_syncingSelection = true;
    int idx = m_zoneCombo->findData(state.currentTimezone);
    if (idx < 0) {
        idx = m_zoneCombo->findText(state.currentTimezone);
    }
    if (idx >= 0) {
        m_zoneCombo->setCurrentIndex(idx);
    }
    m_mapWidget->setSelectedTimezone(state.currentTimezone);
    m_syncingSelection = false;

    {
        const QSignalBlocker blocker(m_dstCheck);
        m_dstCheck->setChecked(state.dstActive);
    }

    m_autoNtpRadio->setChecked(state.ntpEnabled);
    m_manualTimeRadio->setChecked(!state.ntpEnabled);
    m_hwClockUtcCheck->setChecked(state.hardwareClockUtc);

    m_dateEdit->setDate(QDate::currentDate());
    m_timeEdit->setTime(QTime::currentTime());

    onSyncModeChanged();
    updateLiveClockPreview();
}

void TimeConfigWidget::onMapTimezoneSelected(const QString &timezoneId)
{
    if (m_syncingSelection) {
        return;
    }

    m_syncingSelection = true;
    const int idx = m_zoneCombo->findData(timezoneId);
    if (idx >= 0) {
        m_zoneCombo->setCurrentIndex(idx);
    }
    m_syncingSelection = false;

    // Actualizar sugerencia automática de DST según la región seleccionada
    const QTimeZone tz(timezoneId.toUtf8());
    if (tz.isValid()) {
        const QSignalBlocker blocker(m_dstCheck);
        m_dstCheck->setChecked(tz.isDaylightTime(QDateTime::currentDateTimeUtc()));
    }

    updateLiveClockPreview();
}

void TimeConfigWidget::onComboTimezoneChanged(int index)
{
    if (m_syncingSelection || index < 0) {
        return;
    }

    const QString timezoneId = m_zoneCombo->itemData(index).toString();
    if (timezoneId.isEmpty()) {
        return;
    }

    m_syncingSelection = true;
    m_mapWidget->setSelectedTimezone(timezoneId);
    m_syncingSelection = false;

    const QTimeZone tz(timezoneId.toUtf8());
    if (tz.isValid()) {
        const QSignalBlocker blocker(m_dstCheck);
        m_dstCheck->setChecked(tz.isDaylightTime(QDateTime::currentDateTimeUtc()));
    }

    updateLiveClockPreview();
}

void TimeConfigWidget::onDstToggled(bool checked)
{
    Q_UNUSED(checked);
    updateLiveClockPreview();
}

void TimeConfigWidget::onSyncModeChanged()
{
    const bool isAuto = m_autoNtpRadio->isChecked();
    m_ntpServerEdit->setEnabled(isAuto);
    m_dateEdit->setEnabled(!isAuto);
    m_timeEdit->setEnabled(!isAuto);
    updateLiveClockPreview();
}

void TimeConfigWidget::updateLiveClockPreview()
{
    const QString tzId = m_zoneCombo->currentData().toString().isEmpty()
                             ? m_zoneCombo->currentText().trimmed()
                             : m_zoneCombo->currentData().toString();

    const QTimeZone tz(tzId.toUtf8());
    const QDateTime nowUtc = QDateTime::currentDateTimeUtc();

    int effectiveOffsetSec = 0;
    bool nativeDst = false;
    if (tz.isValid()) {
        nativeDst = tz.isDaylightTime(nowUtc);
        const int stdOffset = tz.standardTimeOffset(nowUtc);
        effectiveOffsetSec = m_dstCheck->isChecked() ? (stdOffset + 3600) : stdOffset;
    }

    const QString dstNote = m_dstCheck->isChecked()
                                ? (nativeDst ? "Horario de verano activo por región" : "Horario de verano activado manualmente (+1 h)")
                                : "Hora estándar";

    m_zoneInfoLabel->setText(QString("%1 — %2").arg(TimeManager::formatUtcOffset(effectiveOffsetSec), dstNote));

    if (m_autoNtpRadio->isChecked()) {
        const QDateTime previewDt = nowUtc.addSecs(effectiveOffsetSec);
        m_liveClockLabel->setText(previewDt.toString("dddd dd/MM/yyyy  HH:mm:ss"));
    } else {
        const QDateTime manualDt(m_dateEdit->date(), m_timeEdit->time());
        m_liveClockLabel->setText(manualDt.toString("dddd dd/MM/yyyy  HH:mm:ss") + " (Manual)");
    }
}

void TimeConfigWidget::onApplyClicked()
{
    const QString tzId = m_zoneCombo->currentData().toString().isEmpty()
                             ? m_zoneCombo->currentText().trimmed()
                             : m_zoneCombo->currentData().toString();

    TimeConfigRequest req;
    req.timezoneId = tzId;
    req.enableDst = m_dstCheck->isChecked();
    req.automaticNtp = m_autoNtpRadio->isChecked();
    req.ntpServer = m_ntpServerEdit->text().trimmed();
    req.manualDateTime = QDateTime(m_dateEdit->date(), m_timeEdit->time());
    req.hardwareClockUtc = m_hwClockUtcCheck->isChecked();

    m_applyBtn->setEnabled(false);
    const TimeConfigResult res = TimeManager::applyConfiguration(req);
    m_applyBtn->setEnabled(true);

    showStatusBanner(res.success, res.message);
}

void TimeConfigWidget::showStatusBanner(bool success, const QString &message)
{
    m_statusLabel->setText(message);
    if (success) {
        m_statusLabel->setStyleSheet(
            "background-color: #e8f5e9; color: #1b5e20; border: 1px solid #a5d6a7; "
            "border-radius: 5px; padding: 10px; font-weight: bold;"
        );
    } else {
        m_statusLabel->setStyleSheet(
            "background-color: #ffebee; color: #b71c1c; border: 1px solid #ef9a9a; "
            "border-radius: 5px; padding: 10px; font-weight: bold;"
        );
    }
    m_statusLabel->setVisible(true);
}
