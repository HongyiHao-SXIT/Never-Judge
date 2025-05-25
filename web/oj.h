#ifndef OJ_H
#define OJ_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QUrl>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateEdit>
#include <qcorotask.h>

struct OJProblem {
  QString title;
  QString content;
};

struct OJProblemDetail {
  QString title;
  QString description;
  QString inputDesc;
  QString outputDesc;
  QString sampleInput;
  QString sampleOutput;
  QString hint;
};

struct OJMatch {
  QList<QUrl> problemUrls;
};

struct OJLanguage {
  QString formValue;
  QString name;
};

struct OJSubmitForm {
  QString contestId;
  QString problemNumber;
  QList<OJLanguage> languages;
  QString code;
  QString checked;
  QUrl problemUrl;
};

struct OJSubmitResponse {
  enum Result { W, AC, WA, CE, RE, TLE, MLE, PE, UKE };
  Result result;
  QString message;
};

struct OJPersonalizationForm {
  enum Gender { Male, Female };
  QString nickname;
  QString name;
  QString description;
  Gender gender;
  QString birthday;
  QString city;
  QString school;
};

class PersonalSettingsDialog : public QDialog {
  Q_OBJECT

public:
  explicit PersonalSettingsDialog(QWidget *parent = nullptr);

  private slots:
      QCoro::Task<> onSave();
      QCoro::Task<> loadExisting();

private:
  QLineEdit   *nicknameEdit;
  QLineEdit   *nameEdit;
  QTextEdit   *descriptionEd;
  QComboBox   *genderCombo;
  QDateEdit   *birthdayEdit;
  QLineEdit   *cityEdit;
  QLineEdit   *schoolEdit;
};

#endif // OJ_H
