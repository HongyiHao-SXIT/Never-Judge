#ifndef PROJECT_H
#define PROJECT_H
#include <QFileInfo>

class Project {
    QString root;

public:
    Project();

    explicit Project(QString root);

    QString getRoot() const;
};

enum class Language { C, CPP, CMAKE, PYTHON, UNKNOWN };

class FileInfo : public QFileInfo {
public:
    FileInfo();

    static FileInfo empty();

    FileInfo(const QString& fileName);

    Language getLanguage() const;

    bool isValid() const;
};


#endif // PROJECT_H
