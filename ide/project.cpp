#include "project.h"

#include <utility>

Project::Project() = default;

Project::Project(QString root) : root(std::move(root)) {}

QString Project::getRoot() const { return root; }
