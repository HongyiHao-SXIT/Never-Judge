#ifndef OJ_PERSONAL_H
#define OJ_PERSONAL_H

#include <QComboBox>

#include <QDateEdit>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>
#include <qcorotask.h>

class PersonalSettingsDialog : public QDialog {
    Q_OBJECT
    QLineEdit *nicknameEdit;
    QLineEdit *nameEdit;
    QTextEdit *descriptionEdit;
    QComboBox *genderCombo;
    QDateEdit *birthdayEdit;
    QLineEdit *cityEdit;
    QLineEdit *schoolEdit;

private slots:
    QCoro::Task<> onSave();
    QCoro::Task<> loadExisting();

public:
    explicit PersonalSettingsDialog(QWidget *parent = nullptr);
};

#endif // OJ_PERSONAL_H
