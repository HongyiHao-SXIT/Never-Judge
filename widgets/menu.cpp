#include <QFileDialog>
#include <QInputDialog>
#include <QKeySequence>
#include <QToolButton>
#include <QWidgetAction>

#include "menu.h"

#include <QPushButton>
#include <QVBoxLayout>

#include "../util/file.h"

MenuBarWidget::MenuBarWidget(QWidget *parent) : QMenuBar(parent) {
    user = new QLabel(tr("未登录"), this);
    setup();
}

void MenuBarWidget::newAction(QMenu *menu, const QString &title, const QKeySequence &shortcut,
                              SlotFunc slot) {
    auto *action = new QAction(title, this);
    action->setShortcut(shortcut);
    menu->addAction(action);
    connect(action, &QAction::triggered, this, slot);
}

void MenuBarWidget::setup() {
    // File menu
    QMenu *fileMenu = this->addMenu("文件");
    newAction(fileMenu, tr("保存"), QKeySequence(Qt::CTRL | Qt::Key_S), &MenuBarWidget::onSave);
    newAction(fileMenu, tr("打开项目"), QKeySequence(Qt::CTRL | Qt::Key_O),
              &MenuBarWidget::onOpenFolder);
    QMenu *newMenu = fileMenu->addMenu(tr("新建"));
    newAction(newMenu, tr("文件"), QKeySequence(Qt::CTRL | Qt::Key_N), &MenuBarWidget::onNewFile);
    newAction(newMenu, tr("文件夹"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N),
              &MenuBarWidget::onNewFolder);

    // Run menu
    QMenu *runMenu = this->addMenu(tr("运行"));
    newAction(runMenu, tr("运行"), QKeySequence(Qt::CTRL | Qt::Key_R), &MenuBarWidget::runCode);

    // Edit menu
    QMenu *editMenu = this->addMenu(tr("编辑"));
    newAction(editMenu, tr("设置"), QKeySequence(Qt::Key_F5), &MenuBarWidget::onOpenSettings);

    // OJ menu
    QMenu *ojMenu = this->addMenu(tr("OpenJudge"));
    newAction(ojMenu, tr("登录"), QKeySequence(), &MenuBarWidget::onLoginOJ);
    newAction(ojMenu, tr("个人信息"), QKeySequence(), &MenuBarWidget::onPersonalizeOJ);
    ojMenu->addSeparator();
    newAction(ojMenu, tr("下载"), QKeySequence(), &MenuBarWidget::onDownloadOJ);
    newAction(ojMenu, tr("批量下载"), QKeySequence(), &MenuBarWidget::onBatchDownloadOJ);
    newAction(ojMenu, tr("提交"), QKeySequence(), &MenuBarWidget::onSubmitOJ);

    // show username here
    right = new QWidget(this);
    auto *rightLayout = new QHBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    user->setObjectName("userLabel");
    auto *startBtn = new QPushButton(right);
    startBtn->setIcon(loadIcon("icons/start.svg"));
    connect(startBtn, &QPushButton::clicked, this, &MenuBarWidget::runCode);
    startBtn->setObjectName("startBtn");

    rightLayout->addWidget(user);
    rightLayout->addWidget(startBtn);
    right->setLayout(rightLayout);
    setCornerWidget(right);

    setStyleSheet(loadText("qss/menu.css"));
}

void MenuBarWidget::onSave() { emit saveFile(); }

void MenuBarWidget::onOpenFolder() {
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("选择项目"), QDir::homePath(),
                                                           QFileDialog::ShowDirsOnly);
    if (!folderPath.isEmpty()) {
        emit openFolder(folderPath);
    }
}

void MenuBarWidget::onNewFile() { emit newFile(); }

void MenuBarWidget::onNewFolder() { emit newFolder(); }

void MenuBarWidget::onOpenSettings() { emit openSettings(); }

void MenuBarWidget::onLoginOJ() { emit loginOJ(); }

void MenuBarWidget::onPersonalizeOJ() { emit personalizeOJ(); }

void MenuBarWidget::onDownloadOJ() { emit downloadOJ(); }

void MenuBarWidget::onBatchDownloadOJ() { emit batchDownloadOJ(); }

void MenuBarWidget::onSubmitOJ() { emit submitOJ(); }

void MenuBarWidget::onLogin(const QString &username) {
    user->setText(username);
    setCornerWidget(right);
}
