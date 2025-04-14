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

#endif // PROJECT_H
