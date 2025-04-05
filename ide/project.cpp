#include "project.h"

#include <utility>

Project::Project() = default;

Project::Project(QString root) : root(std::move(root)) {}

QString Project::getRoot() const { return root; }


FileInfo::FileInfo() = default;

FileInfo FileInfo::empty() { return {}; }

FileInfo::FileInfo(QString fileName) : QFileInfo(std::move(fileName)) {}

Language FileInfo::getLanguage() const {
    if (suffix() == "c") {
        return C;
    }
    if (suffix() == "cpp") {
        return CPP;
    }
    if (suffix() == "py") {
        return PYTHON;
    }
    return UNKNOWN;
}

bool FileInfo::isValid() const { return !filePath().isEmpty(); }
