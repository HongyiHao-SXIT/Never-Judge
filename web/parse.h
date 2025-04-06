#ifndef PARSE_H
#define PARSE_H

#include "crawl.h"

struct OJProblem {
    QString title;
    QString content;
};

class OJParser {
public:
    static QCoro::Task<std::expected<OJProblem, QString>> parseProblem(const QByteArray &html);
    static QCoro::Task<std::expected<QStringList, QString>> parseProblemUrlsInMatch(const QByteArray &content);
    static QCoro::Task<std::expected<OJSubmitForm, QString>> parseProblemSubmitForm(const QByteArray &content);
};


#endif // PARSE_H
