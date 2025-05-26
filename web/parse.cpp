#include "parse.h"

#include "../util/file.h"
#include "../util/script.h"
#include "crawl.h"

OJParser::ParseResult<OJProblem> OJParser::parseProblem(const QByteArray &html) {
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

OJParser::ParseResult<OJMatch> OJParser::parseMatch(const QByteArray &html) {
    auto tempFile = TempFiles::create("match", html);
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


OJParser::ParseResult<OJSubmitForm> OJParser::parseProblemSubmitForm(const QByteArray &html) {
    auto tempFile = TempFiles::create("submit", html);
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

OJParser::ParseResult<OJSubmitResponse>
OJParser::parseProblemSubmitResponse(const QByteArray &html) {
    auto tempFile = TempFiles::create("submit_result", html);
    tempFile->close();

    QFile script = loadRes("script/submit_response.py");
    auto output = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    if (!output.success || output.exitCode != 0) {
        co_return std::unexpected(output.stdErr);
    }

    static QMap<QString, OJSubmitResponse::Result> map = {
            {"Waiting", OJSubmitResponse::W},
            {"Accepted", OJSubmitResponse::AC},
            {"Wrong Answer", OJSubmitResponse::WA},
            {"Compile Error", OJSubmitResponse::CE},
            {"Runtime Error", OJSubmitResponse::RE},
            {"Time Limit Exceeded", OJSubmitResponse::TLE},
            {"Memory Limit Exceeded", OJSubmitResponse::MLE},
            {"Presentation Error", OJSubmitResponse::PE}};

    QString outStr = output.stdOut;
    auto index = outStr.indexOf('\n');
    QString resStr = outStr.mid(0, index);
    auto result = map.contains(resStr) ? map[resStr] : OJSubmitResponse::UKE;
    auto message = outStr.count('\n') > 1 ? outStr.mid(index + 1) : "";
    co_return OJSubmitResponse(result, message);
}

OJParser::ParseResult<OJProblemDetail> OJParser::parseProblemDetail(const QByteArray &html) {
    // Create a temporary file to store page content
    auto tempFile = TempFiles::create("problem_detail", html);
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
    co_return detail;
}

OJParser::ParseResult<OJPersonalizationForm>
OJParser::parsePersonalizationForm(const QByteArray &html) {

    auto tempFile = TempFiles::create("personalization", html);
    tempFile->close();

    QFile script(QStringLiteral("../res/script/personalization.py"));
    if (!script.open(QIODevice::ReadOnly)) {
        co_return std::unexpected(QStringLiteral("Cannot open personalization.py at %1")
                                  .arg(script.fileName()));
    }

    auto output = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    if (!output.success || output.exitCode != 0) {
        co_return std::unexpected(output.stdErr);
    }

    const auto lines = output.stdOut.split('\n', Qt::SkipEmptyParts);
    if (lines.size() < 7) {
        co_return std::unexpected("Failed to parse personalization form: incorrect output format");
    }

    OJPersonalizationForm form;
    form.nickname    = lines[0].trimmed();
    form.name        = lines[1].trimmed();
    form.description = lines[2].trimmed();
    form.gender      = (lines[3].trimmed().toLower() == "female"
                          ? OJPersonalizationForm::Female
                          : OJPersonalizationForm::Male);
    form.birthday    = lines[4].trimmed();
    form.city        = lines[5].trimmed();
    form.school      = lines[6].trimmed();

    co_return form;
}