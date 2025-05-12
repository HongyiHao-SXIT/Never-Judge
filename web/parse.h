#ifndef PARSE_H
#define PARSE_H

#include "oj.h"
#include <QObject>
#include <QString>
#include <QUrl>
#include <expected>
#include <qcoro/qcoronetworkreply.h>
#include <qcorotask.h>

/** A wrapper parser for python scripts */
class OJParser : public QObject {
    Q_OBJECT

private:
    // Singleton pattern
    explicit OJParser(QObject *parent = nullptr);
    static OJParser *instance;

public:
    // Get OJParser singleton instance
    static OJParser &getInstance();

    // Static parsing methods
    static QCoro::Task<std::expected<OJProblem, QString>> parseProblem(const QByteArray &html);
    static QCoro::Task<std::expected<OJMatch, QString>> parseProblemUrlsInMatch(const QByteArray &content);
    static QCoro::Task<std::expected<OJSubmitForm, QString>> parseProblemSubmitForm(const QByteArray &content);
    static QCoro::Task<std::expected<OJSubmitResponse, QString>> parseProblemSubmitResponse(const QByteArray &content);

    // Get problem details from URL
    QCoro::Task<std::expected<OJProblemDetail, QString>> getProblemDetail(const QUrl &url);

    // Get problem details from problem ID
    QCoro::Task<std::expected<OJProblemDetail, QString>> getProblemDetail(
        const QString &contestId,
        const QString &problemId
    );
};

#endif // PARSE_H
