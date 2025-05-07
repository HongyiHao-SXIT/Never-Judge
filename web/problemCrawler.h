#ifndef PROBLEMCRAWLER_H
#define PROBLEMCRAWLER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <expected>
#include <qcoro/qcorotask.h>

#include "oj.h"


// Problem crawler class
class ProblemCrawler : public QObject {
    Q_OBJECT

private:
    // Singleton pattern
    explicit ProblemCrawler(QObject *parent = nullptr);
    static ProblemCrawler *instance;

public:
    // Get ProblemCrawler singleton instance
    static ProblemCrawler &getInstance();

    // Get problem details from URL
    QCoro::Task<std::expected<OJProblemDetail, QString>> getProblemDetail(const QUrl &url);

    // Get problem details from problem ID
    QCoro::Task<std::expected<OJProblemDetail, QString>> getProblemDetail(
        const QString &contestId,
        const QString &problemId
    );
};

#endif // PROBLEMCRAWLER_H
