#ifndef OPENJUDGE_H
#define OPENJUDGE_H

#include <QTextEdit>
#include <QLayout>
#include <qcorotask.h>

class OpenJudgePreviewWidget : public QWidget {
    Q_OBJECT
    int curIndex;
    QLayout* textLayout;

    void setup();
    QTextEdit * curPreview() const;

public slots:
    QCoro::Task<> downloadOJ();
    QCoro::Task<> batchDownloadOJ();
    void incrementIndex();
    void decrementIndex();

public:
    explicit OpenJudgePreviewWidget(QWidget *parent = nullptr);
    void clear();
    void setText(const QString &text) const;
    void setHtml(const QString &html) const;
};

#endif // OPENJUDGE_H
