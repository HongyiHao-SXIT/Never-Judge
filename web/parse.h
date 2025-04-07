#ifndef PARSE_H
#define PARSE_H

#include "oj.h"
#include <expected>
#include <qcoro/qcoronetworkreply.h>
#include <qcorotask.h>

/** A wrapper parser for python script */
class OJParser {
public:
    static QCoro::Task<std::expected<OJProblem, QString>> parseProblem(const QByteArray &html);
    static QCoro::Task<std::expected<OJMatch, QString>> parseProblemUrlsInMatch(const QByteArray &content);
    static QCoro::Task<std::expected<OJSubmitForm, QString>> parseProblemSubmitForm(const QByteArray &content);
    static QCoro::Task<std::expected<OJSubmitResponse, QString>> parseProblemSubmitResponse(const QByteArray &content);
};


#endif // PARSE_H
