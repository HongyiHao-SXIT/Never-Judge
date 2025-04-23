#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <QFileInfo>

enum class Language { C, CPP, C_MAKE_LISTS, PYTHON, UNKNOWN };

QString langName(Language lang);

Language stringToLang(QString lang);

class LangFileInfo : public QFileInfo {
public:
    LangFileInfo();

    static LangFileInfo empty();

    LangFileInfo(const QString& fileName);

    Language language() const;

    bool isValid() const;
};

#endif //LANGUAGE_H
