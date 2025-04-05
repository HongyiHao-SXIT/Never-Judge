#include <QPainter>
#include <QSvgRenderer>
#include <QVBoxLayout>

#include "iconNav.h"

#include "icon.h"

#define BTN_SIZE 28
#define ICON_SIZE 20

IconNavigateWidget::IconNavigateWidget(QWidget *parent) : QWidget(parent) {
    layout = new QVBoxLayout(this);
    setup();
}

void IconNavigateWidget::setup() {
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);
    this->setFixedWidth(BTN_SIZE);
}

QPushButton *IconNavigateWidget::newIcon(const QString &iconName, const QString &tooltip) {
    auto *button = new IconToggleButton(this);

    button->setIconFromResName(iconName, ICON_SIZE);

    button->setFixedSize(BTN_SIZE, BTN_SIZE);
    button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    button->setToolTip(tooltip);

    layout->addWidget(button);
    buttons.append(button);
    return button;
}

LeftIconNavigateWidget::LeftIconNavigateWidget(QWidget *parent) : IconNavigateWidget(parent) { addIcons(); }

void LeftIconNavigateWidget::addIcons() {
    QPushButton *fileTreeBtn = newIcon("folder", tr("文件树"));
    fileTreeBtn->setChecked(true);
    connect(fileTreeBtn, &QPushButton::toggled, this, &LeftIconNavigateWidget::onToggleFileTree);
}

void LeftIconNavigateWidget::onToggleFileTree(bool checked) { emit toggleFileTree(checked); }

RightIconNavigateWidget::RightIconNavigateWidget(QWidget *parent) : IconNavigateWidget(parent) { addIcons(); }

void RightIconNavigateWidget::addIcons() {
    QPushButton *previewBtn = newIcon("search", tr("显示预览"));
    connect(previewBtn, &QPushButton::toggled, this, &RightIconNavigateWidget::onTogglePreview);
}

void RightIconNavigateWidget::onTogglePreview(bool checked) { emit togglePreview(checked); }
