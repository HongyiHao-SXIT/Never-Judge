#ifndef OPENJUDGE_H
#define OPENJUDGE_H

#include <QLabel>
#include <QLayout>
#include <QTextEdit>
#include <expected>
#include <qcorotask.h>

#include "../web/parse.h"
#include "icon.h"

class PreviewTextWidget;

class OpenJudgePreviewWidget : public QWidget {
    Q_OBJECT

    QLabel *titleLabel;
    IconPushButton *lastBtn;
    IconPushButton *nextBtn;

    QWidget *emptyWidget;
    QLayout *textLayout;
    int curIndex;

    void setup();
    void reset();
    void refresh() const;
    PreviewTextWidget *curPreview() const;
    void warning(const QString &message);
    static QCoro::Task<std::expected<OJProblem, QString>> downloadAndParse(QUrl url);

signals:
    void previewPagesReset();
    void currentIndexChanged();
    void submitFinished(QUrl redirectUrl);
    void submitResponseReceived(OJSubmitResponse response, QUrl source);
    void loginAs(const QString &username);

private slots:
    void incrementIndex();
    void decrementIndex();

public slots:
    QCoro::Task<> loginOJ();
    QCoro::Task<> submit(QString code);
    QCoro::Task<> waitForResponse(QUrl responseUrl);
    void showSubmitResponse(const OJSubmitResponse& response, QUrl source);
    QCoro::Task<> downloadOJ();
    QCoro::Task<> batchDownloadOJ();

public:
    explicit OpenJudgePreviewWidget(QWidget *parent = nullptr);
    void clear();
};

#endif // OPENJUDGE_H
