#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QDateEdit>
#include <QTimeEdit>
#include <QGridLayout>
#include <QWidget>
#include <QFrame>
#include <QTextBrowser>
#include <QPlainTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QList>

#include "timeManager.h"
#include "timezoneMapWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onNavigationChanged(int index);
    void onNextModuleClicked();
    void onPrevModuleClicked();

    // Módulo 1: Idioma
    void onSaveLocaleClicked();

    // Módulo 2: Teclado
    void onSaveKeyboardClicked();

    // Módulo 3: Añadir usuario
    void onCreateUserClicked();
    void onToggleShowPassword(bool checked);
    void onSelectRecommendedGroups();
    void onSelectAllGroups();

    // Módulo 4: Grupos de usuario
    void onGroupUserSelectionChanged(int index);
    void onRefreshGroupUsersClicked();
    void onGroupPresetRecommendedClicked();
    void onGroupPresetAllClicked();
    void onGroupPresetNoneClicked();
    void onSaveUserGroupsClicked();

    // Módulo 5: Configurar sudo
    void onSudoUserSelectionChanged(int index);
    void onRefreshSudoUsersClicked();
    void onSudoModeRadioToggled();
    void onSaveSudoClicked();

    // Módulo 6: Servidor web + PHP + Vista previa navegador
    void onRescanWebServerClicked();
    void onEnablePhpToggled(bool checked);
    void onUpdatePhpIniToggled(bool checked);
    void onSaveWebServerClicked();
    void onPreviewLocalhostClicked();
    void onPreviewPhpInfoClicked();
    void onPreviewNavigateClicked();
    void onPreviewAnchorClicked(const QUrl &url);
    void onPreviewExternalBrowserClicked();
    void onCloseWebPreviewClicked();
    void onWebPreviewReplyFinished(QNetworkReply *reply);

    // Módulo 7: Base de datos (MariaDB / PostgreSQL)
    void onRescanDatabaseClicked();
    void onDatabaseEngineRadioToggled();
    void onDbSetPasswordToggled(bool checked);
    void onDbShowPasswordToggled(bool checked);
    void onSaveDatabaseConfigClicked();
    void onTestDatabaseSessionClicked();
    void onSaveDatabaseBootClicked();

    // Módulo 8: Hora del sistema y Zona Horaria
    void onTimeMapZoneSelected(const QString &timezoneId);
    void onTimeComboZoneChanged(int index);
    void onTimeDstToggled(bool checked);
    void onTimeSyncModeChanged();
    void updateTimeLivePreview();
    void onSaveTimeConfigClicked();

private:
    void setupUi();
    void setupNavigationMenu();
    void setupPages();
    void updateNavigationButtons();
    void refreshLocalePage();
    void refreshKeyboardPage();
    void refreshAddUserPage();
    void refreshUserGroupsPage();
    void populateUserGroupCheckboxes(const QString &username);
    void refreshSudoPage();
    void refreshWebServerPage();
    void loadUrlInEmbeddedBrowser(const QString &urlString);
    void refreshDatabasePage();
    void refreshTimePage();

    QWidget* createWelcomePage();
    QWidget* createLocalePage();
    QWidget* createKeyboardPage();
    QWidget* createAddUserPage();
    QWidget* createUserGroupsPage();
    QWidget* createSudoPage();
    QWidget* createWebServerPage();
    QWidget* createDatabasePage();
    QWidget* createTimePage();
    QWidget* createModulePlaceholderPage(const QString &title,
                                         const QString &subtitle,
                                         const QString &targetFiles);

    QListWidget *m_navList;
    QStackedWidget *m_pagesStack;
    QPushButton *m_btnPrev;
    QPushButton *m_btnNext;
    QLabel *m_statusPrivilegeLabel;
    QLabel *m_statusModeLabel;

    // Controles del Módulo 1: Idioma del sistema
    QLabel *m_lblCurrentLocale;
    QComboBox *m_comboLocales;
    QPushButton *m_btnSaveLocale;
    QLabel *m_lblLocaleFeedback;

    // Controles del Módulo 2: Teclado
    QLabel *m_lblCurrentKeyboard;
    QComboBox *m_comboKeyboards;
    QLineEdit *m_editKeyboardTest;
    QPushButton *m_btnSaveKeyboard;
    QLabel *m_lblKeyboardFeedback;

    // Controles del Módulo 3: Añadir usuario
    QLabel *m_lblExistingUsers;
    QLineEdit *m_editNewUsername;
    QLineEdit *m_editNewFullName;
    QLineEdit *m_editNewPassword;
    QLineEdit *m_editConfirmPassword;
    QCheckBox *m_chkShowPassword;
    QComboBox *m_comboUserShell;
    QList<QCheckBox*> m_groupCheckboxes;
    QPushButton *m_btnCreateUser;
    QLabel *m_lblUserFeedback;

    // Controles del Módulo 4: Grupos de usuario
    QComboBox *m_comboGroupUsers;
    QLabel *m_lblUserCurrentGroups;
    QWidget *m_userGroupsContainer;
    QGridLayout *m_userGroupsGrid;
    QList<QCheckBox*> m_userGroupCheckboxes;
    QPushButton *m_btnSaveUserGroups;
    QLabel *m_lblGroupFeedback;

    // Controles del Módulo 5: Configurar sudo
    QLabel *m_lblSudoOverview;
    QComboBox *m_comboSudoUsers;
    QLabel *m_lblSudoSelectedUserDetail;
    QRadioButton *m_radioSudoWithPass;
    QRadioButton *m_radioSudoNoPass;
    QRadioButton *m_radioSudoDisabled;
    QCheckBox *m_chkSudoSyncWheel;
    QPushButton *m_btnSaveSudo;
    QLabel *m_lblSudoFeedback;

    // Controles del Módulo 6: Servidor web
    QLabel *m_lblWebDetectionSummary;
    QRadioButton *m_radioWebApache;
    QRadioButton *m_radioWebNginx;
    QCheckBox *m_chkEnablePhp;
    QFrame *m_phpOptionsContainer;
    QLabel *m_lblPhpTimezoneInfo;
    QLabel *m_lblPhpImagickNotice;
    QCheckBox *m_chkUpdatePhpIni;
    QComboBox *m_comboPhpMemoryLimit;
    QComboBox *m_comboPhpUploadMax;
    QComboBox *m_comboPhpPostMax;
    QCheckBox *m_chkPhpDisplayErrors;
    QCheckBox *m_chkWebStartOnBoot;
    QPushButton *m_btnSaveWebServer;
    QLabel *m_lblWebServerFeedback;

    // Vista de navegador incrustada (Módulo 6)
    QFrame *m_webPreviewFrame;
    QPushButton *m_btnPreviewLocalhost;
    QPushButton *m_btnPreviewPhpInfo;
    QLineEdit *m_editPreviewUrl;
    QLabel *m_lblPreviewHttpStatus;
    QTextBrowser *m_webPreviewBrowser;
    QNetworkAccessManager *m_networkManager;
    bool m_lastPhpEnabledState = false;

    // Controles del Módulo 7: Base de datos
    QLabel *m_lblDbDetectionSummary;
    QRadioButton *m_radioDbMariaDb;
    QRadioButton *m_radioDbPostgres;
    QCheckBox *m_chkDbSetRootPassword;
    QWidget *m_dbPasswordFieldsWidget;
    QLineEdit *m_editDbCurrentPassword;
    QLineEdit *m_editDbNewPassword;
    QLineEdit *m_editDbConfirmPassword;
    QCheckBox *m_chkDbShowPasswords;

    // Opciones específicas de MariaDB
    QWidget *m_dbMariaOptionsWidget;
    QCheckBox *m_chkMariaRemoveAnon;
    QCheckBox *m_chkMariaDisallowRemoteRoot;
    QCheckBox *m_chkMariaRemoveTestDb;
    QCheckBox *m_chkMariaReloadPrivs;

    // Opciones específicas de PostgreSQL
    QWidget *m_dbPostgresOptionsWidget;
    QCheckBox *m_chkPgScramAuth;
    QCheckBox *m_chkPgLocalhostOnly;
    QCheckBox *m_chkPgRemoveTestDb;

    QPushButton *m_btnSaveDbConfig;
    QLabel *m_lblDbConfigFeedback;

    // Paso 2 del Módulo 7: Prueba de sesión con contraseña de root/postgres
    QFrame *m_dbSessionTestFrame;
    QLabel *m_lblDbSessionPrompt;
    QLineEdit *m_editDbTestPassword;
    QCheckBox *m_chkDbShowTestPassword;
    QPushButton *m_btnTestDbSession;
    QLabel *m_lblDbSessionStatus;
    QPlainTextEdit *m_txtDbSessionOutput;

    // Paso 3 del Módulo 7: Arranque automático (habilitado tras sesión exitosa)
    QFrame *m_dbBootFrame;
    QLabel *m_lblDbBootQuestion;
    QRadioButton *m_radioDbBootYes;
    QRadioButton *m_radioDbBootNo;
    QPushButton *m_btnSaveDbBoot;
    QLabel *m_lblDbBootFeedback;

    // Controles del Módulo 8: Hora del sistema y Zona Horaria
    QLabel *m_lblTimeCurrentSummary;
    TimezoneMapWidget *m_timeMapWidget;
    QComboBox *m_comboTimezones;
    QCheckBox *m_chkTimeDst;
    QLabel *m_lblTimeOffsetInfo;
    QLabel *m_lblTimeLivePreview;
    QRadioButton *m_radioTimeAutoNtp;
    QRadioButton *m_radioTimeManual;
    QLineEdit *m_editTimeNtpServer;
    QDateEdit *m_dateTimeManualDate;
    QTimeEdit *m_timeTimeManualTime;
    QCheckBox *m_chkTimeHwUtc;
    QPushButton *m_btnSaveTimeConfig;
    QLabel *m_lblTimeFeedback;
    QTimer *m_timePreviewTimer;
    QList<TimezoneEntry> m_timeZonesList;
    bool m_syncingTimeSelection = false;
};

#endif // MAINWINDOW_H
