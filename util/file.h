#ifndef FILE_H
#define FILE_H

#include <QFile>
#include <QFileSystemWatcher>
#include <QJsonObject>
#include <QMutex>
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

class ConfigManager : public QObject {
    Q_OBJECT

    explicit ConfigManager(QObject *parent = nullptr);
    void loadConfig();
    void saveConfig();

    QJsonObject config;
    // use mutex for threading safe
    mutable QMutex mutex;
    QFileSystemWatcher fileWatcher;
    static const QString PATH;

public:
    static ConfigManager &instance();
    static void clear();
    QVariant get(const QString &key) const;
    QJsonObject getAll() const;

    void set(const QString &key, const QVariant &value);
    void setBatch(const QJsonObject &config);

signals:
    void configChanged(const QJsonObject &newConfig);
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
