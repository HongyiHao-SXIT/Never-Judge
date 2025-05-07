#ifndef PROBLEMCRAWLER_H
#define PROBLEMCRAWLER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <expected>
#include <qcoro/qcorotask.h>

/**
 * @brief 题目详情结构体
 */
struct ProblemDetail {
    QString title;           // 题目标题
    QString description;     // 题目描述
    QString inputDesc;       // 输入描述
    QString outputDesc;      // 输出描述
    QString sampleInput;     // 样例输入
    QString sampleOutput;    // 样例输出
    QString hint;            // 提示信息（如果有）
    QString sourceUrl;       // 题目来源URL
    
    ProblemDetail() = default;
    
    ProblemDetail(
        const QString &title,
        const QString &description,
        const QString &inputDesc,
        const QString &outputDesc,
        const QString &sampleInput,
        const QString &sampleOutput,
        const QString &hint = QString(),
        const QString &sourceUrl = QString()
    ) : title(title), description(description), inputDesc(inputDesc),
        outputDesc(outputDesc), sampleInput(sampleInput), sampleOutput(sampleOutput),
        hint(hint), sourceUrl(sourceUrl) {}
};

/**
 * @brief 题目爬虫类
 * 
 * 负责从 OpenJudge 获取题目详情
 */
class ProblemCrawler : public QObject {
    Q_OBJECT
    
private:
    // 单例模式
    explicit ProblemCrawler(QObject *parent = nullptr);
    static ProblemCrawler *instance;
    
public:
    /**
     * @brief 获取 ProblemCrawler 单例
     * @return ProblemCrawler 实例
     */
    static ProblemCrawler &getInstance();
    
    /**
     * @brief 从 URL 获取题目详情
     * @param url 题目 URL
     * @return 成功返回题目详情，失败返回错误信息
     */
    QCoro::Task<std::expected<ProblemDetail, QString>> getProblemDetail(const QUrl &url);
    
    /**
     * @brief 从题目 ID 获取题目详情
     * @param contestId 比赛 ID
     * @param problemId 题目 ID
     * @return 成功返回题目详情，失败返回错误信息
     */
    QCoro::Task<std::expected<ProblemDetail, QString>> getProblemDetail(
        const QString &contestId,
        const QString &problemId
    );
};

#endif // PROBLEMCRAWLER_H
