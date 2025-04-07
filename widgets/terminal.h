#ifndef TERMINAL_H
#define TERMINAL_H

#include <qtermwidget.h>

#include "../ide/cmd.h"
#include "../ide/project.h"

/* A wrapper class for terminal */

class TerminalWidget : public QWidget {
    Q_OBJECT

    const Project *project;
    QVBoxLayout *layout;
    QTermWidget *shell;

    void newTerminal();

public:
    explicit TerminalWidget(QWidget *parent = nullptr);
    void setProject(const Project *project);
    void runCmd(const Command &command) const;
};

#endif // TERMINAL_H
