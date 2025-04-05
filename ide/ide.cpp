#include "ide.h"

IDE::IDE() = default;

void IDE::setProject(const Project &project) {
    this->project = project;
}


Project &IDE::curProject() { return project; }

