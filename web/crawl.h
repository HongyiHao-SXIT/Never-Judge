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

    // Basic wrapper for web request
    // Returns the response or an error message
    template<class T>
    using WebResponse = QCoro::Task<std::expected<T, QString>>;

    WebResponse<QByteArray> get(const QUrl &url) const;
    WebResponse<QByteArray> post(const QUrl &url, QMap<QString, QString> params) const;

    /** if login succeeded, return the response from OJ */
    WebResponse<QByteArray> login(const QString &email, const QString &password);
    /** if submit succeeded, return the redict url */
    WebResponse<QUrl> submit(OJSubmitForm form);
    /** if personalization change succeeded, return the response from OJ */
    WebResponse<QByteArray> personalize(OJPersonalizationForm form);
};

#endif // CRAWL_H
