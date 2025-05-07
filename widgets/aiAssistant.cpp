#include "aiAssistant.h"

#include <QFont>
#include <QFontMetrics>
#include <QInputDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>

#include "../ide/aiChat.h"
#include "../web/aiClient.h"
#include "../widgets/code.h"

AIAssistantWidget::AIAssistantWidget(CodeTabWidget *codeTab, QWidget *parent) :
    QDockWidget(tr("AI 刷题助手"), parent) {
    logDebug("Initializing AI Assistant window");

    setupUI();
    connectSignals();

    if (!AIClient::getInstance().hasApiKey()) {
        logDebug("API key not set, showing information message");
        QMessageBox::information(this, tr("API 密钥未设置"),
                                 tr("请先设置 DeepSeek API 密钥才能使用 AI 助手功能。"));
    }

    m_codeTab = codeTab;
    if (m_codeTab) {
        logDebug("Successfully obtained code editor pointer");
    } else {
        logDebug("Code editor pointer is null");
    }
}

AIAssistantWidget::~AIAssistantWidget() {
    // 清理资源
}

void AIAssistantWidget::setupUI() {
    mainWidget = new QWidget(this);
    mainWidget->setStyleSheet("background-color: #1E1E1E;");
    mainLayout = new QVBoxLayout(mainWidget);

    conversationView = new QTextBrowser(mainWidget);
    conversationView->setOpenLinks(false);
    conversationView->setOpenExternalLinks(true);
    conversationView->setTextInteractionFlags(Qt::TextBrowserInteraction);
    conversationView->setStyleSheet("background-color: #1E1E1E; color: #D4D4D4; border: 1px solid "
                                    "#3F3F46; border-radius: 4px;");

    QFont font("Microsoft YaHei", 10);
    conversationView->setFont(font);

    userInput = new QTextEdit(mainWidget);
    userInput->setPlaceholderText(tr("输入你的问题..."));
    userInput->setMaximumHeight(100);
    userInput->setFont(font);
    userInput->setStyleSheet("background-color: #252526; color: #D4D4D4; border: 1px solid "
                             "#3F3F46; border-radius: 4px;");

    buttonLayout = new QHBoxLayout();

    QString buttonStyle = "QPushButton { background-color: #3E3E42; color: #D4D4D4; border: none; "
                          "border-radius: 4px; padding: 6px 12px; }"
                          "QPushButton:hover { background-color: #505050; }"
                          "QPushButton:pressed { background-color: #2A2D2E; }"
                          "QPushButton:disabled { background-color: #2D2D30; color: #6D6D6D; }";

    QString highlightButtonStyle =
            "QPushButton { background-color: #3574F0; color: #FFFFFF; border: none; border-radius: "
            "4px; padding: 6px 12px; }"
            "QPushButton:hover { background-color: #4A85F5; }"
            "QPushButton:pressed { background-color: #2A65D1; }"
            "QPushButton:disabled { background-color: #2D2D30; color: #6D6D6D; }";

    sendButton = new QPushButton(tr("发送"), mainWidget);
    clearButton = new QPushButton(tr("清空"), mainWidget);
    analyzeButton = new QPushButton(tr("题目解析"), mainWidget);
    codeButton = new QPushButton(tr("示例代码"), mainWidget);
    debugButton = new QPushButton(tr("调试"), mainWidget);
    showProblemButton = new QPushButton(tr("显示题目"), mainWidget);
    insertCodeButton = new QPushButton(tr("插入代码"), mainWidget);
    insertCodeButton->setVisible(false); // show when the code is available

    sendButton->setStyleSheet(buttonStyle);
    clearButton->setStyleSheet(buttonStyle);
    analyzeButton->setStyleSheet(highlightButtonStyle);
    codeButton->setStyleSheet(highlightButtonStyle);
    debugButton->setStyleSheet(buttonStyle);
    showProblemButton->setStyleSheet(buttonStyle);
    insertCodeButton->setStyleSheet(buttonStyle);

    buttonLayout->addWidget(showProblemButton);
    buttonLayout->addWidget(analyzeButton);
    buttonLayout->addWidget(codeButton);
    buttonLayout->addWidget(debugButton);
    buttonLayout->addWidget(insertCodeButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(sendButton);

    settingsLayout = new QHBoxLayout();

    apiKeyLabel = new QLabel(tr("API 密钥:"), mainWidget);
    apiKeyLabel->setStyleSheet("color: #D4D4D4;");
    setApiKeyButton = new QPushButton(tr("设置"), mainWidget);
    setApiKeyButton->setStyleSheet(buttonStyle);

    settingsLayout->addWidget(apiKeyLabel);
    settingsLayout->addWidget(setApiKeyButton);
    settingsLayout->addStretch();

    progressBar = new QProgressBar(mainWidget);
    progressBar->setRange(0, 0);
    progressBar->setVisible(false);
    progressBar->setStyleSheet("QProgressBar {"
                               "   background-color: #2D2D30;"
                               "   color: #D4D4D4;"
                               "   border: 1px solid #3F3F46;"
                               "   border-radius: 3px;"
                               "   text-align: center;"
                               "}"
                               "QProgressBar::chunk {"
                               "   background-color: #569CD6;"
                               "   border-radius: 2px;"
                               "}");

    mainLayout->addLayout(settingsLayout);
    mainLayout->addWidget(conversationView);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(userInput);
    mainLayout->addLayout(buttonLayout);

    setWidget(mainWidget);
    resize(400, 600);
}

void AIAssistantWidget::connectSignals() {
    // 连接按钮信号
    connect(sendButton, &QPushButton::clicked, this, &AIAssistantWidget::onSendClicked);
    connect(clearButton, &QPushButton::clicked, this, &AIAssistantWidget::onClearClicked);
    connect(analyzeButton, &QPushButton::clicked, this, &AIAssistantWidget::onAnalyzeClicked);
    connect(codeButton, &QPushButton::clicked, this, &AIAssistantWidget::onCodeClicked);
    connect(debugButton, &QPushButton::clicked, this, &AIAssistantWidget::onDebugClicked);
    connect(showProblemButton, &QPushButton::clicked, this,
            &AIAssistantWidget::onShowProblemClicked);
    connect(setApiKeyButton, &QPushButton::clicked, this, &AIAssistantWidget::onSetApiKeyClicked);
    connect(insertCodeButton, &QPushButton::clicked, this, &AIAssistantWidget::onInsertCodeClicked);

    connect(&AIChatManager::getInstance(), &AIChatManager::messageAdded, this,
            &AIAssistantWidget::onMessageAdded);
    connect(&AIClient::getInstance(), &AIClient::requestCompleted, this,
            &AIAssistantWidget::onRequestCompleted);
    connect(userInput, &QTextEdit::textChanged, this, [this]() {
        // Ctrl+Enter to send message
        if (userInput->toPlainText().endsWith("\n") &&
            (QApplication::keyboardModifiers() & Qt::ControlModifier)) {
            QString text = userInput->toPlainText();
            text.chop(1);
            userInput->setPlainText(text);
            onSendClicked();
        }
    });
}

QString AIAssistantWidget::getCurrentCode() const {
    logDebug("Attempting to get current code");

    if (m_codeTab) {
        auto currentEdit = m_codeTab->curEdit();
        if (currentEdit) {
            QString code = currentEdit->toPlainText();
            logDebug("Successfully retrieved current code, length: " +
                     QString::number(code.length()) + " characters");
            return code;
        } else {
            logDebug("No active code editor found");
            qWarning() << "AIAssistantWidget: No active code editor found.";
        }
    } else {
        logDebug("CodeTabWidget pointer is null");
        qWarning() << "AIAssistantWidget: CodeTabWidget pointer is null.";
    }

    logDebug("Unable to get code, returning empty string");
    return {};
}

void AIAssistantWidget::displayMessage(const AIMessage &message) {
    QString text = message.content;
    bool isUser = (message.type == AIMessageType::USER);

    displayMarkdown(text, isUser);
}

void AIAssistantWidget::displayMarkdown(const QString &text, bool isUser) {
    QString roleText = isUser ? "<p style='color: #569CD6;'><b><big>用户</b></big></p>"
                              : "<p style='color: #C586C0;'><b><big>AI 助手</b></big></p>";
    historyMarkdown += roleText;
    historyMarkdown += "\n\n --- \n\n";
    historyMarkdown += text + "<br /><br /><br />";
    conversationView->setMarkdown(historyMarkdown);

    QScrollBar *scrollBar = conversationView->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void AIAssistantWidget::setProgressVisible(bool visible) {
    progressBar->setVisible(visible);
    sendButton->setEnabled(!visible);
    clearButton->setEnabled(!visible);
    analyzeButton->setEnabled(!visible);
    codeButton->setEnabled(!visible);
    debugButton->setEnabled(!visible);
    userInput->setEnabled(!visible);
}

void AIAssistantWidget::setProblemInfo(const QString &title, const QString &description,
                                       const QString &inputDesc, const QString &outputDesc,
                                       const QString &sampleInput, const QString &sampleOutput) {
    currentTitle = title;
    currentDescription = description;
    currentInputDesc = inputDesc;
    currentOutputDesc = outputDesc;
    currentSampleInput = sampleInput;
    currentSampleOutput = sampleOutput;

    QString message = tr("已加载题目: %1").arg(title);
    AIMessage systemMsg(AIMessageType::SYSTEM, message);
    displayMessage(systemMsg);
}

void AIAssistantWidget::setUserCode(const QString &code) {
    userInput->setPlainText(code);
}

void AIAssistantWidget::onSendClicked() {
    QString message = userInput->toPlainText().trimmed();
    if (message.isEmpty()) {
        return;
    }

    AIMessage userMsg(AIMessageType::USER, message);
    displayMessage(userMsg);

    userInput->clear();
    setProgressVisible(true);
    sendAIRequest(message, "user information");
}

void AIAssistantWidget::onClearClicked() {
    AIChatManager::getInstance().clearMessages();
    conversationView->clear();
}

bool AIAssistantWidget::tryGetProblemInfo() {
    logDebug("Attempting to actively get problem information");

    QWidget *parent = this;
    while (parent && !parent->inherits("IDEMainWindow")) {
        parent = parent->parentWidget();
    }

    if (!parent) {
        logDebug("Main window not found");
        return false;
    }

    auto *preview = parent->findChild<OpenJudgePreviewWidget *>();
    if (!preview || !preview->isVisible()) {
        logDebug("Preview window not visible or not found");
        return false;
    }

    if (!getProblemInfoFromPreview(preview)) {
        logDebug("Failed to get problem info from preview window");
        return false;
    }

    return true;
}

void AIAssistantWidget::onAnalyzeClicked() {
    qDebug() << "[AI Assistant Debug] Problem analysis button clicked";

    tryGetProblemInfo();

    if (currentTitle.isEmpty() || currentDescription.isEmpty()) {
        qDebug() << "[AI Assistant Debug] Problem information incomplete, cannot generate analysis";
        QMessageBox::warning(this, tr("缺少题目信息"), tr("请先打开一个题目或手动输入题目信息。"));
        return;
    }

    QString fullDescription = getFullProblemDescription();
    qDebug() << "[AI Assistant Debug] Retrieved complete problem description, length:"
             << fullDescription.length() << "characters";

    QString prompt = QString("请详细分析以下算法题目，包括以下内容：\n"
                             "1. 题目的核心问题和考点\n"
                             "2. 解题思路和算法策略\n"
                             "3. 可能的边界情况和注意事项\n"
                             "4. 时间复杂度和空间复杂度分析\n\n"
                             "题目：%1\n\n%2")
                             .arg(currentTitle, fullDescription);

    qDebug() << "[AI Assistant Debug] Generated problem analysis prompt, length:" << prompt.length()
             << "characters";

    AIMessage systemMsg(AIMessageType::SYSTEM, tr("正在生成题目解析..."));
    displayMessage(systemMsg);

    setProgressVisible(true);

    qDebug() << "[AI Assistant Debug] Sending problem analysis request";
    sendAIRequest(prompt, "题目解析");
}

void AIAssistantWidget::onCodeClicked() {
    qDebug() << "[AI Assistant Debug] Example code button clicked";

    tryGetProblemInfo();

    if (currentTitle.isEmpty() || currentDescription.isEmpty()) {
        qDebug() << "[AI Assistant Debug] Problem information incomplete, cannot generate code";
        QMessageBox::warning(this, tr("缺少题目信息"), tr("请先打开一个题目或手动输入题目信息。"));
        return;
    }

    QString fullDescription = getFullProblemDescription();
    qDebug() << "[AI Assistant Debug] Retrieved complete problem description, length:"
             << fullDescription.length() << "characters";

    QString prompt =
            QString("请为以下算法题目生成一个完整、高效、易于理解的C++示例代码。代码应该：\n"
                    "1. 包含详细的注释，解释关键步骤和算法思路\n"
                    "2. 处理各种边界情况\n"
                    "3. 使用合适的数据结构和算法\n"
                    "4. 遵循良好的编程实践\n\n"
                    "题目：%1\n\n%2")
                    .arg(currentTitle, fullDescription);

    qDebug() << "[AI Assistant Debug] Generated example code prompt, length:" << prompt.length()
             << "characters";

    AIMessage systemMsg(AIMessageType::SYSTEM, tr("正在生成示例代码..."));
    displayMessage(systemMsg);

    setProgressVisible(true);

    qDebug() << "[AI Assistant Debug] Sending example code request";
    sendAIRequest(prompt, "示例代码");
}

void AIAssistantWidget::onDebugClicked() {
    qDebug() << "[AI Assistant Debug] Debug button clicked";

    QString userCode = getCurrentCode();
    if (userCode.isEmpty()) {
        qDebug() << "[AI Assistant Debug] No code retrieved";
        QMessageBox::warning(this, tr("缺少代码"), tr("请先打开或编写需要调试的代码。"));
        return;
    }

    qDebug() << "[AI Assistant Debug] Retrieved code, length:" << userCode.length() << "characters";

    tryGetProblemInfo();

    if (currentTitle.isEmpty() || currentDescription.isEmpty()) {
        qDebug() << "[AI Assistant Debug] Problem information incomplete, cannot generate debug "
                    "analysis";
        QMessageBox::warning(this, tr("缺少题目信息"), tr("请先打开一个题目或手动输入题目信息。"));
        return;
    }

    QString fullDescription = getFullProblemDescription();
    qDebug() << "[AI Assistant Debug] Retrieved complete problem description, length:"
             << fullDescription.length() << "characters";

    QString prompt = QString("请帮助调试以下代码，详细分析可能存在的问题：\n"
                             "1. 逻辑错误\n"
                             "2. 边界情况处理\n"
                             "3. 算法实现问题\n"
                             "4. 性能优化建议\n\n"
                             "题目：%1\n\n%2\n\n"
                             "代码：\n```cpp\n%3\n```")
                             .arg(currentTitle, fullDescription, userCode);

    qDebug() << "[AI Assistant Debug] Generated debug prompt, length:" << prompt.length()
             << "characters";

    AIMessage userMsg(AIMessageType::USER, "请帮我调试这段代码");
    displayMessage(userMsg);

    setProgressVisible(true);
    sendAIRequest(prompt, "debug");
}

void AIAssistantWidget::onShowProblemClicked() {
    logDebug("Show problem button clicked");

    if (tryGetProblemInfo()) {
        logDebug("Successfully retrieved problem info, preparing to display");
    } else {
        logDebug("Failed to get new problem info, using current info");
    }

    showCurrentProblemInfo();
}

void AIAssistantWidget::onInsertCodeClicked() {
    logDebug("Insert code button clicked");

    QWidget *parent = this;
    while (parent && !parent->inherits("IDEMainWindow")) {
        parent = parent->parentWidget();
    }

    if (!parent) {
        logDebug("Cannot find main window");
        QMessageBox::warning(this, tr("无法访问编辑器"), tr("无法找到主窗口，无法插入代码。"));
        return;
    }

    logDebug("Found main window, attempting to get code editor");

    // 获取 CodeTabWidget
    CodeTabWidget *codeTab = parent->findChild<CodeTabWidget *>();
    if (!codeTab) {
        logDebug("无法找到代码编辑器");
        QMessageBox::warning(this, tr("无法访问编辑器"), tr("无法找到代码编辑器，无法插入代码。"));
        return;
    }

    logDebug("找到代码编辑器，准备插入代码");

    insertGeneratedCode(codeTab);
}

void AIAssistantWidget::onSetApiKeyClicked() {
    bool ok;
    QString apiKey = QInputDialog::getText(
            this, tr("设置 API 密钥"), tr("请输入 DeepSeek API 密钥:"), QLineEdit::Normal,
            AIClient::getInstance().hasApiKey() ? "********" : "", &ok);

    if (ok && !apiKey.isEmpty()) {
        AIClient::getInstance().setApiKey(apiKey);
        QMessageBox::information(this, tr("API 密钥已设置"), tr("DeepSeek API 密钥已成功设置。"));
    }
}

void AIAssistantWidget::onMessageAdded(const AIMessage &message) { displayMessage(message); }

QString AIAssistantWidget::extractCodeFromMarkdown(const QString &markdown) {
    static QRegularExpression codeBlockRegex(R"(```cpp\s*([\s\S]*?)```)");
    QRegularExpressionMatch match = codeBlockRegex.match(markdown);

    if (match.hasMatch()) {
        QString code = match.captured(1).trimmed();
        return code;
    }

    static QRegularExpression anyCodeBlockRegex(R"(```\w*\s*([\s\S]*?)```)");
    match = anyCodeBlockRegex.match(markdown);

    if (match.hasMatch()) {
        QString code = match.captured(1).trimmed();
        return code;
    }
    return {};
}

void AIAssistantWidget::insertGeneratedCode(CodeTabWidget *codeTabWidget) {
    if (generatedCode.isEmpty()) {
        QMessageBox::warning(this, tr("没有可插入的代码"),
                             tr("请先生成示例代码或从对话中选择代码。"));
        return;
    }

    CodeEditWidget *editor = codeTabWidget->curEdit();
    if (!editor) {
        QMessageBox::warning(this, tr("没有打开的编辑器"), tr("请先打开一个代码文件。"));
        return;
    }

    editor->insertPlainText(generatedCode);
    QMessageBox::information(this, tr("代码已插入"), tr("代码已成功插入到编辑器中。"));
}

void AIAssistantWidget::onRequestCompleted(bool success, const QString &response) {
    setProgressVisible(false);

    if (success) {
        QString code = extractCodeFromMarkdown(response);
        if (!code.isEmpty()) {
            logDebug("从响应中提取到代码，长度: " + QString::number(code.length()) + " 字符");
            generatedCode = code;
            insertCodeButton->setVisible(true);
        } else {
            insertCodeButton->setVisible(false);
            generatedCode.clear();
        }

        AIMessage assistantMsg(AIMessageType::ASSISTANT, response);
        displayMessage(assistantMsg);
    } else {
        logDebug("Failed to get request for AI: " + response);

        QString errorMsg = tr("Error: ") + response;
        AIMessage errorMsg2(AIMessageType::SYSTEM, errorMsg);
        displayMessage(errorMsg2);

        insertCodeButton->setVisible(false);
        generatedCode.clear();
    }
}

bool AIAssistantWidget::getProblemInfoFromPreview(OpenJudgePreviewWidget *preview) {
    if (!preview) {
        logDebug("No preview widget");
        return false;
    }

    auto *titleLabel = preview->findChild<QLabel *>();
    if (!titleLabel) {
        logDebug("No title label in preview");
        return false;
    }

    QString title = titleLabel->text();
    if (title.isEmpty() || title == "题目预览") {
        logDebug("No title in preview");
        return false;
    }

    QList<QTextEdit *> textEdits = preview->findChildren<QTextEdit *>();
    if (textEdits.isEmpty()) {
        logDebug("Cannot find text editor");
        return false;
    }

    QString content;
    for (QTextEdit *textEdit: textEdits) {
        if (textEdit->isVisible()) {
            content = textEdit->toPlainText();
            break;
        }
    }

    if (content.isEmpty()) {
        for (QTextEdit *textEdit: textEdits) {
            if (textEdit->isVisible()) {
                content = textEdit->toHtml();
                break;
            }
        }
    }

    if (content.isEmpty()) {
        logDebug("Cannot find content in text editor");
        content = tr("题目内容无法直接获取，请手动输入或从URL加载题目。");
    }

    QStringList lines = content.split("\n");
    QString description;
    QString inputDesc;
    QString outputDesc;
    QString sampleInput;
    QString sampleOutput;

    enum ParseState { None, Description, Input, Output, SampleInput, SampleOutput };

    ParseState state = None;

    for (const QString &line: lines) {
        if (line.contains("题目描述", Qt::CaseInsensitive)) {
            state = Description;
            continue;
        } else if (line.contains("输入", Qt::CaseInsensitive) &&
                   !line.contains("样例", Qt::CaseInsensitive)) {
            state = Input;
            continue;
        } else if (line.contains("输出", Qt::CaseInsensitive) &&
                   !line.contains("样例", Qt::CaseInsensitive)) {
            state = Output;
            continue;
        } else if (line.contains("样例输入", Qt::CaseInsensitive)) {
            state = SampleInput;
            continue;
        } else if (line.contains("样例输出", Qt::CaseInsensitive)) {
            state = SampleOutput;
            continue;
        }

        switch (state) {
            case Description:
                description += line + "\n";
                break;
            case Input:
                inputDesc += line + "\n";
                break;
            case Output:
                outputDesc += line + "\n";
                break;
            case SampleInput:
                sampleInput += line + "\n";
                break;
            case SampleOutput:
                sampleOutput += line + "\n";
                break;
            default:
                break;
        }
    }

    if (description.isEmpty()) {
        logDebug("Cannot find description, and use content as description.");
        description = content;
    }

    currentTitle = title;
    currentDescription = description.trimmed();
    currentInputDesc = inputDesc.trimmed();
    currentOutputDesc = outputDesc.trimmed();
    currentSampleInput = sampleInput.trimmed();
    currentSampleOutput = sampleOutput.trimmed();

    logCurrentProblemInfo();

    return !currentTitle.isEmpty() && !currentDescription.isEmpty();
}

QCoro::Task<bool> AIAssistantWidget::getProblemInfoFromUrl(const QUrl &url) {
    auto result = co_await ProblemCrawler::getInstance().getProblemDetail(url);

    if (!result.has_value()) {
        const QString& errorMsg = result.error();
        logDebug("Failed to get problem: " + errorMsg);
        QMessageBox::warning(this, tr("获取题目失败"), tr("无法获取题目信息：%1").arg(errorMsg));
        co_return false;
    }

    const OJProblemDetail& detail = result.value();
    currentTitle = detail.title;
    currentDescription = detail.description;
    currentInputDesc = detail.inputDesc;
    currentOutputDesc = detail.outputDesc;
    currentSampleInput = detail.sampleInput;
    currentSampleOutput = detail.sampleOutput;
    logCurrentProblemInfo();

    QString message = tr("已加载题目: %1").arg(currentTitle);
    AIMessage systemMsg(AIMessageType::SYSTEM, message);
    displayMessage(systemMsg);

    co_return true;
}

QString AIAssistantWidget::getFullProblemDescription() const {
    // Build complete problem information
    QString fullDescription = currentDescription;

    if (!currentInputDesc.isEmpty()) {
        fullDescription += "\n\nInput Description:\n" + currentInputDesc;
    }
    if (!currentOutputDesc.isEmpty()) {
        fullDescription += "\n\nOutput Description:\n" + currentOutputDesc;
    }
    if (!currentSampleInput.isEmpty()) {
        fullDescription += "\n\nSample Input:\n" + currentSampleInput;
    }
    if (!currentSampleOutput.isEmpty()) {
        fullDescription += "\n\nSample Output:\n" + currentSampleOutput;
    }

    return fullDescription;
}

void AIAssistantWidget::showCurrentProblemInfo() {
    logDebug("Displaying current problem information");

    if (currentTitle.isEmpty() || currentDescription.isEmpty()) {
        logDebug("Problem information incomplete, cannot display");
        QMessageBox::warning(this, tr("题目信息不完整"), tr("当前没有完整的题目信息可以显示。"));
        return;
    }

    // Build problem information display text
    QString infoText = tr("当前题目信息：\n\n");
    infoText += tr("标题：%1\n\n").arg(currentTitle);
    infoText += tr("描述：\n%1\n\n").arg(currentDescription);

    if (!currentInputDesc.isEmpty()) {
        infoText += tr("输入描述：\n%1\n\n").arg(currentInputDesc);
    }

    if (!currentOutputDesc.isEmpty()) {
        infoText += tr("输出描述：\n%1\n\n").arg(currentOutputDesc);
    }

    if (!currentSampleInput.isEmpty()) {
        infoText += tr("样例输入：\n%1\n\n").arg(currentSampleInput);
    }

    if (!currentSampleOutput.isEmpty()) {
        infoText += tr("样例输出：\n%1\n\n").arg(currentSampleOutput);
    }

    // Display problem information
    AIMessage systemMsg(AIMessageType::SYSTEM, infoText);
    displayMessage(systemMsg);
    logDebug("Problem information displayed");
}

void AIAssistantWidget::logDebug(const QString &message) const {
    qDebug() << "[AI Assistant Debug]" << message;
}

void AIAssistantWidget::logCurrentProblemInfo() const {
    qDebug() << "[AI Assistant Debug] Current problem info:";
    qDebug() << "[AI Assistant Debug]   Title:"
             << (currentTitle.isEmpty() ? "empty" : currentTitle);
    qDebug() << "[AI Assistant Debug]   Description length:" << currentDescription.length()
             << "characters";
    qDebug() << "[AI Assistant Debug]   Input description length:" << currentInputDesc.length()
             << "characters";
    qDebug() << "[AI Assistant Debug]   Output description length:" << currentOutputDesc.length()
             << "characters";
    qDebug() << "[AI Assistant Debug]   Sample input length:" << currentSampleInput.length()
             << "characters";
    qDebug() << "[AI Assistant Debug]   Sample output length:" << currentSampleOutput.length()
             << "characters";
}

void AIAssistantWidget::sendAIRequest(const QString &prompt, const QString &requestType) {
    qDebug() << "[AI Assistant Debug] Sending" << requestType
             << "request, length:" << prompt.length() << "characters";

    // Use synchronous method to send request
    AIClient::getInstance().sendRequestSync(prompt);

    qDebug() << "[AI Assistant Debug]" << requestType << "request sent, waiting for callback";
}
