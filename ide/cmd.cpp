#include "cmd.h"

#include <QList>
#include <utility>

Command::Command() = default;

Command::Command(QString cmd) : cmd(std::move(cmd)) {}

QString Command::text() const { return cmd + "\n"; }

Command Command::merge(const Command &other) const { return {cmd + " && " + other.cmd}; }

Command Command::clearScreen() { return {"clear"}; }

Command Command::changeDirectory(const QString &dir) { return {"cd " + dir}; }

Command Command::runFile(const FileInfo &file) {
    Command cd = changeDirectory(file.absolutePath());
    QString name = file.fileName();
    QString target;

    Language language = file.getLanguage();

    Command compile;
    Command run;

    switch (language) {
        case Language::PYTHON:
            run = {"python " + name};
            break;
        case Language::C:
            target = name.split(".").first();
            compile = Command("gcc " + name + " -o " + target);
            run = {"./" + target};
            run = compile.merge(run);
            break;
        case Language::CPP:
            target = name.split(".").first();
            compile = Command("g++ " + name + " -o " + target);
            run = {"./" + target};
            run = compile.merge(run);
            break;
        default:
            // TODO
            run = "run " + name;
            break;
    }

    return cd.merge(run);
};
