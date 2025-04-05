#ifndef FILE_H
#define FILE_H

#include <QFile>
#include <QProcess>
#include <qcorotask.h>


QFile loadRes(const QString &path);

class TempFiles {
public:
    static const QString PATH;

    using TempId = QString;

    static TempId genID(const QString &filename);
    static void clearCache();
    static std::unique_ptr<QFile> cache(const TempId &id);
    static std::unique_ptr<QFile> create(const TempId &id = "", const QString &content = "");
};

class DynamicResources {

};

struct ScriptResult {
    bool success;
    int exitCode;
    QString stdOut;
    QString stdErr;

    static ScriptResult fail();
};

QCoro::Task<ScriptResult> runPythonScript(QFile &script, QStringList args);

#endif // FILE_H
