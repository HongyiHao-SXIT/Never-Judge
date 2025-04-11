#include "icon.h"

#include <QPainter>
#include <QPushButton>
#include <QSvgRenderer>

#include "../util/file.h"

IconButton::IconButton(QWidget *parent) : QPushButton(parent) {}

void IconButton::setIconFromPath(const QString &iconPath, const int iconSize) {
    QIcon icon(iconPath);
    setIcon(icon);
}

void IconButton::setIconFromResName(const QString &iconName, const int iconSize) {
    QString iconPath = QString(":/res/icons/%1.svg").arg(iconName);
    setIconFromPath(iconPath, iconSize);
};


IconPushButton::IconPushButton(QWidget *parent) : IconButton(parent) { setup(); }

void IconPushButton::setup() {}

IconToggleButton::IconToggleButton(QWidget *parent) : IconButton(parent) { setup(); }

void IconToggleButton::setup() {
    setCheckable(true);
    setStyleSheet(R"(
        IconToggleButton {
            background-color: transparent;
            padding: 10px;
            border-radius: 5px;
        }
        IconToggleButton:hover {
            background-color: #4a4b4e;
        }
        IconToggleButton:checked {
            background-color: #3574F0;
        }
    )");
}
