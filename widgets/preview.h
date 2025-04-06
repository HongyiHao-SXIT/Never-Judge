#ifndef OPENJUDGE_H
#define OPENJUDGE_H

#include <QLayout>
#include <QTextEdit>
#include <QLabel>
#include <expected>
#include <qcorotask.h>

#include "icon.h"
#include "../web/parse.h"

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
    static QCoro::Task<std::expected<OJProblem, QString>> downloadAndParse(const QUrl &url);

signals:
    void previewPagesReset();
    void currentIndexChanged();

private slots:
    void incrementIndex();
    void decrementIndex();

public slots:
    QCoro::Task<> loginOJ();
    QCoro::Task<> submit(QString code);
    QCoro::Task<> downloadOJ();
    QCoro::Task<> batchDownloadOJ();

public:
    explicit OpenJudgePreviewWidget(QWidget *parent = nullptr);
    void clear();
};

#endif // OPENJUDGE_H
