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
    setupUI();
    connectSignals();

    // 检查 API 密钥是否已设置
    if (!AIClient::getInstance().hasApiKey()) {
        QMessageBox::information(this, tr("API 密钥未设置"),
            tr("请先设置 DeepSeek API 密钥才能使用 AI 助手功能。"));
    }

    m_codeTab = codeTab;
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

    // 创建按钮并设置统一样式
    QString buttonStyle = "QPushButton { background-color: #3E3E42; color: #D4D4D4; border: none; border-radius: 4px; padding: 6px 12px; }"
                          "QPushButton:hover { background-color: #505050; }"
                          "QPushButton:pressed { background-color: #2A2D2E; }"
                          "QPushButton:disabled { background-color: #2D2D30; color: #6D6D6D; }";

    sendButton = new QPushButton(tr("发送"), mainWidget);
    clearButton = new QPushButton(tr("清空"), mainWidget);
    analyzeButton = new QPushButton(tr("题目解析"), mainWidget);
    codeButton = new QPushButton(tr("示例代码"), mainWidget);
    debugButton = new QPushButton(tr("调试"), mainWidget);
    insertCodeButton = new QPushButton(tr("插入代码"), mainWidget);
    insertCodeButton->setVisible(false); // 初始隐藏，只有在有代码可插入时才显示

    // 应用样式
    sendButton->setStyleSheet(buttonStyle);
    clearButton->setStyleSheet(buttonStyle);
    analyzeButton->setStyleSheet(buttonStyle);
    codeButton->setStyleSheet(buttonStyle);
    debugButton->setStyleSheet(buttonStyle);
    insertCodeButton->setStyleSheet(buttonStyle);

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
    if (m_codeTab) {
        auto currentEdit = m_codeTab->curEdit();
        if (currentEdit) {
            return currentEdit->toPlainText();
        } else {
            qWarning() << "AIAssistantWidget: No active code editor found.";
        }
    } else {
        qWarning() << "AIAssistantWidget: CodeTabWidget pointer is null.";
    }
    return QString(); // Return empty string if code cannot be retrieved
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
    AIClient::getInstance().sendRequest(message);
}

void AIAssistantWidget::onClearClicked() {
    // 清空对话历史
    AIChatManager::getInstance().clearMessages();
    conversationView->clear();
}

void AIAssistantWidget::onAnalyzeClicked() {
    // 检查是否有题目信息
    if (currentTitle.isEmpty() || currentDescription.isEmpty()) {
        QMessageBox::warning(this, tr("缺少题目信息"),
            tr("请先打开一个题目或手动输入题目信息。"));
        return;
    }

    // 获取完整的题目信息
    QString fullDescription = getFullProblemDescription();

    // 生成题目解析提示词
    QString prompt = QString("请详细分析以下算法题目，包括以下内容：\n"
                            "1. 题目的核心问题和考点\n"
                            "2. 解题思路和算法策略\n"
                            "3. 可能的边界情况和注意事项\n"
                            "4. 时间复杂度和空间复杂度分析\n\n"
                            "题目：%1\n\n%2")
                         .arg(currentTitle, fullDescription);

    // 添加用户消息并显示
    AIMessage userMsg(AIMessageType::USER, "请分析这道题目: " + currentTitle);
    displayMessage(userMsg);

    // 显示进度条
    setProgressVisible(true);

    // 发送请求
    AIClient::getInstance().sendRequest(prompt);
}

void AIAssistantWidget::onCodeClicked() {
    // 检查是否有题目信息
    if (currentTitle.isEmpty() || currentDescription.isEmpty()) {
        QMessageBox::warning(this, tr("缺少题目信息"),
            tr("请先打开一个题目或手动输入题目信息。"));
        return;
    }

    // 获取完整的题目信息
    QString fullDescription = getFullProblemDescription();

    // 生成示例代码提示词
    QString prompt = QString("请为以下算法题目生成一个完整、高效、易于理解的C++示例代码。代码应该：\n"
                            "1. 包含详细的注释，解释关键步骤和算法思路\n"
                            "2. 处理各种边界情况\n"
                            "3. 使用合适的数据结构和算法\n"
                            "4. 遵循良好的编程实践\n\n"
                            "题目：%1\n\n%2")
                         .arg(currentTitle, fullDescription);

    // 添加用户消息并显示
    AIMessage userMsg(AIMessageType::USER, "请为这道题目生成示例代码: " + currentTitle);
    displayMessage(userMsg);

    // 显示进度条
    setProgressVisible(true);

    // 发送请求
    AIClient::getInstance().sendRequest(prompt);
}

void AIAssistantWidget::onDebugClicked() {
    QString userCode = getCurrentCode();
    if (userCode.isEmpty()) {
        QMessageBox::warning(this, tr("缺少代码"),
            tr("请先打开或编写需要调试的代码。"));
        return;
    }

    // 检查是否有题目信息
    if (currentTitle.isEmpty() || currentDescription.isEmpty()) {
        QMessageBox::warning(this, tr("缺少题目信息"),
            tr("请先打开一个题目或手动输入题目信息。"));
        return;
    }

    // 获取完整的题目信息
    QString fullDescription = getFullProblemDescription();

    // 生成调试提示词
    QString prompt = QString("请帮助调试以下代码，详细分析可能存在的问题：\n"
                            "1. 逻辑错误\n"
                            "2. 边界情况处理\n"
                            "3. 算法实现问题\n"
                            "4. 性能优化建议\n\n"
                            "题目：%1\n\n%2\n\n"
                            "代码：\n```cpp\n%3\n```")
                         .arg(currentTitle, fullDescription, userCode);

    // 添加用户消息并显示
    AIMessage userMsg(AIMessageType::USER, "请帮我调试这段代码");
    displayMessage(userMsg);

    // 显示进度条
    setProgressVisible(true);

    // 发送请求
    AIClient::getInstance().sendRequest(prompt);
}

void AIAssistantWidget::onInsertCodeClicked() {
    // 查找主窗口
    QWidget *parent = this;
    while (parent && !parent->inherits("IDEMainWindow")) {
        parent = parent->parentWidget();
    }

    if (!parent) {
        QMessageBox::warning(this, tr("无法访问编辑器"),
            tr("无法找到主窗口，无法插入代码。"));
        return;
    }

    // 获取 CodeTabWidget
    CodeTabWidget *codeTab = parent->findChild<CodeTabWidget*>();
    if (!codeTab) {
        QMessageBox::warning(this, tr("无法访问编辑器"),
            tr("无法找到代码编辑器，无法插入代码。"));
        return;
    }

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
    // 提取代码块，格式为 ```cpp ... ```
    QRegularExpression codeBlockRegex("```cpp\\s*([\\s\\S]*?)```");
    QRegularExpressionMatch match = codeBlockRegex.match(markdown);

    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }

    // 如果没有找到 cpp 代码块，尝试查找任意代码块
    QRegularExpression anyCodeBlockRegex("```\\w*\\s*([\\s\\S]*?)```");
    match = anyCodeBlockRegex.match(markdown);

    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }

    return QString();
}

void AIAssistantWidget::insertGeneratedCode(CodeTabWidget *codeTabWidget) {
    if (generatedCode.isEmpty()) {
        QMessageBox::warning(this, tr("没有可插入的代码"),
            tr("请先生成示例代码或从对话中选择代码。"));
        return;
    }

    CodeEditWidget *editor = codeTabWidget->curEdit();
    if (!editor) {
        QMessageBox::warning(this, tr("没有打开的编辑器"),
            tr("请先打开一个代码文件。"));
        return;
    }

    // 插入代码到当前光标位置
    editor->insertPlainText(generatedCode);

    // 显示成功消息
    QMessageBox::information(this, tr("代码已插入"),
        tr("代码已成功插入到编辑器中。"));
}

void AIAssistantWidget::onRequestCompleted(bool success, const QString &response) {
    // 隐藏进度条
    setProgressVisible(false);

    if (success) {
        // 检查响应中是否包含代码块
        QString code = extractCodeFromMarkdown(response);
        if (!code.isEmpty()) {
            // 存储生成的代码
            generatedCode = code;
            // 显示插入代码按钮
            insertCodeButton->setVisible(true);
        } else {
            // 没有代码块，隐藏插入代码按钮
            insertCodeButton->setVisible(false);
            generatedCode.clear();
        }

        // 添加助手消息并显示
        AIMessage assistantMsg(AIMessageType::ASSISTANT, response);
        displayMessage(assistantMsg);
    } else {
        // 添加错误消息并显示
        QString errorMsg = tr("Error: ") + response;
        AIMessage errorMsg2(AIMessageType::SYSTEM, errorMsg);
        displayMessage(errorMsg2);

        // 隐藏插入代码按钮
        insertCodeButton->setVisible(false);
        generatedCode.clear();
    }
}

bool AIAssistantWidget::getProblemInfoFromPreview(OpenJudgePreviewWidget *preview) {
    if (!preview) {
        return false;
    }

    // 由于我们无法直接访问 curPreview() 方法，我们需要使用其他方式获取题目信息
    // 这里我们简单地从标题栏获取题目标题，内容则无法获取
    QLabel *titleLabel = preview->findChild<QLabel*>();
    if (!titleLabel) {
        return false;
    }

    // 获取题目标题
    QString title = titleLabel->text();
    if (title.isEmpty() || title == "题目预览") {
        return false;
    }

    // 由于无法直接获取内容，我们设置一个默认描述
    QString content = tr("题目内容无法直接获取，请手动输入或从URL加载题目。");

    // 解析题目内容
    // 简单实现：查找关键字段
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
            continue;
        } else if (line.contains("输入", Qt::CaseInsensitive) && !line.contains("样例", Qt::CaseInsensitive)) {
            state = Input;
            continue;
        } else if (line.contains("输出", Qt::CaseInsensitive) && !line.contains("样例", Qt::CaseInsensitive)) {
            state = Output;
            continue;
        } else if (line.contains("样例输入", Qt::CaseInsensitive)) {
            state = SampleInput;
            continue;
        } else if (line.contains("样例输出", Qt::CaseInsensitive)) {
            state = SampleOutput;
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

    // 设置题目信息
    currentTitle = title;
    currentDescription = description.trimmed();
    currentInputDesc = inputDesc.trimmed();
    currentOutputDesc = outputDesc.trimmed();
    currentSampleInput = sampleInput.trimmed();
    currentSampleOutput = sampleOutput.trimmed();

    return !currentTitle.isEmpty() && !currentDescription.isEmpty();
}

QCoro::Task<bool> AIAssistantWidget::getProblemInfoFromUrl(const QUrl &url) {
    // 使用 ProblemCrawler 获取题目详情
    auto result = co_await ProblemCrawler::getInstance().getProblemDetail(url);

    if (!result.has_value()) {
        QMessageBox::warning(this, tr("获取题目失败"),
            tr("无法获取题目信息：%1").arg(result.error()));
        co_return false;
    }

    // 设置题目信息
    ProblemDetail detail = result.value();
    currentTitle = detail.title;
    currentDescription = detail.description;
    currentInputDesc = detail.inputDesc;
    currentOutputDesc = detail.outputDesc;
    currentSampleInput = detail.sampleInput;
    currentSampleOutput = detail.sampleOutput;

    // 显示题目信息
    QString message = tr("已加载题目: %1").arg(currentTitle);
    AIMessage systemMsg(AIMessageType::SYSTEM, message);
    displayMessage(systemMsg);

    co_return true;
}

QString AIAssistantWidget::getFullProblemDescription() const {
    // 构建完整的题目信息
    QString fullDescription = currentDescription;

    if (!currentInputDesc.isEmpty()) {
        fullDescription += "\n\n输入描述：\n" + currentInputDesc;
    }
    if (!currentOutputDesc.isEmpty()) {
        fullDescription += "\n\n输出描述：\n" + currentOutputDesc;
    }
    if (!currentSampleInput.isEmpty()) {
        fullDescription += "\n\n样例输入：\n" + currentSampleInput;
    }
    if (!currentSampleOutput.isEmpty()) {
        fullDescription += "\n\n样例输出：\n" + currentSampleOutput;
    }

    return fullDescription;
}
