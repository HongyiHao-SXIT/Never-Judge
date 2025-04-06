#include "parse.h"
#include <QFile>

#include "../util/file.h"
#include "../util/script.h"

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


QCoro::Task<std::expected<OJSubmitForm, QString>> OJParser::parseProblemSubmitForm(const QByteArray &content) {
    auto tempFile = TempFiles::create("submit", content);
    tempFile->close();

    QFile script = loadRes("script/submit.py");
    qDebug() << tempFile->fileName();
    auto result = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    if (!result.success || result.exitCode != 0) {
        co_return std::unexpected(result.stdErr);
    }

    QStringList lines = result.stdOut.split("\n", Qt::SkipEmptyParts);
    QString contestId = lines[0];
    QString problemNumber = lines[1];
    QList<OJLanguage> languages;
    for (int i = 2; i < lines.size(); ++i) {
        QStringList lang = lines[i].split(" ");
        if (lang.size() == 2) {
            languages.emplace_back(lang[0], lang[1]);
        }
    }

    OJSubmitForm res = {contestId, problemNumber, languages};
    co_return res;
};
