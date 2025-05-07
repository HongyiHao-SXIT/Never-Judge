#ifndef OJ_H
#define OJ_H

#include <QList>
#include <QUrl>

struct OJProblem {
    QString title;
    QString content;
};

struct OJProblemDetail {
    QString title;
    QString description;
    QString inputDesc;
    QString outputDesc;
    QString sampleInput;
    QString sampleOutput;
    QString hint;
    QString sourceUrl;
};

struct OJMatch {
    QList<QUrl> problemUrls;
};

struct OJLanguage {
    QString formValue;
    QString name;
};

struct OJSubmitForm {
    QString contestId;
    QString problemNumber;
    QList<OJLanguage> languages;
    QString code;
    QString checked;
    QUrl problemUrl;
};

enum OJSubmitResult { W, AC, WA, CE, RE, TLE, MLE, PE, UKE };

struct OJSubmitResponse {
    OJSubmitResult result;
    QString message;
};

#endif // OJ_H
