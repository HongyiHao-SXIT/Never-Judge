#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QStackedWidget>

class SettingsDialog : public QDialog {
    Q_OBJECT
    QListWidget *navList;
    QStackedWidget *stackedWidget;

    void createNavigationList();
    void createPages();

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
};

#endif // SETTINGS_H