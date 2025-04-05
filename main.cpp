#include "widgets/window.h"

#include <QApplication>
#include "util/file.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QIcon::setThemeName(QStringLiteral("oxygen"));
    auto *window = new IDEMainWindow(argc, argv);

#ifndef NDEBUG
    TempFiles::clearCache();
#endif

    window->show();
    return QApplication::exec();
}
