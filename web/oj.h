#ifndef OJ_H
#define OJ_H

#include <QList>
#include <QString>
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

struct OJSubmitResponse {
    enum Result { W, AC, WA, CE, RE, TLE, MLE, PE, UKE };
    Result result;
    QString message;
};

struct OJPersonalizationForm {
    enum Gender { Male, Female };
    QString nickname;
    QString name;
    QString description;
    Gender gender;
    QString birthday;
    QString city;
    QString school;
};

#endif // OJ_H
