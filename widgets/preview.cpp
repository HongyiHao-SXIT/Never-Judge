#include "preview.h"

#include <QCheckBox>
#include <QComboBox>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <utility>

#include "../util/file.h"
#include "../web/crawl.h"

class PreviewTextWidget : public QTextEdit {
    Q_OBJECT

    QString title;
    QUrl url;
    void setup() { setReadOnly(true); }

public:
    explicit PreviewTextWidget(const QUrl &url, const QString &title, QWidget *parent) :
        QTextEdit(parent), url(url), title(title) {
        setup();
    }
    QUrl &getUrl() { return url; }
    QString &getTitle() { return title; }
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
    connect(this, &OpenJudgePreviewWidget::submitFinished, this, &OpenJudgePreviewWidget::waitForResponse);
    connect(this, &OpenJudgePreviewWidget::submitResponseReceived, this, &OpenJudgePreviewWidget::showSubmitResponse);
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

    auto *titleLayout = new QHBoxLayout(this);
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

QCoro::Task<std::expected<OJProblem, QString>> OpenJudgePreviewWidget::downloadAndParse(QUrl url) {
    auto content = co_await Crawler::instance().get(url);
    if (!content.has_value()) {
        co_return std::unexpected(tr("下载失败：%1").arg(content.error()));
    }
    auto parsed = co_await OJParser::parseProblem(content.value());
    if (!parsed.has_value()) {
        co_return std::unexpected(tr("解析失败：%1").arg(parsed.error()));
    }

    qDebug() << "Parsed problem from:" << url.url();
    co_return parsed.value();
}

class LoginDialog : public QDialog {
    Q_OBJECT

    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QCheckBox *rememberMeCheckBox;

    void setup() {
        setWindowTitle(tr("登录"));
        setModal(true);
        setMinimumWidth(300);
        auto *layout = new QVBoxLayout(this);

        layout->addWidget(new QLabel(tr("邮箱："), this));
        layout->addWidget(emailEdit);
        layout->addWidget(new QLabel(tr("密码："), this));
        layout->addWidget(passwordEdit);
        passwordEdit->setEchoMode(QLineEdit::Password);
        layout->addWidget(rememberMeCheckBox);

        auto reminder = new QLabel(tr("为了保证账号安全，不支持记住密码"), this);
        reminder->setStyleSheet("color: #999999");
        layout->addWidget(reminder);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &LoginDialog::reject);
        layout->addWidget(buttonBox);

        setLayout(layout);
    }

    void readRemembered() {
        QString email = ConfigManager::instance().get("email", "").toString();
        emailEdit->setText(email);
        rememberMeCheckBox->setChecked(!email.isEmpty());
    }

public:
    explicit LoginDialog(QWidget *parent = nullptr) : QDialog(parent) {
        emailEdit = new QLineEdit(this);
        passwordEdit = new QLineEdit(this);
        rememberMeCheckBox = new QCheckBox(tr("记住我的账号"), this);
        setup();
        readRemembered();
    }

    QPair<QString, QString> getCredentials() const { return {emailEdit->text(), passwordEdit->text()}; }

    bool rememberMe() const { return rememberMeCheckBox->isChecked(); }
};


QCoro::Task<> OpenJudgePreviewWidget::loginOJ() {
    LoginDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        co_return;
    }
    auto [email, password] = dialog.getCredentials();
    if (email.isEmpty() || password.isEmpty()) {
        warning(tr("邮箱和密码不能为空"));
        co_await loginOJ();
        co_return;
    }
    auto res = co_await Crawler::instance().login(email, password);
    if (!res.has_value()) {
        warning(res.error());
        co_await loginOJ();
        co_return;
    }
    QMessageBox::information(this, tr("登录成功"), tr("欢迎，") + email);
    ConfigManager::instance().set("email", dialog.rememberMe() ? email : "");

    emit loginAs(email);
}

class SubmitFromDialog : public QDialog {
    QComboBox *comboBox;

public:
    explicit SubmitFromDialog(const OJSubmitForm &form, QWidget *parent = nullptr) : QDialog(parent) {
        // show the languages in problem and let user choose one
        setWindowTitle(tr("提交"));
        setModal(true);
        auto *layout = new QVBoxLayout(this);
        auto *languageLabel = new QLabel(tr("选择编程语言："), this);
        comboBox = new QComboBox(this);
        for (const auto &lang: form.languages) {
            comboBox->addItem(lang.name, lang.formValue);
        }

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &SubmitFromDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &SubmitFromDialog::reject);
        layout->addWidget(languageLabel);
        layout->addWidget(comboBox);
        layout->addWidget(buttonBox);
        setLayout(layout);
    }

    QString getLanguage() const { return comboBox->currentData().toString(); }
};

QCoro::Task<> OpenJudgePreviewWidget::submit(QString code) {
    if (!curPreview()) {
        warning(tr("你还没有下载选择题目！"));
        co_return;
    }
    if (!Crawler::instance().hasLogin()) {
        co_await loginOJ();
        co_return;
    }
    QUrl submitUrl = curPreview()->getUrl().url() + "/submit";
    auto submitRes = co_await Crawler::instance().get(submitUrl);
    if (!submitRes.has_value()) {
        // The res should be ok here, unless the website changed
        warning(submitRes.error());
        co_return;
    }
    auto formRes = co_await OJParser::parseProblemSubmitForm(submitRes.value());
    if (!formRes.has_value()) {
        warning(formRes.error());
        co_return;
    }
    auto form = formRes.value();
    auto dialog = SubmitFromDialog(form, this);
    if (dialog.exec() != QDialog::Accepted) {
        co_return;
    }

    OJSubmitForm newForm = {form.contestId, form.problemNumber, {}, code, dialog.getLanguage(), curPreview()->getUrl()};
    auto res = co_await Crawler::instance().submit(newForm);
    if (!res.has_value()) {
        warning(res.error());
    }
    emit submitFinished(std::move(res.value()));
    co_return;
}

QCoro::Task<> OpenJudgePreviewWidget::waitForResponse(QUrl responseUrl) {
    auto res = co_await Crawler::instance().get(responseUrl);
    if (!res.has_value()) {
        warning(tr("等待提交结果时出现错误: %1").arg(res.error()));
        co_return;
    }
    auto response = co_await OJParser::parseProblemSubmitResponse(res.value());
    if (!response.has_value()) {
        warning(response.error());
        co_return;
    }
    emit submitResponseReceived(std::move(response.value()), std::move(responseUrl));
    co_return;
}

void OpenJudgePreviewWidget::showSubmitResponse(const OJSubmitResponse &response, QUrl source) {
    QString text;
    bool ok = false;
    switch (response.result) {
        case W:
            // retry to wait for the result;
            emit submitFinished(std::move(source));
            return;
        case AC:
            ok = true;
            text = tr("您 AC 了！太您了！");
            break;
        case WA:
            text = tr("喜报：您 WA 了！");
            break;
        case CE:
            text = tr("CE... 提交前能不能先看看代码跑得起来不？\n%1").arg(response.message);
            break;
        case RE:
            text = tr("您 RE 了！但愿不是段错误...");
            break;
        case TLE:
            text = tr("您 TLE 了！超时判负！");
            break;
        case MLE:
            text = tr("您 MLE 了！也许我该换个更大的内存条...");
            break;
        case PE:
            text = tr("PE！注意一下格式就可以了吧，大概...");
            break;
        case UKE:
            text = tr("是没见过的报错呢~ 还请您手动看看~");
            break;
    }

    if (ok) {
        QMessageBox::information(this, "通过！", text);
    } else {
        QMessageBox::warning(this, "失败！", text);
    }
}

QCoro::Task<> OpenJudgePreviewWidget::downloadOJ() {
    bool ok;
    QString url = QInputDialog::getText(this, tr("下载链接"), tr("请输入要下载的题目："), QLineEdit::Normal, "", &ok);

    if (!ok) {
        co_return;
    }
    if (url.isEmpty()) {
        warning(tr("下载链接不能为空"));
        co_return;
    };

    auto res = co_await downloadAndParse(url);
    if (!res.has_value()) {
        warning(res.error());
        co_return;
    }

    clear();
    auto preview = new PreviewTextWidget(url, res->title, this);
    preview->setHtml(res.value().content);
    textLayout->addWidget(preview);
    emit previewPagesReset();
    co_return;
}

void OpenJudgePreviewWidget::warning(const QString &message) { QMessageBox::warning(this, tr("错误"), message); }

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
    auto content = co_await Crawler::instance().get(match);
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
    for (const auto &relativeUrl: urls.value().problemUrls) {
        // These urls are relative url in the website
        auto url = match.resolved(relativeUrl);

        auto problem = co_await downloadAndParse(url);
        if (problem.has_value()) {
            auto preview = new PreviewTextWidget(url, problem->title, this);
            preview->setHtml(problem.value().content);
            preview->setVisible(false);
            textLayout->addWidget(preview);
        } else {
            warning(tr("处理试题 %1 时出错:\n").arg(url.toString()) + problem.error());
        }
    }
    emit previewPagesReset();
    co_return;
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
