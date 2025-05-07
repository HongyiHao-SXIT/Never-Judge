#include "problemCrawler.h"
#include "crawl.h"
#include "../util/file.h"
#include "../util/script.h"

// Initialize static instance pointer
ProblemCrawler* ProblemCrawler::instance = nullptr;

ProblemCrawler::ProblemCrawler(QObject *parent) : QObject(parent) {
}

ProblemCrawler &ProblemCrawler::getInstance() {
    if (!instance) {
        instance = new ProblemCrawler();
    }
    return *instance;
}

QCoro::Task<std::expected<ProblemDetail, QString>> ProblemCrawler::getProblemDetail(const QUrl &url) {
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

    ProblemDetail detail;
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

QCoro::Task<std::expected<ProblemDetail, QString>> ProblemCrawler::getProblemDetail(
    const QString &contestId,
    const QString &problemId
) {
    // Construct problem URL
    QUrl url(QString("http://cxsjsx.openjudge.cn/%1/problem/%2/").arg(contestId, problemId));

    // Call URL version of the method
    co_return co_await getProblemDetail(url);
}
