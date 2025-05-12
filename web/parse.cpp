#include "parse.h"

#include "../util/file.h"
#include "../util/script.h"
#include "crawl.h"

// Initialize static instance pointer
OJParser* OJParser::instance = nullptr;

OJParser::OJParser(QObject *parent) : QObject(parent) {
}

OJParser &OJParser::getInstance() {
    if (!instance) {
        instance = new OJParser();
    }
    return *instance;
}

QCoro::Task<std::expected<OJProblem, QString>> OJParser::parseProblem(const QByteArray &html) {
    auto tempFile = TempFiles::create("problem", html);
    tempFile->close();

    QFile script = loadRes("script/parser.py");
    auto output = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    if (!output.success || output.exitCode != 0) {
        co_return std::unexpected(output.stdErr);
    }
    auto split = output.stdOut.indexOf('\n');

    QString title = output.stdOut.mid(0, split);
    QString content = output.stdOut.mid(split + 1);

    co_return OJProblem(title, content);
}

QCoro::Task<std::expected<OJMatch, QString>> OJParser::parseProblemUrlsInMatch(const QByteArray &content) {
    auto tempFile = TempFiles::create("match", content);
    tempFile->close();

    QFile script = loadRes("script/match.py");
    auto output = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    if (!output.success || output.exitCode != 0) {
        co_return std::unexpected(output.stdErr);
    }
    QList<QUrl> urls;
    for (auto &url: output.stdOut.split("\n", Qt::SkipEmptyParts)) {
        urls.append(url);
    }
    co_return OJMatch(urls);
};


QCoro::Task<std::expected<OJSubmitForm, QString>> OJParser::parseProblemSubmitForm(const QByteArray &content) {
    auto tempFile = TempFiles::create("submit", content);
    tempFile->close();

    QFile script = loadRes("script/submit.py");
    auto output = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    if (!output.success || output.exitCode != 0) {
        co_return std::unexpected(output.stdErr);
    }

    auto lines = output.stdOut.split("\n", Qt::SkipEmptyParts);
    const QString &contestId = lines[0];
    const QString &problemNumber = lines[1];
    QList<OJLanguage> languages;
    for (int i = 2; i < lines.size(); i++) {
        auto &line = lines[i];
        auto idx = line.indexOf(" ");
        languages.emplace_back(line.mid(0, idx), line.mid(idx + 1));
    }
    qDebug() << languages[0].name << languages[0].formValue;
    co_return OJSubmitForm(contestId, problemNumber, languages);
}

QCoro::Task<std::expected<OJSubmitResponse, QString>> OJParser::parseProblemSubmitResponse(const QByteArray &content) {
    auto tempFile = TempFiles::create("submit_result", content);
    tempFile->close();

    QFile script = loadRes("script/submit_response.py");
    auto output = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    if (!output.success || output.exitCode != 0) {
        co_return std::unexpected(output.stdErr);
    }

    static QMap<QString, OJSubmitResult> map = {{"Waiting", W},
                                                {"Accepted", AC},
                                                {"Wrong Answer", WA},
                                                {"Compile Error", CE},
                                                {"Runtime Error", RE},
                                                {"Time Limit Exceeded", TLE},
                                                {"Memory Limit Exceeded", MLE},
                                                {"Presentation Error", PE}};

    QString outStr = output.stdOut;
    auto index = outStr.indexOf('\n');
    QString resStr = outStr.mid(0, index);
    auto result = map.contains(resStr) ? map[resStr] : UKE;
    auto message = outStr.count('\n') > 1 ? outStr.mid(index + 1) : "";
    co_return OJSubmitResponse(result, message);
}

QCoro::Task<std::expected<OJProblemDetail, QString>> OJParser::getProblemDetail(const QUrl &url) {
    // Use existing crawler to get page content
    auto response = co_await Crawler::instance().get(url);

    if (!response.has_value()) {
        co_return std::unexpected(response.error());
    }

    // Create temporary file to store page content
    auto tempFile = TempFiles::create("problem_detail", response.value());
    tempFile->close();

    // Use Python script to parse page content
    QFile script = loadRes("script/problem_detail.py");
    auto output = co_await runPythonScript(script, QStringList() << tempFile->fileName());

    if (!output.success || output.exitCode != 0) {
        co_return std::unexpected(output.stdErr);
    }

    // Parse script output
    // Output format: title|description|inputDesc|outputDesc|sampleInput|sampleOutput|hint
    QStringList parts = output.stdOut.split("|", Qt::SkipEmptyParts);

    if (parts.size() < 6) {
        co_return std::unexpected("Failed to parse problem details: incorrect output format");
    }

    OJProblemDetail detail;
    detail.title = parts[0];
    detail.description = parts[1];
    detail.inputDesc = parts[2];
    detail.outputDesc = parts[3];
    detail.sampleInput = parts[4];
    detail.sampleOutput = parts[5];

    if (parts.size() > 6) {
        detail.hint = parts[6];
    }

    detail.sourceUrl = url.toString();

    co_return detail;
}

QCoro::Task<std::expected<OJProblemDetail, QString>> OJParser::getProblemDetail(
    const QString &contestId,
    const QString &problemId
) {
    // Construct problem URL
    QUrl url(QString("http://cxsjsx.openjudge.cn/%1/problem/%2/").arg(contestId, problemId));

    // Call URL version of the method
    co_return co_await getProblemDetail(url);
}
