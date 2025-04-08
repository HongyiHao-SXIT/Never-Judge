#include "setting.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <qfontdatabase.h>
#include <qtermwidget.h>

#include "../util/file.h"

template<class Widget, class Variant>
void bindConfig(Widget *widget, const QString &key, void (Widget::*setterSlot)(const Variant &),
                void (Widget::*changedSignal)(const Variant &)) {
    QVariant storedValue = Configs::instance().get(key);
    Variant value = storedValue.value<Variant>();
    (widget->*setterSlot)(value);
    QObject::connect(widget, changedSignal, widget,
                     [key](const Variant &newValue) { Configs::instance().set(key, newValue); });
}


class GeneralPage : public QWidget {
    Q_OBJECT

public:
    explicit GeneralPage(QWidget *parent = nullptr) : QWidget(parent) {
        auto layout = new QVBoxLayout(this);
        setLayout(layout);
    }
};

class AppearancePage : public QWidget {

#define FONT_KEY "codeFont"

    Q_OBJECT

    QComboBox *fontCombo;
    QSpinBox *fontSizeSpin;

private slots:
    void setFont(const QJsonValue &value) {
        QJsonObject obj = value.toObject();
        fontCombo->setCurrentText(obj.value("family").toString());
        fontSizeSpin->setValue(obj.value("size").toInt());
    };

    void onFontFamilyChanged(const QString &family) {
        QJsonObject obj = Configs::instance().get(FONT_KEY).toObject();
        obj["family"] = family;
        emit fontChanged(obj);
    };
    void onFontSizeChanged(int size) {
        QJsonObject obj = Configs::instance().get(FONT_KEY).toObject();
        obj["size"] = size;
        emit fontChanged(obj);
    };
signals:
    void fontChanged(const QJsonValue &value);

public:
    explicit AppearancePage(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);

        auto themeGroup = new QGroupBox(tr("主题"), this);
        auto themeLayout = new QVBoxLayout(themeGroup);
        auto themeCombo = new QComboBox(themeGroup);
        themeCombo->addItems({tr("浅色"), tr("深色"), tr("系统默认")});
        auto terminalThemeCombo = new QComboBox(themeGroup);
        terminalThemeCombo->addItems(QTermWidget::availableColorSchemes());
        bindConfig(terminalThemeCombo,"terminalTheme",&QComboBox::setCurrentText, &QComboBox::currentTextChanged);

        themeLayout->addWidget(new QLabel(tr("选择主题:"), themeGroup));
        themeLayout->addWidget(themeCombo);
        themeLayout->addWidget(new QLabel(tr("终端主题"), themeGroup));
        themeLayout->addWidget(terminalThemeCombo);
        themeGroup->setLayout(themeLayout);

        QGroupBox *fontGroup = new QGroupBox(tr("字体"), this);
        QVBoxLayout *fontLayout = new QVBoxLayout(fontGroup);

        fontLayout->addWidget(new QLabel(tr("字体:"), fontGroup));
        fontCombo = new QComboBox(fontGroup);
        fontCombo->addItems(QFontDatabase::families());
        connect(fontCombo, &QComboBox::currentTextChanged, this, &AppearancePage::onFontFamilyChanged);
        fontLayout->addWidget(fontCombo);

        fontLayout->addWidget(new QLabel(tr("大小:")));
        fontSizeSpin = new QSpinBox(fontGroup);
        fontSizeSpin->setRange(8, 24);
        connect(fontSizeSpin, &QSpinBox::valueChanged, this, &AppearancePage::onFontSizeChanged);
        fontLayout->addWidget(fontSizeSpin);
        fontGroup->setLayout(fontLayout);
        bindConfig(this, FONT_KEY, &AppearancePage::setFont, &AppearancePage::fontChanged);


        layout->addWidget(themeGroup);
        layout->addWidget(fontGroup);
        layout->addStretch();
    }
};

class AdvancedPage : public QWidget {
    Q_OBJECT

public:
    AdvancedPage(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);

        QGroupBox *debugGroup = new QGroupBox(tr("调试选项"));
        QVBoxLayout *debugLayout = new QVBoxLayout();
        QCheckBox *enableLogCheck = new QCheckBox(tr("启用日志记录"));
        QCheckBox *verboseLogCheck = new QCheckBox(tr("详细日志"));
        debugLayout->addWidget(enableLogCheck);
        debugLayout->addWidget(verboseLogCheck);
        debugGroup->setLayout(debugLayout);

        QGroupBox *resetGroup = new QGroupBox(tr("重置选项"));
        QVBoxLayout *resetLayout = new QVBoxLayout();
        QPushButton *resetSettingsBtn = new QPushButton(tr("恢复默认设置"));
        resetLayout->addWidget(resetSettingsBtn);
        resetGroup->setLayout(resetLayout);

        layout->addWidget(debugGroup);
        layout->addWidget(resetGroup);
        layout->addStretch();
    }
};

class AboutPage : public QWidget {
    Q_OBJECT

public:
    AboutPage(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);

        QLabel *titleLabel = new QLabel(tr("<h2>NeverJudge</h2>"), this);
        QLabel *authorLabel = new QLabel(tr("作者: LeoDreamer"), this);
        QLabel *githubLabel = new QLabel(this);
        githubLabel->setText(tr("项目地址: <a href='https://github.com/LeoDreamer2004/Never-Judge'>GitHub</a>"));
        githubLabel->setTextFormat(Qt::RichText);
        githubLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        githubLabel->setOpenExternalLinks(true);

        QFrame *separator = new QFrame(this);
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);

        QHBoxLayout *qtInfoLayout = new QHBoxLayout(this);
        QLabel *qtInfoLabel = new QLabel(tr("此应用程序使用 Qt 构建。"), this);
        QPushButton *qtInfoButton = new QPushButton(tr("关于 Qt"), this);
        qtInfoLabel->setTextFormat(Qt::RichText);
        qtInfoLayout->addWidget(qtInfoLabel);
        qtInfoLayout->addWidget(qtInfoButton);

        layout->addWidget(titleLabel);
        layout->addWidget(authorLabel);
        layout->addWidget(githubLabel);
        layout->addWidget(separator);
        layout->addLayout(qtInfoLayout);
        layout->addStretch();

        connect(qtInfoButton, &QPushButton::clicked, parent, [this] { QMessageBox::aboutQt(this); });
    }
};

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("设置"));
    setMinimumSize(600, 400);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    createNavigationList();
    mainLayout->addWidget(navList, 1);
    createPages();
    mainLayout->addWidget(stackedWidget, 3);

    connect(navList, &QListWidget::currentRowChanged, stackedWidget, &QStackedWidget::setCurrentIndex);
    navList->setCurrentRow(0);
}

void SettingsDialog::createNavigationList() {
    navList = new QListWidget(this);
    navList->setMaximumWidth(150);
    navList->setSpacing(2);

    QListWidgetItem *generalItem = new QListWidgetItem(tr("常规设置"), navList);

    QListWidgetItem *appearanceItem = new QListWidgetItem(tr("外观"), navList);
    appearanceItem->setIcon(loadIcon("icons/palette.svg"));

    QListWidgetItem *advancedItem = new QListWidgetItem(tr("高级"), navList);
    advancedItem->setIcon(loadIcon("icons/setting.svg"));

    QListWidgetItem *aboutItem = new QListWidgetItem(tr("关于"), navList);
    aboutItem->setIcon(loadIcon("icons/info.svg"));
}

void SettingsDialog::createPages() {
    stackedWidget = new QStackedWidget(this);

    stackedWidget->addWidget(new GeneralPage(this));
    stackedWidget->addWidget(new AppearancePage(this));
    stackedWidget->addWidget(new AdvancedPage(this));
    stackedWidget->addWidget(new AboutPage(this));
}

#include "setting.moc"
