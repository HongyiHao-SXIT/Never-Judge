#ifndef MENUBAR_H
#define MENUBAR_H

#include <QMenuBar>

class MenuBarWidget : public QMenuBar {
    Q_OBJECT

    using SlotFunc = void (MenuBarWidget::*)();

    void newAction(QMenu *menu, const QString &title, const QKeySequence &shortcut, SlotFunc slot);
    void setup();

signals:
    void runCode();
    void saveFile();
    void openFolder(const QString &folderPath);
    /* Create a new file (at the root of the project) */
    void newFile();
    /* Create a new folder (at the root of the project) */
    void newFolder();
    /* Download from OJ */
    void downloadOJ();
    /* Batch download from OJ */
    void batchDownloadOJ();

private slots:
    void onSave();
    void onOpenFolder();
    void onNewFile();
    void onNewFolder();
    void onDownloadOJ();
    void onBatchDownloadOJ();

public:
    explicit MenuBarWidget(QWidget *parent = nullptr);
};


#endif // MENUBAR_H
