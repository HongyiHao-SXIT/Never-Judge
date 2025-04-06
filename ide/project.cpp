#include "project.h"

#include <utility>

Project::Project() = default;

Project::Project(QString root) : root(std::move(root)) {}

QString Project::getRoot() const { return root; }


FileInfo::FileInfo() = default;

FileInfo FileInfo::empty() { return {}; }

FileInfo::FileInfo(const QString& fileName) : QFileInfo(fileName) {}

Language FileInfo::getLanguage() const {
    if (suffix() == "c") {
        return Language::C;
    }
    if (suffix() == "cpp") {
        return Language::CPP;
    }
    if (suffix() == "py") {
        return Language::PYTHON;
    }
    return Language::UNKNOWN;
}

bool FileInfo::isValid() const { return !filePath().isEmpty(); }
