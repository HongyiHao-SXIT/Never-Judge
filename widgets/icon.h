#ifndef ICON_H
#define ICON_H

#include <QPushButton>

class IconButton : public QPushButton {
    Q_OBJECT

public:
    explicit IconButton(QWidget *parent);
    void setIconFromPath(const QString &iconPath, int iconSize);
    /* default to find the res in .qrc */
    void setIconFromResName(const QString &iconName, int iconSize);
};

class IconPushButton : public IconButton {
    Q_OBJECT
    void setup();

public:
    explicit IconPushButton(QWidget *parent);
};

class IconToggleButton : public IconButton {
    Q_OBJECT
    void setup();

public:
    explicit IconToggleButton(QWidget *parent);
};


#endif // ICON_H
