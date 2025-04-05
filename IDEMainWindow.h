#ifndef IDE_MAIN_WINDOW_H
#define IDE_MAIN_WINDOW_H

#include <QMainWindow>

#include "ide/ide.h"
#include "widgets/code.h"
#include "widgets/fileTree.h"
#include "widgets/iconNav.h"
#include "widgets/menu.h"
#include "widgets/preview.h"
#include "widgets/terminal.h"

class IDEMainWindow : public QMainWindow {
    Q_OBJECT

    IDE *ide;

    LeftIconNavigateWidget *leftNav;
    RightIconNavigateWidget *rightNav;
    FileTreeWidget *fileTree;
    TerminalWidget *terminal;
    CodeTabWidget *codeTab;
    MenuBarWidget *menuBar;
    OpenJudgePreviewWidget *ojPreview;

    void setup();
    void connectSignals();

public:
    IDEMainWindow(int argc, char *argv[], QWidget *parent = nullptr);

public slots:
    void openFolder(const QString &folder) const;
    void runCurrentFile() const;
};


#endif // IDE_MAIN_WINDOW_H
