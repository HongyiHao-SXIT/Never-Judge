#ifndef PROBLEMCRAWLER_H
#define PROBLEMCRAWLER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <expected>
#include <qcoro/qcorotask.h>

// Problem detail structure
struct ProblemDetail {
    QString title;           // Problem title
    QString description;     // Problem description
    QString inputDesc;       // Input description
    QString outputDesc;      // Output description
    QString sampleInput;     // Sample input
    QString sampleOutput;    // Sample output
    QString hint;            // Hint (if any)
    QString sourceUrl;       // Source URL

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
    QCoro::Task<std::expected<ProblemDetail, QString>> getProblemDetail(const QUrl &url);

    // Get problem details from problem ID
    QCoro::Task<std::expected<ProblemDetail, QString>> getProblemDetail(
        const QString &contestId,
        const QString &problemId
    );
};

#endif // PROBLEMCRAWLER_H
