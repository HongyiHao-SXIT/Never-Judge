#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QFile>
#include <QUrl>
#include <expected>
#include <qcorotask.h>

class Crawler {
    const QUrl url;

public:
    explicit Crawler(QUrl url);
    /* Returns the downloaded content or an error message */
    QCoro::Task<std::expected<QByteArray, QString>> crawl() const;
};

struct OJProblem {
    QString title;
    QString content;
};

class OJParser {
public:
    static QCoro::Task<std::expected<OJProblem, QString>> parseProblem(const QByteArray &html);
    static QCoro::Task<std::expected<QStringList, QString>> parseProblemUrlsInMatch(const QByteArray &content);
};


#endif // DOWNLOAD_H
