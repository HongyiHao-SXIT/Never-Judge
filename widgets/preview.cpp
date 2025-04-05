#include "preview.h"

#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "../oj/web.h"
#include "icon.h"

class PreviewTextWidget : public QTextEdit {
    Q_OBJECT
    void setup() { setReadOnly(true); }

public:
    PreviewTextWidget(QWidget *parent) : QTextEdit(parent) { setup(); }
};

OpenJudgePreviewWidget::OpenJudgePreviewWidget(QWidget *parent) : QWidget(parent), curIndex(0) { setup(); }

void OpenJudgePreviewWidget::setup() {
    auto *mainLayout = new QVBoxLayout(this);
    auto *titleLabel = new QLabel(tr("题目预览"), this);
    titleLabel->setAlignment(Qt::AlignCenter);

    auto lastBtn = new IconPushButton(this);
    lastBtn->setIconFromResName("back", 20);
    connect(lastBtn, &QPushButton::clicked, this, &OpenJudgePreviewWidget::decrementIndex);

    auto nextBtn = new IconPushButton(this);
    nextBtn->setIconFromResName("hash", 20);
    connect(nextBtn, &QPushButton::clicked, this, &OpenJudgePreviewWidget::incrementIndex);

    auto *titleLayout = new QHBoxLayout();
    titleLayout->addWidget(lastBtn);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(nextBtn);

    auto widget = new PreviewTextWidget(this);
    textLayout = new QHBoxLayout();
    textLayout->addWidget(widget);

    mainLayout->addLayout(titleLayout);
    mainLayout->addLayout(textLayout);

    setLayout(mainLayout);
}

QTextEdit *OpenJudgePreviewWidget::curPreview() const {
    auto item = textLayout->itemAt(curIndex);
    return item ? qobject_cast<QTextEdit *>(item->widget()) : nullptr;
}

void OpenJudgePreviewWidget::setText(const QString &text) const { curPreview()->setText(text); }

void OpenJudgePreviewWidget::setHtml(const QString &html) const { curPreview()->setHtml(html); }

QCoro::Task<> OpenJudgePreviewWidget::downloadOJ() {
    bool ok;
    QString url = QInputDialog::getText(this, tr("下载链接"), tr("请输入要下载的题目："), QLineEdit::Normal, "", &ok);

    if (!ok) {
        co_return;
    }
    if (url.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("下载链接不能为空"));
        co_return;
    };

    Crawler crawler(url);
    auto content = co_await crawler.crawl();
    if (!content.has_value()) {
        QMessageBox::warning(this, tr("错误"), tr("下载失败：%1").arg(content.error()));
        co_return;
    }

    OJParser parser;
    auto parsed = co_await parser.parse(content.value());
    if (!parsed.has_value()) {
        QMessageBox::warning(this, tr("错误"), tr("解析失败：%1\n请检查 OJ 链接是否正确！").arg(parsed.error()));
        co_return;
    }

    setHtml(parsed.value());
    co_return;
}

QCoro::Task<> OpenJudgePreviewWidget::batchDownloadOJ() {
    bool ok;
    QString url = QInputDialog::getText(this, tr("下载链接"), tr("请输入要下载的比赛："), QLineEdit::Normal, "", &ok);

    if (!ok) {
        co_return;
    }
    if (url.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("下载链接不能为空"));
        co_return;
    };

    Crawler crawler(url);
    auto content = co_await crawler.crawl();
    if (!content.has_value()) {
        QMessageBox::warning(this, tr("错误"), tr("下载失败：%1").arg(content.error()));
        co_return;
    }

    co_return;
}

void OpenJudgePreviewWidget::incrementIndex() {
    if (curIndex < textLayout->count() - 1) {
        curIndex++;
        // 更新显示内容
    }
}

void OpenJudgePreviewWidget::decrementIndex() {
    if (curIndex > 0) {
        curIndex--;
        // 更新显示内容
    }
}

#include "preview.moc"
