#ifndef PARSE_H
#define PARSE_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <expected>
#include <qcoro/qcoronetworkreply.h>
#include <qcorotask.h>
#include "oj.h"

/** A wrapper parser for python scripts */
class OJParser : public QObject {
    Q_OBJECT

    // Singleton pattern
    explicit OJParser(QObject *parent = nullptr);
    static OJParser *instance;

public:
    // Get OJParser singleton instance
    static OJParser &getInstance();

    // When succeeded, return the expected object, else return an error message
    template<class T>
    using ParseResult = QCoro::Task<std::expected<T, QString>>;

    // Static parsing methods
    static ParseResult<OJProblem> parseProblem(const QByteArray &html);

    static ParseResult<OJMatch> parseMatch(const QByteArray &html);

    static ParseResult<OJSubmitForm> parseProblemSubmitForm(const QByteArray &html);

    static ParseResult<OJSubmitResponse> parseProblemSubmitResponse(const QByteArray &html);

    static ParseResult<OJProblemDetail> parseProblemDetail(const QByteArray &html);

    static ParseResult<OJPersonalizationForm> parsePersonalizationForm(const QByteArray &html);
};

#endif // PARSE_H
