#ifndef IDE_H
#define IDE_H
#include "project.h"


class IDE {
    Project project;

public:
    IDE();
    void setProject(const Project &project);
    Project& curProject();
};


#endif //IDE_H
