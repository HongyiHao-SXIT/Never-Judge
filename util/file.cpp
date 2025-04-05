#include "file.h"

#include <QDir>
#include <QStandardPaths>

QFile loadRes(const QString &path) { return QFile(":/res/" + path); }

const QString TempFiles::PATH = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/never-judge";
#define TempFileName(id) (PATH + "/temp" + (id.isEmpty() ? "" : "-") + id)

TempFiles::TempId TempFiles::genID(const QString &filename) {
    QString id = filename;
    int lastSlash = id.lastIndexOf('/');
    if (lastSlash != -1) {
        id = id.mid(lastSlash + 1);
    }
    return id;
}

std::unique_ptr<QFile> TempFiles::cache(const TempId &id) {
    QDir dir;
    if (dir.mkpath(PATH))
        return nullptr;
    QString tempFile = TempFileName(id);
    auto file = std::make_unique<QFile>(tempFile);
    return file->exists() ? std::move(file) : nullptr;
}

std::unique_ptr<QFile> TempFiles::create(const TempId &id, const QString &content) {
    // make a temp folder if not exist
    QDir dir;
    dir.mkpath(PATH);
    // create a temp file
    QString tempFile = TempFileName(id);
    auto file = std::make_unique<QFile>(tempFile);
    if (file->exists()) {
        file->remove();
    }
    file->open(QIODevice::WriteOnly);
    if (!content.isEmpty()) {
        QTextStream out(file.get());
        out << content;
        out.flush();
    }
    return file;
}

void TempFiles::clearCache() {
    QDir dir;
    if (!dir.exists(PATH)) {
        return;
    }
    dir.setPath(PATH);
    dir.removeRecursively();
    qDebug() << "TempFiles::cleared cache:" << PATH;
}

ScriptResult ScriptResult::fail() { return {false}; }


QCoro::Task<ScriptResult> runPythonScript(QFile &script, QStringList args) {
    QString id = TempFiles::genID(script.fileName());
    auto file = TempFiles::cache(id);
    if (!file) {
        if (!script.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to read script: " << script.fileName();
            co_return ScriptResult::fail();
        }
        QTextStream in(&script);
        QString content = in.readAll();
        script.close();
        file = TempFiles::create(id, content);
    }
    file->close();

    auto process = std::make_unique<QProcess>();
    process->start("python3", QStringList() << file->fileName() << args);
    if (!process->waitForStarted()) {
        qWarning() << "Failed to start script: " << process->errorString();
        co_return ScriptResult::fail();
    }
    if (!process->waitForFinished()) {
        qWarning() << "Script cost too long time: " << process->errorString();
        co_return ScriptResult::fail();
    }

    QString stdOut = process->readAllStandardOutput();
    QString stdErr = process->readAllStandardError();
    qDebug() << "stdout:" << stdErr;
    int exitCode = process->exitCode();
    co_return {true, exitCode, stdOut, stdErr};
}
