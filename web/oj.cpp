#include "oj.h"

#include <QFile>
#include <qcoro/qcoronetworkreply.h>
#include <utility>

#include "../util/file.h"

Crawler::Crawler(QUrl url) : url(std::move(url)) {}

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

QCoro::Task<std::expected<OJProblem, QString>> OJParser::parseProblem(const QByteArray &html) {
    auto tempFile = TempFiles::create("problem", html);
    tempFile->close();

    QFile script = loadRes("script/parser.py");
    auto result = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    if (!result.success || result.exitCode != 0) {
        co_return std::unexpected(result.stdErr);
    }
    auto split = result.stdOut.indexOf('\n');

    QString title = result.stdOut.mid(0, split);
    QString content = result.stdOut.mid(split + 1);

    co_return OJProblem{title, content};
}

QCoro::Task<std::expected<QStringList, QString>> OJParser::parseProblemUrlsInMatch(const QByteArray &content) {
    auto tempFile = TempFiles::create("match", content);
    tempFile->close();

    QFile script = loadRes("script/match.py");
    auto result = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    if (!result.success || result.exitCode != 0) {
        co_return std::unexpected(result.stdErr);
    }
    co_return result.stdOut.split("\n", Qt::SkipEmptyParts);
};
