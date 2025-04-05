#include "preview.h"

#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

class PreviewTextWidget : public QTextEdit {
    Q_OBJECT

    QString title;
    void setup() { setReadOnly(true); }

public:
    explicit PreviewTextWidget(QWidget *parent) : QTextEdit(parent) { setup(); }
    void setTitle(const QString &title) { this->title = title; }
    QString &getTitle() { return this->title; }
};

class EmptyPreviewWidget : public QTextEdit {
    Q_OBJECT

    void setup() {
        setReadOnly(true);
        setText(tr("请下载题目或比赛以进行预览"));
    }

public:
    explicit EmptyPreviewWidget(QWidget *parent) : QTextEdit(parent) { setup(); }
};

#define DEFAULT_LABEL "题目预览"

OpenJudgePreviewWidget::OpenJudgePreviewWidget(QWidget *parent) : QWidget(parent), curIndex(0) {
    titleLabel = new QLabel(tr(DEFAULT_LABEL), this);
    lastBtn = new IconPushButton(this);
    nextBtn = new IconPushButton(this);

    textLayout = new QHBoxLayout();
    emptyWidget = new EmptyPreviewWidget(this);
    setup();

    connect(this, &OpenJudgePreviewWidget::previewPagesReset, this, &OpenJudgePreviewWidget::reset);
    connect(this, &OpenJudgePreviewWidget::currentIndexChanged, this, &OpenJudgePreviewWidget::refresh);
}

void OpenJudgePreviewWidget::setup() {
    auto *mainLayout = new QVBoxLayout(this);
    titleLabel->setAlignment(Qt::AlignCenter);

    lastBtn->setIconFromResName("left", 20);
    lastBtn->setDisabled(true);
    lastBtn->setFixedSize(25, 25);
    connect(lastBtn, &QPushButton::clicked, this, &OpenJudgePreviewWidget::decrementIndex);

    nextBtn->setIconFromResName("right", 20);
    nextBtn->setDisabled(true);
    nextBtn->setFixedSize(25, 25);
    connect(nextBtn, &QPushButton::clicked, this, &OpenJudgePreviewWidget::incrementIndex);

    auto *titleLayout = new QHBoxLayout();
    titleLayout->addWidget(lastBtn);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(nextBtn);

    mainLayout->addLayout(titleLayout);
    mainLayout->addWidget(emptyWidget);
    mainLayout->addLayout(textLayout);

    setLayout(mainLayout);
}

void OpenJudgePreviewWidget::reset() {
    curIndex = 0;
    emptyWidget->setVisible(textLayout->count() == 0);
    emit currentIndexChanged();
}

void OpenJudgePreviewWidget::refresh() const {
    lastBtn->setDisabled(curIndex == 0);
    int cnt = textLayout->count();
    bool empty = cnt == 0;
    nextBtn->setDisabled(empty || curIndex == cnt - 1);
    if (!empty) {
        // only show the current preview page
        for (int i = 0; i < textLayout->count(); i++) {
            textLayout->itemAt(i)->widget()->setVisible(false);
        }
        curPreview()->setVisible(true);
    }
    titleLabel->setText(empty ? DEFAULT_LABEL : curPreview()->getTitle());
}

PreviewTextWidget *OpenJudgePreviewWidget::curPreview() const {
    auto item = textLayout->itemAt(curIndex);
    return item ? qobject_cast<PreviewTextWidget *>(item->widget()) : nullptr;
}

void OpenJudgePreviewWidget::clear() {
    // remove all items in textLayout
    while (textLayout->count() > 0) {
        auto item = textLayout->takeAt(0);
        if (item) {
            delete item->widget();
            delete item;
        }
    }
    emit previewPagesReset();
}

QCoro::Task<std::expected<OJProblem, QString>> OpenJudgePreviewWidget::downloadAndParse(const QUrl &url) {
    Crawler crawler(url);
    auto content = co_await crawler.crawl();
    if (!content.has_value()) {
        co_return std::unexpected(tr("下载失败：%1").arg(content.error()));
    }
    auto parsed = co_await OJParser::parseProblem(content.value());
    if (!parsed.has_value()) {
        co_return std::unexpected(tr("解析失败：%1").arg(parsed.error()));
    }

    qDebug() << "Parsed problem from: " << url;
    co_return parsed.value();
}


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

    auto res = co_await downloadAndParse(url);
    if (!res.has_value()) {
        QMessageBox::warning(this, tr("错误"), res.error());
        co_return;
    }

    clear();
    auto preview = new PreviewTextWidget(this);
    preview->setHtml(res.value().content);
    textLayout->addWidget(preview);
    emit previewPagesReset();
}

void OpenJudgePreviewWidget::warning(const QString &message) { QMessageBox::warning(this, tr("错误"), message); };

QCoro::Task<> OpenJudgePreviewWidget::batchDownloadOJ() {
    bool ok;
    QString matchUrl =
            QInputDialog::getText(this, tr("下载链接"), tr("请输入要下载的比赛："), QLineEdit::Normal, "", &ok);

    if (!ok) {
        co_return;
    }
    if (matchUrl.isEmpty()) {
        warning(tr("下载链接不能为空"));
        co_return;
    }

    QUrl match(matchUrl);
    Crawler crawler(match);
    auto content = co_await crawler.crawl();
    if (!content.has_value()) {
        warning(tr("下载比赛失败：%1").arg(content.error()));
        co_return;
    }

    auto urls = co_await OJParser::parseProblemUrlsInMatch(content.value());
    if (!urls.has_value()) {
        warning(tr("解析比赛失败：%1").arg(urls.error()));
        co_return;
    }

    clear();
    for (const auto &relativeUrl: urls.value()) {
        // These urls are relative url in the website
        auto url = match.resolved(relativeUrl);

        auto problem = co_await downloadAndParse(url);
        if (problem.has_value()) {
            auto preview = new PreviewTextWidget(this);
            preview->setHtml(problem.value().content);
            preview->setVisible(false);
            preview->setTitle(problem->title);
            textLayout->addWidget(preview);
        } else {
            warning(tr("处理试题 %1 时出错:\n").arg(url.toString()) + problem.error());
        }
    }
    emit previewPagesReset();
}

void OpenJudgePreviewWidget::incrementIndex() {
    if (curIndex < textLayout->count() - 1) {
        curIndex++;
    }
    emit currentIndexChanged();
}

void OpenJudgePreviewWidget::decrementIndex() {
    if (curIndex > 0) {
        curIndex--;
    }
    emit currentIndexChanged();
}

#include "preview.moc"
