#include "icon.h"

#include <QPainter>
#include <QPushButton>
#include <QSvgRenderer>

IconButton::IconButton(QWidget *parent) : QPushButton(parent) {}

QIcon IconButton::renderIcon(const QString &iconPath, const int iconSize) {
    QSvgRenderer renderer(iconPath);
    QPixmap map(QSize(iconSize, iconSize));
    map.fill(Qt::transparent);
    QPainter painter(&map);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    renderer.render(&painter);
    QImage image = map.toImage();
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);
            if (color.alpha() > 0) {
                color.setRgb(255, 255, 255, color.alpha());
                image.setPixelColor(x, y, color);
            }
        }
    }
    QPixmap whiteMap = QPixmap::fromImage(image);
    return {whiteMap};
}

void IconButton::setIconFromPath(const QString &iconPath, const int iconSize) { setIcon(renderIcon(iconPath, iconSize)); }

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
