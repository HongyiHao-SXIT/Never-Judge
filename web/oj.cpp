#include "oj.h"
#include "crawl.h"
#include "parse.h"

#include <QFormLayout>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDate>
#include <QUrl>

PersonalSettingsDialog::PersonalSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("OpenJudge – Personal Settings"));

    nicknameEdit = new QLineEdit;
    QRegularExpression nickRx(QStringLiteral("^[\\p{Han}A-Za-z0-9]{1,15}$"));
    nicknameEdit->setValidator(new QRegularExpressionValidator(nickRx, this));

    nameEdit      = new QLineEdit;
    descriptionEd = new QTextEdit;
    genderCombo   = new QComboBox;
    genderCombo->addItems({tr("Male"), tr("Female")});

    birthdayEdit = new QDateEdit;
    birthdayEdit->setDisplayFormat("yyyy-MM-dd");
    birthdayEdit->setCalendarPopup(true);

    cityEdit   = new QLineEdit;
    schoolEdit = new QLineEdit;

    auto *form = new QFormLayout;
    form->addRow(tr("Nickname:"),    nicknameEdit);
    form->addRow(tr("Real Name:"),   nameEdit);
    form->addRow(tr("Description:"), descriptionEd);
    form->addRow(tr("Gender:"),      genderCombo);
    form->addRow(tr("Birthday:"),    birthdayEdit);
    form->addRow(tr("City:"),        cityEdit);
    form->addRow(tr("School:"),      schoolEdit);

    auto *saveBtn   = new QPushButton(tr("Save"));
    auto *cancelBtn = new QPushButton(tr("Cancel"));
    connect(saveBtn, &QPushButton::clicked, this, &PersonalSettingsDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto *btnLay = new QHBoxLayout;
    btnLay->addStretch();
    btnLay->addWidget(saveBtn);
    btnLay->addWidget(cancelBtn);

    auto *mainLay = new QVBoxLayout;
    mainLay->addLayout(form);
    mainLay->addLayout(btnLay);
    setLayout(mainLay);
}

QCoro::Task<> PersonalSettingsDialog::onSave()
{
    if (nicknameEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"),
                             tr("Nickname cannot be empty."));
        co_return;
    }

    OJPersonalizationForm form;
    form.nickname    = nicknameEdit->text();
    form.name        = nameEdit->text();
    form.description = descriptionEd->toPlainText();
    form.gender      = genderCombo->currentIndex() == 0
                          ? OJPersonalizationForm::Male
                          : OJPersonalizationForm::Female;
    form.birthday    = birthdayEdit->date().toString(Qt::ISODate);
    form.city        = cityEdit->text();
    form.school      = schoolEdit->text();

    auto res = co_await Crawler::instance().personalize(form);
    if (!res.has_value()) {
        QMessageBox::critical(this, tr("Save Failed"), res.error());
        co_return;
    }

    QMessageBox::information(this, tr("Success"),
                             tr("Your personal info was updated."));
    accept();
}

QCoro::Task<> PersonalSettingsDialog::loadExisting()
{
    auto resp = co_await Crawler::instance().get(QUrl("http://openjudge.cn/settings/"));
    if (!resp.has_value()) co_return;

    auto parsed = co_await OJParser::getInstance().parsePersonalizationForm(resp.value());
    if (!parsed.has_value()) co_return;

    auto &f = parsed.value();
    nicknameEdit->setText(f.nickname);
    nameEdit->setText(f.name);
    descriptionEd->setPlainText(f.description);
    genderCombo->setCurrentIndex(f.gender == OJPersonalizationForm::Male ? 0 : 1);
    birthdayEdit->setDate(QDate::fromString(f.birthday, Qt::ISODate));
    cityEdit->setText(f.city);
    schoolEdit->setText(f.school);
}