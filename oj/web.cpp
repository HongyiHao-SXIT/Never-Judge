#include "web.h"

#include <QFile>
#include <qcoro/qcoronetworkreply.h>

#include "../util/file.h"

Crawler::Crawler(const QString &url) : url(url) {}

QCoro::Task<std::expected<QByteArray, QString>> Crawler::crawl() const {
    QNetworkAccessManager nam;
    QNetworkRequest request(url);
    request.setTransferTimeout(5000);
    QNetworkReply *reply = co_await nam.get(request);
    if (reply->error()) {
        co_return std::unexpected(reply->errorString());
    }
    if (reply->bytesAvailable() == 0) {
        co_return std::unexpected("No data received");
    }
    QByteArray data = reply->readAll();
    co_return data;
}

QCoro::Task<std::expected<QString, QString>> OJParser::parse(const QByteArray &content) const {
    auto tempFile = TempFiles::create("problem", content);
    tempFile->close();

    QFile script = loadRes("script/parser.py");
    auto result = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    if (!result.success || result.exitCode != 0) {
        co_return std::unexpected(result.stdErr);
    }
    co_return result.stdOut;
}
