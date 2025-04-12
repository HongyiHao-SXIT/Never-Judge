#include "icon.h"

#include <QPushButton>
#include <QSvgRenderer>

IconButton::IconButton(QWidget *parent) : QPushButton(parent) {}

void IconButton::setIconFromPath(const QString &iconPath) {
    QIcon icon(iconPath);
    setIcon(icon);
}

void IconButton::setIconFromResName(const QString &iconName) {
    QString iconPath = QString(":/res/icons/%1.svg").arg(iconName);
    setIconFromPath(iconPath);
};


IconPushButton::IconPushButton(QWidget *parent) : IconButton(parent) { setup(); }

void IconPushButton::setup() {
    setStyleSheet(R"(
        IconPushButton {
            background-color: transparent;
            padding: 10px;
            border-radius: 5px;
        }
        IconPushButton:hover {
            background-color: #4a4b4e;
        }
    )");
}

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
