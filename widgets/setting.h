#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QStackedWidget>

class SettingsDialog : public QDialog {
    Q_OBJECT
    QListWidget *navList;
    QStackedWidget *stackedWidget;

    void createNavigationList();
    void createPages();
    QWidget *createGeneralPage();
    QWidget *createAppearancePage();
    QWidget *createNetworkPage();
    QWidget *createAdvancedPage();

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
};

#endif // SETTINGS_H
