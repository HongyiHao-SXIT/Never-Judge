#ifndef OPENJUDGE_H
#define OPENJUDGE_H

#include <QLayout>
#include <QTextEdit>
#include <QLabel>
#include <expected>
#include <qcorotask.h>

#include "../web/oj.h"
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
    static QCoro::Task<std::expected<OJProblem, QString>> downloadAndParse(const QUrl &url);

signals:
    void previewPagesReset();
    void currentIndexChanged();

public slots:
    QCoro::Task<> downloadOJ();
    QCoro::Task<> batchDownloadOJ();
    void incrementIndex();
    void decrementIndex();

public:
    explicit OpenJudgePreviewWidget(QWidget *parent = nullptr);
    void clear();
};

#endif // OPENJUDGE_H
