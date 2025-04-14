#include "cmd.h"

#include <QList>
#include <utility>

#include "../util/file.h"

class RunCommandManager : public QObject {
    Q_OBJECT

    QMap<Language, QString> cmdRules{};
    RunCommandManager() {
        Configs::bindHotUpdateOn(this, "runCommand", &RunCommandManager::onCmdChanged);
        Configs::instance().manuallyUpdate("runCommand");
    }

private slots:
    void onCmdChanged(const QJsonValue &cmds) {
        auto obj = cmds.toObject();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            Language lang = stringToLang(it.key());
            if (lang == Language::UNKNOWN) {
                continue;
            }
            cmdRules[lang] = it.value().toString();
        }
    }

public:
    static RunCommandManager &instance() {
        static RunCommandManager instance;
        return instance;
    }
    QString getCommand(const LangFileInfo &file) {
        Language lang = file.language();
        QString rule = cmdRules[lang];

        QString name = file.fileName();
        QString nameNoExt = name.split(".").first();
        QString dir = file.absolutePath();
        // replace $dir, $filename, $filenameNoExt
        rule.replace("$dir", "\"" + dir + "\"").replace("$filenameNoExt", nameNoExt).replace("$filename", name);
        return rule;
    }
};

Command::Command(QString cmd) : cmd(std::move(cmd)) {}

QString Command::text() const { return cmd + "\n"; }

Command Command::merge(const Command &other) const { return {cmd + " && " + other.cmd}; }

Command Command::clearScreen() { return {"clear"}; }

Command Command::changeDirectory(const QString &dir) { return {"cd '" + dir + "'"}; }

Command Command::runFile(const LangFileInfo &file) { return RunCommandManager::instance().getCommand(file); };

#include "cmd.moc"
