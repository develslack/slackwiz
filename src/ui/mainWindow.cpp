#include "logger.h"
#include "commandRunner.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QStatusBar>
#include <QStyle>
#include <QFont>
#include "mainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_navList(nullptr),
      m_pagesStack(nullptr),
      m_btnPrev(nullptr),
      m_btnNext(nullptr),
      m_statusPrivilegeLabel(nullptr),
      m_statusModeLabel(nullptr)
{
    LOG_INFO("MainWindow", "Inicializando interfaz gráfica principal...");
    setupUi();
    setupNavigationMenu();
    setupPages();

    m_navList->setCurrentRow(0);
    updateNavigationButtons();
    LOG_SUCCESS("MainWindow", "Interfaz gráfica cargada correctamente.");
}

void MainWindow::setupUi()
{
    setWindowTitle("Slackware Wizard (slackwiz) - Asistente de Configuración");
    resize(960, 620);
    setMinimumSize(840, 540);

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
        { "8. Hora del sistema",    QStyle::SP_BrowserReload }
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

    m_pagesStack->addWidget(createModulePlaceholderPage(
        "1. Configuración del Idioma del Sistema",
        "Selecciona el idioma y codificación UTF-8 del sistema (ej. es_AR.UTF-8, es_ES.UTF-8, en_US.UTF-8) sin editar archivos manualmente.",
        "/etc/profile.d/lang.sh"
    ));

    m_pagesStack->addWidget(createModulePlaceholderPage(
        "2. Configuración del Teclado (Consola y Entorno Gráfico)",
        "Configura la distribución de teclas tanto para las terminales TTY como para el entorno gráfico X11.",
        "/etc/rc.d/rc.keymap y /etc/X11/xorg.conf.d/90-keyboard-layout.conf"
    ));

    m_pagesStack->addWidget(createModulePlaceholderPage(
        "3. Añadir un Nuevo Usuario",
        "Crea una cuenta de usuario estándar con su directorio /home y shell por defecto listo para trabajar.",
        "/etc/passwd y /etc/shadow (mediante useradd)"
    ));

    m_pagesStack->addWidget(createModulePlaceholderPage(
        "4. Gestionar Grupos del Usuario",
        "Asigna los grupos esenciales de Slackware (wheel, audio, video, netdev, plugdev, power, cdrom) con un clic.",
        "/etc/group (mediante usermod)"
    ));

    m_pagesStack->addWidget(createModulePlaceholderPage(
        "5. Habilitar y Configurar Sudo",
        "Activa el uso de sudo para usuarios administradores (grupo wheel) de forma segura y estándar.",
        "/etc/sudoers.d/wheel"
    ));

    m_pagesStack->addWidget(createModulePlaceholderPage(
        "6. Configurar Servidor Web (Apache / Nginx)",
        "Habilita el arranque automático y la configuración inicial de Apache HTTP Server o Nginx.",
        "/etc/rc.d/rc.httpd o /etc/rc.d/rc.nginx"
    ));

    m_pagesStack->addWidget(createModulePlaceholderPage(
        "7. Configurar Motor de Base de Datos (MariaDB / PostgreSQL)",
        "Inicializa las tablas del sistema, permisos iniciales y servicio de arranque para MariaDB o PostgreSQL.",
        "/etc/rc.d/rc.mysqld o /etc/rc.d/rc.postgresql"
    ));

    m_pagesStack->addWidget(createModulePlaceholderPage(
        "8. Configurar Fecha, Hora y Zona Horaria",
        "Selecciona tu zona horaria local, ajusta el reloj del sistema y habilita la sincronización por red (NTP).",
        "/etc/localtime y /etc/rc.d/rc.ntpd"
    ));
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
                "<b>Módulos disponibles:</b> Idioma, Teclado, Usuarios, Grupos, Sudo, Servidor Web, Base de Datos y Hora.")
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
