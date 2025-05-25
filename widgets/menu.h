#ifndef MENUBAR_H
#define MENUBAR_H

#include <QLabel>
#include <QMenuBar>

class MenuBarWidget : public QMenuBar {
    Q_OBJECT

    QLabel *user;
    QWidget* right;

    using SlotFunc = void (MenuBarWidget::*)();

    void newAction(QMenu *menu, const QString &title, const QKeySequence &shortcut, SlotFunc slot);
    void setup();

signals:
    void runCode();
    void saveFile();
    void openFolder(const QString &folderPath);
    /** Create a new file (at the root of the project) */
    void newFile();
    /** Create a new folder (at the root of the project) */
    void newFolder();
    /** Open the settings */
    void openSettings();
    /** Login to OJ */
    void loginOJ();
    /** Download from OJ */
    void downloadOJ();
    /** Batch download from OJ */
    void batchDownloadOJ();
    /** Submit the code to OJ */
    void submitOJ();
    /** Open personal information settings */
    void openPersonalInfo();

private slots:
    void onSave();
    void onOpenFolder();
    void onNewFile();
    void onNewFolder();
    void onOpenSettings();
    void onLoginOJ();
    void onDownloadOJ();
    void onBatchDownloadOJ();
    void onSubmitOJ();

public slots:
    void onLogin(const QString& username) ;

public:
    explicit MenuBarWidget(QWidget *parent = nullptr);
};


#endif // MENUBAR_H
