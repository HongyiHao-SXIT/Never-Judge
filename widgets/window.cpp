#include "window.h"

#include <QLayout>
#include <QMessageBox>
#include <QSplitter>

IDEMainWindow::IDEMainWindow(int argc, char *argv[], QWidget *parent) : QMainWindow(parent) {
    ide = new IDE();
    leftNav = new LeftIconNavigateWidget(this);
    rightNav = new RightIconNavigateWidget(this);
    fileTree = new FileTreeWidget(this);
    terminal = new TerminalWidget(this);
    codeTab = new CodeTabWidget(this);
    menuBar = new MenuBarWidget(this);
    ojPreview = new OpenJudgePreviewWidget(this);
    ojPreview->setVisible(false);

    setup();
    connectSignals();

    QString project = QString::fromStdString(std::filesystem::current_path());
    openFolder(project);
}

void IDEMainWindow::setup() {
    this->setWindowTitle(tr("Simple IDE"));
    this->setMinimumSize(1366, 768);
    this->setMenuBar(menuBar);

    auto *mainLayout = new QHBoxLayout(this);
    auto *editLayout = new QVBoxLayout(this);
    mainLayout->addWidget(leftNav);
    mainLayout->addLayout(editLayout);
    mainLayout->addWidget(rightNav);

    auto *hSplitter = new QSplitter(Qt::Horizontal, this);
    hSplitter->addWidget(fileTree);
    hSplitter->addWidget(codeTab);
    hSplitter->addWidget(ojPreview);

    hSplitter->setStretchFactor(0, 1);
    hSplitter->setStretchFactor(1, 3);
    hSplitter->setStretchFactor(2, 1);

    auto *vSplitter = new QSplitter(Qt::Vertical, this);
    vSplitter->addWidget(hSplitter);
    vSplitter->addWidget(terminal);
    vSplitter->setStretchFactor(0, 3);
    vSplitter->setStretchFactor(1, 1);

    editLayout->addWidget(vSplitter);

    auto *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    this->setCentralWidget(centralWidget);
}

void IDEMainWindow::connectSignals() {
    // Icon navigate
    connect(leftNav, &LeftIconNavigateWidget::toggleFileTree, fileTree, &FileTreeWidget::setVisible);
    connect(rightNav, &RightIconNavigateWidget::togglePreview, ojPreview, &OpenJudgePreviewWidget::setVisible);

    // File system
    connect(menuBar, &MenuBarWidget::newFile, fileTree,
            [this] { fileTree->createNewFile(ide->curProject().getRoot()); });
    connect(menuBar, &MenuBarWidget::newFolder, fileTree,
            [this] { fileTree->createNewFolder(ide->curProject().getRoot()); });

    connect(menuBar, &MenuBarWidget::saveFile, codeTab, &CodeTabWidget::save);
    connect(menuBar, &MenuBarWidget::openFolder, this, &IDEMainWindow::openFolder);
    connect(fileTree, &FileTreeWidget::operateFile, codeTab, &CodeTabWidget::handleFileOperation);

    // Running
    connect(menuBar, &MenuBarWidget::runCode, this, &IDEMainWindow::runCurrentFile);

    // OJ
    connect(menuBar, &MenuBarWidget::downloadOJ, ojPreview, &OpenJudgePreviewWidget::downloadOJ);
    connect(menuBar, &MenuBarWidget::batchDownloadOJ, ojPreview, &OpenJudgePreviewWidget::batchDownloadOJ);
}

void IDEMainWindow::openFolder(const QString &folder) const {
    qDebug() << "Opening folder " << folder;
    Project project(folder);
    ide->setProject(project);
    codeTab->clearAll();
    fileTree->setRoot(project.getRoot());
    terminal->setProject(&ide->curProject());
}

void IDEMainWindow::runCurrentFile() const {
    FileInfo file = codeTab->currentFile();

    if (!file.isValid()) {
        QMessageBox msgBox;
        msgBox.setText(tr("不存在的文件或非法文件"));
        msgBox.setInformativeText(tr("请先打开一个文件"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("错误"));
        msgBox.exec();
        return;
    }
    if (file.getLanguage() == Language::UNKNOWN) {
        QMessageBox msgBox;
        msgBox.setText(tr("不支持运行的文件类型"));
        msgBox.setInformativeText(tr("仅支持 C/C++/CMAKE/Python 文件"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("错误"));
        msgBox.exec();
        return;
    }

    codeTab->save(); // save the current file before running
    terminal->runCmd(Command::runFile(file));
}
