#include <QPainter>
#include <QSvgRenderer>

#include "iconNav.h"

#include "icon.h"

#define NAV_BTN_SIZE 28
#define NAV_ICON_SIZE 20

IconNavigateWidget::IconNavigateWidget(QWidget *parent) : QFrame(parent) {
    layout = new QVBoxLayout(this);
    setup();
}

void IconNavigateWidget::setup() {
    setFrameShape(VLine);
    setFrameShadow(Plain);
    setStyleSheet("QFrame {"
                  "border-top: 1px solid palette(mid);"
                  "border-bottom: 1px solid palette(mid);"
                  "border-left: none;"
                  "border-right: none;"
                  "}");

    layout->setContentsMargins(5, 10, 5, 10);
    layout->setSpacing(10);
    layout->setAlignment(Qt::AlignTop);

    setLayout(layout);
    setFixedWidth(NAV_BTN_SIZE + 10);
}

QPushButton *IconNavigateWidget::newIcon(const QString &iconName, const QString &tooltip,
                                         bool toggle) {
    IconButton *button;
    if (toggle) {
        button = new IconToggleButton(this);
    } else {
        button = new IconPushButton(this);
    }

    button->setIconFromResName(iconName);

    button->setFixedSize(NAV_BTN_SIZE, NAV_BTN_SIZE);
    button->setIconSize(QSize(NAV_ICON_SIZE, NAV_ICON_SIZE));
    button->setToolTip(tooltip);

    layout->addWidget(button);
    buttons.append(button);
    return button;
}

LeftIconNavigateWidget::LeftIconNavigateWidget(QWidget *parent) : IconNavigateWidget(parent) {
    addIcons();
}

void LeftIconNavigateWidget::addIcons() {
    auto *fileTreeBtn = newIcon("folder", tr("文件树"));
    fileTreeBtn->setChecked(true);
    connect(fileTreeBtn, &QPushButton::toggled, this, &LeftIconNavigateWidget::onToggleFileTree);

    layout->addStretch();

    auto *terminalButton = newIcon("terminal", tr("集成终端"));
    connect(terminalButton, &QPushButton::toggled, this, &LeftIconNavigateWidget::onToggleTerminal);
}

void LeftIconNavigateWidget::onToggleFileTree(const bool checked) { emit toggleFileTree(checked); }

void LeftIconNavigateWidget::onToggleTerminal(const bool checked) { emit toggleTerminal(checked); }

RightIconNavigateWidget::RightIconNavigateWidget(QWidget *parent) : IconNavigateWidget(parent) {
    addIcons();
}

void RightIconNavigateWidget::addIcons() {
    auto *loginBtn = newIcon("user", tr("登录到 OpenJudge"), false);
    connect(loginBtn, &QPushButton::clicked, this, &RightIconNavigateWidget::onLoginOJ);
    auto *previewBtn = newIcon("search", tr("显示题目预览"));
    connect(previewBtn, &QPushButton::toggled, this, &RightIconNavigateWidget::onTogglePreview);
    
    auto *aiAssistantBtn = newIcon("robot", tr("AI 刷题助手"));
    connect(aiAssistantBtn, &QPushButton::toggled, this, &RightIconNavigateWidget::onToggleAIAssistant);
    
    auto *settingBtn = newIcon("gear", tr("设置"), false);
    connect(settingBtn, &QPushButton::clicked, this, &RightIconNavigateWidget::onOpenSetting);
}

void RightIconNavigateWidget::onLoginOJ() { emit loginOJ(); }

void RightIconNavigateWidget::onTogglePreview(const bool checked) { emit togglePreview(checked); }

void RightIconNavigateWidget::onOpenSetting() { emit openSetting(); }

void RightIconNavigateWidget::onToggleAIAssistant(const bool checked) { emit toggleAIAssistant(checked); }
