#include "file.h"

#include <QDir>
#include <QStandardPaths>
#include <qcoro/qcoroprocess.h>

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

const QString ConfigManager::PATH =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/never-judge/config.json";

ConfigManager &ConfigManager::instance() {
    static ConfigManager instance = ConfigManager(nullptr);
    return instance;
}

void ConfigManager::clear() {
    QFile file(PATH);
    if (file.exists()) {
        file.remove();
    }
}

ConfigManager::ConfigManager(QObject *parent) : QObject(parent) {
    QDir dir = QFileInfo(PATH).absoluteDir();
    if (!dir.exists() && !dir.mkpath(PATH)) {
        qDebug() << "ConfigManager::ConfigManager cannot make dir:" << PATH;
    }
    QFile file(PATH);
    if (!file.exists()) {
        file.open(QIODevice::WriteOnly);
        QFile defaultFile = loadRes("setting/settings.json");
        if (!defaultFile.open(QIODevice::ReadOnly)) {
            qWarning() << "Cannot open default config file:" << defaultFile.errorString();
            return;
        }
        QByteArray data = defaultFile.readAll();
        file.write(data);
        file.flush();
        file.close();
        defaultFile.close();
        qDebug() << "ConfigManager::ConfigManager created default config file:" << PATH;
    }

    loadConfig();

    fileWatcher.addPath(PATH);
    connect(&fileWatcher, &QFileSystemWatcher::fileChanged, this, [this] {
        qDebug() << "Config file modified, reloading...";
        loadConfig();
    });
}

void ConfigManager::loadConfig() {
    QMutexLocker locker(&mutex);

    QFile file(PATH);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open config file:" << file.errorString();
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return;
    }

    config = doc.object();
    emit configChanged(config);
}

void ConfigManager::saveConfig() {
    QMutexLocker locker(&mutex);

    QFile file(PATH);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot save config file:" << file.errorString();
        return;
    }

    file.write(QJsonDocument(config).toJson());
    fileWatcher.addPath(PATH);
}

QVariant ConfigManager::get(const QString &key) const {
    QMutexLocker locker(&mutex);
    return config.value(key).toVariant();
}

QJsonObject ConfigManager::getAll() const {
    QMutexLocker locker(&mutex);
    return config;
}

void ConfigManager::set(const QString &key, const QVariant &value) {
    {
        QMutexLocker locker(&mutex);
        config[key] = QJsonValue::fromVariant(value);
    }
    saveConfig();
}

void ConfigManager::setBatch(const QJsonObject &config) {
    {
        QMutexLocker locker(&mutex);
        for (auto it = config.begin(); it != config.end(); ++it) {
            config[it.key()] = it.value();
        }
    }
    saveConfig();
}
