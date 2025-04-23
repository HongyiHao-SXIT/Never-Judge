#ifndef CRAWL_H
#define CRAWL_H

#include <expected>
#include <qcoro/qcoronetworkreply.h>
#include <qcorotask.h>

#include "oj.h"

class Crawler {
    QNetworkAccessManager nam;

    QString email;
    QString password;

    explicit Crawler();

public:
    static Crawler &instance();
    bool hasLogin() const;

    // Returns the response or an error message
    QCoro::Task<std::expected<QByteArray, QString>> get(const QUrl &url);
    QCoro::Task<std::expected<QByteArray, QString>> post(const QUrl &url, QMap<QString, QString> params);

    QCoro::Task<std::expected<QByteArray, QString>> login(const QString &email, const QString &password);
    /** if submit succeeded, return the redict url */
    QCoro::Task<std::expected<QUrl, QString>> submit(OJSubmitForm form);
};

#endif // CRAWL_H
