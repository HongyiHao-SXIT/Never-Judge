#include "terminal.h"
#include <QApplication>
#include <QLayout>
#include <QtDebug>

class MyShell : public QTermWidget {
    Q_OBJECT

    void setup() {
        QIcon::setThemeName(QStringLiteral("oxygen"));
        QFont font = QApplication::font();
        font.setFamily(QStringLiteral("Consolas"));
        font.setPointSize(12);
        this->setTerminalFont(font);
        qDebug() << " availableColorSchemes:" << availableColorSchemes();
        this->setColorScheme("BreezeModified");

        this->setScrollBarPosition(ScrollBarRight);
    }

public:
    explicit MyShell(QWidget *parent = nullptr) : QTermWidget(parent) { setup(); }
};

TerminalWidget::TerminalWidget(QWidget *parent) : QWidget(parent), project(nullptr), shell(nullptr) {
    layout = new QVBoxLayout(this);
    this->setLayout(layout);
}

void TerminalWidget::newTerminal() {

    // remove the old terminal
    if (shell) {
        layout->removeWidget(shell);
        delete shell;
    }

    shell = new MyShell(this);

    // change the directory to the project root
    runCmd(Command::changeDirectory(project->getRoot()));
    runCmd(Command::clearScreen());
    layout->addWidget(shell);

    // when the terminal is finished, create a new one
    connect(shell, &QTermWidget::finished, this, &TerminalWidget::newTerminal);
}


void TerminalWidget::setProject(const Project *project) {
    this->project = project;
    newTerminal();
}

void TerminalWidget::runCmd(const Command &command) const { shell->sendText(command.text()); }

#include "terminal.moc"
