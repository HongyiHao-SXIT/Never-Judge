#ifndef ICON_NAV_H
#define ICON_NAV_H

#include <QPushButton>
#include <QWidget>

class IconNavigateWidget : public QWidget {
    Q_OBJECT

    QLayout *layout;
    QList<QPushButton *> buttons;

protected:
    void setup();
    QPushButton *newIcon(const QString &iconPath, const QString &tooltip);

public:
    explicit IconNavigateWidget(QWidget *parent = nullptr);
};

class LeftIconNavigateWidget : public IconNavigateWidget {
    Q_OBJECT

    void addIcons();

private slots:
    void onToggleFileTree(bool checked);

public:
    explicit LeftIconNavigateWidget(QWidget *parent = nullptr);

signals:
    void toggleFileTree(bool show);
};

class RightIconNavigateWidget : public IconNavigateWidget {
    Q_OBJECT

    void addIcons();

private slots:
    void onTogglePreview(bool checked);

public:
    explicit RightIconNavigateWidget(QWidget *parent = nullptr);

signals:
    void togglePreview(bool show);
};

#endif // ICON_NAV_H
