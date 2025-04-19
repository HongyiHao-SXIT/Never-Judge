#include "widgets/window.h"

#include <QApplication>

#ifndef NDEBUG
#include "util/file.h"
#endif

int main(int argc, char *argv[]) {
#ifndef NDEBUG
    // clear the temp file cache in case of a crash
    TempFiles::clearCache();
    Configs::clear();
#endif

    QApplication app(argc, argv);
    auto *window = new IDEMainWindow();

    // open the running folder
    QString project = QString::fromStdString(std::filesystem::current_path());
    window->openFolder(project);

    window->show();
    return QApplication::exec();
}
