#include "widgets/window.h"

#include <QApplication>
#include "util/file.h"

int main(int argc, char *argv[]) {
#ifndef NDEBUG
    // clear the temp file cache in case of a crash
    TempFiles::clearCache();
    Configs::clear();
#endif

    QApplication app(argc, argv);
    auto *window = new IDEMainWindow(argc, argv);
    window->show();
    return QApplication::exec();
}
