#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QFile>
#include <QUrl>
#include <expected>
#include <qcorotask.h>

class Crawler {
    const QUrl url;

public:
    explicit Crawler(const QString &url);
    /* Returns the downloaded content or an error message */
    QCoro::Task<std::expected<QByteArray, QString>> crawl() const;
};

class OJParser {
public:
    QCoro::Task<std::expected<QString, QString>> parse(const QByteArray &content) const;
};


#endif // DOWNLOAD_H
