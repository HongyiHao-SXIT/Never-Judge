#include "ojPersonal.h"

#include "../web/crawl.h"
#include "../web/oj.h"
#include "../web/parse.h"

#include <QDate>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QUrl>
#include <QVBoxLayout>

PersonalSettingsDialog::PersonalSettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("OpenJudge 个人设置"));
    auto *form = new QFormLayout(this);

    nicknameEdit = new QLineEdit(this);
    QRegularExpression nickRx(QStringLiteral("^[\\p{Han}A-Za-z0-9]{1,15}$"));
    nicknameEdit->setValidator(new QRegularExpressionValidator(nickRx, this));

    nameEdit = new QLineEdit(this);
    descriptionEdit = new QTextEdit(this);

    genderCombo = new QComboBox(this);
    genderCombo->addItems({tr("男"), tr("女")});

    birthdayEdit = new QDateEdit(this);
    birthdayEdit->setDisplayFormat("yyyy-MM-dd");
    birthdayEdit->setCalendarPopup(true);

    cityEdit = new QLineEdit(this);
    schoolEdit = new QLineEdit(this);

    form->addRow(tr("昵称:"), nicknameEdit);
    form->addRow(tr("真实姓名:"), nameEdit);
    form->addRow(tr("个人简介:"), descriptionEdit);
    form->addRow(tr("性别:"), genderCombo);
    form->addRow(tr("生日:"), birthdayEdit);
    form->addRow(tr("城市:"), cityEdit);
    form->addRow(tr("学校:"), schoolEdit);

    auto *pullBtn = new QPushButton(tr("拉取"), this);
    auto *updateBtn = new QPushButton(tr("更新"), this);
    auto *cancelBtn = new QPushButton(tr("取消"), this);
    connect(pullBtn, &QPushButton::clicked, this, &PersonalSettingsDialog::loadExisting);
    connect(updateBtn, &QPushButton::clicked, this, &PersonalSettingsDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto *btnLay = new QHBoxLayout(this);
    btnLay->addStretch();
    btnLay->addWidget(pullBtn);
    btnLay->addWidget(updateBtn);
    btnLay->addWidget(cancelBtn);
    form->addRow(btnLay);

    setLayout(form);
}

QCoro::Task<> PersonalSettingsDialog::onSave() {
    if (nicknameEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("表单错误"), tr("昵称不能为空！"));
        co_return;
    }

    OJPersonalizationForm form;
    form.nickname = nicknameEdit->text();
    form.name = nameEdit->text();
    form.description = descriptionEdit->toPlainText();
    form.gender = genderCombo->currentIndex() == 0 ? OJPersonalizationForm::Male
                                                   : OJPersonalizationForm::Female;
    form.birthday = birthdayEdit->date().toString(Qt::ISODate);
    form.city = cityEdit->text();
    form.school = schoolEdit->text();

    auto res = co_await Crawler::instance().personalize(form);
    if (!res.has_value()) {
        QMessageBox::critical(this, tr("保存失败"), res.error());
        co_return;
    }

    QMessageBox::information(this, tr("保存成功"), tr("你的个人设置已成功保存。"));
    accept();
}

QCoro::Task<> PersonalSettingsDialog::loadExisting() {
    if (!Crawler::instance().hasLogin()) {
        QMessageBox::warning(this, tr("未登录"), tr("请先登录 OpenJudge！"));
        co_return;
    }

    auto response = co_await Crawler::instance().get(QUrl("http://openjudge.cn/settings/"));
    if (!response.has_value()) {
        QMessageBox::warning(this, tr("拉取失败"), response.error());
        co_return;
    }

    auto parsed = co_await OJParser::parsePersonalizationForm(response.value());
    if (!parsed.has_value()) {
        QMessageBox::warning(this, tr("拉取失败"), parsed.error());
        co_return;
    }

    auto &form = parsed.value();
    nicknameEdit->setText(form.nickname);
    nameEdit->setText(form.name);
    descriptionEdit->setPlainText(form.description);
    genderCombo->setCurrentIndex(form.gender == OJPersonalizationForm::Male ? 0 : 1);
    birthdayEdit->setDate(QDate::fromString(form.birthday, Qt::ISODate));
    cityEdit->setText(form.city);
    schoolEdit->setText(form.school);
}
