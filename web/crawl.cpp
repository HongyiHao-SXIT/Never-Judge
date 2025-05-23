#include "crawl.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkCookieJar>
#include <QUrlQuery>

#include "../util/file.h"
#include "../util/script.h"

Crawler::Crawler() { nam.setCookieJar(new QNetworkCookieJar()); };

Crawler &Crawler::instance() {
    static Crawler instance;
    return instance;
}

bool Crawler::hasLogin() const { return !email.isEmpty() && !password.isEmpty(); }

// Crawler::WebResponse<QByteArray> Crawler::get(const QUrl &url) {
//     QNetworkRequest request(url);
//     request.setRawHeader("User-Agent", "Mozilla/5.0");
//     request.setRawHeader("Referer", "http://cxsjsx.openjudge.cn/");
//
//     request.setTransferTimeout(5000);
//     QNetworkReply *reply = co_await nam.get(request);
//     if (reply->error()) {
//         QString error = reply->errorString();
//         delete reply;
//         co_return std::unexpected(error);
//     }
//     if (reply->bytesAvailable() == 0) {
//         delete reply;
//         co_return std::unexpected("No data received");
//     }
//     QByteArray response = reply->readAll();
//     delete reply;
//     co_return response;
// }
//
// Crawler::WebResponse<QByteArray> Crawler::post(const QUrl &url, QMap<QString, QString> params) {
//     QNetworkRequest request(url);
//     request.setTransferTimeout(5000);
//     request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
//     request.setRawHeader("User-Agent", "Mozilla/5.0");
//     request.setRawHeader("Referer", "http://cxsjsx.openjudge.cn/");
//
//     QUrlQuery postParams;
//     for (auto it = params.begin(); it != params.end(); ++it) {
//         postParams.addQueryItem(it.key(), it.value());
//     }
//     QByteArray postData = postParams.toString(QUrl::FullyEncoded).toUtf8();
//
//     QNetworkReply *reply = co_await nam.post(request, postData);
//     if (reply->error()) {
//         QString error = reply->errorString();
//         delete reply;
//         co_return std::unexpected(error);
//     }
//     if (reply->bytesAvailable() == 0) {
//         delete reply;
//         co_return std::unexpected("No data received");
//     }
//     QByteArray response = reply->readAll();
//     delete reply;
//     co_return response;
// }
//
// Crawler::WebResponse<QByteArray> Crawler::loginToOJ(const QString &email, const QString
// &password) {
//     QString loginUrl = "http://openjudge.cn/api/auth/login/";
//     QMap<QString, QString> params = {{"email", email}, {"password", password}};
//     auto res = co_await Crawler::instance().post(loginUrl, params);
//     if (!res.has_value()) {
//         co_return std::unexpected(res.error());
//     }
//
//     QTextCodec *codec = QTextCodec::codecForName("GBK");
//     QString response = codec->toUnicode(res.value());
//     // use a json doc to parse the response
//     QJsonParseError error;
//     QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8(), &error);
//     if (error.error != QJsonParseError::NoError) {
//         co_return std::unexpected("JSON parse error: " + error.errorString());
//     }
//     QJsonObject obj = doc.object();
//     if (obj["result"].toString() == "SUCCESS") {
//         co_return res;
//     }
//     QString message = obj["message"].toString();
//     co_return std::unexpected(message);
// }


/* TODO: I know this implement is foolish
 * So fix the network manager later */

Crawler::WebResponse<QByteArray> Crawler::get(const QUrl &url) {
    QStringList args = {"-u", url.toString()};

    if (hasLogin()) {
        args << "-e" << email << "-p" << password;
    }

    QFile crawl = loadRes("script/crawl.py");
    auto res = co_await runPythonScript(crawl, args);
    if (!res.success || res.exitCode != 0) {
        co_return std::unexpected(res.stdErr);
    }
    co_return res.stdOut.toUtf8();
}


Crawler::WebResponse<QByteArray> Crawler::post(const QUrl &url, QMap<QString, QString> params) {

    QStringList args = {"-u", url.toString(), "-m", "post"};

    QString paramsList = "{";
    for (auto it = params.begin(); it != params.end(); ++it) {
        paramsList += "'" + it.key() + "': '" + it.value() + "', ";
    }
    paramsList += "}";
    args << "-a" << paramsList;

    if (hasLogin()) {
        args << "-e" << email << "-p" << password;
    }

    QFile crawl = loadRes("script/crawl.py");
    auto res = co_await runPythonScript(crawl, args);
    if (!res.success || res.exitCode != 0) {
        co_return std::unexpected(res.stdErr);
    }
    co_return res.stdOut.toUtf8();
}

Crawler::WebResponse<QByteArray> Crawler::login(const QString &email, const QString &password) {
    this->email = email;
    this->password = password;
    // go to the homepage for a test
    return get(QUrl("http://openjudge.cn/"));
}

Crawler::WebResponse<QUrl> Crawler::submit(OJSubmitForm form) {
    QUrl url = form.problemUrl.resolved(QUrl("/api/solution/submitv2/"));
    QByteArray encodedCode = form.code.toUtf8().toBase64();
    QMap<QString, QString> params = {
            {"contestId", form.contestId},
            {"problemNumber", form.problemNumber},
            {"sourceEncode", "base64"},
            {"language", form.checked},
            {"source", QString::fromUtf8(encodedCode)},
    };

    auto response = co_await post(url, params);
    // read the response as a json object
    if (!response.has_value()) {
        co_return std::unexpected(response.error());
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response.value(), &error);
    if (error.error != QJsonParseError::NoError) {
        co_return std::unexpected("JSON parse error: " + error.errorString());
    }
    QJsonObject obj = doc.object();
    if (obj["result"].toString() == "SUCCESS") {
        co_return obj["redirect"].toString();
    }
    QString message = obj["message"].toString();
    co_return std::unexpected(message);
}

Crawler::WebResponse<QByteArray> Crawler::personalize(OJPersonalizationForm form) {
    co_return std::unexpected("not implemented");
};
