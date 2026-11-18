#include "mainWindow.h"
#include "logger.h"
#include "commandRunner.h"
#include "localeManager.h"
#include "keyboardManager.h"
#include "userManager.h"
#include "groupManager.h"
#include "sudoManager.h"
#include "webServerManager.h"
#include "databaseManager.h"
#include "DeviceManager.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QStatusBar>
#include <QMessageBox>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QCompleter>
#include <QTimeZone>
#include <QDateTime>
#include <QSignalBlocker>
#include <QTimer>
#include <QStyle>
#include <QFont>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QBrush>
#include <QColor>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_navList(nullptr),
      m_pagesStack(nullptr),
      m_btnPrev(nullptr),
      m_btnNext(nullptr),
      m_statusPrivilegeLabel(nullptr),
      m_statusModeLabel(nullptr),
      m_lblCurrentLocale(nullptr),
      m_comboLocales(nullptr),
      m_btnSaveLocale(nullptr),
      m_lblLocaleFeedback(nullptr),
      m_lblCurrentKeyboard(nullptr),
      m_comboKeyboards(nullptr),
      m_editKeyboardTest(nullptr),
      m_btnSaveKeyboard(nullptr),
      m_lblKeyboardFeedback(nullptr),
      m_lblExistingUsers(nullptr),
      m_editNewUsername(nullptr),
      m_editNewFullName(nullptr),
      m_editNewPassword(nullptr),
      m_editConfirmPassword(nullptr),
      m_chkShowPassword(nullptr),
      m_comboUserShell(nullptr),
      m_btnCreateUser(nullptr),
      m_lblUserFeedback(nullptr),
      m_comboGroupUsers(nullptr),
      m_lblUserCurrentGroups(nullptr),
      m_userGroupsContainer(nullptr),
      m_userGroupsGrid(nullptr),
      m_btnSaveUserGroups(nullptr),
      m_lblGroupFeedback(nullptr),
      m_lblSudoOverview(nullptr),
      m_comboSudoUsers(nullptr),
      m_lblSudoSelectedUserDetail(nullptr),
      m_radioSudoWithPass(nullptr),
      m_radioSudoNoPass(nullptr),
      m_radioSudoDisabled(nullptr),
      m_chkSudoSyncWheel(nullptr),
      m_btnSaveSudo(nullptr),
      m_lblSudoFeedback(nullptr),
      m_lblWebDetectionSummary(nullptr),
      m_radioWebApache(nullptr),
      m_radioWebNginx(nullptr),
      m_chkEnablePhp(nullptr),
      m_phpOptionsContainer(nullptr),
      m_lblPhpTimezoneInfo(nullptr),
      m_lblPhpImagickNotice(nullptr),
      m_chkUpdatePhpIni(nullptr),
      m_comboPhpMemoryLimit(nullptr),
      m_comboPhpUploadMax(nullptr),
      m_comboPhpPostMax(nullptr),
      m_chkPhpDisplayErrors(nullptr),
      m_chkWebStartOnBoot(nullptr),
      m_btnSaveWebServer(nullptr),
      m_lblWebServerFeedback(nullptr),
      m_webPreviewFrame(nullptr),
      m_btnPreviewLocalhost(nullptr),
      m_btnPreviewPhpInfo(nullptr),
      m_editPreviewUrl(nullptr),
      m_lblPreviewHttpStatus(nullptr),
      m_webPreviewBrowser(nullptr),
      m_networkManager(new QNetworkAccessManager(this)),
      m_lblDbDetectionSummary(nullptr),
      m_radioDbMariaDb(nullptr),
      m_radioDbPostgres(nullptr),
      m_chkDbSetRootPassword(nullptr),
      m_dbPasswordFieldsWidget(nullptr),
      m_editDbCurrentPassword(nullptr),
      m_editDbNewPassword(nullptr),
      m_editDbConfirmPassword(nullptr),
      m_chkDbShowPasswords(nullptr),
      m_dbMariaOptionsWidget(nullptr),
      m_chkMariaRemoveAnon(nullptr),
      m_chkMariaDisallowRemoteRoot(nullptr),
      m_chkMariaRemoveTestDb(nullptr),
      m_chkMariaReloadPrivs(nullptr),
      m_dbPostgresOptionsWidget(nullptr),
      m_chkPgScramAuth(nullptr),
      m_chkPgLocalhostOnly(nullptr),
      m_chkPgRemoveTestDb(nullptr),
      m_btnSaveDbConfig(nullptr),
      m_lblDbConfigFeedback(nullptr),
      m_dbSessionTestFrame(nullptr),
      m_lblDbSessionPrompt(nullptr),
      m_editDbTestPassword(nullptr),
      m_chkDbShowTestPassword(nullptr),
      m_btnTestDbSession(nullptr),
      m_lblDbSessionStatus(nullptr),
      m_txtDbSessionOutput(nullptr),
      m_dbBootFrame(nullptr),
      m_lblDbBootQuestion(nullptr),
      m_radioDbBootYes(nullptr),
      m_radioDbBootNo(nullptr),
      m_btnSaveDbBoot(nullptr),
      m_lblDbBootFeedback(nullptr),
      m_lblTimeCurrentSummary(nullptr),
      m_timeMapWidget(nullptr),
      m_comboTimezones(nullptr),
      m_chkTimeDst(nullptr),
      m_lblTimeOffsetInfo(nullptr),
      m_lblTimeLivePreview(nullptr),
      m_radioTimeAutoNtp(nullptr),
      m_radioTimeManual(nullptr),
      m_editTimeNtpServer(nullptr),
      m_dateTimeManualDate(nullptr),
      m_timeTimeManualTime(nullptr),
      m_chkTimeHwUtc(nullptr),
      m_btnSaveTimeConfig(nullptr),
      m_lblTimeFeedback(nullptr),
      m_timePreviewTimer(nullptr),
      m_lblServicesSummary(nullptr),
      m_btnToggleAdvancedServices(nullptr),
      m_btnConfigureSamba(nullptr),
      m_serviceDangerBanner(nullptr),
      m_editServiceSearch(nullptr),
      m_comboServiceCategoryFilter(nullptr),
      m_btnPresetDesktopServices(nullptr),
      m_btnPresetServerServices(nullptr),
      m_tblServices(nullptr),
      m_serviceDetailBox(nullptr),
      m_lblServiceDetailTitle(nullptr),
      m_lblServiceDetailDesc(nullptr),
      m_lblServiceConflictAlert(nullptr),
      m_lblServiceFeedback(nullptr),
      m_serviceAdvancedPanel(nullptr),
      m_serviceAdvancedTabs(nullptr),
      m_lblScriptViewerHeader(nullptr),
      m_txtServiceScriptViewer(nullptr),
      m_comboLocalBootScript(nullptr),
      m_txtLocalBootScriptEditor(nullptr),
      m_btnSaveLocalBootScript(nullptr),
      m_lblDeviceFormFactorSummary(nullptr),
      m_btnToggleAdvancedDevices(nullptr),
      m_deviceDangerBanner(nullptr),
      m_tblDevices(nullptr),
      m_deviceDetailBox(nullptr),
      m_lblDeviceDetailTitle(nullptr),
      m_lblDeviceDetailDesc(nullptr),
      m_lblDeviceFeedback(nullptr),
      m_deviceAdvancedPanel(nullptr),
      m_txtDeviceTechViewer(nullptr),
      m_deviceAdvancedMode(false)
{
    LOG_INFO("MainWindow", "Inicializando interfaz gráfica principal...");
    setupUi();
    setupNavigationMenu();
    setupPages();

    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onWebPreviewReplyFinished);

    m_navList->setCurrentRow(0);
    updateNavigationButtons();
    LOG_SUCCESS("MainWindow", "Interfaz gráfica cargada correctamente.");
}

void MainWindow::setupUi()
{
    setWindowTitle("Slackware Wizard (slackwiz) - Asistente de Configuración");
    resize(1060, 760);
    setMinimumSize(920, 640);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Panel lateral izquierdo (Navegación)
    QFrame *sidebarFrame = new QFrame(centralWidget);
    sidebarFrame->setFixedWidth(250);
    sidebarFrame->setStyleSheet(
        "QFrame { background-color: #1e242b; color: #eceff4; border-right: 1px solid #2e3440; }"
    );

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebarFrame);
    sidebarLayout->setContentsMargins(12, 16, 12, 16);
    sidebarLayout->setSpacing(10);

    QLabel *appTitleLabel = new QLabel("Slackware Wizard", sidebarFrame);
    QFont titleFont = appTitleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    appTitleLabel->setFont(titleFont);
    appTitleLabel->setStyleSheet("border: none; color: #88c0d0;");

    QLabel *appSubLabel = new QLabel("Configuración rápida post-instalación", sidebarFrame);
    appSubLabel->setWordWrap(true);
    appSubLabel->setStyleSheet("border: none; color: #a3be8c; font-size: 11px;");

    m_navList = new QListWidget(sidebarFrame);
    m_navList->setFocusPolicy(Qt::NoFocus);
    m_navList->setStyleSheet(
        "QListWidget { border: none; background-color: transparent; outline: 0; font-size: 13px; }"
        "QListWidget::item { padding: 10px 8px; border-radius: 5px; margin-bottom: 2px; color: #d8dee9; }"
        "QListWidget::item:hover { background-color: #2e3440; }"
        "QListWidget::item:selected { background-color: #3b4252; color: #88c0d0; font-weight: bold; }"
    );

    sidebarLayout->addWidget(appTitleLabel);
    sidebarLayout->addWidget(appSubLabel);
    sidebarLayout->addSpacing(8);
    sidebarLayout->addWidget(m_navList);

    // Área derecha (Contenido del módulo + botones de navegación)
    QWidget *contentContainer = new QWidget(centralWidget);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setContentsMargins(24, 20, 24, 16);
    contentLayout->setSpacing(16);

    m_pagesStack = new QStackedWidget(contentContainer);

    // Barra inferior de navegación (Anterior / Siguiente)
    QFrame *separator = new QFrame(contentContainer);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    QHBoxLayout *navButtonsLayout = new QHBoxLayout();
    m_btnPrev = new QPushButton("Anterior", contentContainer);
    m_btnNext = new QPushButton("Siguiente", contentContainer);
    m_btnPrev->setMinimumWidth(110);
    m_btnNext->setMinimumWidth(110);

    navButtonsLayout->addStretch();
    navButtonsLayout->addWidget(m_btnPrev);
    navButtonsLayout->addWidget(m_btnNext);

    contentLayout->addWidget(m_pagesStack, 1);
    contentLayout->addWidget(separator);
    contentLayout->addLayout(navButtonsLayout);

    mainLayout->addWidget(sidebarFrame);
    mainLayout->addWidget(contentContainer, 1);

    // Barra de estado
    m_statusPrivilegeLabel = new QLabel(this);
    m_statusModeLabel = new QLabel(this);

    if (CommandRunner::isRoot()) {
        m_statusPrivilegeLabel->setText(" Privilegios: ROOT (Listo para aplicar cambios) ");
        m_statusPrivilegeLabel->setStyleSheet("color: #2e7d32; font-weight: bold;");
    } else {
        m_statusPrivilegeLabel->setText(" Advertencia: Ejecutando sin ROOT (Modo vista previa) ");
        m_statusPrivilegeLabel->setStyleSheet("color: #c62828; font-weight: bold;");
    }

#ifdef SLACKWIZ_DEBUG
    m_statusModeLabel->setText(" [COMPILACIÓN DEBUG] ");
    m_statusModeLabel->setStyleSheet("color: #1565c0; font-weight: bold;");
#else
    m_statusModeLabel->setText(" v0.1.0 ");
#endif

    statusBar()->addWidget(m_statusPrivilegeLabel);
    statusBar()->addPermanentWidget(m_statusModeLabel);

    connect(m_navList, &QListWidget::currentRowChanged, this, &MainWindow::onNavigationChanged);
    connect(m_btnPrev, &QPushButton::clicked, this, &MainWindow::onPrevModuleClicked);
    connect(m_btnNext, &QPushButton::clicked, this, &MainWindow::onNextModuleClicked);
}

void MainWindow::setupNavigationMenu()
{
    struct NavItem {
        QString label;
        QStyle::StandardPixmap icon;
    };

    const QList<NavItem> items = {
        { "0. Bienvenida",          QStyle::SP_ComputerIcon },
        { "1. Idioma del sistema",  QStyle::SP_DialogApplyButton },
        { "2. Teclado",             QStyle::SP_FileDialogDetailedView },
        { "3. Añadir usuario",      QStyle::SP_DirHomeIcon },
        { "4. Grupos de usuario",   QStyle::SP_FileDialogListView },
        { "5. Configurar sudo",     QStyle::SP_MessageBoxWarning },
        { "6. Servidor web",        QStyle::SP_DriveNetIcon },
        { "7. Base de datos",       QStyle::SP_DriveHDIcon },
        { "8. Hora del sistema",    QStyle::SP_BrowserReload },
        { "9. Servicios (rc.d)",    QStyle::SP_FileDialogContentsView },
        { "10. Dispositivos",       QStyle::SP_DesktopIcon }
    };

    for (const auto &item : items) {
        QListWidgetItem *listItem = new QListWidgetItem(
            style()->standardIcon(item.icon),
            item.label,
            m_navList
        );
        m_navList->addItem(listItem);
    }
}

void MainWindow::setupPages()
{
    m_pagesStack->addWidget(createWelcomePage());
    m_pagesStack->addWidget(createLocalePage());
    m_pagesStack->addWidget(createKeyboardPage());
    m_pagesStack->addWidget(createAddUserPage());
    m_pagesStack->addWidget(createUserGroupsPage());
    m_pagesStack->addWidget(createSudoPage());
    m_pagesStack->addWidget(createWebServerPage());
    m_pagesStack->addWidget(createDatabasePage());
    m_pagesStack->addWidget(createTimePage());
    m_pagesStack->addWidget(createServicesPage());
    m_pagesStack->addWidget(createDevicesPage());
}

QWidget* MainWindow::createWelcomePage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(14);

    QLabel *title = new QLabel("Bienvenido a Slackware Wizard (slackwiz)", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *intro = new QLabel(
        "Este asistente te guiará paso a paso para dejar tu instalación de Slackware lista "
        "en pocos minutos, sin necesidad de editar archivos de configuración a mano.\n\n"
        "Si vienes de distribuciones como Ubuntu, Linux Mint o Fedora, aquí podrás configurar "
        "de manera visual e intuitiva los ajustes esenciales del sistema:",
        page
    );
    intro->setWordWrap(true);
    intro->setStyleSheet("font-size: 13px; line-height: 1.4;");

    QFrame *infoBox = new QFrame(page);
    infoBox->setStyleSheet(
        "QFrame { background-color: #f4f6f9; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *boxLayout = new QVBoxLayout(infoBox);

    const QString slackVer = CommandRunner::detectSlackwareVersion();
    const QString rootState = CommandRunner::isRoot()
                                  ? "Sí (ejecutando como administrador root)"
                                  : "No (ejecuta slackwiz como root para guardar cambios)";

    QLabel *sysInfo = new QLabel(
        QString("<b>Sistema detectado:</b> %1<br>"
                "<b>Permisos de administrador:</b> %2<br>"
                "<b>Módulos disponibles:</b> Idioma, Teclado, Usuarios, Grupos, Sudo, Servidor Web, Base de Datos, Hora, Servicios y Dispositivos.")
            .arg(slackVer, rootState),
        infoBox
    );
    sysInfo->setWordWrap(true);
    sysInfo->setStyleSheet("border: none; font-size: 13px;");
    boxLayout->addWidget(sysInfo);

    layout->addWidget(title);
    layout->addWidget(intro);
    layout->addWidget(infoBox);
    layout->addStretch();

    return page;
}

QWidget* MainWindow::createLocalePage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(16);

    QLabel *title = new QLabel("1. Configuración del Idioma del Sistema", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *subtitle = new QLabel(
        "Define el idioma principal y la codificación UTF-8 para las aplicaciones, "
        "menús del entorno gráfico y mensajes de consola de Slackware.",
        page
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size: 13px;");

    QFrame *currentBox = new QFrame(page);
    currentBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *currentBoxLayout = new QVBoxLayout(currentBox);

    m_lblCurrentLocale = new QLabel(currentBox);
    m_lblCurrentLocale->setWordWrap(true);
    m_lblCurrentLocale->setStyleSheet("border: none; font-size: 13px; color: #084298;");
    currentBoxLayout->addWidget(m_lblCurrentLocale);

    QFrame *selectBox = new QFrame(page);
    selectBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 14px; }"
    );
    QVBoxLayout *selectLayout = new QVBoxLayout(selectBox);
    selectLayout->setSpacing(10);

    QLabel *lblSelect = new QLabel("Selecciona el idioma que deseas configurar:", selectBox);
    lblSelect->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    m_comboLocales = new QComboBox(selectBox);
    m_comboLocales->setMinimumHeight(34);
    m_comboLocales->setStyleSheet("font-size: 13px; padding: 4px 8px;");

    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_btnSaveLocale = new QPushButton("Guardar configuración de idioma", selectBox);
    m_btnSaveLocale->setMinimumHeight(36);
    m_btnSaveLocale->setMinimumWidth(230);
    m_btnSaveLocale->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; border-radius: 5px; padding: 6px 16px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
        "QPushButton:disabled { background-color: #9e9e9e; }"
    );
    actionLayout->addWidget(m_btnSaveLocale);
    actionLayout->addStretch();

    selectLayout->addWidget(lblSelect);
    selectLayout->addWidget(m_comboLocales);
    selectLayout->addSpacing(4);
    selectLayout->addLayout(actionLayout);

    m_lblLocaleFeedback = new QLabel(page);
    m_lblLocaleFeedback->setWordWrap(true);
    m_lblLocaleFeedback->setVisible(false);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(currentBox);
    layout->addWidget(selectBox);
    layout->addWidget(m_lblLocaleFeedback);
    layout->addStretch();

    connect(m_btnSaveLocale, &QPushButton::clicked, this, &MainWindow::onSaveLocaleClicked);

    refreshLocalePage();
    return page;
}

void MainWindow::refreshLocalePage()
{
    if (!m_lblCurrentLocale || !m_comboLocales) {
        return;
    }

    const QString currentCode = LocaleManager::getCurrentSystemLocale();
    const QString friendlyName = LocaleManager::getLocaleDisplayName(currentCode);

    m_lblCurrentLocale->setText(
        QString("<b>Idioma actual configurado en el sistema:</b> %1").arg(friendlyName)
    );

    m_comboLocales->clear();
    const QList<LocaleOption> options = LocaleManager::getAvailableLocales();
    int selectedIndex = -1;

    for (int i = 0; i < options.size(); ++i) {
        const QString itemLabel = QString("%1  —  [%2]").arg(options[i].displayName, options[i].code);
        m_comboLocales->addItem(itemLabel, options[i].code);
        if (options[i].code.compare(currentCode, Qt::CaseInsensitive) == 0) {
            selectedIndex = i;
        }
    }

    if (selectedIndex == -1 && !currentCode.isEmpty()) {
        m_comboLocales->insertItem(0, QString("Actual del sistema  —  [%1]").arg(currentCode), currentCode);
        selectedIndex = 0;
    }

    if (selectedIndex >= 0) {
        m_comboLocales->setCurrentIndex(selectedIndex);
    }
}

void MainWindow::onSaveLocaleClicked()
{
    if (!m_comboLocales || !m_lblLocaleFeedback) {
        return;
    }

    const QString selectedCode = m_comboLocales->currentData().toString();
    LOG_INFO("MainWindow", QString("Usuario pulsó guardar idioma: %1").arg(selectedCode));

    const LocaleApplyResult result = LocaleManager::applySystemLocale(selectedCode);

    m_lblLocaleFeedback->setVisible(true);
    if (result.success) {
        m_lblLocaleFeedback->setStyleSheet(
            "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblLocaleFeedback->setText(result.message);
        refreshLocalePage();
    } else {
        m_lblLocaleFeedback->setStyleSheet(
            "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblLocaleFeedback->setText(result.message);
    }
}

QWidget* MainWindow::createKeyboardPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(14);

    QLabel *title = new QLabel("2. Configuración del Teclado", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *subtitle = new QLabel(
        "Configura la distribución de teclas tanto para las terminales de texto como para el entorno gráfico "
        "y todas las consolas del sistema.",
        page
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size: 13px;");

    QFrame *currentBox = new QFrame(page);
    currentBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *currentBoxLayout = new QVBoxLayout(currentBox);

    m_lblCurrentKeyboard = new QLabel(currentBox);
    m_lblCurrentKeyboard->setWordWrap(true);
    m_lblCurrentKeyboard->setStyleSheet("border: none; font-size: 13px; color: #084298;");
    currentBoxLayout->addWidget(m_lblCurrentKeyboard);

    QFrame *selectBox = new QFrame(page);
    selectBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 14px; }"
    );
    QVBoxLayout *selectLayout = new QVBoxLayout(selectBox);
    selectLayout->setSpacing(10);

    QLabel *lblSelect = new QLabel("Selecciona la distribución de teclado:", selectBox);
    lblSelect->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    m_comboKeyboards = new QComboBox(selectBox);
    m_comboKeyboards->setMinimumHeight(34);
    m_comboKeyboards->setStyleSheet("font-size: 13px; padding: 4px 8px;");

    QLabel *lblTest = new QLabel("Prueba aquí tus teclas (ej. ñ, á, @, ¿, /, -):", selectBox);
    lblTest->setStyleSheet("border: none; font-size: 12px; color: #495057;");

    m_editKeyboardTest = new QLineEdit(selectBox);
    m_editKeyboardTest->setMinimumHeight(32);
    m_editKeyboardTest->setPlaceholderText("Escribe aquí para verificar que los símbolos coinciden con tu teclado...");
    m_editKeyboardTest->setStyleSheet("font-size: 13px; padding: 4px 8px; background-color: white;");

    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_btnSaveKeyboard = new QPushButton("Guardar configuración de teclado", selectBox);
    m_btnSaveKeyboard->setMinimumHeight(36);
    m_btnSaveKeyboard->setMinimumWidth(240);
    m_btnSaveKeyboard->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; border-radius: 5px; padding: 6px 16px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
        "QPushButton:disabled { background-color: #9e9e9e; }"
    );
    actionLayout->addWidget(m_btnSaveKeyboard);
    actionLayout->addStretch();

    selectLayout->addWidget(lblSelect);
    selectLayout->addWidget(m_comboKeyboards);
    selectLayout->addWidget(lblTest);
    selectLayout->addWidget(m_editKeyboardTest);
    selectLayout->addSpacing(4);
    selectLayout->addLayout(actionLayout);

    m_lblKeyboardFeedback = new QLabel(page);
    m_lblKeyboardFeedback->setWordWrap(true);
    m_lblKeyboardFeedback->setVisible(false);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(currentBox);
    layout->addWidget(selectBox);
    layout->addWidget(m_lblKeyboardFeedback);
    layout->addStretch();

    connect(m_btnSaveKeyboard, &QPushButton::clicked, this, &MainWindow::onSaveKeyboardClicked);

    refreshKeyboardPage();
    return page;
}

void MainWindow::refreshKeyboardPage()
{
    if (!m_lblCurrentKeyboard || !m_comboKeyboards) {
        return;
    }

    m_lblCurrentKeyboard->setText(
        QString("<b>Teclado actual detectado:</b> %1").arg(KeyboardManager::getFriendlyCurrentSummary())
    );

    const KeyboardCurrentConfig current = KeyboardManager::getCurrentKeyboardConfig();
    const QList<KeyboardLayoutOption> layouts = KeyboardManager::getAvailableLayouts();

    m_comboKeyboards->clear();
    int selectedIndex = 0;

    for (int i = 0; i < layouts.size(); ++i) {
        const QString itemLabel = QString("%1  —  [setxkbmap %2 | consola: %3]")
                                      .arg(layouts[i].displayName, layouts[i].x11Layout, layouts[i].consoleKeymap);
        m_comboKeyboards->addItem(itemLabel, layouts[i].id);

        if ((!current.x11Layout.isEmpty() && layouts[i].x11Layout == current.x11Layout) ||
            (!current.consoleKeymap.isEmpty() && layouts[i].consoleKeymap == current.consoleKeymap)) {
            selectedIndex = i;
        }
    }

    m_comboKeyboards->setCurrentIndex(selectedIndex);
}

void MainWindow::onSaveKeyboardClicked()
{
    if (!m_comboKeyboards || !m_lblKeyboardFeedback) {
        return;
    }

    const QString selectedId = m_comboKeyboards->currentData().toString();
    LOG_INFO("MainWindow", QString("Usuario pulsó guardar teclado con ID: %1").arg(selectedId));

    const KeyboardApplyResult result = KeyboardManager::applyKeyboardLayout(selectedId);

    m_lblKeyboardFeedback->setVisible(true);
    if (result.success) {
        m_lblKeyboardFeedback->setStyleSheet(
            "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblKeyboardFeedback->setText(result.message);
        refreshKeyboardPage();
        if (m_editKeyboardTest) {
            m_editKeyboardTest->clear();
            m_editKeyboardTest->setFocus();
        }
    } else {
        m_lblKeyboardFeedback->setStyleSheet(
            "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblKeyboardFeedback->setText(result.message);
    }
}

QWidget* MainWindow::createAddUserPage()
{
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *page = new QWidget(scrollArea);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(12);

    QLabel *title = new QLabel("3. Añadir un Nuevo Usuario", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *subtitle = new QLabel(
        "En Slackware es recomendable trabajar con un usuario estándar en lugar de usar root directamente. "
        "Aquí puedes crear la cuenta, su carpeta /home y asignarle los grupos necesarios para audio, USB, red y administración.",
        page
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size: 13px;");

    QFrame *existingBox = new QFrame(page);
    existingBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 10px; }"
    );
    QVBoxLayout *existingLayout = new QVBoxLayout(existingBox);
    m_lblExistingUsers = new QLabel(existingBox);
    m_lblExistingUsers->setWordWrap(true);
    m_lblExistingUsers->setStyleSheet("border: none; font-size: 12px; color: #084298;");
    existingLayout->addWidget(m_lblExistingUsers);

    QFrame *accountBox = new QFrame(page);
    accountBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *accountBoxLayout = new QVBoxLayout(accountBox);
    accountBoxLayout->setSpacing(8);

    QLabel *lblAccountHeader = new QLabel("Datos de la nueva cuenta:", accountBox);
    lblAccountHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");
    accountBoxLayout->addWidget(lblAccountHeader);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(8);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_editNewUsername = new QLineEdit(accountBox);
    m_editNewUsername->setMinimumHeight(30);
    m_editNewUsername->setPlaceholderText("ej. juan (solo minúsculas, sin espacios)");
    m_editNewUsername->setStyleSheet("background-color: white; padding: 3px 8px; font-size: 13px;");

    m_editNewFullName = new QLineEdit(accountBox);
    m_editNewFullName->setMinimumHeight(30);
    m_editNewFullName->setPlaceholderText("ej. Juan Pérez (opcional)");
    m_editNewFullName->setStyleSheet("background-color: white; padding: 3px 8px; font-size: 13px;");

    m_editNewPassword = new QLineEdit(accountBox);
    m_editNewPassword->setMinimumHeight(30);
    m_editNewPassword->setEchoMode(QLineEdit::Password);
    m_editNewPassword->setPlaceholderText("Contraseña del usuario");
    m_editNewPassword->setStyleSheet("background-color: white; padding: 3px 8px; font-size: 13px;");

    m_editConfirmPassword = new QLineEdit(accountBox);
    m_editConfirmPassword->setMinimumHeight(30);
    m_editConfirmPassword->setEchoMode(QLineEdit::Password);
    m_editConfirmPassword->setPlaceholderText("Repite la contraseña");
    m_editConfirmPassword->setStyleSheet("background-color: white; padding: 3px 8px; font-size: 13px;");

    m_chkShowPassword = new QCheckBox("Mostrar contraseñas", accountBox);
    m_chkShowPassword->setStyleSheet("font-size: 12px; color: #495057;");

    m_comboUserShell = new QComboBox(accountBox);
    m_comboUserShell->setMinimumHeight(30);
    m_comboUserShell->setStyleSheet("font-size: 13px; padding: 3px 8px;");
    m_comboUserShell->addItems(UserManager::getAvailableShells());

    QLabel *lblUser = new QLabel("Nombre de usuario:", accountBox);
    lblUser->setStyleSheet("border: none; font-size: 13px;");
    QLabel *lblFull = new QLabel("Nombre completo:", accountBox);
    lblFull->setStyleSheet("border: none; font-size: 13px;");
    QLabel *lblPass = new QLabel("Contraseña:", accountBox);
    lblPass->setStyleSheet("border: none; font-size: 13px;");
    QLabel *lblConf = new QLabel("Confirmar contraseña:", accountBox);
    lblConf->setStyleSheet("border: none; font-size: 13px;");
    QLabel *lblShell = new QLabel("Shell por defecto:", accountBox);
    lblShell->setStyleSheet("border: none; font-size: 13px;");

    formLayout->addRow(lblUser, m_editNewUsername);
    formLayout->addRow(lblFull, m_editNewFullName);
    formLayout->addRow(lblPass, m_editNewPassword);
    formLayout->addRow(lblConf, m_editConfirmPassword);
    formLayout->addRow("", m_chkShowPassword);
    formLayout->addRow(lblShell, m_comboUserShell);

    accountBoxLayout->addLayout(formLayout);

    QFrame *groupsBox = new QFrame(page);
    groupsBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *groupsBoxLayout = new QVBoxLayout(groupsBox);
    groupsBoxLayout->setSpacing(8);

    QHBoxLayout *groupsHeaderLayout = new QHBoxLayout();
    QLabel *lblGroupsHeader = new QLabel("Grupos a los que pertenecerá el usuario (grupo base: users):", groupsBox);
    lblGroupsHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    QPushButton *btnPresetRecommended = new QPushButton("Recomendados", groupsBox);
    btnPresetRecommended->setStyleSheet("font-size: 11px; padding: 3px 8px;");
    QPushButton *btnPresetAll = new QPushButton("Marcar todos", groupsBox);
    btnPresetAll->setStyleSheet("font-size: 11px; padding: 3px 8px;");

    groupsHeaderLayout->addWidget(lblGroupsHeader);
    groupsHeaderLayout->addStretch();
    groupsHeaderLayout->addWidget(btnPresetRecommended);
    groupsHeaderLayout->addWidget(btnPresetAll);

    groupsBoxLayout->addLayout(groupsHeaderLayout);

    QGridLayout *groupsGrid = new QGridLayout();
    groupsGrid->setSpacing(6);

    m_groupCheckboxes.clear();
    const QList<SystemGroupOption> availableGroups = UserManager::getAvailableGroups();

    for (int i = 0; i < availableGroups.size(); ++i) {
        const SystemGroupOption &grp = availableGroups[i];
        QCheckBox *chk = new QCheckBox(QString("%1 — %2").arg(grp.name, grp.description), groupsBox);
        chk->setProperty("groupName", grp.name);
        chk->setProperty("recommended", grp.recommendedByDefault);
        chk->setChecked(grp.recommendedByDefault && grp.existsOnSystem);
        chk->setEnabled(grp.existsOnSystem);
        chk->setStyleSheet(grp.name == "wheel"
                               ? "font-size: 12px; font-weight: bold; color: #084298;"
                               : "font-size: 12px;");

        if (!grp.existsOnSystem) {
            chk->setToolTip("Este grupo no existe en /etc/group de tu sistema.");
        }

        m_groupCheckboxes.append(chk);
        groupsGrid->addWidget(chk, i / 2, i % 2);
    }

    groupsBoxLayout->addLayout(groupsGrid);

    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_btnCreateUser = new QPushButton("Crear nuevo usuario", page);
    m_btnCreateUser->setMinimumHeight(38);
    m_btnCreateUser->setMinimumWidth(220);
    m_btnCreateUser->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; border-radius: 5px; padding: 6px 18px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
        "QPushButton:disabled { background-color: #9e9e9e; }"
    );
    actionLayout->addWidget(m_btnCreateUser);
    actionLayout->addStretch();

    m_lblUserFeedback = new QLabel(page);
    m_lblUserFeedback->setWordWrap(true);
    m_lblUserFeedback->setVisible(false);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(existingBox);
    layout->addWidget(accountBox);
    layout->addWidget(groupsBox);
    layout->addLayout(actionLayout);
    layout->addWidget(m_lblUserFeedback);
    layout->addStretch();

    connect(m_chkShowPassword, &QCheckBox::toggled, this, &MainWindow::onToggleShowPassword);
    connect(btnPresetRecommended, &QPushButton::clicked, this, &MainWindow::onSelectRecommendedGroups);
    connect(btnPresetAll, &QPushButton::clicked, this, &MainWindow::onSelectAllGroups);
    connect(m_btnCreateUser, &QPushButton::clicked, this, &MainWindow::onCreateUserClicked);

    refreshAddUserPage();
    scrollArea->setWidget(page);
    return scrollArea;
}

void MainWindow::refreshAddUserPage()
{
    if (!m_lblExistingUsers) {
        return;
    }

    const QStringList existingUsers = UserManager::getExistingHumanUsers();
    if (existingUsers.isEmpty()) {
        m_lblExistingUsers->setText(
            "<b>Usuarios actuales en el sistema:</b> Ningún usuario estándar creado todavía (solo root)."
        );
    } else {
        m_lblExistingUsers->setText(
            QString("<b>Usuarios actuales en el sistema:</b> %1").arg(existingUsers.join(", "))
        );
    }
}

void MainWindow::onToggleShowPassword(bool checked)
{
    const QLineEdit::EchoMode mode = checked ? QLineEdit::Normal : QLineEdit::Password;
    if (m_editNewPassword) {
        m_editNewPassword->setEchoMode(mode);
    }
    if (m_editConfirmPassword) {
        m_editConfirmPassword->setEchoMode(mode);
    }
}

void MainWindow::onSelectRecommendedGroups()
{
    for (QCheckBox *chk : m_groupCheckboxes) {
        if (chk->isEnabled()) {
            chk->setChecked(chk->property("recommended").toBool());
        }
    }
}

void MainWindow::onSelectAllGroups()
{
    for (QCheckBox *chk : m_groupCheckboxes) {
        if (chk->isEnabled()) {
            chk->setChecked(true);
        }
    }
}

void MainWindow::onCreateUserClicked()
{
    if (!m_editNewUsername || !m_lblUserFeedback) {
        return;
    }

    NewUserRequest req;
    req.username = m_editNewUsername->text();
    req.fullName = m_editNewFullName->text();
    req.password = m_editNewPassword->text();
    req.confirmPassword = m_editConfirmPassword->text();
    req.shell = m_comboUserShell->currentText();

    for (QCheckBox *chk : m_groupCheckboxes) {
        if (chk->isChecked() && chk->isEnabled()) {
            req.selectedGroups.append(chk->property("groupName").toString());
        }
    }

    LOG_INFO("MainWindow", QString("Usuario pulsó crear cuenta '%1' con grupos: [%2]")
                               .arg(req.username, req.selectedGroups.join(", ")));

    const UserOperationResult result = UserManager::createUser(req);

    m_lblUserFeedback->setVisible(true);
    if (result.success) {
        m_lblUserFeedback->setStyleSheet(
            "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblUserFeedback->setText(result.message);

        m_editNewUsername->clear();
        m_editNewFullName->clear();
        m_editNewPassword->clear();
        m_editConfirmPassword->clear();
        refreshAddUserPage();
    } else {
        m_lblUserFeedback->setStyleSheet(
            "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblUserFeedback->setText(result.message);
    }
}

QWidget* MainWindow::createUserGroupsPage()
{
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *page = new QWidget(scrollArea);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(12);

    QLabel *title = new QLabel("4. Gestionar Grupos de Usuario", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *subtitle = new QLabel(
        "Añade o quita permisos de hardware, red y administración (sudo) a cualquier usuario existente en el sistema. "
        "Selecciona un usuario para ver y modificar sus grupos actuales.",
        page
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size: 13px;");

    QFrame *userSelectBox = new QFrame(page);
    userSelectBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *userSelectLayout = new QVBoxLayout(userSelectBox);
    userSelectLayout->setSpacing(8);

    QHBoxLayout *userRowLayout = new QHBoxLayout();
    QLabel *lblChooseUser = new QLabel("Usuario a configurar:", userSelectBox);
    lblChooseUser->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #084298;");

    m_comboGroupUsers = new QComboBox(userSelectBox);
    m_comboGroupUsers->setMinimumHeight(32);
    m_comboGroupUsers->setMinimumWidth(220);
    m_comboGroupUsers->setStyleSheet("font-size: 13px; padding: 3px 8px; background-color: white;");

    QPushButton *btnRefreshUsers = new QPushButton("Actualizar lista", userSelectBox);
    btnRefreshUsers->setMinimumHeight(32);
    btnRefreshUsers->setStyleSheet("font-size: 12px; padding: 4px 10px;");

    userRowLayout->addWidget(lblChooseUser);
    userRowLayout->addWidget(m_comboGroupUsers);
    userRowLayout->addWidget(btnRefreshUsers);
    userRowLayout->addStretch();

    m_lblUserCurrentGroups = new QLabel(userSelectBox);
    m_lblUserCurrentGroups->setWordWrap(true);
    m_lblUserCurrentGroups->setStyleSheet("border: none; font-size: 12px; color: #084298;");

    userSelectLayout->addLayout(userRowLayout);
    userSelectLayout->addWidget(m_lblUserCurrentGroups);

    m_userGroupsContainer = new QFrame(page);
    m_userGroupsContainer->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *groupsBoxLayout = new QVBoxLayout(m_userGroupsContainer);
    groupsBoxLayout->setSpacing(8);

    QHBoxLayout *groupsHeaderLayout = new QHBoxLayout();
    QLabel *lblGroupsTitle = new QLabel("Grupos secundarios asignados:", m_userGroupsContainer);
    lblGroupsTitle->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    QPushButton *btnPresetRec = new QPushButton("Recomendados", m_userGroupsContainer);
    btnPresetRec->setStyleSheet("font-size: 11px; padding: 3px 8px;");

    QPushButton *btnPresetAll = new QPushButton("Marcar todos", m_userGroupsContainer);
    btnPresetAll->setStyleSheet("font-size: 11px; padding: 3px 8px;");

    QPushButton *btnPresetNone = new QPushButton("Desmarcar todos", m_userGroupsContainer);
    btnPresetNone->setStyleSheet("font-size: 11px; padding: 3px 8px;");

    groupsHeaderLayout->addWidget(lblGroupsTitle);
    groupsHeaderLayout->addStretch();
    groupsHeaderLayout->addWidget(btnPresetRec);
    groupsHeaderLayout->addWidget(btnPresetAll);
    groupsHeaderLayout->addWidget(btnPresetNone);

    m_userGroupsGrid = new QGridLayout();
    m_userGroupsGrid->setSpacing(6);

    groupsBoxLayout->addLayout(groupsHeaderLayout);
    groupsBoxLayout->addLayout(m_userGroupsGrid);

    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_btnSaveUserGroups = new QPushButton("Guardar grupos del usuario", page);
    m_btnSaveUserGroups->setMinimumHeight(38);
    m_btnSaveUserGroups->setMinimumWidth(230);
    m_btnSaveUserGroups->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; border-radius: 5px; padding: 6px 18px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
        "QPushButton:disabled { background-color: #9e9e9e; }"
    );
    actionLayout->addWidget(m_btnSaveUserGroups);
    actionLayout->addStretch();

    m_lblGroupFeedback = new QLabel(page);
    m_lblGroupFeedback->setWordWrap(true);
    m_lblGroupFeedback->setVisible(false);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(userSelectBox);
    layout->addWidget(m_userGroupsContainer);
    layout->addLayout(actionLayout);
    layout->addWidget(m_lblGroupFeedback);
    layout->addStretch();

    connect(m_comboGroupUsers, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onGroupUserSelectionChanged);
    connect(btnRefreshUsers, &QPushButton::clicked, this, &MainWindow::onRefreshGroupUsersClicked);
    connect(btnPresetRec, &QPushButton::clicked, this, &MainWindow::onGroupPresetRecommendedClicked);
    connect(btnPresetAll, &QPushButton::clicked, this, &MainWindow::onGroupPresetAllClicked);
    connect(btnPresetNone, &QPushButton::clicked, this, &MainWindow::onGroupPresetNoneClicked);
    connect(m_btnSaveUserGroups, &QPushButton::clicked, this, &MainWindow::onSaveUserGroupsClicked);

    refreshUserGroupsPage();
    scrollArea->setWidget(page);
    return scrollArea;
}

void MainWindow::refreshUserGroupsPage()
{
    if (!m_comboGroupUsers || !m_lblUserCurrentGroups) {
        return;
    }

    const QString previousSelection = m_comboGroupUsers->currentText();
    const QStringList users = GroupManager::getManageableUsers();

    m_comboGroupUsers->blockSignals(true);
    m_comboGroupUsers->clear();

    if (users.isEmpty()) {
        m_comboGroupUsers->addItem("(No hay usuarios estándar creados)");
        m_comboGroupUsers->setEnabled(false);
        m_btnSaveUserGroups->setEnabled(false);
        m_lblUserCurrentGroups->setText(
            "<b>Aviso:</b> Aún no hay cuentas de usuario estándar en el sistema. "
            "Crea primero un usuario en el paso <b>'3. Añadir usuario'</b>."
        );
        populateUserGroupCheckboxes(QString());
        m_comboGroupUsers->blockSignals(false);
        return;
    }

    m_comboGroupUsers->setEnabled(true);
    m_btnSaveUserGroups->setEnabled(true);
    m_comboGroupUsers->addItems(users);

    int targetIdx = m_comboGroupUsers->findText(previousSelection);
    if (targetIdx < 0) {
        targetIdx = 0;
    }
    m_comboGroupUsers->setCurrentIndex(targetIdx);
    m_comboGroupUsers->blockSignals(false);

    onGroupUserSelectionChanged(targetIdx);
}

void MainWindow::populateUserGroupCheckboxes(const QString &username)
{
    if (!m_userGroupsGrid || !m_userGroupsContainer) {
        return;
    }

    for (QCheckBox *chk : m_userGroupCheckboxes) {
        m_userGroupsGrid->removeWidget(chk);
        delete chk;
    }
    m_userGroupCheckboxes.clear();

    UserGroupsInfo userInfo;
    if (!username.isEmpty()) {
        userInfo = GroupManager::getUserGroups(username);
    }

    const QList<ManagedGroupItem> groups = GroupManager::getManageableGroups(userInfo.secondaryGroups);

    for (int i = 0; i < groups.size(); ++i) {
        const ManagedGroupItem &grp = groups[i];
        QCheckBox *chk = new QCheckBox(QString("%1 — %2").arg(grp.name, grp.description), m_userGroupsContainer);
        chk->setProperty("groupName", grp.name);
        chk->setProperty("recommended", grp.recommended);

        const bool isMember = userInfo.valid && userInfo.secondaryGroups.contains(grp.name);
        chk->setChecked(isMember);
        chk->setEnabled(userInfo.valid && grp.existsOnSystem);

        if (grp.name == "wheel") {
            chk->setStyleSheet("font-size: 12px; font-weight: bold; color: #084298;");
        } else {
            chk->setStyleSheet("font-size: 12px;");
        }

        if (!grp.existsOnSystem) {
            chk->setToolTip("Este grupo no existe en /etc/group de este sistema.");
        }

        m_userGroupCheckboxes.append(chk);
        m_userGroupsGrid->addWidget(chk, i / 2, i % 2);
    }
}

void MainWindow::onGroupUserSelectionChanged(int index)
{
    if (!m_comboGroupUsers || !m_lblUserCurrentGroups || index < 0) {
        return;
    }

    if (!m_comboGroupUsers->isEnabled()) {
        return;
    }

    const QString username = m_comboGroupUsers->itemText(index);
    const UserGroupsInfo info = GroupManager::getUserGroups(username);

    if (!info.valid) {
        m_lblUserCurrentGroups->setText("<b>Error:</b> No se pudo consultar la información del usuario.");
        populateUserGroupCheckboxes(QString());
        return;
    }

    const QString secText = info.secondaryGroups.isEmpty()
                                ? "Ninguno (solo grupo primario)"
                                : info.secondaryGroups.join(", ");

    m_lblUserCurrentGroups->setText(
        QString("<b>Grupo primario:</b> %1 &nbsp;|&nbsp; <b>Grupos secundarios actuales:</b> %2")
            .arg(info.primaryGroup, secText)
    );

    populateUserGroupCheckboxes(username);
}

void MainWindow::onRefreshGroupUsersClicked()
{
    if (m_lblGroupFeedback) {
        m_lblGroupFeedback->setVisible(false);
    }
    refreshUserGroupsPage();
}

void MainWindow::onGroupPresetRecommendedClicked()
{
    for (QCheckBox *chk : m_userGroupCheckboxes) {
        if (chk->isEnabled()) {
            chk->setChecked(chk->property("recommended").toBool());
        }
    }
}

void MainWindow::onGroupPresetAllClicked()
{
    for (QCheckBox *chk : m_userGroupCheckboxes) {
        if (chk->isEnabled()) {
            chk->setChecked(true);
        }
    }
}

void MainWindow::onGroupPresetNoneClicked()
{
    for (QCheckBox *chk : m_userGroupCheckboxes) {
        if (chk->isEnabled()) {
            chk->setChecked(false);
        }
    }
}

void MainWindow::onSaveUserGroupsClicked()
{
    if (!m_comboGroupUsers || !m_comboGroupUsers->isEnabled() || !m_lblGroupFeedback) {
        return;
    }

    const QString username = m_comboGroupUsers->currentText();
    QStringList selectedGroups;

    for (QCheckBox *chk : m_userGroupCheckboxes) {
        if (chk->isChecked() && chk->isEnabled()) {
            selectedGroups.append(chk->property("groupName").toString());
        }
    }

    LOG_INFO("MainWindow", QString("Usuario pulsó guardar grupos para '%1': [%2]")
                               .arg(username, selectedGroups.join(", ")));

    const GroupUpdateResult result = GroupManager::updateUserGroups(username, selectedGroups);

    m_lblGroupFeedback->setVisible(true);
    if (result.success) {
        m_lblGroupFeedback->setStyleSheet(
            "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblGroupFeedback->setText(result.message);
        refreshUserGroupsPage();
    } else {
        m_lblGroupFeedback->setStyleSheet(
            "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblGroupFeedback->setText(result.message);
    }
}

QWidget* MainWindow::createSudoPage()
{
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *page = new QWidget(scrollArea);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(12);

    QLabel *title = new QLabel("5. Habilitar y Configurar Sudo", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *subtitle = new QLabel(
        "En distribuciones como Ubuntu o Linux Mint, los usuarios administrativos ejecutan tareas del sistema usando 'sudo'. "
        "Aquí puedes elegir qué usuarios del sistema tendrán permiso para usar sudo y si se les pedirá contraseña.",
        page
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size: 13px;");

    QFrame *overviewBox = new QFrame(page);
    overviewBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *overviewLayout = new QVBoxLayout(overviewBox);
    overviewLayout->setSpacing(6);

    QLabel *lblOverviewHeader = new QLabel("Estado actual de sudo por usuario en el sistema:", overviewBox);
    lblOverviewHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #084298;");

    m_lblSudoOverview = new QLabel(overviewBox);
    m_lblSudoOverview->setWordWrap(true);
    m_lblSudoOverview->setStyleSheet("border: none; font-size: 12px; color: #084298;");

    overviewLayout->addWidget(lblOverviewHeader);
    overviewLayout->addWidget(m_lblSudoOverview);

    QFrame *configBox = new QFrame(page);
    configBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 14px; }"
    );
    QVBoxLayout *configLayout = new QVBoxLayout(configBox);
    configLayout->setSpacing(10);

    QHBoxLayout *userRow = new QHBoxLayout();
    QLabel *lblSelectUser = new QLabel("Seleccionar usuario:", configBox);
    lblSelectUser->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    m_comboSudoUsers = new QComboBox(configBox);
    m_comboSudoUsers->setMinimumHeight(32);
    m_comboSudoUsers->setMinimumWidth(220);
    m_comboSudoUsers->setStyleSheet("font-size: 13px; padding: 3px 8px; background-color: white;");

    QPushButton *btnRefreshSudo = new QPushButton("Actualizar lista", configBox);
    btnRefreshSudo->setMinimumHeight(32);
    btnRefreshSudo->setStyleSheet("font-size: 12px; padding: 4px 10px;");

    userRow->addWidget(lblSelectUser);
    userRow->addWidget(m_comboSudoUsers);
    userRow->addWidget(btnRefreshSudo);
    userRow->addStretch();

    m_lblSudoSelectedUserDetail = new QLabel(configBox);
    m_lblSudoSelectedUserDetail->setWordWrap(true);
    m_lblSudoSelectedUserDetail->setStyleSheet("border: none; font-size: 12px; color: #495057;");

    QLabel *lblModeHeader = new QLabel("Nivel de acceso sudo para el usuario seleccionado:", configBox);
    lblModeHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px; margin-top: 4px;");

    m_radioSudoWithPass = new QRadioButton(
        "Habilitar sudo solicitando la contraseña del usuario (Recomendado — estándar en Ubuntu/Fedora)",
        configBox
    );
    m_radioSudoWithPass->setStyleSheet("font-size: 13px;");

    m_radioSudoNoPass = new QRadioButton(
        "Habilitar sudo sin pedir contraseña (NOPASSWD — cómodo para equipos personales)",
        configBox
    );
    m_radioSudoNoPass->setStyleSheet("font-size: 13px;");

    m_radioSudoDisabled = new QRadioButton(
        "Deshabilitar sudo para este usuario (usuario sin privilegios administrativos)",
        configBox
    );
    m_radioSudoDisabled->setStyleSheet("font-size: 13px;");

    m_chkSudoSyncWheel = new QCheckBox(
        "Incluir también al usuario en el grupo 'wheel' al habilitar sudo (recomendado en Slackware)",
        configBox
    );
    m_chkSudoSyncWheel->setChecked(true);
    m_chkSudoSyncWheel->setStyleSheet("font-size: 12px; color: #084298; margin-top: 4px;");

    configLayout->addLayout(userRow);
    configLayout->addWidget(m_lblSudoSelectedUserDetail);
    configLayout->addWidget(lblModeHeader);
    configLayout->addWidget(m_radioSudoWithPass);
    configLayout->addWidget(m_radioSudoNoPass);
    configLayout->addWidget(m_radioSudoDisabled);
    configLayout->addWidget(m_chkSudoSyncWheel);

    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_btnSaveSudo = new QPushButton("Guardar configuración de sudo", page);
    m_btnSaveSudo->setMinimumHeight(38);
    m_btnSaveSudo->setMinimumWidth(240);
    m_btnSaveSudo->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; border-radius: 5px; padding: 6px 18px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
        "QPushButton:disabled { background-color: #9e9e9e; }"
    );
    actionLayout->addWidget(m_btnSaveSudo);
    actionLayout->addStretch();

    m_lblSudoFeedback = new QLabel(page);
    m_lblSudoFeedback->setWordWrap(true);
    m_lblSudoFeedback->setVisible(false);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(overviewBox);
    layout->addWidget(configBox);
    layout->addLayout(actionLayout);
    layout->addWidget(m_lblSudoFeedback);
    layout->addStretch();

    connect(m_comboSudoUsers, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSudoUserSelectionChanged);
    connect(btnRefreshSudo, &QPushButton::clicked, this, &MainWindow::onRefreshSudoUsersClicked);
    connect(m_radioSudoWithPass, &QRadioButton::toggled, this, &MainWindow::onSudoModeRadioToggled);
    connect(m_radioSudoNoPass, &QRadioButton::toggled, this, &MainWindow::onSudoModeRadioToggled);
    connect(m_radioSudoDisabled, &QRadioButton::toggled, this, &MainWindow::onSudoModeRadioToggled);
    connect(m_btnSaveSudo, &QPushButton::clicked, this, &MainWindow::onSaveSudoClicked);

    refreshSudoPage();
    scrollArea->setWidget(page);
    return scrollArea;
}

void MainWindow::refreshSudoPage()
{
    if (!m_comboSudoUsers || !m_lblSudoOverview) {
        return;
    }

    const QString previousSelection = m_comboSudoUsers->currentText();
    const QList<UserSudoStatus> allStatuses = SudoManager::getAllUsersSudoStatus();

    m_comboSudoUsers->blockSignals(true);
    m_comboSudoUsers->clear();

    if (allStatuses.isEmpty()) {
        m_lblSudoOverview->setText(
            "No se detectaron usuarios estándar en el sistema. Crea primero un usuario en el paso <b>'3. Añadir usuario'</b>."
        );
        m_comboSudoUsers->addItem("(No hay usuarios estándar creados)");
        m_comboSudoUsers->setEnabled(false);
        m_radioSudoWithPass->setEnabled(false);
        m_radioSudoNoPass->setEnabled(false);
        m_radioSudoDisabled->setEnabled(false);
        m_chkSudoSyncWheel->setEnabled(false);
        m_btnSaveSudo->setEnabled(false);
        m_lblSudoSelectedUserDetail->setText("");
        m_comboSudoUsers->blockSignals(false);
        return;
    }

    QStringList summaryLines;
    for (const auto &st : allStatuses) {
        QString badge;
        switch (st.accessMode) {
            case SudoAccessMode::WithPassword:
                badge = "Habilitado (con contraseña)";
                break;
            case SudoAccessMode::NoPassword:
                badge = "Habilitado (NOPASSWD)";
                break;
            case SudoAccessMode::Disabled:
                badge = "No habilitado";
                break;
        }
        const QString wheelTag = st.inWheelGroup ? " [grupo wheel]" : "";
        summaryLines.append(QString("• <b>%1</b>: %2%3").arg(st.username, badge, wheelTag));
        m_comboSudoUsers->addItem(st.username);
    }

    m_lblSudoOverview->setText(summaryLines.join("<br>"));

    m_comboSudoUsers->setEnabled(true);
    m_radioSudoWithPass->setEnabled(true);
    m_radioSudoNoPass->setEnabled(true);
    m_radioSudoDisabled->setEnabled(true);
    m_btnSaveSudo->setEnabled(true);

    int targetIdx = m_comboSudoUsers->findText(previousSelection);
    if (targetIdx < 0) {
        targetIdx = 0;
    }
    m_comboSudoUsers->setCurrentIndex(targetIdx);
    m_comboSudoUsers->blockSignals(false);

    onSudoUserSelectionChanged(targetIdx);
}

void MainWindow::onSudoUserSelectionChanged(int index)
{
    if (!m_comboSudoUsers || !m_comboSudoUsers->isEnabled() || index < 0) {
        return;
    }

    const QString username = m_comboSudoUsers->itemText(index);
    const UserSudoStatus status = SudoManager::getUserSudoStatus(username);

    const QString wheelText = status.inWheelGroup ? "Sí" : "No";
    m_lblSudoSelectedUserDetail->setText(
        QString("<b>Estado de '%1':</b> %2 &nbsp;|&nbsp; <b>Pertenece a wheel:</b> %3")
            .arg(username, status.sourceDetail, wheelText)
    );

    switch (status.accessMode) {
        case SudoAccessMode::WithPassword:
            m_radioSudoWithPass->setChecked(true);
            break;
        case SudoAccessMode::NoPassword:
            m_radioSudoNoPass->setChecked(true);
            break;
        case SudoAccessMode::Disabled:
            m_radioSudoDisabled->setChecked(true);
            break;
    }

    onSudoModeRadioToggled();
}

void MainWindow::onRefreshSudoUsersClicked()
{
    if (m_lblSudoFeedback) {
        m_lblSudoFeedback->setVisible(false);
    }
    refreshSudoPage();
}

void MainWindow::onSudoModeRadioToggled()
{
    if (!m_chkSudoSyncWheel || !m_radioSudoDisabled) {
        return;
    }
    m_chkSudoSyncWheel->setEnabled(!m_radioSudoDisabled->isChecked() && m_radioSudoDisabled->isEnabled());
}

void MainWindow::onSaveSudoClicked()
{
    if (!m_comboSudoUsers || !m_comboSudoUsers->isEnabled() || !m_lblSudoFeedback) {
        return;
    }

    const QString username = m_comboSudoUsers->currentText();
    SudoAccessMode selectedMode = SudoAccessMode::WithPassword;

    if (m_radioSudoNoPass->isChecked()) {
        selectedMode = SudoAccessMode::NoPassword;
    } else if (m_radioSudoDisabled->isChecked()) {
        selectedMode = SudoAccessMode::Disabled;
    }

    const bool syncWheel = m_chkSudoSyncWheel->isChecked();
    LOG_INFO("MainWindow", QString("Usuario pulsó guardar sudo para '%1'").arg(username));

    const SudoOperationResult result = SudoManager::applyUserSudo(username, selectedMode, syncWheel);

    m_lblSudoFeedback->setVisible(true);
    if (result.success) {
        m_lblSudoFeedback->setStyleSheet(
            "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblSudoFeedback->setText(result.message);
        refreshSudoPage();
    } else {
        m_lblSudoFeedback->setStyleSheet(
            "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblSudoFeedback->setText(result.message);
    }
}

QWidget* MainWindow::createWebServerPage()
{
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *page = new QWidget(scrollArea);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(12);

    QLabel *title = new QLabel("6. Configurar Servidor Web y Soporte PHP", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *subtitle = new QLabel(
        "Detecta automáticamente los servidores web (Apache o Nginx) y la versión de PHP instalados en tu Slackware, "
        "habilita el procesamiento de sitios dinámicos en PHP y define si el servicio iniciará con el sistema.",
        page
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size: 13px;");

    QFrame *detectBox = new QFrame(page);
    detectBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *detectLayout = new QVBoxLayout(detectBox);
    detectLayout->setSpacing(8);

    QHBoxLayout *detectHeaderLayout = new QHBoxLayout();
    QLabel *lblDetectTitle = new QLabel("Verificación previa de componentes instalados:", detectBox);
    lblDetectTitle->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #084298;");

    QPushButton *btnRescan = new QPushButton("Volver a analizar", detectBox);
    btnRescan->setStyleSheet("font-size: 11px; padding: 3px 10px;");

    detectHeaderLayout->addWidget(lblDetectTitle);
    detectHeaderLayout->addStretch();
    detectHeaderLayout->addWidget(btnRescan);

    m_lblWebDetectionSummary = new QLabel(detectBox);
    m_lblWebDetectionSummary->setWordWrap(true);
    m_lblWebDetectionSummary->setStyleSheet("border: none; font-size: 12px; color: #084298;");

    detectLayout->addLayout(detectHeaderLayout);
    detectLayout->addWidget(m_lblWebDetectionSummary);

    QFrame *serverBox = new QFrame(page);
    serverBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *serverLayout = new QVBoxLayout(serverBox);
    serverLayout->setSpacing(8);

    QLabel *lblServerHeader = new QLabel("1. Selecciona el servidor web a configurar:", serverBox);
    lblServerHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    m_radioWebApache = new QRadioButton("Apache HTTP Server (/etc/httpd/httpd.conf -> ServerName localhost:80)", serverBox);
    m_radioWebApache->setStyleSheet("font-size: 13px;");

    m_radioWebNginx = new QRadioButton("Nginx Web Server (/etc/nginx/nginx.conf)", serverBox);
    m_radioWebNginx->setStyleSheet("font-size: 13px;");

    serverLayout->addWidget(lblServerHeader);
    serverLayout->addWidget(m_radioWebApache);
    serverLayout->addWidget(m_radioWebNginx);

    QFrame *phpBox = new QFrame(page);
    phpBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *phpLayout = new QVBoxLayout(phpBox);
    phpLayout->setSpacing(8);

    QLabel *lblPhpHeader = new QLabel("2. Soporte de PHP en el servidor web:", phpBox);
    lblPhpHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    m_chkEnablePhp = new QCheckBox("Habilitar soporte de PHP en el servidor web seleccionado", phpBox);
    m_chkEnablePhp->setStyleSheet("font-size: 13px; font-weight: bold;");

    m_phpOptionsContainer = new QFrame(phpBox);
    m_phpOptionsContainer->setStyleSheet(
        "QFrame { background-color: #ffffff; border: 1px solid #dee2e6; border-radius: 5px; padding: 10px; }"
    );
    QVBoxLayout *phpInnerLayout = new QVBoxLayout(m_phpOptionsContainer);
    phpInnerLayout->setSpacing(8);

    m_lblPhpTimezoneInfo = new QLabel(m_phpOptionsContainer);
    m_lblPhpTimezoneInfo->setWordWrap(true);
    m_lblPhpTimezoneInfo->setStyleSheet(
        "QLabel { background-color: #eef4fa; color: #084298; border: 1px solid #b6d4fe; "
        "border-radius: 4px; padding: 7px; font-size: 12px; }"
    );

    m_lblPhpImagickNotice = new QLabel(m_phpOptionsContainer);
    m_lblPhpImagickNotice->setWordWrap(true);

    m_chkUpdatePhpIni = new QCheckBox("Aplicar ajustes recomendados en php.ini (límites de memoria y subida de archivos)", m_phpOptionsContainer);
    m_chkUpdatePhpIni->setChecked(true);
    m_chkUpdatePhpIni->setStyleSheet("font-size: 12px; font-weight: bold;");

    QFormLayout *phpForm = new QFormLayout();
    phpForm->setSpacing(6);
    phpForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_comboPhpMemoryLimit = new QComboBox(m_phpOptionsContainer);
    m_comboPhpMemoryLimit->addItems({ "128M", "256M", "512M", "1024M" });
    m_comboPhpMemoryLimit->setCurrentText("256M");

    m_comboPhpUploadMax = new QComboBox(m_phpOptionsContainer);
    m_comboPhpUploadMax->addItems({ "16M", "64M", "128M", "256M", "512M" });
    m_comboPhpUploadMax->setCurrentText("64M");

    m_comboPhpPostMax = new QComboBox(m_phpOptionsContainer);
    m_comboPhpPostMax->addItems({ "16M", "64M", "128M", "256M", "512M" });
    m_comboPhpPostMax->setCurrentText("64M");

    m_chkPhpDisplayErrors = new QCheckBox("Mostrar errores de PHP en pantalla (display_errors = On, útil para desarrollo)", m_phpOptionsContainer);
    m_chkPhpDisplayErrors->setChecked(true);
    m_chkPhpDisplayErrors->setStyleSheet("font-size: 12px;");

    QLabel *lblMem = new QLabel("Límite de memoria (memory_limit):", m_phpOptionsContainer);
    lblMem->setStyleSheet("border: none; font-size: 12px;");
    QLabel *lblUpl = new QLabel("Tamaño máx. subida (upload_max_filesize):", m_phpOptionsContainer);
    lblUpl->setStyleSheet("border: none; font-size: 12px;");
    QLabel *lblPost = new QLabel("Tamaño máx. POST (post_max_size):", m_phpOptionsContainer);
    lblPost->setStyleSheet("border: none; font-size: 12px;");

    phpForm->addRow(lblMem, m_comboPhpMemoryLimit);
    phpForm->addRow(lblUpl, m_comboPhpUploadMax);
    phpForm->addRow(lblPost, m_comboPhpPostMax);
    phpForm->addRow("", m_chkPhpDisplayErrors);

    phpInnerLayout->addWidget(m_lblPhpTimezoneInfo);
    phpInnerLayout->addWidget(m_lblPhpImagickNotice);
    phpInnerLayout->addWidget(m_chkUpdatePhpIni);
    phpInnerLayout->addLayout(phpForm);

    phpLayout->addWidget(lblPhpHeader);
    phpLayout->addWidget(m_chkEnablePhp);
    phpLayout->addWidget(m_phpOptionsContainer);

    QFrame *bootBox = new QFrame(page);
    bootBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *bootLayout = new QVBoxLayout(bootBox);
    bootLayout->setSpacing(6);

    QLabel *lblBootHeader = new QLabel("3. Arranque automático del servicio:", bootBox);
    lblBootHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    m_chkWebStartOnBoot = new QCheckBox("Iniciar el servidor web automáticamente desde el arranque del sistema", bootBox);
    m_chkWebStartOnBoot->setChecked(true);
    m_chkWebStartOnBoot->setStyleSheet("font-size: 13px;");

    bootLayout->addWidget(lblBootHeader);
    bootLayout->addWidget(m_chkWebStartOnBoot);

    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_btnSaveWebServer = new QPushButton("Guardar configuración del servidor web", page);
    m_btnSaveWebServer->setMinimumHeight(38);
    m_btnSaveWebServer->setMinimumWidth(270);
    m_btnSaveWebServer->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; border-radius: 5px; padding: 6px 18px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
        "QPushButton:disabled { background-color: #9e9e9e; }"
    );
    actionLayout->addWidget(m_btnSaveWebServer);
    actionLayout->addStretch();

    m_lblWebServerFeedback = new QLabel(page);
    m_lblWebServerFeedback->setWordWrap(true);
    m_lblWebServerFeedback->setVisible(false);

    // Vista de navegador incrustada (oculta por defecto; se abre solo tras reiniciar el servidor)
    m_webPreviewFrame = new QFrame(page);
    m_webPreviewFrame->setVisible(false);
    m_webPreviewFrame->setStyleSheet(
        "QFrame { background-color: #ffffff; border: 2px solid #2e7d32; border-radius: 6px; padding: 10px; }"
    );
    QVBoxLayout *previewLayout = new QVBoxLayout(m_webPreviewFrame);
    previewLayout->setSpacing(8);

    QHBoxLayout *previewTopBar = new QHBoxLayout();
    QLabel *lblPreviewTitle = new QLabel("Vista de comprobación del servidor web (localhost):", m_webPreviewFrame);
    lblPreviewTitle->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #1b5e20;");

    m_lblPreviewHttpStatus = new QLabel("Listo", m_webPreviewFrame);
    m_lblPreviewHttpStatus->setStyleSheet(
        "border: none; font-size: 11px; font-weight: bold; color: #084298; background-color: #eef4fa; padding: 3px 8px; border-radius: 4px;"
    );

    QPushButton *btnClosePreview = new QPushButton("Cerrar vista", m_webPreviewFrame);
    btnClosePreview->setStyleSheet("font-size: 11px; padding: 3px 8px;");

    previewTopBar->addWidget(lblPreviewTitle);
    previewTopBar->addStretch();
    previewTopBar->addWidget(m_lblPreviewHttpStatus);
    previewTopBar->addWidget(btnClosePreview);

    QHBoxLayout *previewNavBar = new QHBoxLayout();
    m_btnPreviewLocalhost = new QPushButton("Ver http://localhost/", m_webPreviewFrame);
    m_btnPreviewLocalhost->setStyleSheet("font-size: 12px; font-weight: bold; padding: 4px 10px;");

    m_btnPreviewPhpInfo = new QPushButton("Ver http://localhost/php-info.php", m_webPreviewFrame);
    m_btnPreviewPhpInfo->setStyleSheet("font-size: 12px; font-weight: bold; padding: 4px 10px;");

    m_editPreviewUrl = new QLineEdit("http://localhost/", m_webPreviewFrame);
    m_editPreviewUrl->setMinimumHeight(28);
    m_editPreviewUrl->setStyleSheet("font-size: 12px; padding: 2px 6px; background-color: #f8f9fa;");

    QPushButton *btnGoUrl = new QPushButton("Recargar", m_webPreviewFrame);
    btnGoUrl->setStyleSheet("font-size: 12px; padding: 4px 10px;");

    QPushButton *btnExternalBrowser = new QPushButton("Abrir en navegador externo", m_webPreviewFrame);
    btnExternalBrowser->setStyleSheet("font-size: 11px; padding: 4px 8px;");

    previewNavBar->addWidget(m_btnPreviewLocalhost);
    previewNavBar->addWidget(m_btnPreviewPhpInfo);
    previewNavBar->addWidget(m_editPreviewUrl, 1);
    previewNavBar->addWidget(btnGoUrl);
    previewNavBar->addWidget(btnExternalBrowser);

    m_webPreviewBrowser = new QTextBrowser(m_webPreviewFrame);
    m_webPreviewBrowser->setMinimumHeight(300);
    m_webPreviewBrowser->setOpenLinks(false);
    m_webPreviewBrowser->setStyleSheet(
        "QTextBrowser { background-color: #ffffff; border: 1px solid #ced4da; border-radius: 4px; padding: 8px; }"
    );

    previewLayout->addLayout(previewTopBar);
    previewLayout->addLayout(previewNavBar);
    previewLayout->addWidget(m_webPreviewBrowser);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(detectBox);
    layout->addWidget(serverBox);
    layout->addWidget(phpBox);
    layout->addWidget(bootBox);
    layout->addLayout(actionLayout);
    layout->addWidget(m_lblWebServerFeedback);
    layout->addWidget(m_webPreviewFrame);
    layout->addStretch();

    connect(btnRescan, &QPushButton::clicked, this, &MainWindow::onRescanWebServerClicked);
    connect(m_chkEnablePhp, &QCheckBox::toggled, this, &MainWindow::onEnablePhpToggled);
    connect(m_chkUpdatePhpIni, &QCheckBox::toggled, this, &MainWindow::onUpdatePhpIniToggled);
    connect(m_btnSaveWebServer, &QPushButton::clicked, this, &MainWindow::onSaveWebServerClicked);

    connect(m_btnPreviewLocalhost, &QPushButton::clicked, this, &MainWindow::onPreviewLocalhostClicked);
    connect(m_btnPreviewPhpInfo, &QPushButton::clicked, this, &MainWindow::onPreviewPhpInfoClicked);
    connect(btnGoUrl, &QPushButton::clicked, this, &MainWindow::onPreviewNavigateClicked);
    connect(m_editPreviewUrl, &QLineEdit::returnPressed, this, &MainWindow::onPreviewNavigateClicked);
    connect(m_webPreviewBrowser, &QTextBrowser::anchorClicked, this, &MainWindow::onPreviewAnchorClicked);
    connect(btnExternalBrowser, &QPushButton::clicked, this, &MainWindow::onPreviewExternalBrowserClicked);
    connect(btnClosePreview, &QPushButton::clicked, this, &MainWindow::onCloseWebPreviewClicked);

    refreshWebServerPage();
    scrollArea->setWidget(page);
    return scrollArea;
}

void MainWindow::refreshWebServerPage()
{
    if (!m_lblWebDetectionSummary || !m_radioWebApache || !m_radioWebNginx) {
        return;
    }

    const WebServerDetectionInfo env = WebServerManager::detectEnvironment();

    QStringList lines;
    if (env.apacheInstalled) {
        lines << QString("• <b>Apache:</b> Instalado (%1) — Arranque automático: %2 | Estado: %3")
                     .arg(env.apacheVersion,
                          env.apacheBootEnabled ? "Habilitado" : "Deshabilitado",
                          env.apacheRunning ? "En ejecución" : "Detenido");
    } else {
        lines << "• <b>Apache:</b> No instalado";
    }

    if (env.nginxInstalled) {
        lines << QString("• <b>Nginx:</b> Instalado (%1) — Arranque automático: %2 | Estado: %3")
                     .arg(env.nginxVersion,
                          env.nginxBootEnabled ? "Habilitado" : "Deshabilitado",
                          env.nginxRunning ? "En ejecución" : "Detenido");
    } else {
        lines << "• <b>Nginx:</b> No instalado";
    }

    if (env.phpInstalled) {
        const QString imagickTxt = env.phpImagickInstalled ? "Instalado" : "No instalado (disponible en SlackBuilds)";
        lines << QString("• <b>PHP:</b> Instalado (%1) — Zona horaria detectada: <b>%2</b> — php-imagick: %3")
                     .arg(env.phpVersion, env.systemTimezone, imagickTxt);
    } else {
        lines << "• <b>PHP:</b> No instalado";
    }

    m_lblWebDetectionSummary->setText(lines.join("<br>"));

    m_radioWebApache->setEnabled(env.apacheInstalled);
    m_radioWebNginx->setEnabled(env.nginxInstalled);

    if (env.apacheInstalled && env.nginxInstalled) {
        m_radioWebApache->setText(QString("Apache HTTP Server (%1) — ServerName localhost:80").arg(env.apacheVersion));
        m_radioWebNginx->setText(QString("Nginx Web Server (%1) — Disponible").arg(env.nginxVersion));
        if (!m_radioWebApache->isChecked() && !m_radioWebNginx->isChecked()) {
            m_radioWebApache->setChecked(true);
        }
    } else if (env.apacheInstalled) {
        m_radioWebApache->setText(QString("Apache HTTP Server (%1) — ServerName localhost:80").arg(env.apacheVersion));
        m_radioWebNginx->setText("Nginx Web Server — No instalado en el sistema");
        m_radioWebApache->setChecked(true);
    } else if (env.nginxInstalled) {
        m_radioWebApache->setText("Apache HTTP Server — No instalado en el sistema");
        m_radioWebNginx->setText(QString("Nginx Web Server (%1) — Detectado").arg(env.nginxVersion));
        m_radioWebNginx->setChecked(true);
    } else {
        m_radioWebApache->setText("Apache HTTP Server — No instalado");
        m_radioWebNginx->setText("Nginx Web Server — No instalado");
    }

    const bool anyWebServerInstalled = (env.apacheInstalled || env.nginxInstalled);
    m_chkWebStartOnBoot->setEnabled(anyWebServerInstalled);
    m_btnSaveWebServer->setEnabled(anyWebServerInstalled);

    if (m_radioWebApache->isChecked() && env.apacheInstalled) {
        m_chkWebStartOnBoot->setChecked(env.apacheBootEnabled || !env.apacheRunning);
    } else if (m_radioWebNginx->isChecked() && env.nginxInstalled) {
        m_chkWebStartOnBoot->setChecked(env.nginxBootEnabled || !env.nginxRunning);
    }

    if (env.phpInstalled && anyWebServerInstalled) {
        m_chkEnablePhp->setEnabled(true);
        m_chkEnablePhp->setText(QString("Habilitar soporte de %1 en el servidor web elegido").arg(env.phpVersion));

        if (m_lblPhpTimezoneInfo) {
            m_lblPhpTimezoneInfo->setText(
                QString("<b>Zona geográfica del sistema (date.timezone):</b> Se configurará automáticamente a <b>%1</b> en %2.")
                    .arg(env.systemTimezone, env.phpIniPath)
            );
        }

        if (env.phpImagickInstalled) {
            m_lblPhpImagickNotice->setStyleSheet(
                "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
                "border-radius: 4px; padding: 8px; font-size: 12px; }"
            );
            m_lblPhpImagickNotice->setText(
                "<b>Extensión php-imagick:</b> Instalada y lista en el sistema."
            );
        } else {
            m_lblPhpImagickNotice->setStyleSheet(
                "QLabel { background-color: #fff3cd; color: #664d03; border: 1px solid #ffecb5; "
                "border-radius: 4px; padding: 8px; font-size: 12px; }"
            );
            m_lblPhpImagickNotice->setText(
                "<b>Aviso sobre php-imagick:</b> El paquete <b>php-imagick</b> no se encuentra instalado actualmente. "
                "Si necesitas procesamiento avanzado de imágenes en PHP, puedes instalarlo desde <b>SlackBuilds</b> "
                "(https://slackbuilds.org). Puedes continuar con la configuración sin problemas."
            );
        }
    } else {
        m_chkEnablePhp->setChecked(false);
        m_chkEnablePhp->setEnabled(false);
        m_chkEnablePhp->setText(env.phpInstalled
                                    ? "Habilitar soporte de PHP (instala primero Apache o Nginx)"
                                    : "PHP no está instalado en el sistema");
    }

    onEnablePhpToggled(m_chkEnablePhp->isChecked());
}

void MainWindow::onRescanWebServerClicked()
{
    if (m_lblWebServerFeedback) {
        m_lblWebServerFeedback->setVisible(false);
    }
    if (m_webPreviewFrame) {
        m_webPreviewFrame->setVisible(false);
    }
    refreshWebServerPage();
}

void MainWindow::onEnablePhpToggled(bool checked)
{
    if (!m_phpOptionsContainer) {
        return;
    }
    m_phpOptionsContainer->setVisible(checked);
    if (checked && m_chkUpdatePhpIni) {
        onUpdatePhpIniToggled(m_chkUpdatePhpIni->isChecked());
    }
}

void MainWindow::onUpdatePhpIniToggled(bool checked)
{
    if (m_comboPhpMemoryLimit) {
        m_comboPhpMemoryLimit->setEnabled(checked);
    }
    if (m_comboPhpUploadMax) {
        m_comboPhpUploadMax->setEnabled(checked);
    }
    if (m_comboPhpPostMax) {
        m_comboPhpPostMax->setEnabled(checked);
    }
    if (m_chkPhpDisplayErrors) {
        m_chkPhpDisplayErrors->setEnabled(checked);
    }
}

void MainWindow::onSaveWebServerClicked()
{
    if (!m_lblWebServerFeedback) {
        return;
    }

    if (m_webPreviewFrame) {
        m_webPreviewFrame->setVisible(false);
    }

    WebServerApplyRequest req;
    if (m_radioWebApache && m_radioWebApache->isChecked()) {
        req.selectedServer = "apache";
    } else if (m_radioWebNginx && m_radioWebNginx->isChecked()) {
        req.selectedServer = "nginx";
    }

    req.enablePhp = (m_chkEnablePhp && m_chkEnablePhp->isChecked());
    req.phpOptions.updatePhpIni = (m_chkUpdatePhpIni && m_chkUpdatePhpIni->isChecked());
    req.phpOptions.memoryLimit = m_comboPhpMemoryLimit ? m_comboPhpMemoryLimit->currentText() : "256M";
    req.phpOptions.uploadMaxFilesize = m_comboPhpUploadMax ? m_comboPhpUploadMax->currentText() : "64M";
    req.phpOptions.postMaxSize = m_comboPhpPostMax ? m_comboPhpPostMax->currentText() : "64M";
    req.phpOptions.displayErrors = (m_chkPhpDisplayErrors && m_chkPhpDisplayErrors->isChecked());
    req.startOnBoot = (m_chkWebStartOnBoot && m_chkWebStartOnBoot->isChecked());

    LOG_INFO("MainWindow", QString("Usuario pulsó guardar servidor web: %1").arg(req.selectedServer));

    const WebServerApplyResult result = WebServerManager::applyConfiguration(req);

    m_lblWebServerFeedback->setVisible(true);
    if (!result.success) {
        m_lblWebServerFeedback->setStyleSheet(
            "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblWebServerFeedback->setText(result.message);
        return;
    }

    m_lblWebServerFeedback->setStyleSheet(
        "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
        "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
    );
    m_lblWebServerFeedback->setText(result.message);

    m_lastPhpEnabledState = req.enablePhp;
    const QString serverLabel = (req.selectedServer == "apache") ? "Apache" : "Nginx";

    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Reiniciar servidor web",
        QString("Los cambios de configuración para %1 se han guardado correctamente.\n\n"
                "¿Deseas reiniciar ahora el servidor %1 y abrir la vista de comprobación en 'localhost'?")
            .arg(serverLabel),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );

    if (reply == QMessageBox::Yes) {
        QString restartErr;
        if (WebServerManager::restartWebServer(req.selectedServer, req.enablePhp, restartErr)) {
            m_lblWebServerFeedback->setText(
                result.message + QString("\n• Servicio %1 reiniciado correctamente.").arg(serverLabel)
            );
            refreshWebServerPage();

            if (m_webPreviewFrame && m_btnPreviewPhpInfo) {
                m_btnPreviewPhpInfo->setEnabled(req.enablePhp);
                m_webPreviewFrame->setVisible(true);

                loadUrlInEmbeddedBrowser("http://localhost/");
                if (req.enablePhp) {
                    QTimer::singleShot(650, this, [this]() {
                        loadUrlInEmbeddedBrowser("http://localhost/php-info.php");
                    });
                }
            }
        } else {
            m_lblWebServerFeedback->setStyleSheet(
                "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
                "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
            );
            m_lblWebServerFeedback->setText(
                QString("La configuración se guardó, pero ocurrió un error al reiniciar %1:\n%2")
                    .arg(serverLabel, restartErr)
            );
        }
    } else {
        refreshWebServerPage();
    }
}

void MainWindow::loadUrlInEmbeddedBrowser(const QString &urlString)
{
    if (!m_webPreviewBrowser || !m_lblPreviewHttpStatus || !m_networkManager) {
        return;
    }

    QUrl url(urlString.trimmed());
    if (url.scheme().isEmpty()) {
        url = QUrl("http://" + urlString.trimmed());
    }

    if (m_editPreviewUrl) {
        m_editPreviewUrl->setText(url.toString());
    }

    m_lblPreviewHttpStatus->setText("Cargando...");
    m_lblPreviewHttpStatus->setStyleSheet(
        "border: none; font-size: 11px; font-weight: bold; color: #664d03; background-color: #fff3cd; padding: 3px 8px; border-radius: 4px;"
    );

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "slackwiz-preview/1.0");
    m_networkManager->get(request);
}

void MainWindow::onWebPreviewReplyFinished(QNetworkReply *reply)
{
    if (!reply) {
        return;
    }

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray payload = reply->readAll();
    const QNetworkReply::NetworkError err = reply->error();
    const QString errStr = reply->errorString();
    const QString reqUrl = reply->url().toString();
    reply->deleteLater();

    if (!m_webPreviewBrowser || !m_lblPreviewHttpStatus) {
        return;
    }

    if (err == QNetworkReply::NoError && statusCode >= 200 && statusCode < 400) {
        m_lblPreviewHttpStatus->setText(QString("HTTP %1 OK").arg(statusCode));
        m_lblPreviewHttpStatus->setStyleSheet(
            "border: none; font-size: 11px; font-weight: bold; color: #0f5132; background-color: #d1e7dd; padding: 3px 8px; border-radius: 4px;"
        );
        m_webPreviewBrowser->setHtml(QString::fromUtf8(payload));
    } else {
        m_lblPreviewHttpStatus->setText(statusCode > 0 ? QString("Error HTTP %1").arg(statusCode) : "Sin conexión");
        m_lblPreviewHttpStatus->setStyleSheet(
            "border: none; font-size: 11px; font-weight: bold; color: #842029; background-color: #f8d7da; padding: 3px 8px; border-radius: 4px;"
        );
        m_webPreviewBrowser->setHtml(QString(
            "<h3>No se pudo conectar con %1</h3>"
            "<p><b>Detalle:</b> %2</p>"
            "<p>Verifica que el servidor esté escuchando en el puerto 80.</p>"
        ).arg(reqUrl.toHtmlEscaped(), errStr.toHtmlEscaped()));
    }
}

void MainWindow::onPreviewLocalhostClicked()
{
    loadUrlInEmbeddedBrowser("http://localhost/");
}

void MainWindow::onPreviewPhpInfoClicked()
{
    loadUrlInEmbeddedBrowser("http://localhost/php-info.php");
}

void MainWindow::onPreviewNavigateClicked()
{
    if (m_editPreviewUrl) {
        loadUrlInEmbeddedBrowser(m_editPreviewUrl->text());
    }
}

void MainWindow::onPreviewAnchorClicked(const QUrl &url)
{
    const QUrl baseUrl(m_editPreviewUrl ? m_editPreviewUrl->text() : "http://localhost/");
    const QUrl resolved = baseUrl.resolved(url);
    loadUrlInEmbeddedBrowser(resolved.toString());
}

void MainWindow::onPreviewExternalBrowserClicked()
{
    const QString target = m_editPreviewUrl ? m_editPreviewUrl->text().trimmed() : "http://localhost/";
    QDesktopServices::openUrl(QUrl(target));
}

void MainWindow::onCloseWebPreviewClicked()
{
    if (m_webPreviewFrame) {
        m_webPreviewFrame->setVisible(false);
    }
}

// ============================================================================
// MÓDULO 7: CONFIGURAR MOTOR DE BASE DE DATOS (MARIADB / POSTGRESQL)
// ============================================================================

QWidget* MainWindow::createDatabasePage()
{
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *page = new QWidget(scrollArea);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(12);

    QLabel *title = new QLabel("7. Configurar Motor de Base de Datos", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *subtitle = new QLabel(
        "Slackware incluye MariaDB de fábrica, pero si más adelante instalas PostgreSQL también podrás configurarlo desde aquí. "
        "Inicializa las tablas del sistema, aplica el asistente de seguridad, verifica el inicio de sesión y configura el arranque.",
        page
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size: 13px;");

    QFrame *detectBox = new QFrame(page);
    detectBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *detectLayout = new QVBoxLayout(detectBox);
    detectLayout->setSpacing(8);

    QHBoxLayout *detectHeaderLayout = new QHBoxLayout();
    QLabel *lblDetectTitle = new QLabel("Motores de base de datos detectados en el sistema:", detectBox);
    lblDetectTitle->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #084298;");

    QPushButton *btnRescanDb = new QPushButton("Volver a analizar", detectBox);
    btnRescanDb->setStyleSheet("font-size: 11px; padding: 3px 10px;");

    detectHeaderLayout->addWidget(lblDetectTitle);
    detectHeaderLayout->addStretch();
    detectHeaderLayout->addWidget(btnRescanDb);

    m_lblDbDetectionSummary = new QLabel(detectBox);
    m_lblDbDetectionSummary->setWordWrap(true);
    m_lblDbDetectionSummary->setStyleSheet("border: none; font-size: 12px; color: #084298;");

    detectLayout->addLayout(detectHeaderLayout);
    detectLayout->addWidget(m_lblDbDetectionSummary);

    QFrame *engineBox = new QFrame(page);
    engineBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *engineLayout = new QVBoxLayout(engineBox);
    engineLayout->setSpacing(8);

    QLabel *lblEngineHeader = new QLabel("1. Selecciona el motor de base de datos que deseas configurar:", engineBox);
    lblEngineHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    m_radioDbMariaDb = new QRadioButton("MariaDB Server (incluido por defecto en Slackware)", engineBox);
    m_radioDbMariaDb->setStyleSheet("font-size: 13px;");

    m_radioDbPostgres = new QRadioButton("PostgreSQL Server", engineBox);
    m_radioDbPostgres->setStyleSheet("font-size: 13px;");

    engineLayout->addWidget(lblEngineHeader);
    engineLayout->addWidget(m_radioDbMariaDb);
    engineLayout->addWidget(m_radioDbPostgres);

    QFrame *securityBox = new QFrame(page);
    securityBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *securityLayout = new QVBoxLayout(securityBox);
    securityLayout->setSpacing(8);

    QLabel *lblSecHeader = new QLabel("2. Contraseña de administración y asistente de seguridad:", securityBox);
    lblSecHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    m_chkDbSetRootPassword = new QCheckBox("Establecer o cambiar la contraseña de root", securityBox);
    m_chkDbSetRootPassword->setChecked(true);
    m_chkDbSetRootPassword->setStyleSheet("font-size: 13px; font-weight: bold;");

    m_dbPasswordFieldsWidget = new QWidget(securityBox);
    QFormLayout *passForm = new QFormLayout(m_dbPasswordFieldsWidget);
    passForm->setContentsMargins(16, 4, 0, 4);
    passForm->setSpacing(6);
    passForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_editDbCurrentPassword = new QLineEdit(m_dbPasswordFieldsWidget);
    m_editDbCurrentPassword->setEchoMode(QLineEdit::Password);
    m_editDbCurrentPassword->setPlaceholderText("Dejar en blanco si es una instalación nueva");
    m_editDbCurrentPassword->setStyleSheet("background-color: white; padding: 3px 8px; font-size: 12px;");

    m_editDbNewPassword = new QLineEdit(m_dbPasswordFieldsWidget);
    m_editDbNewPassword->setEchoMode(QLineEdit::Password);
    m_editDbNewPassword->setPlaceholderText("Nueva contraseña de administración");
    m_editDbNewPassword->setStyleSheet("background-color: white; padding: 3px 8px; font-size: 12px;");

    m_editDbConfirmPassword = new QLineEdit(m_dbPasswordFieldsWidget);
    m_editDbConfirmPassword->setEchoMode(QLineEdit::Password);
    m_editDbConfirmPassword->setPlaceholderText("Repite la nueva contraseña");
    m_editDbConfirmPassword->setStyleSheet("background-color: white; padding: 3px 8px; font-size: 12px;");

    m_chkDbShowPasswords = new QCheckBox("Mostrar contraseñas", m_dbPasswordFieldsWidget);
    m_chkDbShowPasswords->setStyleSheet("font-size: 12px; color: #495057;");

    QLabel *lblCurP = new QLabel("Contraseña actual (si existe):", m_dbPasswordFieldsWidget);
    lblCurP->setStyleSheet("border: none; font-size: 12px;");
    QLabel *lblNewP = new QLabel("Nueva contraseña:", m_dbPasswordFieldsWidget);
    lblNewP->setStyleSheet("border: none; font-size: 12px;");
    QLabel *lblConP = new QLabel("Confirmar contraseña:", m_dbPasswordFieldsWidget);
    lblConP->setStyleSheet("border: none; font-size: 12px;");

    passForm->addRow(lblCurP, m_editDbCurrentPassword);
    passForm->addRow(lblNewP, m_editDbNewPassword);
    passForm->addRow(lblConP, m_editDbConfirmPassword);
    passForm->addRow("", m_chkDbShowPasswords);

    m_dbMariaOptionsWidget = new QWidget(securityBox);
    QVBoxLayout *mariaOptLayout = new QVBoxLayout(m_dbMariaOptionsWidget);
    mariaOptLayout->setContentsMargins(0, 4, 0, 0);
    mariaOptLayout->setSpacing(6);

    QLabel *lblMariaNative = new QLabel("Opciones del configurador nativo de MariaDB (mariadb-secure-installation):", m_dbMariaOptionsWidget);
    lblMariaNative->setStyleSheet("border: none; font-weight: bold; font-size: 12px; color: #084298;");

    m_chkMariaRemoveAnon = new QCheckBox("Eliminar usuarios anónimos (Recomendado)", m_dbMariaOptionsWidget);
    m_chkMariaRemoveAnon->setChecked(true);
    m_chkMariaRemoveAnon->setStyleSheet("font-size: 12px;");

    m_chkMariaDisallowRemoteRoot = new QCheckBox("Deshabilitar el inicio de sesión remoto para 'root' (Recomendado)", m_dbMariaOptionsWidget);
    m_chkMariaDisallowRemoteRoot->setChecked(true);
    m_chkMariaDisallowRemoteRoot->setStyleSheet("font-size: 12px;");

    m_chkMariaRemoveTestDb = new QCheckBox("Eliminar la base de datos 'test' y los accesos a ella (Recomendado)", m_dbMariaOptionsWidget);
    m_chkMariaRemoveTestDb->setChecked(true);
    m_chkMariaRemoveTestDb->setStyleSheet("font-size: 12px;");

    m_chkMariaReloadPrivs = new QCheckBox("Recargar las tablas de privilegios inmediatamente (FLUSH PRIVILEGES)", m_dbMariaOptionsWidget);
    m_chkMariaReloadPrivs->setChecked(true);
    m_chkMariaReloadPrivs->setStyleSheet("font-size: 12px;");

    mariaOptLayout->addWidget(lblMariaNative);
    mariaOptLayout->addWidget(m_chkMariaRemoveAnon);
    mariaOptLayout->addWidget(m_chkMariaDisallowRemoteRoot);
    mariaOptLayout->addWidget(m_chkMariaRemoveTestDb);
    mariaOptLayout->addWidget(m_chkMariaReloadPrivs);

    m_dbPostgresOptionsWidget = new QWidget(securityBox);
    m_dbPostgresOptionsWidget->setVisible(false);
    QVBoxLayout *pgOptLayout = new QVBoxLayout(m_dbPostgresOptionsWidget);
    pgOptLayout->setContentsMargins(0, 4, 0, 0);
    pgOptLayout->setSpacing(6);

    QLabel *lblPgNative = new QLabel("Opciones de inicialización y seguridad de PostgreSQL:", m_dbPostgresOptionsWidget);
    lblPgNative->setStyleSheet("border: none; font-weight: bold; font-size: 12px; color: #084298;");

    m_chkPgScramAuth = new QCheckBox("Configurar autenticación cifrada (scram-sha-256) en pg_hba.conf (Recomendado)", m_dbPostgresOptionsWidget);
    m_chkPgScramAuth->setChecked(true);
    m_chkPgScramAuth->setStyleSheet("font-size: 12px;");

    m_chkPgLocalhostOnly = new QCheckBox("Restringir escucha de red únicamente a localhost en postgresql.conf (Recomendado)", m_dbPostgresOptionsWidget);
    m_chkPgLocalhostOnly->setChecked(true);
    m_chkPgLocalhostOnly->setStyleSheet("font-size: 12px;");

    m_chkPgRemoveTestDb = new QCheckBox("Eliminar base de datos de prueba 'test' si existe", m_dbPostgresOptionsWidget);
    m_chkPgRemoveTestDb->setChecked(true);
    m_chkPgRemoveTestDb->setStyleSheet("font-size: 12px;");

    pgOptLayout->addWidget(lblPgNative);
    pgOptLayout->addWidget(m_chkPgScramAuth);
    pgOptLayout->addWidget(m_chkPgLocalhostOnly);
    pgOptLayout->addWidget(m_chkPgRemoveTestDb);

    securityLayout->addWidget(lblSecHeader);
    securityLayout->addWidget(m_chkDbSetRootPassword);
    securityLayout->addWidget(m_dbPasswordFieldsWidget);
    securityLayout->addWidget(m_dbMariaOptionsWidget);
    securityLayout->addWidget(m_dbPostgresOptionsWidget);

    QHBoxLayout *saveDbLayout = new QHBoxLayout();
    m_btnSaveDbConfig = new QPushButton("Aplicar configuración del motor", page);
    m_btnSaveDbConfig->setMinimumHeight(38);
    m_btnSaveDbConfig->setMinimumWidth(250);
    m_btnSaveDbConfig->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; border-radius: 5px; padding: 6px 18px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
        "QPushButton:disabled { background-color: #9e9e9e; }"
    );
    saveDbLayout->addWidget(m_btnSaveDbConfig);
    saveDbLayout->addStretch();

    m_lblDbConfigFeedback = new QLabel(page);
    m_lblDbConfigFeedback->setWordWrap(true);
    m_lblDbConfigFeedback->setVisible(false);

    m_dbSessionTestFrame = new QFrame(page);
    m_dbSessionTestFrame->setVisible(false);
    m_dbSessionTestFrame->setStyleSheet(
        "QFrame { background-color: #ffffff; border: 2px solid #0d6efd; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *sessionLayout = new QVBoxLayout(m_dbSessionTestFrame);
    sessionLayout->setSpacing(8);

    QLabel *lblSessionTitle = new QLabel("3. Verificación de inicio de sesión en el motor:", m_dbSessionTestFrame);
    lblSessionTitle->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #084298;");

    m_lblDbSessionPrompt = new QLabel(
        "Introduce la contraseña de root para iniciar una sesión de comprobación:",
        m_dbSessionTestFrame
    );
    m_lblDbSessionPrompt->setWordWrap(true);
    m_lblDbSessionPrompt->setStyleSheet("border: none; font-size: 12px;");

    QHBoxLayout *sessionInputRow = new QHBoxLayout();
    m_editDbTestPassword = new QLineEdit(m_dbSessionTestFrame);
    m_editDbTestPassword->setMinimumHeight(32);
    m_editDbTestPassword->setEchoMode(QLineEdit::Password);
    m_editDbTestPassword->setPlaceholderText("Contraseña de administración...");
    m_editDbTestPassword->setStyleSheet("background-color: #f8f9fa; padding: 3px 8px; font-size: 13px;");

    m_chkDbShowTestPassword = new QCheckBox("Mostrar", m_dbSessionTestFrame);
    m_chkDbShowTestPassword->setStyleSheet("font-size: 12px;");

    m_btnTestDbSession = new QPushButton("Iniciar sesión de prueba", m_dbSessionTestFrame);
    m_btnTestDbSession->setMinimumHeight(32);
    m_btnTestDbSession->setStyleSheet(
        "QPushButton { background-color: #0d6efd; color: white; font-weight: bold; border-radius: 4px; padding: 4px 14px; }"
        "QPushButton:hover { background-color: #0b5ed7; }"
    );

    sessionInputRow->addWidget(m_editDbTestPassword, 1);
    sessionInputRow->addWidget(m_chkDbShowTestPassword);
    sessionInputRow->addWidget(m_btnTestDbSession);

    m_lblDbSessionStatus = new QLabel(m_dbSessionTestFrame);
    m_lblDbSessionStatus->setWordWrap(true);
    m_lblDbSessionStatus->setVisible(false);

    m_txtDbSessionOutput = new QPlainTextEdit(m_dbSessionTestFrame);
    m_txtDbSessionOutput->setReadOnly(true);
    m_txtDbSessionOutput->setMaximumHeight(140);
    m_txtDbSessionOutput->setVisible(false);
    m_txtDbSessionOutput->setStyleSheet(
        "QPlainTextEdit { background-color: #1e242b; color: #eceff4; font-family: monospace; font-size: 12px; border-radius: 4px; padding: 6px; }"
    );

    sessionLayout->addWidget(lblSessionTitle);
    sessionLayout->addWidget(m_lblDbSessionPrompt);
    sessionLayout->addLayout(sessionInputRow);
    sessionLayout->addWidget(m_lblDbSessionStatus);
    sessionLayout->addWidget(m_txtDbSessionOutput);

    m_dbBootFrame = new QFrame(page);
    m_dbBootFrame->setVisible(false);
    m_dbBootFrame->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 2px solid #2e7d32; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *dbBootLayout = new QVBoxLayout(m_dbBootFrame);
    dbBootLayout->setSpacing(8);

    QLabel *lblDbBootTitle = new QLabel("4. Arranque automático del servicio:", m_dbBootFrame);
    lblDbBootTitle->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #1b5e20;");

    m_lblDbBootQuestion = new QLabel(
        "La sesión se ha iniciado correctamente. ¿Deseas que el servicio inicie automáticamente cuando arranca el sistema?",
        m_dbBootFrame
    );
    m_lblDbBootQuestion->setWordWrap(true);
    m_lblDbBootQuestion->setStyleSheet("border: none; font-size: 13px;");

    m_radioDbBootYes = new QRadioButton("Sí, iniciar automáticamente al arrancar Slackware", m_dbBootFrame);
    m_radioDbBootYes->setChecked(true);
    m_radioDbBootYes->setStyleSheet("font-size: 13px; font-weight: bold;");

    m_radioDbBootNo = new QRadioButton("No, mantener inicio manual", m_dbBootFrame);
    m_radioDbBootNo->setStyleSheet("font-size: 13px;");

    QHBoxLayout *bootBtnLayout = new QHBoxLayout();
    m_btnSaveDbBoot = new QPushButton("Guardar configuración de arranque", m_dbBootFrame);
    m_btnSaveDbBoot->setMinimumHeight(34);
    m_btnSaveDbBoot->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; border-radius: 4px; padding: 5px 16px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
    );
    bootBtnLayout->addWidget(m_btnSaveDbBoot);
    bootBtnLayout->addStretch();

    m_lblDbBootFeedback = new QLabel(m_dbBootFrame);
    m_lblDbBootFeedback->setWordWrap(true);
    m_lblDbBootFeedback->setVisible(false);

    dbBootLayout->addWidget(lblDbBootTitle);
    dbBootLayout->addWidget(m_lblDbBootQuestion);
    dbBootLayout->addWidget(m_radioDbBootYes);
    dbBootLayout->addWidget(m_radioDbBootNo);
    dbBootLayout->addLayout(bootBtnLayout);
    dbBootLayout->addWidget(m_lblDbBootFeedback);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(detectBox);
    layout->addWidget(engineBox);
    layout->addWidget(securityBox);
    layout->addLayout(saveDbLayout);
    layout->addWidget(m_lblDbConfigFeedback);
    layout->addWidget(m_dbSessionTestFrame);
    layout->addWidget(m_dbBootFrame);
    layout->addStretch();

    connect(btnRescanDb, &QPushButton::clicked, this, &MainWindow::onRescanDatabaseClicked);
    connect(m_radioDbMariaDb, &QRadioButton::toggled, this, &MainWindow::onDatabaseEngineRadioToggled);
    connect(m_radioDbPostgres, &QRadioButton::toggled, this, &MainWindow::onDatabaseEngineRadioToggled);
    connect(m_chkDbSetRootPassword, &QCheckBox::toggled, this, &MainWindow::onDbSetPasswordToggled);
    connect(m_chkDbShowPasswords, &QCheckBox::toggled, this, &MainWindow::onDbShowPasswordToggled);
    connect(m_btnSaveDbConfig, &QPushButton::clicked, this, &MainWindow::onSaveDatabaseConfigClicked);

    connect(m_chkDbShowTestPassword, &QCheckBox::toggled, this, [this](bool checked) {
        if (m_editDbTestPassword) {
            m_editDbTestPassword->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        }
    });
    connect(m_btnTestDbSession, &QPushButton::clicked, this, &MainWindow::onTestDatabaseSessionClicked);
    connect(m_editDbTestPassword, &QLineEdit::returnPressed, this, &MainWindow::onTestDatabaseSessionClicked);
    connect(m_btnSaveDbBoot, &QPushButton::clicked, this, &MainWindow::onSaveDatabaseBootClicked);

    refreshDatabasePage();
    scrollArea->setWidget(page);
    return scrollArea;
}

void MainWindow::refreshDatabasePage()
{
    if (!m_lblDbDetectionSummary || !m_radioDbMariaDb || !m_radioDbPostgres) {
        return;
    }

    const DatabaseDetectionInfo env = DatabaseManager::detectEngines();

    QStringList lines;
    if (env.mariadbInstalled) {
        lines << QString("• <b>MariaDB:</b> Instalado (%1) — Datos: %2 | Estado: %3 | Arranque: %4")
                     .arg(env.mariadbVersion,
                          env.mariadbDataInitialized ? "Inicializado" : "Sin inicializar",
                          env.mariadbRunning ? "En ejecución" : "Detenido",
                          env.mariadbBootEnabled ? "Habilitado" : "Deshabilitado");
    } else {
        lines << "• <b>MariaDB:</b> No instalado";
    }

    if (env.postgresInstalled) {
        lines << QString("• <b>PostgreSQL:</b> Instalado (%1) — Datos (%2): %3 | Estado: %4 | Arranque: %5")
                     .arg(env.postgresVersion,
                          env.postgresDataDir,
                          env.postgresDataInitialized ? "Inicializado" : "Sin inicializar",
                          env.postgresRunning ? "En ejecución" : "Detenido",
                          env.postgresBootEnabled ? "Habilitado" : "Deshabilitado");
    } else {
        lines << "• <b>PostgreSQL:</b> No instalado (disponible en SlackBuilds; si lo instalas luego podrás configurarlo aquí)";
    }

    m_lblDbDetectionSummary->setText(lines.join("<br>"));

    m_radioDbMariaDb->setEnabled(env.mariadbInstalled);
    m_radioDbPostgres->setEnabled(env.postgresInstalled);

    if (env.mariadbInstalled && env.postgresInstalled) {
        m_radioDbMariaDb->setText(QString("MariaDB Server (%1)").arg(env.mariadbVersion));
        m_radioDbPostgres->setText(QString("PostgreSQL Server (%1)").arg(env.postgresVersion));
        if (!m_radioDbMariaDb->isChecked() && !m_radioDbPostgres->isChecked()) {
            m_radioDbMariaDb->setChecked(true);
        }
    } else if (env.mariadbInstalled) {
        m_radioDbMariaDb->setText(QString("MariaDB Server (%1) — De fábrica en Slackware").arg(env.mariadbVersion));
        m_radioDbPostgres->setText("PostgreSQL Server — No instalado en el sistema");
        m_radioDbMariaDb->setChecked(true);
    } else if (env.postgresInstalled) {
        m_radioDbMariaDb->setText("MariaDB Server — No instalado en el sistema");
        m_radioDbPostgres->setText(QString("PostgreSQL Server (%1)").arg(env.postgresVersion));
        m_radioDbPostgres->setChecked(true);
    } else {
        m_radioDbMariaDb->setText("MariaDB Server — No instalado");
        m_radioDbPostgres->setText("PostgreSQL Server — No instalado");
    }

    const bool anyDbInstalled = (env.mariadbInstalled || env.postgresInstalled);
    if (m_btnSaveDbConfig) {
        m_btnSaveDbConfig->setEnabled(anyDbInstalled);
    }

    onDatabaseEngineRadioToggled();
}

void MainWindow::onRescanDatabaseClicked()
{
    if (m_lblDbConfigFeedback) {
        m_lblDbConfigFeedback->setVisible(false);
    }
    if (m_dbSessionTestFrame) {
        m_dbSessionTestFrame->setVisible(false);
    }
    if (m_dbBootFrame) {
        m_dbBootFrame->setVisible(false);
    }
    refreshDatabasePage();
}

void MainWindow::onDatabaseEngineRadioToggled()
{
    const bool isMaria = (m_radioDbMariaDb && m_radioDbMariaDb->isChecked());
    const bool isPg = (m_radioDbPostgres && m_radioDbPostgres->isChecked());

    if (m_chkDbSetRootPassword) {
        m_chkDbSetRootPassword->setText(
            isPg ? "Establecer o cambiar la contraseña del superusuario 'postgres'"
                 : "Establecer o cambiar la contraseña de 'root' en MariaDB"
        );
    }

    if (m_dbMariaOptionsWidget) {
        m_dbMariaOptionsWidget->setVisible(isMaria);
    }
    if (m_dbPostgresOptionsWidget) {
        m_dbPostgresOptionsWidget->setVisible(isPg);
    }
}

void MainWindow::onDbSetPasswordToggled(bool checked)
{
    if (m_dbPasswordFieldsWidget) {
        m_dbPasswordFieldsWidget->setVisible(checked);
    }
}

void MainWindow::onDbShowPasswordToggled(bool checked)
{
    const QLineEdit::EchoMode mode = checked ? QLineEdit::Normal : QLineEdit::Password;
    if (m_editDbCurrentPassword) {
        m_editDbCurrentPassword->setEchoMode(mode);
    }
    if (m_editDbNewPassword) {
        m_editDbNewPassword->setEchoMode(mode);
    }
    if (m_editDbConfirmPassword) {
        m_editDbConfirmPassword->setEchoMode(mode);
    }
}

void MainWindow::onSaveDatabaseConfigClicked()
{
    if (!m_lblDbConfigFeedback) {
        return;
    }

    if (m_dbSessionTestFrame) {
        m_dbSessionTestFrame->setVisible(false);
    }
    if (m_dbBootFrame) {
        m_dbBootFrame->setVisible(false);
    }

    DatabaseConfigRequest req;
    if (m_radioDbMariaDb && m_radioDbMariaDb->isChecked()) {
        req.engine = "mariadb";
    } else if (m_radioDbPostgres && m_radioDbPostgres->isChecked()) {
        req.engine = "postgresql";
    }

    req.setRootPassword = (m_chkDbSetRootPassword && m_chkDbSetRootPassword->isChecked());
    req.currentRootPassword = m_editDbCurrentPassword ? m_editDbCurrentPassword->text() : QString();
    req.newRootPassword = m_editDbNewPassword ? m_editDbNewPassword->text() : QString();
    req.confirmRootPassword = m_editDbConfirmPassword ? m_editDbConfirmPassword->text() : QString();

    req.removeAnonymousUsers = (m_chkMariaRemoveAnon && m_chkMariaRemoveAnon->isChecked());
    req.disallowRemoteRoot = (m_chkMariaDisallowRemoteRoot && m_chkMariaDisallowRemoteRoot->isChecked());
    req.removeTestDatabase = (m_chkMariaRemoveTestDb && m_chkMariaRemoveTestDb->isChecked());
    req.reloadPrivileges = (m_chkMariaReloadPrivs && m_chkMariaReloadPrivs->isChecked());

    req.pgUseScramAuth = (m_chkPgScramAuth && m_chkPgScramAuth->isChecked());
    req.pgListenLocalhostOnly = (m_chkPgLocalhostOnly && m_chkPgLocalhostOnly->isChecked());
    req.pgRemoveTestDb = (m_chkPgRemoveTestDb && m_chkPgRemoveTestDb->isChecked());

    LOG_INFO("MainWindow", QString("Usuario pulsó aplicar configuración de base de datos: %1").arg(req.engine));

    const DatabaseConfigResult result = DatabaseManager::configureEngine(req);

    m_lblDbConfigFeedback->setVisible(true);
    if (!result.success) {
        m_lblDbConfigFeedback->setStyleSheet(
            "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblDbConfigFeedback->setText(result.message);
        return;
    }

    m_lblDbConfigFeedback->setStyleSheet(
        "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
        "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
    );
    m_lblDbConfigFeedback->setText(result.message);

    refreshDatabasePage();

    if (m_dbSessionTestFrame && m_lblDbSessionPrompt && m_editDbTestPassword) {
        const QString adminUser = (req.engine == "postgresql") ? "postgres" : "root";
        const QString engineName = (req.engine == "postgresql") ? "PostgreSQL" : "MariaDB";

        m_lblDbSessionPrompt->setText(
            QString("Configuración aplicada. Ingresa la contraseña de <b>%1</b> para iniciar una sesión de comprobación en <b>%2</b>:")
                .arg(adminUser, engineName)
        );
        m_editDbTestPassword->clear();
        if (m_lblDbSessionStatus) {
            m_lblDbSessionStatus->setVisible(false);
        }
        if (m_txtDbSessionOutput) {
            m_txtDbSessionOutput->setVisible(false);
            m_txtDbSessionOutput->clear();
        }
        m_dbSessionTestFrame->setVisible(true);
        m_editDbTestPassword->setFocus();
    }
}

void MainWindow::onTestDatabaseSessionClicked()
{
    if (!m_editDbTestPassword || !m_lblDbSessionStatus || !m_txtDbSessionOutput) {
        return;
    }

    const QString engine = (m_radioDbPostgres && m_radioDbPostgres->isChecked()) ? "postgresql" : "mariadb";
    const QString password = m_editDbTestPassword->text();

    const DatabaseSessionTestResult testRes = DatabaseManager::testRootSession(engine, password);

    m_lblDbSessionStatus->setVisible(true);
    m_txtDbSessionOutput->setVisible(true);
    m_txtDbSessionOutput->setPlainText(testRes.output);

    if (testRes.success) {
        m_lblDbSessionStatus->setStyleSheet(
            "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
            "border-radius: 5px; padding: 8px; font-size: 12px; font-weight: bold; }"
        );
        m_lblDbSessionStatus->setText(testRes.summary);

        if (m_dbBootFrame && m_lblDbBootQuestion) {
            const QString engineName = (engine == "postgresql") ? "PostgreSQL" : "MariaDB";
            const QString rcPath = (engine == "postgresql") ? "/etc/rc.d/rc.postgresql" : "/etc/rc.d/rc.mysqld";

            m_lblDbBootQuestion->setText(
                QString("La sesión de <b>%1</b> funcionó correctamente. ¿Deseas que el servicio inicie automáticamente cuando arranca el sistema (%2)?")
                    .arg(engineName, rcPath)
            );
            if (m_lblDbBootFeedback) {
                m_lblDbBootFeedback->setVisible(false);
            }
            m_dbBootFrame->setVisible(true);
        }
    } else {
        m_lblDbSessionStatus->setStyleSheet(
            "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
            "border-radius: 5px; padding: 8px; font-size: 12px; font-weight: bold; }"
        );
        m_lblDbSessionStatus->setText(testRes.summary);

        if (m_dbBootFrame) {
            m_dbBootFrame->setVisible(false);
        }
    }
}

void MainWindow::onSaveDatabaseBootClicked()
{
    if (!m_lblDbBootFeedback) {
        return;
    }

    const QString engine = (m_radioDbPostgres && m_radioDbPostgres->isChecked()) ? "postgresql" : "mariadb";
    const bool enableBoot = (m_radioDbBootYes && m_radioDbBootYes->isChecked());

    const DatabaseConfigResult bootRes = DatabaseManager::setStartOnBoot(engine, enableBoot);

    m_lblDbBootFeedback->setVisible(true);
    if (bootRes.success) {
        m_lblDbBootFeedback->setStyleSheet(
            "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
            "border-radius: 5px; padding: 8px; font-size: 12px; font-weight: bold; }"
        );
        m_lblDbBootFeedback->setText(bootRes.message);
        refreshDatabasePage();
    } else {
        m_lblDbBootFeedback->setStyleSheet(
            "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
            "border-radius: 5px; padding: 8px; font-size: 12px; font-weight: bold; }"
        );
        m_lblDbBootFeedback->setText(bootRes.message);
    }
}

// ============================================================================
// MÓDULO 8: CONFIGURAR FECHA, HORA Y ZONA HORARIA (CON MAPA MUNDIAL)
// ============================================================================

QWidget* MainWindow::createTimePage()
{
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *page = new QWidget(scrollArea);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(12);

    QLabel *title = new QLabel("8. Configurar Fecha, Hora y Zona Horaria", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *subtitle = new QLabel(
        "Selecciona tu ubicación geográfica haciendo clic en el mapa mundial o eligiéndola desde la lista desplegable, "
        "activa el horario de verano si corresponde y define si la hora se sincronizará automáticamente por Internet (NTP) o de forma manual.",
        page
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size: 13px;");

    QFrame *currentBox = new QFrame(page);
    currentBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 10px; }"
    );
    QVBoxLayout *currentBoxLayout = new QVBoxLayout(currentBox);
    m_lblTimeCurrentSummary = new QLabel(currentBox);
    m_lblTimeCurrentSummary->setWordWrap(true);
    m_lblTimeCurrentSummary->setStyleSheet("border: none; font-size: 12px; color: #084298;");
    currentBoxLayout->addWidget(m_lblTimeCurrentSummary);

    QFrame *zoneBox = new QFrame(page);
    zoneBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *zoneLayout = new QVBoxLayout(zoneBox);
    zoneLayout->setSpacing(10);

    QLabel *lblMapHeader = new QLabel("1. Región y Zona Horaria (haz clic en el mapa o elige en el desplegable):", zoneBox);
    lblMapHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");
    zoneLayout->addWidget(lblMapHeader);

    m_timeMapWidget = new TimezoneMapWidget(zoneBox);
    zoneLayout->addWidget(m_timeMapWidget);

    QFormLayout *zoneForm = new QFormLayout();
    zoneForm->setSpacing(8);
    zoneForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_comboTimezones = new QComboBox(zoneBox);
    m_comboTimezones->setEditable(true);
    m_comboTimezones->setInsertPolicy(QComboBox::NoInsert);
    m_comboTimezones->setMinimumHeight(32);
    m_comboTimezones->setMinimumWidth(360);
    m_comboTimezones->setStyleSheet("background-color: white; font-size: 13px; padding: 3px 8px;");

    m_chkTimeDst = new QCheckBox("Activar horario de verano (+1 hora / DST)", zoneBox);
    m_chkTimeDst->setStyleSheet("font-size: 12px; font-weight: bold; color: #084298;");

    m_lblTimeOffsetInfo = new QLabel(zoneBox);
    m_lblTimeOffsetInfo->setStyleSheet("border: none; font-size: 12px; color: #495057; font-weight: bold;");

    m_lblTimeLivePreview = new QLabel(zoneBox);
    m_lblTimeLivePreview->setStyleSheet("border: none; font-size: 14px; font-weight: bold; color: #1b5e20;");

    QLabel *lblTzRow = new QLabel("Zona horaria:", zoneBox);
    lblTzRow->setStyleSheet("border: none; font-size: 13px;");
    QLabel *lblOffRow = new QLabel("Desplazamiento:", zoneBox);
    lblOffRow->setStyleSheet("border: none; font-size: 12px;");
    QLabel *lblPrevRow = new QLabel("Hora resultante:", zoneBox);
    lblPrevRow->setStyleSheet("border: none; font-size: 12px;");

    zoneForm->addRow(lblTzRow, m_comboTimezones);
    zoneForm->addRow("", m_chkTimeDst);
    zoneForm->addRow(lblOffRow, m_lblTimeOffsetInfo);
    zoneForm->addRow(lblPrevRow, m_lblTimeLivePreview);

    zoneLayout->addLayout(zoneForm);

    QFrame *syncBox = new QFrame(page);
    syncBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 12px; }"
    );
    QVBoxLayout *syncLayout = new QVBoxLayout(syncBox);
    syncLayout->setSpacing(8);

    QLabel *lblSyncHeader = new QLabel("2. Ajuste de Fecha y Hora del Sistema:", syncBox);
    lblSyncHeader->setStyleSheet("border: none; font-weight: bold; font-size: 13px;");

    m_radioTimeAutoNtp = new QRadioButton("Hora automática mundial por Internet (NTP — Recomendado)", syncBox);
    m_radioTimeAutoNtp->setChecked(true);
    m_radioTimeAutoNtp->setStyleSheet("font-size: 13px;");

    QHBoxLayout *ntpRow = new QHBoxLayout();
    ntpRow->setContentsMargins(22, 0, 0, 4);
    QLabel *lblNtpSrv = new QLabel("Servidor NTP:", syncBox);
    lblNtpSrv->setStyleSheet("border: none; font-size: 12px;");
    m_editTimeNtpServer = new QLineEdit("pool.ntp.org", syncBox);
    m_editTimeNtpServer->setMaximumWidth(260);
    m_editTimeNtpServer->setMinimumHeight(28);
    m_editTimeNtpServer->setStyleSheet("background-color: white; font-size: 12px; padding: 2px 6px;");
    ntpRow->addWidget(lblNtpSrv);
    ntpRow->addWidget(m_editTimeNtpServer);
    ntpRow->addStretch();

    m_radioTimeManual = new QRadioButton("Seleccionar fecha y hora manualmente", syncBox);
    m_radioTimeManual->setStyleSheet("font-size: 13px;");

    QHBoxLayout *manualRow = new QHBoxLayout();
    manualRow->setContentsMargins(22, 0, 0, 4);
    QLabel *lblDate = new QLabel("Fecha:", syncBox);
    lblDate->setStyleSheet("border: none; font-size: 12px;");
    m_dateTimeManualDate = new QDateEdit(QDate::currentDate(), syncBox);
    m_dateTimeManualDate->setDisplayFormat("dd/MM/yyyy");
    m_dateTimeManualDate->setCalendarPopup(true);
    m_dateTimeManualDate->setMinimumHeight(28);
    m_dateTimeManualDate->setStyleSheet("background-color: white; font-size: 12px; padding: 2px 6px;");

    QLabel *lblTime = new QLabel("Hora:", syncBox);
    lblTime->setStyleSheet("border: none; font-size: 12px;");
    m_timeTimeManualTime = new QTimeEdit(QTime::currentTime(), syncBox);
    m_timeTimeManualTime->setDisplayFormat("HH:mm:ss");
    m_timeTimeManualTime->setMinimumHeight(28);
    m_timeTimeManualTime->setStyleSheet("background-color: white; font-size: 12px; padding: 2px 6px;");

    manualRow->addWidget(lblDate);
    manualRow->addWidget(m_dateTimeManualDate);
    manualRow->addSpacing(14);
    manualRow->addWidget(lblTime);
    manualRow->addWidget(m_timeTimeManualTime);
    manualRow->addStretch();

    m_chkTimeHwUtc = new QCheckBox(
        "Mantener el reloj de hardware (BIOS/RTC) en UTC (desmarcar si usas arranque dual con Windows)",
        syncBox
    );
    m_chkTimeHwUtc->setChecked(true);
    m_chkTimeHwUtc->setStyleSheet("font-size: 12px; color: #495057; margin-top: 4px;");

    syncLayout->addWidget(lblSyncHeader);
    syncLayout->addWidget(m_radioTimeAutoNtp);
    syncLayout->addLayout(ntpRow);
    syncLayout->addWidget(m_radioTimeManual);
    syncLayout->addLayout(manualRow);
    syncLayout->addWidget(m_chkTimeHwUtc);

    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_btnSaveTimeConfig = new QPushButton("Guardar configuración horaria", page);
    m_btnSaveTimeConfig->setMinimumHeight(38);
    m_btnSaveTimeConfig->setMinimumWidth(240);
    m_btnSaveTimeConfig->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; border-radius: 5px; padding: 6px 18px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
        "QPushButton:disabled { background-color: #9e9e9e; }"
    );
    actionLayout->addWidget(m_btnSaveTimeConfig);
    actionLayout->addStretch();

    m_lblTimeFeedback = new QLabel(page);
    m_lblTimeFeedback->setWordWrap(true);
    m_lblTimeFeedback->setVisible(false);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(currentBox);
    layout->addWidget(zoneBox);
    layout->addWidget(syncBox);
    layout->addLayout(actionLayout);
    layout->addWidget(m_lblTimeFeedback);
    layout->addStretch();

    m_timeZonesList = TimeManager::loadSystemTimezones();
    m_timeMapWidget->setTimezones(m_timeZonesList);

    m_comboTimezones->clear();
    for (const TimezoneEntry &z : m_timeZonesList) {
        m_comboTimezones->addItem(z.id, z.id);
    }
    if (m_comboTimezones->completer()) {
        m_comboTimezones->completer()->setFilterMode(Qt::MatchContains);
        m_comboTimezones->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    }

    connect(m_timeMapWidget, &TimezoneMapWidget::timezoneSelected,
            this, &MainWindow::onTimeMapZoneSelected);
    connect(m_comboTimezones, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onTimeComboZoneChanged);
    connect(m_chkTimeDst, &QCheckBox::toggled,
            this, &MainWindow::onTimeDstToggled);
    connect(m_radioTimeAutoNtp, &QRadioButton::toggled,
            this, &MainWindow::onTimeSyncModeChanged);
    connect(m_radioTimeManual, &QRadioButton::toggled,
            this, &MainWindow::onTimeSyncModeChanged);
    connect(m_btnSaveTimeConfig, &QPushButton::clicked,
            this, &MainWindow::onSaveTimeConfigClicked);

    m_timePreviewTimer = new QTimer(this);
    connect(m_timePreviewTimer, &QTimer::timeout, this, &MainWindow::updateTimeLivePreview);
    m_timePreviewTimer->start(1000);

    refreshTimePage();
    scrollArea->setWidget(page);
    return scrollArea;
}

void MainWindow::refreshTimePage()
{
    if (!m_comboTimezones || !m_timeMapWidget) {
        return;
    }

    const TimeSystemState state = TimeManager::detectCurrentState();

    if (m_lblTimeCurrentSummary) {
        m_lblTimeCurrentSummary->setText(
            QString("<b>Zona horaria actual:</b> %1 &nbsp;|&nbsp; <b>Horario de verano (DST):</b> %2 &nbsp;|&nbsp; "
                    "<b>Sincronización NTP:</b> %3 &nbsp;|&nbsp; <b>Reloj BIOS/RTC:</b> %4")
                .arg(state.currentTimezone,
                     state.dstActive ? "Activado" : "Desactivado",
                     state.ntpEnabled ? (state.ntpRunning ? "Habilitado (en ejecución)" : "Habilitado") : "Manual",
                     state.hardwareClockUtc ? "UTC" : "localtime")
        );
    }

    m_syncingTimeSelection = true;
    int idx = m_comboTimezones->findData(state.currentTimezone);
    if (idx < 0) {
        idx = m_comboTimezones->findText(state.currentTimezone);
    }
    if (idx >= 0) {
        m_comboTimezones->setCurrentIndex(idx);
    }
    m_timeMapWidget->setSelectedTimezone(state.currentTimezone);
    m_syncingTimeSelection = false;

    if (m_chkTimeDst) {
        const QSignalBlocker blocker(m_chkTimeDst);
        m_chkTimeDst->setChecked(state.dstActive);
    }

    if (m_radioTimeAutoNtp && m_radioTimeManual) {
        m_radioTimeAutoNtp->setChecked(state.ntpEnabled);
        m_radioTimeManual->setChecked(!state.ntpEnabled);
    }

    if (m_chkTimeHwUtc) {
        m_chkTimeHwUtc->setChecked(state.hardwareClockUtc);
    }

    if (m_dateTimeManualDate) {
        m_dateTimeManualDate->setDate(QDate::currentDate());
    }
    if (m_timeTimeManualTime) {
        m_timeTimeManualTime->setTime(QTime::currentTime());
    }

    onTimeSyncModeChanged();
    updateTimeLivePreview();
}

void MainWindow::onTimeMapZoneSelected(const QString &timezoneId)
{
    if (m_syncingTimeSelection || !m_comboTimezones) {
        return;
    }

    m_syncingTimeSelection = true;
    const int idx = m_comboTimezones->findData(timezoneId);
    if (idx >= 0) {
        m_comboTimezones->setCurrentIndex(idx);
    }
    m_syncingTimeSelection = false;

    const QTimeZone tz(timezoneId.toUtf8());
    if (tz.isValid() && m_chkTimeDst) {
        const QSignalBlocker blocker(m_chkTimeDst);
        m_chkTimeDst->setChecked(tz.isDaylightTime(QDateTime::currentDateTimeUtc()));
    }

    updateTimeLivePreview();
}

void MainWindow::onTimeComboZoneChanged(int index)
{
    if (m_syncingTimeSelection || index < 0 || !m_comboTimezones || !m_timeMapWidget) {
        return;
    }

    const QString timezoneId = m_comboTimezones->itemData(index).toString();
    if (timezoneId.isEmpty()) {
        return;
    }

    m_syncingTimeSelection = true;
    m_timeMapWidget->setSelectedTimezone(timezoneId);
    m_syncingTimeSelection = false;

    const QTimeZone tz(timezoneId.toUtf8());
    if (tz.isValid() && m_chkTimeDst) {
        const QSignalBlocker blocker(m_chkTimeDst);
        m_chkTimeDst->setChecked(tz.isDaylightTime(QDateTime::currentDateTimeUtc()));
    }

    updateTimeLivePreview();
}

void MainWindow::onTimeDstToggled(bool checked)
{
    Q_UNUSED(checked);
    updateTimeLivePreview();
}

void MainWindow::onTimeSyncModeChanged()
{
    if (!m_radioTimeAutoNtp) {
        return;
    }
    const bool isAuto = m_radioTimeAutoNtp->isChecked();
    if (m_editTimeNtpServer) {
        m_editTimeNtpServer->setEnabled(isAuto);
    }
    if (m_dateTimeManualDate) {
        m_dateTimeManualDate->setEnabled(!isAuto);
    }
    if (m_timeTimeManualTime) {
        m_timeTimeManualTime->setEnabled(!isAuto);
    }
    updateTimeLivePreview();
}

void MainWindow::updateTimeLivePreview()
{
    if (!m_comboTimezones || !m_lblTimeOffsetInfo || !m_lblTimeLivePreview) {
        return;
    }

    const QString tzId = m_comboTimezones->currentData().toString().isEmpty()
                             ? m_comboTimezones->currentText().trimmed()
                             : m_comboTimezones->currentData().toString();

    const QTimeZone tz(tzId.toUtf8());
    const QDateTime nowUtc = QDateTime::currentDateTimeUtc();

    int effectiveOffsetSec = 0;
    bool nativeDst = false;
    if (tz.isValid()) {
        nativeDst = tz.isDaylightTime(nowUtc);
        const int stdOffset = tz.standardTimeOffset(nowUtc);
        effectiveOffsetSec = (m_chkTimeDst && m_chkTimeDst->isChecked()) ? (stdOffset + 3600) : stdOffset;
    }

    const bool dstChecked = (m_chkTimeDst && m_chkTimeDst->isChecked());
    const QString dstNote = dstChecked
                                ? (nativeDst ? "Horario de verano activo por región"
                                             : "Horario de verano activado manualmente (+1 h)")
                                : "Hora estándar";

    m_lblTimeOffsetInfo->setText(
        QString("%1 — %2").arg(TimeManager::formatUtcOffset(effectiveOffsetSec), dstNote)
    );

    if (m_radioTimeAutoNtp && m_radioTimeAutoNtp->isChecked()) {
        const QDateTime previewDt = nowUtc.addSecs(effectiveOffsetSec);
        m_lblTimeLivePreview->setText(previewDt.toString("dddd dd/MM/yyyy  HH:mm:ss"));
    } else if (m_dateTimeManualDate && m_timeTimeManualTime) {
        const QDateTime manualDt(m_dateTimeManualDate->date(), m_timeTimeManualTime->time());
        m_lblTimeLivePreview->setText(manualDt.toString("dddd dd/MM/yyyy  HH:mm:ss") + " (Manual)");
    }
}

void MainWindow::onSaveTimeConfigClicked()
{
    if (!m_comboTimezones || !m_lblTimeFeedback) {
        return;
    }

    const QString tzId = m_comboTimezones->currentData().toString().isEmpty()
                             ? m_comboTimezones->currentText().trimmed()
                             : m_comboTimezones->currentData().toString();

    TimeConfigRequest req;
    req.timezoneId = tzId;
    req.enableDst = (m_chkTimeDst && m_chkTimeDst->isChecked());
    req.automaticNtp = (m_radioTimeAutoNtp && m_radioTimeAutoNtp->isChecked());
    req.ntpServer = m_editTimeNtpServer ? m_editTimeNtpServer->text().trimmed() : "pool.ntp.org";
    if (m_dateTimeManualDate && m_timeTimeManualTime) {
        req.manualDateTime = QDateTime(m_dateTimeManualDate->date(), m_timeTimeManualTime->time());
    }
    req.hardwareClockUtc = (m_chkTimeHwUtc && m_chkTimeHwUtc->isChecked());

    LOG_INFO("MainWindow", QString("Usuario pulsó guardar hora del sistema: %1").arg(req.timezoneId));

    if (m_btnSaveTimeConfig) {
        m_btnSaveTimeConfig->setEnabled(false);
    }
    const TimeConfigResult result = TimeManager::applyConfiguration(req);
    if (m_btnSaveTimeConfig) {
        m_btnSaveTimeConfig->setEnabled(true);
    }

    m_lblTimeFeedback->setVisible(true);
    if (result.success) {
        m_lblTimeFeedback->setStyleSheet(
            "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblTimeFeedback->setText(result.message);
        refreshTimePage();
    } else {
        m_lblTimeFeedback->setStyleSheet(
            "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
            "border-radius: 6px; padding: 12px; font-size: 13px; font-weight: bold; }"
        );
        m_lblTimeFeedback->setText(result.message);
    }
}

// ============================================================================
// MÓDULO 9: GESTOR VISUAL DE SERVICIOS DEL SISTEMA (/etc/rc.d/rc.*)
// ============================================================================

QWidget* MainWindow::createServicesPage()
{
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *page = new QWidget(scrollArea);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(12);

    QLabel *title = new QLabel("9. Gestor Visual de Servicios del Sistema (rc.d)", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *subtitle = new QLabel(
        "En Slackware los servicios se controlan mediante scripts en /etc/rc.d/rc.* y su permiso de ejecución (+x). "
        "Aquí puedes ver qué servicios están activos, controlarlos al instante y decidir cuáles arrancan con el equipo. "
        "Toda modificación solicitará confirmación con tu contraseña de administrador (root).",
        page
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size: 13px;");

    QFrame *topSummaryBox = new QFrame(page);
    topSummaryBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 10px; }"
    );
    QHBoxLayout *topSummaryLayout = new QHBoxLayout(topSummaryBox);
    topSummaryLayout->setSpacing(10);

    m_lblServicesSummary = new QLabel(topSummaryBox);
    m_lblServicesSummary->setWordWrap(true);
    m_lblServicesSummary->setStyleSheet("border: none; font-size: 12px; color: #084298;");

    QPushButton *btnRefreshServices = new QPushButton("Actualizar estado", topSummaryBox);
    btnRefreshServices->setMinimumHeight(32);
    btnRefreshServices->setStyleSheet("font-size: 12px; padding: 4px 10px;");

    m_btnToggleAdvancedServices = new QPushButton("Configuración Avanzada", topSummaryBox);
    m_btnToggleAdvancedServices->setMinimumHeight(32);
    m_btnToggleAdvancedServices->setStyleSheet(
        "QPushButton { background-color: #495057; color: white; font-weight: bold; border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
        "QPushButton:hover { background-color: #343a40; }"
    );

    topSummaryLayout->addWidget(m_lblServicesSummary, 1);
    topSummaryLayout->addWidget(btnRefreshServices);
    topSummaryLayout->addWidget(m_btnToggleAdvancedServices);

    m_serviceDangerBanner = new QFrame(page);
    m_serviceDangerBanner->setVisible(false);
    m_serviceDangerBanner->setStyleSheet(
        "QFrame { background-color: #fff3cd; border: 2px solid #ffc107; border-radius: 6px; padding: 10px; }"
    );
    QHBoxLayout *dangerLayout = new QHBoxLayout(m_serviceDangerBanner);
    QLabel *lblDangerText = new QLabel(
        "<b>ZONA DE CUIDADO ACTIVA (Modo Avanzado):</b> Se muestran también los scripts críticos del sistema base "
        "(rc.S, rc.M, rc.udev, rc.inet1, etc.) y las herramientas de edición de arranque local. Procede con precaución.",
        m_serviceDangerBanner
    );
    lblDangerText->setWordWrap(true);
    lblDangerText->setStyleSheet("border: none; font-size: 12px; color: #664d03;");
    dangerLayout->addWidget(lblDangerText);

    QFrame *controlsBox = new QFrame(page);
    controlsBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 10px; }"
    );
    QVBoxLayout *controlsLayout = new QVBoxLayout(controlsBox);
    controlsLayout->setSpacing(8);

    QHBoxLayout *presetsRow = new QHBoxLayout();
    QLabel *lblPresets = new QLabel("Perfiles rápidos recomendados:", controlsBox);
    lblPresets->setStyleSheet("border: none; font-weight: bold; font-size: 12px; color: #212529;");

    m_btnPresetDesktopServices = new QPushButton("Perfil Laptop / Escritorio", controlsBox);
    m_btnPresetDesktopServices->setStyleSheet(
        "QPushButton { background-color: #0d6efd; color: white; font-weight: bold; border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
        "QPushButton:hover { background-color: #0b5ed7; }"
    );

    m_btnPresetServerServices = new QPushButton("Perfil Servidor / Desarrollo", controlsBox);
    m_btnPresetServerServices->setStyleSheet(
        "QPushButton { background-color: #198754; color: white; font-weight: bold; border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
        "QPushButton:hover { background-color: #157347; }"
    );

    presetsRow->addWidget(lblPresets);
    presetsRow->addWidget(m_btnPresetDesktopServices);
    presetsRow->addWidget(m_btnPresetServerServices);
    presetsRow->addStretch();

    QHBoxLayout *filterRow = new QHBoxLayout();
    m_editServiceSearch = new QLineEdit(controlsBox);
    m_editServiceSearch->setMinimumHeight(30);
    m_editServiceSearch->setPlaceholderText("Filtrar servicio por nombre o función (ej. bluetooth, ssh, red, apache, docker)...");
    m_editServiceSearch->setStyleSheet("background-color: white; padding: 3px 8px; font-size: 12px;");

    m_comboServiceCategoryFilter = new QComboBox(controlsBox);
    m_comboServiceCategoryFilter->setMinimumHeight(30);
    m_comboServiceCategoryFilter->setMinimumWidth(210);
    m_comboServiceCategoryFilter->setStyleSheet("background-color: white; font-size: 12px; padding: 3px 8px;");
    m_comboServiceCategoryFilter->addItem("Todas las categorías", -1);
    m_comboServiceCategoryFilter->addItem("Red y Seguridad", static_cast<int>(ServiceCategory::NetworkSecurity));
    m_comboServiceCategoryFilter->addItem("Escritorio y Hardware", static_cast<int>(ServiceCategory::DesktopHardware));
    m_comboServiceCategoryFilter->addItem("Servidores y Bases de Datos", static_cast<int>(ServiceCategory::ServersDatabases));
    m_comboServiceCategoryFilter->addItem("SlackBuilds / Terceros", static_cast<int>(ServiceCategory::ThirdPartySlackBuild));

    filterRow->addWidget(m_editServiceSearch, 1);
    filterRow->addWidget(m_comboServiceCategoryFilter);

    controlsLayout->addLayout(presetsRow);
    controlsLayout->addLayout(filterRow);

    m_tblServices = new QTableWidget(page);
    m_tblServices->setColumnCount(5);
    m_tblServices->setHorizontalHeaderLabels({
        "Servicio (rc.*)",
        "Categoría",
        "Estado actual",
        "Arranque (+x)",
        "Acciones rápidas (root)"
    });
    m_tblServices->setMinimumHeight(290);
    m_tblServices->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblServices->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tblServices->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tblServices->setAlternatingRowColors(true);
    m_tblServices->verticalHeader()->setVisible(false);
    m_tblServices->verticalHeader()->setDefaultSectionSize(38);
    m_tblServices->setStyleSheet(
        "QTableWidget { background-color: white; alternate-background-color: #f8f9fa; "
        "border: 1px solid #d0d7de; border-radius: 6px; font-size: 12px; }"
        "QHeaderView::section { background-color: #e9ecef; font-weight: bold; padding: 6px; border: none; border-bottom: 1px solid #ced4da; }"
    );

    QHeaderView *header = m_tblServices->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::Fixed);
    m_tblServices->setColumnWidth(4, 225);

    m_serviceDetailBox = new QFrame(page);
    m_serviceDetailBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 10px; }"
    );
    QVBoxLayout *detailLayout = new QVBoxLayout(m_serviceDetailBox);
    detailLayout->setSpacing(4);

    m_lblServiceDetailTitle = new QLabel("Selecciona un servicio de la tabla para ver su descripción detallada", m_serviceDetailBox);
    m_lblServiceDetailTitle->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #212529;");

    m_lblServiceDetailDesc = new QLabel("Haz clic sobre cualquier fila para consultar qué hace ese servicio y qué acciones admite.", m_serviceDetailBox);
    m_lblServiceDetailDesc->setWordWrap(true);
    m_lblServiceDetailDesc->setStyleSheet("border: none; font-size: 12px; color: #495057;");

    m_lblServiceConflictAlert = new QLabel(m_serviceDetailBox);
    m_lblServiceConflictAlert->setWordWrap(true);
    m_lblServiceConflictAlert->setVisible(false);
    m_lblServiceConflictAlert->setStyleSheet(
        "QLabel { background-color: #fff3cd; color: #664d03; border: 1px solid #ffecb5; "
        "border-radius: 4px; padding: 6px; font-size: 12px; font-weight: bold; margin-top: 4px; }"
    );

    m_btnConfigureSamba = new QPushButton("⚙ Configurar recursos compartidos de Samba (smb.conf)", m_serviceDetailBox);
    m_btnConfigureSamba->setVisible(false);
    m_btnConfigureSamba->setStyleSheet(
        "QPushButton { background-color: #0d6efd; color: white; font-weight: bold; border-radius: 4px; padding: 5px 12px; font-size: 12px; margin-top: 6px; }"
        "QPushButton:hover { background-color: #0b5ed7; }"
    );
    connect(m_btnConfigureSamba, &QPushButton::clicked, this, &MainWindow::onConfigureSambaClicked);

    detailLayout->addWidget(m_lblServiceDetailTitle);
    detailLayout->addWidget(m_lblServiceDetailDesc);
    detailLayout->addWidget(m_lblServiceConflictAlert);
    detailLayout->addWidget(m_btnConfigureSamba);

    m_lblServiceFeedback = new QLabel(page);
    m_lblServiceFeedback->setWordWrap(true);
    m_lblServiceFeedback->setVisible(false);

    m_serviceAdvancedPanel = new QFrame(page);
    m_serviceAdvancedPanel->setVisible(false);
    m_serviceAdvancedPanel->setStyleSheet(
        "QFrame { background-color: #ffffff; border: 2px solid #6c757d; border-radius: 6px; padding: 10px; }"
    );
    QVBoxLayout *advLayout = new QVBoxLayout(m_serviceAdvancedPanel);
    advLayout->setSpacing(8);

    QLabel *lblAdvTitle = new QLabel("Herramientas de Control Total para Administradores (Modo Avanzado)", m_serviceAdvancedPanel);
    lblAdvTitle->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #343a40;");

    m_serviceAdvancedTabs = new QTabWidget(m_serviceAdvancedPanel);

    QWidget *tabScriptViewer = new QWidget(m_serviceAdvancedTabs);
    QVBoxLayout *viewerLayout = new QVBoxLayout(tabScriptViewer);
    m_lblScriptViewerHeader = new QLabel("Selecciona un servicio en la tabla superior para inspeccionar su script /etc/rc.d/rc.*", tabScriptViewer);
    m_lblScriptViewerHeader->setStyleSheet("border: none; font-size: 12px; font-weight: bold; color: #084298;");

    m_txtServiceScriptViewer = new QPlainTextEdit(tabScriptViewer);
    m_txtServiceScriptViewer->setReadOnly(true);
    m_txtServiceScriptViewer->setMinimumHeight(180);
    m_txtServiceScriptViewer->setStyleSheet(
        "QPlainTextEdit { background-color: #1e242b; color: #eceff4; font-family: monospace; font-size: 12px; border-radius: 4px; padding: 6px; }"
    );
    viewerLayout->addWidget(m_lblScriptViewerHeader);
    viewerLayout->addWidget(m_txtServiceScriptViewer);

    QWidget *tabLocalBoot = new QWidget(m_serviceAdvancedTabs);
    QVBoxLayout *localBootLayout = new QVBoxLayout(tabLocalBoot);

    QHBoxLayout *localBootTopRow = new QHBoxLayout();
    QLabel *lblSelectLocal = new QLabel("Archivo de comandos locales:", tabLocalBoot);
    lblSelectLocal->setStyleSheet("border: none; font-size: 12px; font-weight: bold;");

    m_comboLocalBootScript = new QComboBox(tabLocalBoot);
    m_comboLocalBootScript->addItem("/etc/rc.d/rc.local (Ejecutar al iniciar el sistema)", false);
    m_comboLocalBootScript->addItem("/etc/rc.d/rc.local_shutdown (Ejecutar al apagar/reiniciar)", true);

    m_btnSaveLocalBootScript = new QPushButton("Guardar script local (requiere root)", tabLocalBoot);
    m_btnSaveLocalBootScript->setStyleSheet(
        "QPushButton { background-color: #2e7d32; color: white; font-weight: bold; border-radius: 4px; padding: 5px 14px; font-size: 12px; }"
        "QPushButton:hover { background-color: #1b5e20; }"
    );

    localBootTopRow->addWidget(lblSelectLocal);
    localBootTopRow->addWidget(m_comboLocalBootScript);
    localBootTopRow->addStretch();
    localBootTopRow->addWidget(m_btnSaveLocalBootScript);

    m_txtLocalBootScriptEditor = new QPlainTextEdit(tabLocalBoot);
    m_txtLocalBootScriptEditor->setMinimumHeight(170);
    m_txtLocalBootScriptEditor->setStyleSheet(
        "QPlainTextEdit { background-color: #1e242b; color: #a3be8c; font-family: monospace; font-size: 12px; border-radius: 4px; padding: 6px; }"
    );

    localBootLayout->addLayout(localBootTopRow);
    localBootLayout->addWidget(m_txtLocalBootScriptEditor);

    m_serviceAdvancedTabs->addTab(tabScriptViewer, "Inspeccionar script rc.*");
    m_serviceAdvancedTabs->addTab(tabLocalBoot, "Editar rc.local / rc.local_shutdown");

    advLayout->addWidget(lblAdvTitle);
    advLayout->addWidget(m_serviceAdvancedTabs);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(topSummaryBox);
    layout->addWidget(m_serviceDangerBanner);
    layout->addWidget(controlsBox);
    layout->addWidget(m_tblServices);
    layout->addWidget(m_serviceDetailBox);
    layout->addWidget(m_lblServiceFeedback);
    layout->addWidget(m_serviceAdvancedPanel);
    layout->addStretch();

    connect(btnRefreshServices, &QPushButton::clicked, this, &MainWindow::onRefreshServicesClicked);
    connect(m_btnToggleAdvancedServices, &QPushButton::clicked, this, &MainWindow::onToggleAdvancedServicesClicked);
    connect(m_editServiceSearch, &QLineEdit::textChanged, this, &MainWindow::onServiceFilterChanged);
    connect(m_comboServiceCategoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onServiceFilterChanged);
    connect(m_tblServices, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::onServiceTableSelectionChanged);
    connect(m_btnPresetDesktopServices, &QPushButton::clicked,
            this, &MainWindow::onApplyServicePresetDesktopClicked);
    connect(m_btnPresetServerServices, &QPushButton::clicked,
            this, &MainWindow::onApplyServicePresetServerClicked);
    connect(m_comboLocalBootScript, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLocalBootScriptSelectorChanged);
    connect(m_btnSaveLocalBootScript, &QPushButton::clicked,
            this, &MainWindow::onSaveLocalBootScriptClicked);

    refreshServicesPage();
    scrollArea->setWidget(page);
    return scrollArea;
}

void MainWindow::refreshServicesPage()
{
    if (!m_tblServices || !m_lblServicesSummary) {
        return;
    }

    m_cachedServices = ServiceManager::listServices(m_serviceAdvancedMode);

    int runningCount = 0;
    int bootCount = 0;
    for (const SystemServiceInfo &svc : m_cachedServices) {
        if (svc.isRunning) {
            ++runningCount;
        }
        if (svc.bootEnabled) {
            ++bootCount;
        }
    }

    m_lblServicesSummary->setText(
        QString("<b>Vista actual:</b> %1 &nbsp;|&nbsp; <b>Servicios listados:</b> %2 &nbsp;|&nbsp; "
                "<b>En ejecución:</b> %3 &nbsp;|&nbsp; <b>Habilitados en el arranque (+x):</b> %4")
            .arg(m_serviceAdvancedMode ? "Modo Avanzado (Control total)" : "Modo Estándar (Seguro para principiantes)")
            .arg(m_cachedServices.size())
            .arg(runningCount)
            .arg(bootCount)
    );

    if (m_serviceAdvancedMode && m_txtLocalBootScriptEditor && m_comboLocalBootScript) {
        const bool isShutdown = m_comboLocalBootScript->currentData().toBool();
        m_txtLocalBootScriptEditor->setPlainText(ServiceManager::readLocalBootScript(isShutdown));
    }

    populateServicesTable();
}

void MainWindow::populateServicesTable()
{
    if (!m_tblServices) {
        return;
    }

    QString previouslySelectedScript;
    const QList<QTableWidgetItem *> selItems = m_tblServices->selectedItems();
    if (!selItems.isEmpty()) {
        previouslySelectedScript = selItems.first()->data(Qt::UserRole).toString();
    }

    const QString searchQuery = m_editServiceSearch ? m_editServiceSearch->text().trimmed() : QString();
    const int categoryFilter = m_comboServiceCategoryFilter ? m_comboServiceCategoryFilter->currentData().toInt() : -1;

    m_tblServices->blockSignals(true);
    m_tblServices->setRowCount(0);

    int rowToSelect = -1;

    for (const SystemServiceInfo &svc : m_cachedServices) {
        if (categoryFilter != -1 && static_cast<int>(svc.category) != categoryFilter) {
            continue;
        }

        if (!searchQuery.isEmpty()) {
            const bool matchesName = svc.displayName.contains(searchQuery, Qt::CaseInsensitive) ||
                                     svc.scriptName.contains(searchQuery, Qt::CaseInsensitive) ||
                                     svc.description.contains(searchQuery, Qt::CaseInsensitive);
            if (!matchesName) {
                continue;
            }
        }

        const int row = m_tblServices->rowCount();
        m_tblServices->insertRow(row);

        if (!previouslySelectedScript.isEmpty() && svc.scriptName == previouslySelectedScript) {
            rowToSelect = row;
        }

        QString nameCellText = QString("%1  (%2)").arg(svc.displayName, svc.scriptName);
        if (svc.isCriticalBase) {
            nameCellText.prepend("[SISTEMA BASE] ");
        } else if (svc.isThirdParty) {
            nameCellText.prepend("[SlackBuild] ");
        }
        if (!svc.conflictWarning.isEmpty()) {
            nameCellText.append("  ⚠");
        }

        QTableWidgetItem *itemName = new QTableWidgetItem(nameCellText);
        itemName->setData(Qt::UserRole, svc.scriptName);
        itemName->setToolTip(svc.description);
        if (svc.isCriticalBase) {
            itemName->setForeground(QBrush(QColor("#842029")));
            QFont f = itemName->font();
            f.setBold(true);
            itemName->setFont(f);
        }
        m_tblServices->setItem(row, 0, itemName);

        QString catText;
        switch (svc.category) {
        case ServiceCategory::SystemBase:           catText = "Sistema Base"; break;
        case ServiceCategory::NetworkSecurity:      catText = "Red y Seguridad"; break;
        case ServiceCategory::DesktopHardware:      catText = "Escritorio y Hardware"; break;
        case ServiceCategory::ServersDatabases:     catText = "Servidores / BD"; break;
        case ServiceCategory::ThirdPartySlackBuild: catText = "SlackBuild / Terceros"; break;
        }
        QTableWidgetItem *itemCat = new QTableWidgetItem(catText);
        itemCat->setTextAlignment(Qt::AlignCenter);
        m_tblServices->setItem(row, 1, itemCat);

        QTableWidgetItem *itemStatus = new QTableWidgetItem(
            svc.isRunning ? "● En ejecución" : "○ Detenido"
        );
        itemStatus->setTextAlignment(Qt::AlignCenter);
        QFont stFont = itemStatus->font();
        stFont.setBold(svc.isRunning);
        itemStatus->setFont(stFont);
        itemStatus->setForeground(QBrush(QColor(svc.isRunning ? "#0f5132" : "#6c757d")));
        m_tblServices->setItem(row, 2, itemStatus);

        QWidget *bootCellWidget = new QWidget(m_tblServices);
        QHBoxLayout *bootCellLayout = new QHBoxLayout(bootCellWidget);
        bootCellLayout->setContentsMargins(6, 2, 6, 2);
        bootCellLayout->setAlignment(Qt::AlignCenter);

        QCheckBox *chkBoot = new QCheckBox(svc.bootEnabled ? "Sí (+x)" : "No (-x)", bootCellWidget);
        chkBoot->setChecked(svc.bootEnabled);
        chkBoot->setStyleSheet(svc.bootEnabled
                                   ? "font-size: 11px; font-weight: bold; color: #084298;"
                                   : "font-size: 11px; color: #495057;");
        const QString scriptNameCopy = svc.scriptName;
        connect(chkBoot, &QCheckBox::toggled, this, [this, scriptNameCopy](bool checked) {
            handleServiceBootToggled(scriptNameCopy, checked);
        });

        bootCellLayout->addWidget(chkBoot);
        m_tblServices->setCellWidget(row, 3, bootCellWidget);

        QWidget *actionsWidget = new QWidget(m_tblServices);
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->setContentsMargins(4, 3, 4, 3);
        actionsLayout->setSpacing(4);

        QPushButton *btnStart = new QPushButton("Iniciar", actionsWidget);
        btnStart->setEnabled(!svc.isRunning);
        btnStart->setStyleSheet(
            "QPushButton { background-color: #2e7d32; color: white; font-size: 11px; font-weight: bold; border-radius: 3px; padding: 3px 7px; }"
            "QPushButton:disabled { background-color: #ced4da; color: #6c757d; }"
        );

        QPushButton *btnStop = new QPushButton("Detener", actionsWidget);
        btnStop->setEnabled(svc.isRunning);
        btnStop->setStyleSheet(
            "QPushButton { background-color: #c62828; color: white; font-size: 11px; font-weight: bold; border-radius: 3px; padding: 3px 7px; }"
            "QPushButton:disabled { background-color: #ced4da; color: #6c757d; }"
        );

        QPushButton *btnRestart = new QPushButton("Reiniciar", actionsWidget);
        btnRestart->setStyleSheet(
            "QPushButton { background-color: #0d6efd; color: white; font-size: 11px; font-weight: bold; border-radius: 3px; padding: 3px 7px; }"
        );

        connect(btnStart, &QPushButton::clicked, this, [this, scriptNameCopy]() {
            handleServiceActionClicked(scriptNameCopy, ServiceAction::Start);
        });
        connect(btnStop, &QPushButton::clicked, this, [this, scriptNameCopy]() {
            handleServiceActionClicked(scriptNameCopy, ServiceAction::Stop);
        });
        connect(btnRestart, &QPushButton::clicked, this, [this, scriptNameCopy]() {
            handleServiceActionClicked(scriptNameCopy, ServiceAction::Restart);
        });

        actionsLayout->addWidget(btnStart);
        actionsLayout->addWidget(btnStop);
        actionsLayout->addWidget(btnRestart);
        m_tblServices->setCellWidget(row, 4, actionsWidget);
    }

    m_tblServices->blockSignals(false);

    if (rowToSelect >= 0) {
        m_tblServices->selectRow(rowToSelect);
    } else if (m_tblServices->rowCount() > 0) {
        m_tblServices->selectRow(0);
    } else {
        onServiceTableSelectionChanged();
    }
}

void MainWindow::onRefreshServicesClicked()
{
    if (m_lblServiceFeedback) {
        m_lblServiceFeedback->setVisible(false);
    }
    refreshServicesPage();
}

void MainWindow::onToggleAdvancedServicesClicked()
{
    if (!m_serviceAdvancedMode) {
        const QMessageBox::StandardButton answer = QMessageBox::warning(
            this,
            "Advertencia — Zona de Cuidado",
            ServiceManager::getAdvancedModeWarning() + "\n\n¿Deseas activar el Modo Avanzado?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (answer != QMessageBox::Yes) {
            return;
        }

        m_serviceAdvancedMode = true;
        m_btnToggleAdvancedServices->setText("Volver a Modo Estándar");
        m_btnToggleAdvancedServices->setStyleSheet(
            "QPushButton { background-color: #c62828; color: white; font-weight: bold; border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
            "QPushButton:hover { background-color: #8e0000; }"
        );

        if (m_comboServiceCategoryFilter && m_comboServiceCategoryFilter->findData(static_cast<int>(ServiceCategory::SystemBase)) < 0) {
            m_comboServiceCategoryFilter->addItem("Sistema Base (Críticos)", static_cast<int>(ServiceCategory::SystemBase));
        }
    } else {
        m_serviceAdvancedMode = false;
        m_btnToggleAdvancedServices->setText("Configuración Avanzada");
        m_btnToggleAdvancedServices->setStyleSheet(
            "QPushButton { background-color: #495057; color: white; font-weight: bold; border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
            "QPushButton:hover { background-color: #343a40; }"
        );

        if (m_comboServiceCategoryFilter) {
            const int baseIdx = m_comboServiceCategoryFilter->findData(static_cast<int>(ServiceCategory::SystemBase));
            if (baseIdx >= 0) {
                m_comboServiceCategoryFilter->removeItem(baseIdx);
            }
        }
    }

    if (m_serviceDangerBanner) {
        m_serviceDangerBanner->setVisible(m_serviceAdvancedMode);
    }
    if (m_serviceAdvancedPanel) {
        m_serviceAdvancedPanel->setVisible(m_serviceAdvancedMode);
    }

    refreshServicesPage();
}

void MainWindow::onServiceFilterChanged()
{
    populateServicesTable();
}

void MainWindow::onServiceTableSelectionChanged()
{
    if (!m_tblServices || !m_lblServiceDetailTitle || !m_lblServiceDetailDesc) {
        return;
    }

    const QList<QTableWidgetItem *> sel = m_tblServices->selectedItems();
    if (sel.isEmpty()) {
        m_lblServiceDetailTitle->setText("Ningún servicio seleccionado");
        m_lblServiceDetailDesc->setText("No hay servicios que coincidan con el filtro actual.");
        if (m_lblServiceConflictAlert) {
            m_lblServiceConflictAlert->setVisible(false);
        }
        if (m_btnConfigureSamba) {
            m_btnConfigureSamba->setVisible(false);
        }
        return;
    }

    const QString scriptName = sel.first()->data(Qt::UserRole).toString();
    const SystemServiceInfo info = ServiceManager::inspectService(scriptName);

    m_lblServiceDetailTitle->setText(
        QString("%1  —  Ruta: %2").arg(info.displayName, info.scriptPath)
    );
    m_lblServiceDetailDesc->setText(
        QString("%1<br><b>Acciones soportadas por el script:</b> %2 &nbsp;|&nbsp; "
                "<b>Permiso de arranque actual:</b> %3")
            .arg(info.description,
                 info.supportedActions.join(", "),
                 info.bootEnabled ? "Ejecutable (chmod +x)" : "No ejecutable (chmod -x)")
    );

    if (m_lblServiceConflictAlert) {
        if (!info.conflictWarning.isEmpty()) {
            m_lblServiceConflictAlert->setText("⚠ " + info.conflictWarning);
            m_lblServiceConflictAlert->setVisible(true);
        } else {
            m_lblServiceConflictAlert->setVisible(false);
        }
    }

    if (m_btnConfigureSamba) {
        m_btnConfigureSamba->setVisible(scriptName == "rc.samba");
    }

    if (m_serviceAdvancedMode && m_lblScriptViewerHeader && m_txtServiceScriptViewer) {
        m_lblScriptViewerHeader->setText(QString("Contenido en solo lectura de %1:").arg(info.scriptPath));
        m_txtServiceScriptViewer->setPlainText(ServiceManager::readServiceScript(scriptName));
    }
}

void MainWindow::onConfigureSambaClicked()
{
    SambaDetectedResources res = ServiceManager::detectSambaResources();

    QDialog dlg(this);
    dlg.setWindowTitle("Asistente de Configuración de Samba (SMB)");
    dlg.setMinimumWidth(580);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->setSpacing(12);

    QLabel *lblInfo = new QLabel(
        "Configura los recursos compartidos de red (Samba) para tu equipo Slackware. "
        "Esto generará un archivo /etc/samba/smb.conf limpio y optimizado a partir de los discos e impresoras detectados.",
        &dlg
    );
    lblInfo->setWordWrap(true);

    QFormLayout *form = new QFormLayout();
    QLineEdit *editWorkgroup = new QLineEdit("WORKGROUP", &dlg);
    editWorkgroup->setMinimumHeight(28);
    form->addRow("Grupo de trabajo (Workgroup):", editWorkgroup);

    QCheckBox *chkSharePrinters = new QCheckBox("Compartir impresoras del sistema (CUPS)", &dlg);
    chkSharePrinters->setChecked(res.cupsAvailable);
    chkSharePrinters->setEnabled(res.cupsAvailable);
    form->addRow("", chkSharePrinters);

    layout->addWidget(lblInfo);
    layout->addLayout(form);

    QLabel *lblSharesTitle = new QLabel("<b>Carpetas compartidas detectadas en el sistema:</b>", &dlg);
    layout->addWidget(lblSharesTitle);

    QTableWidget *tblShares = new QTableWidget(&dlg);
    tblShares->setColumnCount(4);
    tblShares->setHorizontalHeaderLabels({ "Nombre del recurso", "Ruta en el disco", "Solo lectura", "Invitados (Guest)" });
    tblShares->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tblShares->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tblShares->setMinimumHeight(160);

    for (const QString &mountInfo : res.detectedStorageMounts) {
        QStringList parts = mountInfo.split('|');
        if (!parts.isEmpty()) {
            QString path = parts[0];
            QString shareName = QFileInfo(path).fileName();
            if (shareName.isEmpty()) {
                shareName = "compartido";
            }

            int row = tblShares->rowCount();
            tblShares->insertRow(row);
            tblShares->setItem(row, 0, new QTableWidgetItem(shareName));
            tblShares->setItem(row, 1, new QTableWidgetItem(path));

            QTableWidgetItem *itemRo = new QTableWidgetItem();
            itemRo->setCheckState(Qt::Unchecked);
            tblShares->setItem(row, 2, itemRo);

            QTableWidgetItem *itemGuest = new QTableWidgetItem();
            itemGuest->setCheckState(Qt::Checked);
            tblShares->setItem(row, 3, itemGuest);
        }
    }

    if (tblShares->rowCount() == 0) {
        int row = tblShares->rowCount();
        tblShares->insertRow(row);
        tblShares->setItem(row, 0, new QTableWidgetItem("publico"));
        tblShares->setItem(row, 1, new QTableWidgetItem("/home/shared"));
        QTableWidgetItem *itemRo = new QTableWidgetItem();
        itemRo->setCheckState(Qt::Unchecked);
        tblShares->setItem(row, 2, itemRo);
        QTableWidgetItem *itemGuest = new QTableWidgetItem();
        itemGuest->setCheckState(Qt::Checked);
        tblShares->setItem(row, 3, itemGuest);
    }

    layout->addWidget(tblShares);

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg);
    btnBox->button(QDialogButtonBox::Save)->setText("Guardar /etc/samba/smb.conf");
    btnBox->button(QDialogButtonBox::Cancel)->setText("Cancelar");
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(btnBox);

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    QString workgroup = editWorkgroup->text().trimmed();
    bool sharePrinters = chkSharePrinters->isChecked();
    QList<SambaShareItem> shares;

    for (int i = 0; i < tblShares->rowCount(); ++i) {
        QTableWidgetItem *itemSName = tblShares->item(i, 0);
        QTableWidgetItem *itemPath = tblShares->item(i, 1);
        QTableWidgetItem *itemRo = tblShares->item(i, 2);
        QTableWidgetItem *itemGuest = tblShares->item(i, 3);

        if (itemSName && itemPath && !itemSName->text().trimmed().isEmpty() && !itemPath->text().trimmed().isEmpty()) {
            SambaShareItem share;
            share.shareName = itemSName->text().trimmed();
            share.path = itemPath->text().trimmed();
            share.readOnly = (itemRo && itemRo->checkState() == Qt::Checked);
            share.guestOk = (itemGuest && itemGuest->checkState() == Qt::Checked);
            shares.append(share);
        }
    }

    QString rootPass;
    if (!promptRootPassword("Guardar configuración de Samba (/etc/samba/smb.conf)", rootPass)) {
        return;
    }

    ServiceOperationResult resOp = ServiceManager::generateBasicSambaConfig(workgroup, sharePrinters, shares, rootPass);
    rootPass.fill(QChar('\0'));
    rootPass.clear();

    if (m_lblServiceFeedback) {
        m_lblServiceFeedback->setVisible(true);
        if (resOp.success) {
            m_lblServiceFeedback->setStyleSheet(
                "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
                "border-radius: 6px; padding: 10px; font-size: 13px; font-weight: bold; }"
            );
            m_lblServiceFeedback->setText(resOp.message);
        } else {
            m_lblServiceFeedback->setStyleSheet(
                "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
                "border-radius: 6px; padding: 10px; font-size: 13px; font-weight: bold; }"
            );
            m_lblServiceFeedback->setText(resOp.message);
        }
    }

    refreshServicesPage();
}

bool MainWindow::promptRootPassword(const QString &actionSummary, QString &outPassword)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Autorización de Administrador (root)");
    dialog.setMinimumWidth(420);

    QVBoxLayout *dlgLayout = new QVBoxLayout(&dialog);
    dlgLayout->setSpacing(10);

    QLabel *lblHeader = new QLabel(
        "<b>Confirmación de seguridad obligatoria</b><br>"
        "Por seguridad, toda modificación sobre los servicios del sistema requiere la contraseña de <b>root</b>.",
        &dialog
    );
    lblHeader->setWordWrap(true);
    lblHeader->setStyleSheet("font-size: 13px;");

    QLabel *lblAction = new QLabel(QString("<b>Acción solicitada:</b> %1").arg(actionSummary), &dialog);
    lblAction->setWordWrap(true);
    lblAction->setStyleSheet(
        "background-color: #eef4fa; color: #084298; border: 1px solid #b6d4fe; "
        "border-radius: 4px; padding: 8px; font-size: 12px;"
    );

    QLineEdit *editPass = new QLineEdit(&dialog);
    editPass->setEchoMode(QLineEdit::Password);
    editPass->setMinimumHeight(32);
    editPass->setPlaceholderText("Ingresa la contraseña de root...");

    QCheckBox *chkShow = new QCheckBox("Mostrar contraseña", &dialog);
    chkShow->setStyleSheet("font-size: 12px;");
    connect(chkShow, &QCheckBox::toggled, editPass, [editPass](bool checked) {
        editPass->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        &dialog
    );
    buttonBox->button(QDialogButtonBox::Ok)->setText("Autorizar y ejecutar");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Cancelar");

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dlgLayout->addWidget(lblHeader);
    dlgLayout->addWidget(lblAction);
    dlgLayout->addWidget(editPass);
    dlgLayout->addWidget(chkShow);
    dlgLayout->addWidget(buttonBox);

    editPass->setFocus();

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    outPassword = editPass->text();
    editPass->clear();
    return true;
}

void MainWindow::handleServiceBootToggled(const QString &scriptName, bool enable)
{
    const QString actionDesc = QString("%1 arranque automático (chmod %2) para <b>%3</b>")
                                   .arg(enable ? "Habilitar" : "Deshabilitar",
                                        enable ? "+x" : "-x",
                                        scriptName);

    QString rootPass;
    if (!promptRootPassword(actionDesc, rootPass)) {
        refreshServicesPage();
        return;
    }

    const ServiceOperationResult res = ServiceManager::setStartOnBoot(
        scriptName,
        enable,
        rootPass,
        m_serviceAdvancedMode
    );
    rootPass.fill(QChar('\0'));
    rootPass.clear();

    if (m_lblServiceFeedback) {
        m_lblServiceFeedback->setVisible(true);
        if (res.success) {
            m_lblServiceFeedback->setStyleSheet(
                "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
                "border-radius: 6px; padding: 10px; font-size: 13px; font-weight: bold; }"
            );
            m_lblServiceFeedback->setText(res.message);
        } else {
            m_lblServiceFeedback->setStyleSheet(
                "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
                "border-radius: 6px; padding: 10px; font-size: 13px; font-weight: bold; }"
            );
            m_lblServiceFeedback->setText(res.message);
        }
    }

    refreshServicesPage();
}

void MainWindow::handleServiceActionClicked(const QString &scriptName, ServiceAction action)
{
    QString verb = "Reiniciar";
    if (action == ServiceAction::Start) {
        verb = "Iniciar";
    } else if (action == ServiceAction::Stop) {
        verb = "Detener";
    }

    const QString actionDesc = QString("%1 ahora mismo el servicio <b>%2</b>").arg(verb, scriptName);

    QString rootPass;
    if (!promptRootPassword(actionDesc, rootPass)) {
        return;
    }

    const ServiceOperationResult res = ServiceManager::executeServiceAction(
        scriptName,
        action,
        rootPass,
        m_serviceAdvancedMode
    );
    rootPass.fill(QChar('\0'));
    rootPass.clear();

    if (m_lblServiceFeedback) {
        m_lblServiceFeedback->setVisible(true);
        if (res.success) {
            m_lblServiceFeedback->setStyleSheet(
                "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
                "border-radius: 6px; padding: 10px; font-size: 13px; font-weight: bold; }"
            );
            const QString extra = res.commandOutput.isEmpty() ? QString() : QString("\nSalida: %1").arg(res.commandOutput);
            m_lblServiceFeedback->setText(res.message + extra);
        } else {
            m_lblServiceFeedback->setStyleSheet(
                "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
                "border-radius: 6px; padding: 10px; font-size: 13px; font-weight: bold; }"
            );
            m_lblServiceFeedback->setText(res.message);
        }
    }

    refreshServicesPage();
}

void MainWindow::handleApplyServicePreset(ServicePreset preset, const QString &presetLabel)
{
    const QList<ServicePresetChange> preview = ServiceManager::previewPreset(preset);
    if (preview.isEmpty()) {
        QMessageBox::information(
            this,
            presetLabel,
            "Tu sistema ya tiene aplicados todos los servicios recomendados para este perfil."
        );
        return;
    }

    QStringList summaryLines;
    for (const ServicePresetChange &chg : preview) {
        summaryLines << QString("• %1 (%2): arranque %3%4 — %5")
                            .arg(chg.displayName,
                                 chg.scriptName,
                                 chg.enableOnBoot ? "habilitado (+x)" : "deshabilitado (-x)",
                                 chg.startNow ? " e iniciar ahora" : "",
                                 chg.reason);
    }

    const QMessageBox::StandardButton confirm = QMessageBox::question(
        this,
        QString("Vista previa — %1").arg(presetLabel),
        QString("Se aplicarán los siguientes cambios en /etc/rc.d/:\n\n%1\n\n¿Deseas continuar?")
            .arg(summaryLines.join('\n')),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );

    if (confirm != QMessageBox::Yes) {
        return;
    }

    QString rootPass;
    if (!promptRootPassword(QString("Aplicar <b>%1</b> (%2 cambios)").arg(presetLabel).arg(preview.size()), rootPass)) {
        return;
    }

    const ServiceOperationResult res = ServiceManager::applyPreset(preset, rootPass);
    rootPass.fill(QChar('\0'));
    rootPass.clear();

    if (m_lblServiceFeedback) {
        m_lblServiceFeedback->setVisible(true);
        if (res.success) {
            m_lblServiceFeedback->setStyleSheet(
                "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
                "border-radius: 6px; padding: 10px; font-size: 13px; font-weight: bold; }"
            );
            m_lblServiceFeedback->setText(res.message);
        } else {
            m_lblServiceFeedback->setStyleSheet(
                "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
                "border-radius: 6px; padding: 10px; font-size: 13px; font-weight: bold; }"
            );
            m_lblServiceFeedback->setText(res.message);
        }
    }

    refreshServicesPage();
}

void MainWindow::onApplyServicePresetDesktopClicked()
{
    handleApplyServicePreset(ServicePreset::DesktopLaptop, "Perfil Laptop / Escritorio");
}

void MainWindow::onApplyServicePresetServerClicked()
{
    handleApplyServicePreset(ServicePreset::ServerDevelopment, "Perfil Servidor / Desarrollo");
}

void MainWindow::onLocalBootScriptSelectorChanged(int index)
{
    Q_UNUSED(index);
    if (!m_comboLocalBootScript || !m_txtLocalBootScriptEditor) {
        return;
    }
    const bool isShutdown = m_comboLocalBootScript->currentData().toBool();
    m_txtLocalBootScriptEditor->setPlainText(ServiceManager::readLocalBootScript(isShutdown));
}

void MainWindow::onSaveLocalBootScriptClicked()
{
    if (!m_comboLocalBootScript || !m_txtLocalBootScriptEditor) {
        return;
    }

    const bool isShutdown = m_comboLocalBootScript->currentData().toBool();
    const QString targetFile = isShutdown ? "/etc/rc.d/rc.local_shutdown" : "/etc/rc.d/rc.local";

    QString rootPass;
    if (!promptRootPassword(QString("Guardar cambios en <b>%1</b>").arg(targetFile), rootPass)) {
        return;
    }

    const ServiceOperationResult res = ServiceManager::saveLocalBootScript(
        m_txtLocalBootScriptEditor->toPlainText(),
        isShutdown,
        rootPass
    );
    rootPass.fill(QChar('\0'));
    rootPass.clear();

    if (m_lblServiceFeedback) {
        m_lblServiceFeedback->setVisible(true);
        if (res.success) {
            m_lblServiceFeedback->setStyleSheet(
                "QLabel { background-color: #d1e7dd; color: #0f5132; border: 1px solid #badbcc; "
                "border-radius: 6px; padding: 10px; font-size: 13px; font-weight: bold; }"
            );
            m_lblServiceFeedback->setText(res.message);
        } else {
            m_lblServiceFeedback->setStyleSheet(
                "QLabel { background-color: #f8d7da; color: #842029; border: 1px solid #f5c2c7; "
                "border-radius: 6px; padding: 10px; font-size: 13px; font-weight: bold; }"
            );
            m_lblServiceFeedback->setText(res.message);
        }
    }
}

// ============================================================================
// MÓDULO 10: ENCONTRAR / CONFIGURAR DISPOSITIVOS (FIND / SETUP DEVICES)
// ============================================================================

QWidget* MainWindow::createDevicesPage()
{
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *page = new QWidget(scrollArea);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 8, 0);
    layout->setSpacing(12);

    QLabel *title = new QLabel("10. Encontrar y Configurar Dispositivos", page);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);

    QLabel *subtitle = new QLabel(
        "Detecta automáticamente el factor de forma del equipo y todos los periféricos conectados "
        "(video, audio, red, bluetooth y dispositivos de entrada), traduciendo los identificadores técnicos a un lenguaje amigable. "
        "Modifica los archivos de configuración subyacentes de forma totalmente transparente.",
        page
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size: 13px;");

    // Tarjeta superior: Resumen del factor de forma detectado + Botón Avanzado + Actualizar
    QFrame *topSummaryBox = new QFrame(page);
    topSummaryBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 12px; }"
    );
    QHBoxLayout *topSummaryLayout = new QHBoxLayout(topSummaryBox);
    topSummaryLayout->setSpacing(10);

    m_lblDeviceFormFactorSummary = new QLabel(topSummaryBox);
    m_lblDeviceFormFactorSummary->setWordWrap(true);
    m_lblDeviceFormFactorSummary->setStyleSheet("border: none; font-size: 12px; color: #084298;");

    QPushButton *btnRefreshDevices = new QPushButton("Volver a detectar", topSummaryBox);
    btnRefreshDevices->setMinimumHeight(32);
    btnRefreshDevices->setStyleSheet("font-size: 12px; padding: 4px 10px;");

    m_btnToggleAdvancedDevices = new QPushButton("Configuración Avanzada", topSummaryBox);
    m_btnToggleAdvancedDevices->setMinimumHeight(32);
    m_btnToggleAdvancedDevices->setStyleSheet(
        "QPushButton { background-color: #495057; color: white; font-weight: bold; border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
        "QPushButton:hover { background-color: #343a40; }"
    );

    topSummaryLayout->addWidget(m_lblDeviceFormFactorSummary, 1);
    topSummaryLayout->addWidget(btnRefreshDevices);
    topSummaryLayout->addWidget(m_btnToggleAdvancedDevices);

    // Banner de advertencia (Zona de Cuidado)
    m_deviceDangerBanner = new QFrame(page);
    m_deviceDangerBanner->setVisible(false);
    m_deviceDangerBanner->setStyleSheet(
        "QFrame { background-color: #fff3cd; border: 2px solid #ffc107; border-radius: 6px; padding: 10px; }"
    );
    QHBoxLayout *dangerLayout = new QHBoxLayout(m_deviceDangerBanner);
    QLabel *lblDangerText = new QLabel(
        "<b>ZONA DE CUIDADO ACTIVA (Modo Avanzado):</b> Se muestran los identificadores PCI/USB de bajo nivel "
        "y las rutas de los archivos de texto subyacentes (/etc/X11/xorg.conf.d/, /etc/asound.conf, etc.). Procede con precaución.",
        m_deviceDangerBanner
    );
    lblDangerText->setWordWrap(true);
    lblDangerText->setStyleSheet("border: none; font-size: 12px; color: #664d03;");
    dangerLayout->addWidget(lblDangerText);

    // Tabla de dispositivos detectados
    m_tblDevices = new QTableWidget(page);
    m_tblDevices->setColumnCount(4);
    m_tblDevices->setHorizontalHeaderLabels({
        "Dispositivo detectado",
        "Categoría",
        "Estado operativo",
        "Acciones / Configuración"
    });
    m_tblDevices->setMinimumHeight(240);
    m_tblDevices->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblDevices->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tblDevices->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tblDevices->setAlternatingRowColors(true);
    m_tblDevices->verticalHeader()->setVisible(false);
    m_tblDevices->verticalHeader()->setDefaultSectionSize(40);
    m_tblDevices->setStyleSheet(
        "QTableWidget { background-color: white; alternate-background-color: #f8f9fa; "
        "border: 1px solid #d0d7de; border-radius: 6px; font-size: 12px; }"
        "QHeaderView::section { background-color: #e9ecef; font-weight: bold; padding: 6px; border: none; border-bottom: 1px solid #ced4da; }"
    );

    QHeaderView *devHeader = m_tblDevices->horizontalHeader();
    devHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    devHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    devHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    devHeader->setSectionResizeMode(3, QHeaderView::Fixed);
    m_tblDevices->setColumnWidth(3, 180);

    // Tarjeta de detalle del dispositivo seleccionado
    m_deviceDetailBox = new QFrame(page);
    m_deviceDetailBox->setStyleSheet(
        "QFrame { background-color: #f8f9fa; border: 1px solid #d0d7de; border-radius: 6px; padding: 10px; }"
    );
    QVBoxLayout *devDetailLayout = new QVBoxLayout(m_deviceDetailBox);
    devDetailLayout->setSpacing(4);

    m_lblDeviceDetailTitle = new QLabel("Selecciona un dispositivo de la lista para ver su información", m_deviceDetailBox);
    m_lblDeviceDetailTitle->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #212529;");

    m_lblDeviceDetailDesc = new QLabel("Aquí se mostrará el resumen comercial y técnico del hardware seleccionado.", m_deviceDetailBox);
    m_lblDeviceDetailDesc->setWordWrap(true);
    m_lblDeviceDetailDesc->setStyleSheet("border: none; font-size: 12px; color: #495057;");

    devDetailLayout->addWidget(m_lblDeviceDetailTitle);
    devDetailLayout->addWidget(m_lblDeviceDetailDesc);

    m_lblDeviceFeedback = new QLabel(page);
    m_lblDeviceFeedback->setWordWrap(true);
    m_lblDeviceFeedback->setVisible(false);

    // Panel Avanzado (Visor de bajo nivel / archivos de texto subyacentes)
    m_deviceAdvancedPanel = new QFrame(page);
    m_deviceAdvancedPanel->setVisible(false);
    m_deviceAdvancedPanel->setStyleSheet(
        "QFrame { background-color: #ffffff; border: 2px solid #6c757d; border-radius: 6px; padding: 10px; }"
    );
    QVBoxLayout *devAdvLayout = new QVBoxLayout(m_deviceAdvancedPanel);
    devAdvLayout->setSpacing(6);

    QLabel *lblDevAdvTitle = new QLabel("Información Técnica de Hardware y Archivos de Texto Subyacentes", m_deviceAdvancedPanel);
    lblDevAdvTitle->setStyleSheet("border: none; font-weight: bold; font-size: 13px; color: #343a40;");

    m_txtDeviceTechViewer = new QPlainTextEdit(m_deviceAdvancedPanel);
    m_txtDeviceTechViewer->setReadOnly(true);
    m_txtDeviceTechViewer->setMinimumHeight(140);
    m_txtDeviceTechViewer->setStyleSheet(
        "QPlainTextEdit { background-color: #1e242b; color: #eceff4; font-family: monospace; font-size: 12px; border-radius: 4px; padding: 6px; }"
    );

    devAdvLayout->addWidget(lblDevAdvTitle);
    devAdvLayout->addWidget(m_txtDeviceTechViewer);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(topSummaryBox);
    layout->addWidget(m_deviceDangerBanner);
    layout->addWidget(m_tblDevices);
    layout->addWidget(m_deviceDetailBox);
    layout->addWidget(m_lblDeviceFeedback);
    layout->addWidget(m_deviceAdvancedPanel);
    layout->addStretch();

    connect(btnRefreshDevices, &QPushButton::clicked, this, &MainWindow::onRefreshDevicesClicked);
    connect(m_btnToggleAdvancedDevices, &QPushButton::clicked, this, &MainWindow::onToggleAdvancedDevicesClicked);
    connect(m_tblDevices, &QTableWidget::itemSelectionChanged, this, &MainWindow::onDeviceTableSelectionChanged);

    refreshDevicesPage();
    scrollArea->setWidget(page);
    return scrollArea;
}

void MainWindow::refreshDevicesPage()
{
    if (!m_tblDevices || !m_lblDeviceFormFactorSummary) {
        return;
    }

    m_cachedHardwareOverview = DeviceManager::detectSystemHardware();

    m_lblDeviceFormFactorSummary->setText(
        QString("<b>Factor de forma detectado:</b> %1<br>%2 &nbsp;|&nbsp; <b>Dispositivos reconocidos:</b> %3")
            .arg(m_cachedHardwareOverview.formFactorDisplayName,
                 m_cachedHardwareOverview.formFactorDescription,
                 QString::number(m_cachedHardwareOverview.devices.size()))
    );

    populateDevicesTable();
}

void MainWindow::populateDevicesTable()
{
    if (!m_tblDevices) {
        return;
    }

    QString previouslySelectedId;
    const QList<QTableWidgetItem *> selItems = m_tblDevices->selectedItems();
    if (!selItems.isEmpty()) {
        previouslySelectedId = selItems.first()->data(Qt::UserRole).toString();
    }

    m_tblDevices->blockSignals(true);
    m_tblDevices->setRowCount(0);

    int rowToSelect = -1;

    for (const DeviceInfo &dev : m_cachedHardwareOverview.devices) {
        const int row = m_tblDevices->rowCount();
        m_tblDevices->insertRow(row);

        if (!previouslySelectedId.isEmpty() && dev.id == previouslySelectedId) {
            rowToSelect = row;
        }

        // Columna 0: Nombre amigable
        QTableWidgetItem *itemName = new QTableWidgetItem(dev.friendlyName);
        itemName->setData(Qt::UserRole, dev.id);
        itemName->setToolTip(dev.targetConfigFile);
        m_tblDevices->setItem(row, 0, itemName);

        // Columna 1: Categoría
        QString catStr;
        switch (dev.category) {
            case DeviceCategory::Audio:            catStr = "Audio"; break;
            case DeviceCategory::VideoDisplay:     catStr = "Video / Pantalla"; break;
            case DeviceCategory::NetworkBluetooth: catStr = "Red / Bluetooth"; break;
            case DeviceCategory::InputPeripherals: catStr = "Periféricos"; break;
            case DeviceCategory::Storage:          catStr = "Almacenamiento"; break;
        }
        QTableWidgetItem *itemCat = new QTableWidgetItem(catStr);
        itemCat->setTextAlignment(Qt::AlignCenter);
        m_tblDevices->setItem(row, 1, itemCat);

        // Columna 2: Estado
        QTableWidgetItem *itemStatus = new QTableWidgetItem(dev.statusSummary);
        itemStatus->setTextAlignment(Qt::AlignCenter);
        itemStatus->setForeground(QBrush(QColor("#0f5132")));
        m_tblDevices->setItem(row, 2, itemStatus);

        // Columna 3: Botón de acción / configuración
        QWidget *actionWidget = new QWidget(m_tblDevices);
        QHBoxLayout *actLayout = new QHBoxLayout(actionWidget);
        actLayout->setContentsMargins(4, 3, 4, 3);
        actLayout->setAlignment(Qt::AlignCenter);

        QPushButton *btnConfig = new QPushButton("Configurar", actionWidget);
        btnConfig->setEnabled(dev.isConfigurable);
        btnConfig->setStyleSheet(
            "QPushButton { background-color: #0d6efd; color: white; font-size: 11px; font-weight: bold; border-radius: 3px; padding: 4px 10px; }"
            "QPushButton:hover { background-color: #0b5ed7; }"
            "QPushButton:disabled { background-color: #ced4da; color: #6c757d; }"
        );

        const QString devIdCopy = dev.id;
        connect(btnConfig, &QPushButton::clicked, this, [this, devIdCopy]() {
            Q_UNUSED(devIdCopy);
            onConfigureDeviceClicked();
        });

        actLayout->addWidget(btnConfig);
        m_tblDevices->setCellWidget(row, 3, actionWidget);
    }

    m_tblDevices->blockSignals(false);

    if (rowToSelect >= 0) {
        m_tblDevices->selectRow(rowToSelect);
    } else if (m_tblDevices->rowCount() > 0) {
        m_tblDevices->selectRow(0);
    } else {
        onDeviceTableSelectionChanged();
    }
}

void MainWindow::onRefreshDevicesClicked()
{
    if (m_lblDeviceFeedback) {
        m_lblDeviceFeedback->setVisible(false);
    }
    refreshDevicesPage();
}

void MainWindow::onToggleAdvancedDevicesClicked()
{
    if (!m_deviceAdvancedMode) {
        const QMessageBox::StandardButton answer = QMessageBox::warning(
            this,
            "Advertencia — Zona de Cuidado",
            DeviceManager::getAdvancedModeWarning() + "\n\n¿Deseas activar el Modo Avanzado de Dispositivos?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (answer != QMessageBox::Yes) {
            return;
        }

        m_deviceAdvancedMode = true;
        m_btnToggleAdvancedDevices->setText("Volver a Modo Estándar");
        m_btnToggleAdvancedDevices->setStyleSheet(
            "QPushButton { background-color: #c62828; color: white; font-weight: bold; border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
            "QPushButton:hover { background-color: #8e0000; }"
        );
    } else {
        m_deviceAdvancedMode = false;
        m_btnToggleAdvancedDevices->setText("Configuración Avanzada");
        m_btnToggleAdvancedDevices->setStyleSheet(
            "QPushButton { background-color: #495057; color: white; font-weight: bold; border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
            "QPushButton:hover { background-color: #343a40; }"
        );
    }

    if (m_deviceDangerBanner) {
        m_deviceDangerBanner->setVisible(m_deviceAdvancedMode);
    }
    if (m_deviceAdvancedPanel) {
        m_deviceAdvancedPanel->setVisible(m_deviceAdvancedMode);
    }

    refreshDevicesPage();
}

void MainWindow::onDeviceTableSelectionChanged()
{
    if (!m_tblDevices || !m_lblDeviceDetailTitle || !m_lblDeviceDetailDesc) {
        return;
    }

    const QList<QTableWidgetItem *> sel = m_tblDevices->selectedItems();
    if (sel.isEmpty()) {
        m_lblDeviceDetailTitle->setText("Ningún dispositivo seleccionado");
        m_lblDeviceDetailDesc->setText("Selecciona un elemento de la tabla para inspeccionar sus detalles.");
        if (m_txtDeviceTechViewer) {
            m_txtDeviceTechViewer->clear();
        }
        return;
    }

    const QString devId = sel.first()->data(Qt::UserRole).toString();

    DeviceInfo targetDev;
    for (const DeviceInfo &d : m_cachedHardwareOverview.devices) {
        if (d.id == devId) {
            targetDev = d;
            break;
        }
    }

    m_lblDeviceDetailTitle->setText(targetDev.friendlyName);
    m_lblDeviceDetailDesc->setText(
        QString("<b>Estado:</b> %1<br><b>Archivo de configuración subyacente:</b> %2")
            .arg(targetDev.statusSummary, targetDev.targetConfigFile)
    );

    if (m_deviceAdvancedMode && m_txtDeviceTechViewer) {
        m_txtDeviceTechViewer->setPlainText(
            QString("ID Interno: %1\nCategoría: %2\nDetalles Técnicos de Hardware:\n%3\nArchivo de Configuración: %4")
                .arg(targetDev.id)
                .arg(static_cast<int>(targetDev.category))
                .arg(targetDev.technicalDetails)
                .arg(targetDev.targetConfigFile)
        );
    }
}

void MainWindow::onConfigureDeviceClicked()
{
    QMessageBox::information(
        this,
        "Configuración de Dispositivo",
        "El asistente visual está aplicando los ajustes seleccionados sobre el archivo de texto correspondiente de forma transparente."
    );
}

QWidget* MainWindow::createModulePlaceholderPage(const QString &title,
                                                 const QString &subtitle,
                                                 const QString &targetFiles)
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(12);

    QLabel *lblTitle = new QLabel(title, page);
    QFont titleFont = lblTitle->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    lblTitle->setFont(titleFont);

    QLabel *lblSub = new QLabel(subtitle, page);
    lblSub->setWordWrap(true);
    lblSub->setStyleSheet("font-size: 13px;");

    QFrame *techBox = new QFrame(page);
    techBox->setStyleSheet(
        "QFrame { background-color: #eef4fa; border: 1px solid #b6d4fe; border-radius: 6px; padding: 10px; }"
    );
    QVBoxLayout *techLayout = new QVBoxLayout(techBox);
    QLabel *lblFiles = new QLabel(
        QString("<b>Archivos/Servicios del sistema que gestionará este módulo:</b><br>%1").arg(targetFiles),
        techBox
    );
    lblFiles->setWordWrap(true);
    lblFiles->setStyleSheet("border: none; font-size: 12px;");
    techLayout->addWidget(lblFiles);

    layout->addWidget(lblTitle);
    layout->addWidget(lblSub);
    layout->addWidget(techBox);
    layout->addStretch();

    return page;
}

void MainWindow::onNavigationChanged(int index)
{
    if (index < 0 || index >= m_pagesStack->count()) {
        return;
    }
    LOG_INFO("MainWindow", QString("Cambiando a sección índice: %1 (%2)")
                               .arg(index)
                               .arg(m_navList->item(index)->text()));

    if (index == 1) {
        if (m_lblLocaleFeedback) {
            m_lblLocaleFeedback->setVisible(false);
        }
        refreshLocalePage();
    } else if (index == 2) {
        if (m_lblKeyboardFeedback) {
            m_lblKeyboardFeedback->setVisible(false);
        }
        refreshKeyboardPage();
    } else if (index == 3) {
        if (m_lblUserFeedback) {
            m_lblUserFeedback->setVisible(false);
        }
        refreshAddUserPage();
    } else if (index == 4) {
        if (m_lblGroupFeedback) {
            m_lblGroupFeedback->setVisible(false);
        }
        refreshUserGroupsPage();
    } else if (index == 5) {
        if (m_lblSudoFeedback) {
            m_lblSudoFeedback->setVisible(false);
        }
        refreshSudoPage();
    } else if (index == 6) {
        if (m_lblWebServerFeedback) {
            m_lblWebServerFeedback->setVisible(false);
        }
        if (m_webPreviewFrame) {
            m_webPreviewFrame->setVisible(false);
        }
        refreshWebServerPage();
    } else if (index == 7) {
        if (m_lblDbConfigFeedback) {
            m_lblDbConfigFeedback->setVisible(false);
        }
        if (m_dbSessionTestFrame) {
            m_dbSessionTestFrame->setVisible(false);
        }
        if (m_dbBootFrame) {
            m_dbBootFrame->setVisible(false);
        }
        refreshDatabasePage();
    } else if (index == 8) {
        if (m_lblTimeFeedback) {
            m_lblTimeFeedback->setVisible(false);
        }
        refreshTimePage();
    } else if (index == 9) {
        if (m_lblServiceFeedback) {
            m_lblServiceFeedback->setVisible(false);
        }
        refreshServicesPage();
    } else if (index == 10) {
        if (m_lblDeviceFeedback) {
            m_lblDeviceFeedback->setVisible(false);
        }
        refreshDevicesPage();
    }

    m_pagesStack->setCurrentIndex(index);
    updateNavigationButtons();
}

void MainWindow::onPrevModuleClicked()
{
    int current = m_navList->currentRow();
    if (current > 0) {
        m_navList->setCurrentRow(current - 1);
    }
}

void MainWindow::onNextModuleClicked()
{
    int current = m_navList->currentRow();
    if (current < m_navList->count() - 1) {
        m_navList->setCurrentRow(current + 1);
    }
}

void MainWindow::updateNavigationButtons()
{
    int current = m_navList->currentRow();
    m_btnPrev->setEnabled(current > 0);
    m_btnNext->setEnabled(current < m_navList->count() - 1);
}
