#include "problemCrawler.h"
#include "crawl.h"
#include "../util/file.h"
#include "../util/script.h"

// 初始化静态实例指针
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
    // 使用现有的爬虫获取页面内容
    auto response = co_await Crawler::instance().get(url);
    
    if (!response.has_value()) {
        co_return std::unexpected(response.error());
    }
    
    // 创建临时文件存储页面内容
    auto tempFile = TempFiles::create("problem_detail", response.value());
    tempFile->close();
    
    // 使用 Python 脚本解析页面内容
    QFile script = loadRes("script/problem_detail.py");
    auto output = co_await runPythonScript(script, QStringList() << tempFile->fileName());
    
    if (!output.success || output.exitCode != 0) {
        co_return std::unexpected(output.stdErr);
    }
    
    // 解析脚本输出
    // 输出格式：标题|描述|输入描述|输出描述|样例输入|样例输出|提示
    QStringList parts = output.stdOut.split("|", Qt::SkipEmptyParts);
    
    if (parts.size() < 6) {
        co_return std::unexpected("解析题目详情失败：输出格式不正确");
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
    // 构造题目 URL
    QUrl url(QString("http://cxsjsx.openjudge.cn/%1/problem/%2/").arg(contestId, problemId));
    
    // 调用 URL 版本的方法
    co_return co_await getProblemDetail(url);
}
