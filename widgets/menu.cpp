#include <QFileDialog>
#include <QInputDialog>
#include <QKeySequence>
#include <QToolButton>
#include <QWidgetAction>

#include "menu.h"

MenuBarWidget::MenuBarWidget(QWidget *parent) : QMenuBar(parent) { setup(); }

void MenuBarWidget::newAction(QMenu *menu, const QString &title, const QKeySequence &shortcut, SlotFunc slot) {
    auto *action = new QAction(title, this);
    action->setShortcut(shortcut);
    menu->addAction(action);
    connect(action, &QAction::triggered, this, slot);
}

void MenuBarWidget::setup() {
    // File menu
    QMenu *fileMenu = this->addMenu("文件");
    newAction(fileMenu, "保存", QKeySequence(Qt::CTRL | Qt::Key_S), &MenuBarWidget::onSave);
    newAction(fileMenu, "打开项目", QKeySequence(Qt::CTRL | Qt::Key_O), &MenuBarWidget::onOpenFolder);
    QMenu *newMenu = fileMenu->addMenu("新建");
    newAction(newMenu, "文件", QKeySequence(Qt::CTRL | Qt::Key_N), &MenuBarWidget::onNewFile);
    newAction(newMenu, "文件夹", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N), &MenuBarWidget::onNewFolder);

    // Run menu
    QMenu *runMenu = this->addMenu("运行");
    newAction(runMenu, "运行", QKeySequence(Qt::CTRL | Qt::Key_R), &MenuBarWidget::runCode);

    // OJ menu
    QMenu *ojMenu = this->addMenu("OpenJudge");
    newAction(ojMenu, "登录", QKeySequence(), &MenuBarWidget::onLoginOJ);
    newAction(ojMenu, "下载", QKeySequence(), &MenuBarWidget::onDownloadOJ);
    newAction(ojMenu, "批量下载", QKeySequence(), &MenuBarWidget::onBatchDownloadOJ);
    newAction(ojMenu, "提交", QKeySequence(), &MenuBarWidget::onSubmitOJ);

    // show username here
    user = new QLabel(tr("未登录"), this);
    user->setStyleSheet("padding: 10px");
    setCornerWidget(user);
}

void MenuBarWidget::onSave() { emit saveFile(); }

void MenuBarWidget::onOpenFolder() {
    QString folderPath =
            QFileDialog::getExistingDirectory(this, tr("选择项目"), QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (!folderPath.isEmpty()) {
        emit openFolder(folderPath);
    }
}

void MenuBarWidget::onNewFile() { emit newFile(); }

void MenuBarWidget::onNewFolder() { emit newFolder(); }

void MenuBarWidget::onLoginOJ() { emit loginOJ(); }

void MenuBarWidget::onDownloadOJ() { emit downloadOJ(); }

void MenuBarWidget::onBatchDownloadOJ() { emit batchDownloadOJ(); }

void MenuBarWidget::onSubmitOJ() { emit submitOJ(); }

void MenuBarWidget::onLogin(const QString &username)  {
    user->setText(username);
    setCornerWidget(user);
}
