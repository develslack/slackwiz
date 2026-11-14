#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

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

private:
    void setupUi();
    void setupNavigationMenu();
    void setupPages();
    void updateNavigationButtons();

    QWidget* createWelcomePage();
    QWidget* createModulePlaceholderPage(const QString &title,
                                         const QString &subtitle,
                                         const QString &targetFiles);

    QListWidget *m_navList;
    QStackedWidget *m_pagesStack;
    QPushButton *m_btnPrev;
    QPushButton *m_btnNext;
    QLabel *m_statusPrivilegeLabel;
    QLabel *m_statusModeLabel;
};

#endif // MAINWINDOW_H
