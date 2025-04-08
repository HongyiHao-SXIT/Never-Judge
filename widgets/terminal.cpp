#include "terminal.h"
#include <QLayout>
#include <QtDebug>

#include "../util/file.h"

class MyShell : public QTermWidget {
    Q_OBJECT

private slots:
    void setFont(const QJsonValue& fontJson) {
        QJsonObject obj = fontJson.toObject();
        QFont font;
        font.setFamily(obj["family"].toString());
        font.setPointSize(obj["size"].toInt() - 3);  // a little smaller
        setTerminalFont(font);
    }

    void setColorScheme(const QString &name) override {
        qDebug() << "setColorScheme"<< name;
        QTermWidget::setColorScheme(name);
    }

    void setup() {
        QIcon::setThemeName("oxygen");

        Configs::bindHotUpdateOn(this, "codeFont", &MyShell::setFont);
        Configs::instance().manuallyUpdate("codeFont");
        Configs::bindHotUpdateOn(this,"terminalTheme", &MyShell::setColorScheme);
        Configs::instance().manuallyUpdate("terminalTheme");
        setScrollBarPosition(ScrollBarRight);
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
