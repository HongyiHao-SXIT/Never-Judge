#include "parse.h"
#include <QFile>

#include "../util/file.h"
#include "../util/script.h"

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
    QString contestId = lines[0];
    QString problemNumber = lines[1];
    QList<OJLanguage> languages;
    for (int i = 2; i < lines.size(); ++i) {
        QStringList lang = lines[i].split(" ");
        if (lang.size() == 2) {
            languages.emplace_back(lang[0], lang[1]);
        }
    }

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
