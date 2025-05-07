#include "aiAssistant.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QFont>
#include <QFontMetrics>
#include <QTextBlock>
#include <QTextCursor>

#include "../web/aiClient.h"
#include "../ide/aiChat.h"
#include "../widgets/code.h"

AIAssistantWidget::AIAssistantWidget(CodeTabWidget* codeTab, QWidget *parent) : QDockWidget(tr("AI 刷题助手"), parent) {
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
    // 创建主窗口部件
    mainWidget = new QWidget(this);
    mainWidget->setStyleSheet("background-color: #1E1E1E;");
    mainLayout = new QVBoxLayout(mainWidget);

    // 对话历史显示区
    conversationView = new QTextBrowser(mainWidget);
    conversationView->setOpenLinks(false);
    conversationView->setOpenExternalLinks(true);
    conversationView->setTextInteractionFlags(Qt::TextBrowserInteraction);
    conversationView->setStyleSheet("background-color: #1E1E1E; color: #D4D4D4; border: 1px solid #3F3F46; border-radius: 4px;");

    // 设置字体
    QFont font("Microsoft YaHei", 10);
    conversationView->setFont(font);

    // 用户输入区
    userInput = new QTextEdit(mainWidget);
    userInput->setPlaceholderText(tr("输入你的问题..."));
    userInput->setMaximumHeight(100);
    userInput->setFont(font);
    userInput->setStyleSheet("background-color: #252526; color: #D4D4D4; border: 1px solid #3F3F46; border-radius: 4px;");

    // 功能按钮区
    buttonLayout = new QHBoxLayout();

    // 创建按钮并设置统一样式 - 使用与IDE匹配的颜色方案
    QString buttonStyle = "QPushButton { background-color: #3E3E42; color: #D4D4D4; border: none; border-radius: 4px; padding: 6px 12px; }"
                          "QPushButton:hover { background-color: #505050; }"
                          "QPushButton:pressed { background-color: #2A2D2E; }"
                          "QPushButton:disabled { background-color: #2D2D30; color: #6D6D6D; }";

    // 为快捷功能按钮设置突出显示的样式
    QString highlightButtonStyle = "QPushButton { background-color: #3574F0; color: #FFFFFF; border: none; border-radius: 4px; padding: 6px 12px; }"
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
    insertCodeButton->setVisible(false); // 初始隐藏，只有在有代码可插入时才显示

    // 应用样式 - 为快捷功能按钮应用突出显示的样式
    sendButton->setStyleSheet(buttonStyle);
    clearButton->setStyleSheet(buttonStyle);
    analyzeButton->setStyleSheet(highlightButtonStyle); // 使用突出显示的样式
    codeButton->setStyleSheet(highlightButtonStyle); // 使用突出显示的样式
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

    // 设置区
    settingsLayout = new QHBoxLayout();

    apiKeyLabel = new QLabel(tr("API 密钥:"), mainWidget);
    apiKeyLabel->setStyleSheet("color: #D4D4D4;");
    setApiKeyButton = new QPushButton(tr("设置"), mainWidget);
    setApiKeyButton->setStyleSheet(buttonStyle);

    settingsLayout->addWidget(apiKeyLabel);
    settingsLayout->addWidget(setApiKeyButton);
    settingsLayout->addStretch();

    // 进度条
    progressBar = new QProgressBar(mainWidget);
    progressBar->setRange(0, 0); // 设置为不确定模式
    progressBar->setVisible(false);
    progressBar->setStyleSheet(
        "QProgressBar {"
        "   background-color: #2D2D30;"
        "   color: #D4D4D4;"
        "   border: 1px solid #3F3F46;"
        "   border-radius: 3px;"
        "   text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: #569CD6;"
        "   border-radius: 2px;"
        "}"
    );

    // 添加所有组件到主布局
    mainLayout->addLayout(settingsLayout);
    mainLayout->addWidget(conversationView);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(userInput);
    mainLayout->addLayout(buttonLayout);

    // 设置主窗口部件
    setWidget(mainWidget);

    // 设置初始大小
    resize(400, 600);
}

void AIAssistantWidget::connectSignals() {
    // 连接按钮信号
    connect(sendButton, &QPushButton::clicked, this, &AIAssistantWidget::onSendClicked);
    connect(clearButton, &QPushButton::clicked, this, &AIAssistantWidget::onClearClicked);
    connect(analyzeButton, &QPushButton::clicked, this, &AIAssistantWidget::onAnalyzeClicked);
    connect(codeButton, &QPushButton::clicked, this, &AIAssistantWidget::onCodeClicked);
    connect(debugButton, &QPushButton::clicked, this, &AIAssistantWidget::onDebugClicked);
    connect(showProblemButton, &QPushButton::clicked, this, &AIAssistantWidget::onShowProblemClicked);
    connect(setApiKeyButton, &QPushButton::clicked, this, &AIAssistantWidget::onSetApiKeyClicked);
    connect(insertCodeButton, &QPushButton::clicked, this, &AIAssistantWidget::onInsertCodeClicked);

    // 连接对话管理器信号
    connect(&AIChatManager::getInstance(), &AIChatManager::messageAdded,
            this, &AIAssistantWidget::onMessageAdded);

    // 连接 AI 客户端信号
    connect(&AIClient::getInstance(), &AIClient::requestCompleted,
            this, &AIAssistantWidget::onRequestCompleted);

    // 连接回车键发送
    connect(userInput, &QTextEdit::textChanged, this, [this]() {
        // 检查是否按下了 Ctrl+Enter
        if (userInput->toPlainText().endsWith("\n") &&
            (QApplication::keyboardModifiers() & Qt::ControlModifier)) {
            // 移除末尾的换行符
            QString text = userInput->toPlainText();
            text.chop(1);
            userInput->setPlainText(text);

            // 发送消息
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
            logDebug("Successfully retrieved current code, length: " + QString::number(code.length()) + " characters");
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
    return QString();
}

void AIAssistantWidget::displayMessage(const AIMessage &message) {
    QString text = message.content;
    bool isUser = (message.type == AIMessageType::USER);

    displayMarkdown(text, isUser);
}

void AIAssistantWidget::displayMarkdown(const QString &text, bool isUser) {
    // 设置文本颜色和样式 - 使用深色主题
    QString style = isUser ?
        "background-color: #2D2D30; color: #D4D4D4; border-radius: 5px; padding: 8px; margin: 8px; border-left: 3px solid #569CD6;" :
        "background-color: #252526; color: #D4D4D4; border-radius: 5px; padding: 8px; margin: 8px; border-left: 3px solid #C586C0;";

    QString roleText = isUser ?
        "<div style='margin-bottom: 5px; color: #569CD6;'><b>用户:</b></div>" :
        "<div style='margin-bottom: 5px; color: #C586C0;'><b>AI 助手:</b></div>";

    // 处理Markdown中的代码块，改进显示效果
    QString formattedText = text;
    formattedText.replace(QRegularExpression("```(\\w*)\\s*([\\s\\S]*?)```"),
                         "<pre style='background-color: #1E1E1E; border: 1px solid #3F3F46; border-radius: 3px; padding: 8px; overflow-x: auto; color: #DCDCAA;'>$2</pre>");

    // 处理Markdown中的行内代码
    formattedText.replace(QRegularExpression("`([^`]+)`"),
                         "<code style='background-color: #1E1E1E; padding: 2px 4px; border-radius: 3px; font-family: Consolas, monospace; color: #DCDCAA;'>$1</code>");

    // 创建 HTML
    QString html = QString("<div style='%1'>%2%3</div>")
        .arg(style, roleText, formattedText);

    // 添加到对话历史
    conversationView->append(html);

    // 滚动到底部
    QScrollBar *scrollBar = conversationView->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void AIAssistantWidget::setProgressVisible(bool visible) {
    progressBar->setVisible(visible);

    // 禁用/启用按钮
    sendButton->setEnabled(!visible);
    clearButton->setEnabled(!visible);
    analyzeButton->setEnabled(!visible);
    codeButton->setEnabled(!visible);
    debugButton->setEnabled(!visible);
    userInput->setEnabled(!visible);
}

void AIAssistantWidget::setProblemInfo(
    const QString &title,
    const QString &description,
    const QString &inputDesc,
    const QString &outputDesc,
    const QString &sampleInput,
    const QString &sampleOutput
) {
    // 设置题目信息
    currentTitle = title;
    currentDescription = description;
    currentInputDesc = inputDesc;
    currentOutputDesc = outputDesc;
    currentSampleInput = sampleInput;
    currentSampleOutput = sampleOutput;

    // 显示题目信息
    QString message = tr("已加载题目: %1").arg(title);
    AIMessage systemMsg(AIMessageType::SYSTEM, message);
    displayMessage(systemMsg);
}

void AIAssistantWidget::setUserCode(const QString &code) {
    // 存储当前用户代码，用于调试功能
    userInput->setPlainText(code);
}

void AIAssistantWidget::onSendClicked() {
    QString message = userInput->toPlainText().trimmed();
    if (message.isEmpty()) {
        return;
    }

    // 添加用户消息
    AIMessage userMsg(AIMessageType::USER, message);
    displayMessage(userMsg);

    // 清空输入框
    userInput->clear();

    // 显示进度条
    setProgressVisible(true);

    // 发送请求
    sendAIRequest(message, "用户消息");
}

void AIAssistantWidget::onClearClicked() {
    // 清空对话历史
    AIChatManager::getInstance().clearMessages();
    conversationView->clear();
}

bool AIAssistantWidget::tryGetProblemInfo() {
    logDebug("Attempting to actively get problem information");

    QWidget *parent = this;
    while (parent && !parent->inherits("IDEMainWindow")) {
        parent = parent->parentWidget();
    }

    if (parent) {
        logDebug("Found main window, attempting to get preview window");
        OpenJudgePreviewWidget *preview = parent->findChild<OpenJudgePreviewWidget*>();
        if (preview && preview->isVisible()) {
            logDebug("Found visible preview window, attempting to get problem info");
            if (getProblemInfoFromPreview(preview)) {
                logDebug("Successfully retrieved problem info from preview window");
                return true;
            } else {
                logDebug("Failed to get problem info from preview window");
            }
        } else {
            logDebug("Preview window not visible or not found");
        }
    } else {
        logDebug("Main window not found");
    }

    return false;
}

void AIAssistantWidget::onAnalyzeClicked() {
    qDebug() << "[AI Assistant Debug] Problem analysis button clicked";

    tryGetProblemInfo();

    if (currentTitle.isEmpty() || currentDescription.isEmpty()) {
        qDebug() << "[AI Assistant Debug] Problem information incomplete, cannot generate analysis";
        QMessageBox::warning(this, tr("缺少题目信息"),
            tr("请先打开一个题目或手动输入题目信息。"));
        return;
    }

    QString fullDescription = getFullProblemDescription();
    qDebug() << "[AI Assistant Debug] Retrieved complete problem description, length:" << fullDescription.length() << "characters";

    QString prompt = QString("请详细分析以下算法题目，包括以下内容：\n"
                            "1. 题目的核心问题和考点\n"
                            "2. 解题思路和算法策略\n"
                            "3. 可能的边界情况和注意事项\n"
                            "4. 时间复杂度和空间复杂度分析\n\n"
                            "题目：%1\n\n%2")
                         .arg(currentTitle, fullDescription);

    qDebug() << "[AI Assistant Debug] Generated problem analysis prompt, length:" << prompt.length() << "characters";

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
        QMessageBox::warning(this, tr("缺少题目信息"),
            tr("请先打开一个题目或手动输入题目信息。"));
        return;
    }

    QString fullDescription = getFullProblemDescription();
    qDebug() << "[AI Assistant Debug] Retrieved complete problem description, length:" << fullDescription.length() << "characters";

    QString prompt = QString("请为以下算法题目生成一个完整、高效、易于理解的C++示例代码。代码应该：\n"
                            "1. 包含详细的注释，解释关键步骤和算法思路\n"
                            "2. 处理各种边界情况\n"
                            "3. 使用合适的数据结构和算法\n"
                            "4. 遵循良好的编程实践\n\n"
                            "题目：%1\n\n%2")
                         .arg(currentTitle, fullDescription);

    qDebug() << "[AI Assistant Debug] Generated example code prompt, length:" << prompt.length() << "characters";

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
        QMessageBox::warning(this, tr("缺少代码"),
            tr("请先打开或编写需要调试的代码。"));
        return;
    }

    qDebug() << "[AI Assistant Debug] Retrieved code, length:" << userCode.length() << "characters";

    tryGetProblemInfo();

    if (currentTitle.isEmpty() || currentDescription.isEmpty()) {
        qDebug() << "[AI Assistant Debug] Problem information incomplete, cannot generate debug analysis";
        QMessageBox::warning(this, tr("缺少题目信息"),
            tr("请先打开一个题目或手动输入题目信息。"));
        return;
    }

    QString fullDescription = getFullProblemDescription();
    qDebug() << "[AI Assistant Debug] Retrieved complete problem description, length:" << fullDescription.length() << "characters";

    QString prompt = QString("请帮助调试以下代码，详细分析可能存在的问题：\n"
                            "1. 逻辑错误\n"
                            "2. 边界情况处理\n"
                            "3. 算法实现问题\n"
                            "4. 性能优化建议\n\n"
                            "题目：%1\n\n%2\n\n"
                            "代码：\n```cpp\n%3\n```")
                         .arg(currentTitle, fullDescription, userCode);

    qDebug() << "[AI Assistant Debug] Generated debug prompt, length:" << prompt.length() << "characters";

    AIMessage userMsg(AIMessageType::USER, "请帮我调试这段代码");
    displayMessage(userMsg);

    setProgressVisible(true);

    qDebug() << "[AI Assistant Debug] Sending debug request";
    sendAIRequest(prompt, "调试");
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
        QMessageBox::warning(this, tr("无法访问编辑器"),
            tr("无法找到主窗口，无法插入代码。"));
        return;
    }

    logDebug("Found main window, attempting to get code editor");

    // 获取 CodeTabWidget
    CodeTabWidget *codeTab = parent->findChild<CodeTabWidget*>();
    if (!codeTab) {
        logDebug("无法找到代码编辑器");
        QMessageBox::warning(this, tr("无法访问编辑器"),
            tr("无法找到代码编辑器，无法插入代码。"));
        return;
    }

    logDebug("找到代码编辑器，准备插入代码");

    // 插入代码
    insertGeneratedCode(codeTab);
}

void AIAssistantWidget::onSetApiKeyClicked() {
    bool ok;
    QString apiKey = QInputDialog::getText(this, tr("设置 API 密钥"),
        tr("请输入 DeepSeek API 密钥:"), QLineEdit::Normal,
        AIClient::getInstance().hasApiKey() ? "********" : "", &ok);

    if (ok && !apiKey.isEmpty()) {
        AIClient::getInstance().setApiKey(apiKey);
        QMessageBox::information(this, tr("API 密钥已设置"),
            tr("DeepSeek API 密钥已成功设置。"));
    }
}

void AIAssistantWidget::onMessageAdded(const AIMessage &message) {
    displayMessage(message);
}

QString AIAssistantWidget::extractCodeFromMarkdown(const QString &markdown) {
    logDebug("尝试从Markdown提取代码，Markdown长度: " + QString::number(markdown.length()) + " 字符");

    // 提取代码块，格式为 ```cpp ... ```
    QRegularExpression codeBlockRegex("```cpp\\s*([\\s\\S]*?)```");
    QRegularExpressionMatch match = codeBlockRegex.match(markdown);

    if (match.hasMatch()) {
        QString code = match.captured(1).trimmed();
        logDebug("找到cpp代码块，代码长度: " + QString::number(code.length()) + " 字符");
        return code;
    }

    logDebug("未找到cpp代码块，尝试查找任意代码块");

    // 如果没有找到 cpp 代码块，尝试查找任意代码块
    QRegularExpression anyCodeBlockRegex("```\\w*\\s*([\\s\\S]*?)```");
    match = anyCodeBlockRegex.match(markdown);

    if (match.hasMatch()) {
        QString code = match.captured(1).trimmed();
        logDebug("找到任意代码块，代码长度: " + QString::number(code.length()) + " 字符");
        return code;
    }

    logDebug("未找到任何代码块");
    return QString();
}

void AIAssistantWidget::insertGeneratedCode(CodeTabWidget *codeTabWidget) {
    logDebug("尝试插入生成的代码");

    if (generatedCode.isEmpty()) {
        logDebug("没有可插入的代码");
        QMessageBox::warning(this, tr("没有可插入的代码"),
            tr("请先生成示例代码或从对话中选择代码。"));
        return;
    }

    logDebug("生成的代码长度: " + QString::number(generatedCode.length()) + " 字符");

    CodeEditWidget *editor = codeTabWidget->curEdit();
    if (!editor) {
        logDebug("没有打开的编辑器");
        QMessageBox::warning(this, tr("没有打开的编辑器"),
            tr("请先打开一个代码文件。"));
        return;
    }

    logDebug("找到活动的编辑器，准备插入代码");

    // 插入代码到当前光标位置
    editor->insertPlainText(generatedCode);
    logDebug("代码已插入到编辑器");

    // 显示成功消息
    QMessageBox::information(this, tr("代码已插入"),
        tr("代码已成功插入到编辑器中。"));
    logDebug("显示成功消息");
}

void AIAssistantWidget::onRequestCompleted(bool success, const QString &response) {
    logDebug("AI请求完成回调被触发，成功状态: " + QString(success ? "成功" : "失败"));

    // 隐藏进度条
    setProgressVisible(false);

    if (success) {
        logDebug("AI请求成功，响应长度: " + QString::number(response.length()) + " 字符");

        // 检查响应中是否包含代码块
        QString code = extractCodeFromMarkdown(response);
        if (!code.isEmpty()) {
            logDebug("从响应中提取到代码，长度: " + QString::number(code.length()) + " 字符");
            // 存储生成的代码
            generatedCode = code;
            // 显示插入代码按钮
            insertCodeButton->setVisible(true);
            logDebug("显示插入代码按钮");
        } else {
            logDebug("未从响应中提取到代码");
            // 没有代码块，隐藏插入代码按钮
            insertCodeButton->setVisible(false);
            generatedCode.clear();
        }

        // 添加助手消息并显示
        AIMessage assistantMsg(AIMessageType::ASSISTANT, response);
        displayMessage(assistantMsg);
        logDebug("显示AI助手消息");
    } else {
        logDebug("AI请求失败，错误信息: " + response);

        // 添加错误消息并显示
        QString errorMsg = tr("Error: ") + response;
        AIMessage errorMsg2(AIMessageType::SYSTEM, errorMsg);
        displayMessage(errorMsg2);
        logDebug("显示错误消息");

        // 隐藏插入代码按钮
        insertCodeButton->setVisible(false);
        generatedCode.clear();
    }
}

bool AIAssistantWidget::getProblemInfoFromPreview(OpenJudgePreviewWidget *preview) {
    logDebug("尝试从预览窗口获取题目信息");

    if (!preview) {
        logDebug("预览窗口指针为空");
        return false;
    }

    // 获取标题
    QLabel *titleLabel = preview->findChild<QLabel*>();
    if (!titleLabel) {
        logDebug("无法找到标题标签");
        return false;
    }

    // 获取题目标题
    QString title = titleLabel->text();
    if (title.isEmpty() || title == "题目预览") {
        logDebug("标题为空或为默认值");
        return false;
    }

    logDebug("找到题目标题: " + title);

    // 尝试获取题目内容
    // 查找预览窗口中的文本编辑器，它应该包含题目内容
    QList<QTextEdit*> textEdits = preview->findChildren<QTextEdit*>();
    if (textEdits.isEmpty()) {
        logDebug("无法找到包含题目内容的文本编辑器");
        return false;
    }

    // 获取第一个可见的文本编辑器的内容
    QString content;
    for (QTextEdit* textEdit : textEdits) {
        if (textEdit->isVisible()) {
            content = textEdit->toPlainText();
            logDebug("从可见的文本编辑器获取到内容，长度: " + QString::number(content.length()) + " 字符");
            break;
        }
    }

    if (content.isEmpty()) {
        logDebug("无法获取题目内容");
        // 尝试获取HTML内容
        for (QTextEdit* textEdit : textEdits) {
            if (textEdit->isVisible()) {
                content = textEdit->toHtml();
                logDebug("尝试获取HTML内容，长度: " + QString::number(content.length()) + " 字符");
                break;
            }
        }
    }

    if (content.isEmpty()) {
        logDebug("无法获取任何内容，使用默认描述");
        content = tr("题目内容无法直接获取，请手动输入或从URL加载题目。");
    }

    // 解析题目内容
    logDebug("开始解析题目内容");
    QStringList lines = content.split("\n");
    QString description;
    QString inputDesc;
    QString outputDesc;
    QString sampleInput;
    QString sampleOutput;

    enum ParseState {
        None,
        Description,
        Input,
        Output,
        SampleInput,
        SampleOutput
    };

    ParseState state = None;

    for (const QString &line : lines) {
        if (line.contains("题目描述", Qt::CaseInsensitive)) {
            state = Description;
            logDebug("找到题目描述部分");
            continue;
        } else if (line.contains("输入", Qt::CaseInsensitive) && !line.contains("样例", Qt::CaseInsensitive)) {
            state = Input;
            logDebug("找到输入描述部分");
            continue;
        } else if (line.contains("输出", Qt::CaseInsensitive) && !line.contains("样例", Qt::CaseInsensitive)) {
            state = Output;
            logDebug("找到输出描述部分");
            continue;
        } else if (line.contains("样例输入", Qt::CaseInsensitive)) {
            state = SampleInput;
            logDebug("找到样例输入部分");
            continue;
        } else if (line.contains("样例输出", Qt::CaseInsensitive)) {
            state = SampleOutput;
            logDebug("找到样例输出部分");
            continue;
        }

        // 根据当前状态添加内容
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

    // 如果解析失败，尝试使用整个内容作为描述
    if (description.isEmpty()) {
        logDebug("解析失败，使用整个内容作为描述");
        description = content;
    }

    // 设置题目信息
    currentTitle = title;
    currentDescription = description.trimmed();
    currentInputDesc = inputDesc.trimmed();
    currentOutputDesc = outputDesc.trimmed();
    currentSampleInput = sampleInput.trimmed();
    currentSampleOutput = sampleOutput.trimmed();

    logDebug("题目信息设置完成");
    logCurrentProblemInfo();

    return !currentTitle.isEmpty() && !currentDescription.isEmpty();
}

QCoro::Task<bool> AIAssistantWidget::getProblemInfoFromUrl(const QUrl &url) {
    logDebug("开始从URL获取题目信息: " + url.toString());

    // 使用 ProblemCrawler 获取题目详情
    auto result = co_await ProblemCrawler::getInstance().getProblemDetail(url);

    if (!result.has_value()) {
        QString errorMsg = result.error();
        logDebug("获取题目失败: " + errorMsg);
        QMessageBox::warning(this, tr("获取题目失败"),
            tr("无法获取题目信息：%1").arg(errorMsg));
        co_return false;
    }

    logDebug("成功获取题目信息");

    // 设置题目信息
    ProblemDetail detail = result.value();
    currentTitle = detail.title;
    currentDescription = detail.description;
    currentInputDesc = detail.inputDesc;
    currentOutputDesc = detail.outputDesc;
    currentSampleInput = detail.sampleInput;
    currentSampleOutput = detail.sampleOutput;

    logDebug("题目信息设置完成");
    logCurrentProblemInfo();

    // 显示题目信息
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
        QMessageBox::warning(this, tr("题目信息不完整"),
            tr("当前没有完整的题目信息可以显示。"));
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
    qDebug() << "[AI Assistant Debug]   Title:" << (currentTitle.isEmpty() ? "empty" : currentTitle);
    qDebug() << "[AI Assistant Debug]   Description length:" << currentDescription.length() << "characters";
    qDebug() << "[AI Assistant Debug]   Input description length:" << currentInputDesc.length() << "characters";
    qDebug() << "[AI Assistant Debug]   Output description length:" << currentOutputDesc.length() << "characters";
    qDebug() << "[AI Assistant Debug]   Sample input length:" << currentSampleInput.length() << "characters";
    qDebug() << "[AI Assistant Debug]   Sample output length:" << currentSampleOutput.length() << "characters";
}

void AIAssistantWidget::sendAIRequest(const QString &prompt, const QString &requestType) {
    qDebug() << "[AI Assistant Debug] Sending" << requestType << "request, length:" << prompt.length() << "characters";

    // Use synchronous method to send request
    AIClient::getInstance().sendRequestSync(prompt);

    qDebug() << "[AI Assistant Debug]" << requestType << "request sent, waiting for callback";
}
