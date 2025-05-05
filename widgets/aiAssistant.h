#ifndef AIASSISTANT_H
#define AIASSISTANT_H

#include <QDockWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QProgressBar>

#include "../ide/aiChat.h"
#include "code.h"
#include "preview.h"

/**
 * @brief AI 助手对话面板类
 * 
 * 提供与 AI 助手交互的用户界面
 */
class AIAssistantWidget : public QDockWidget {
    Q_OBJECT
    
private:
    QWidget *mainWidget;
    QVBoxLayout *mainLayout;
    
    // 对话历史显示区
    QTextBrowser *conversationView;
    
    // 用户输入区
    QTextEdit *userInput;
    
    // 功能按钮区
    QHBoxLayout *buttonLayout;
    QPushButton *sendButton;
    QPushButton *clearButton;
    QPushButton *analyzeButton;
    QPushButton *codeButton;
    QPushButton *debugButton;
    QPushButton *insertCodeButton;
    
    // 设置区
    QHBoxLayout *settingsLayout;
    QLabel *apiKeyLabel;
    QPushButton *setApiKeyButton;
    
    // 进度条
    QProgressBar *progressBar;
    
    // 当前处理的题目信息
    QString currentTitle;
    QString currentDescription;
    QString currentInputDesc;
    QString currentOutputDesc;
    QString currentSampleInput;
    QString currentSampleOutput;
    
    // 当前生成的代码
    QString generatedCode;
    
    /**
     * @brief 设置 UI 组件
     */
    void setupUI();
    
    /**
     * @brief 连接信号和槽
     */
    void connectSignals();
    
    /**
     * @brief 显示消息
     * @param message AI 消息
     */
    void displayMessage(const AIMessage &message);
    
    /**
     * @brief 显示 Markdown 格式文本
     * @param text Markdown 文本
     * @param isUser 是否是用户消息
     */
    void displayMarkdown(const QString &text, bool isUser = false);
    
    /**
     * @brief 设置进度条状态
     * @param visible 是否可见
     */
    void setProgressVisible(bool visible);
    
public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit AIAssistantWidget(QWidget *parent = nullptr);
    
    /**
     * @brief 设置当前题目信息
     * @param title 题目标题
     * @param description 题目描述
     * @param inputDesc 输入描述
     * @param outputDesc 输出描述
     * @param sampleInput 样例输入
     * @param sampleOutput 样例输出
     */
    void setProblemInfo(
        const QString &title,
        const QString &description,
        const QString &inputDesc,
        const QString &outputDesc,
        const QString &sampleInput,
        const QString &sampleOutput
    );
    
    /**
     * @brief 设置用户代码
     * @param code 用户代码
     */
    void setUserCode(const QString &code);
    
    /**
     * @brief 将生成的代码插入到当前编辑器
     * @param codeTabWidget 代码标签页组件
     */
    void insertGeneratedCode(CodeTabWidget *codeTabWidget);
    
    /**
     * @brief 从 Markdown 文本中提取代码块
     * @param markdown Markdown 文本
     * @return 提取的代码
     */
    QString extractCodeFromMarkdown(const QString &markdown);
    
    /**
     * @brief 从 OpenJudge 预览组件获取当前题目信息
     * @param preview OpenJudge 预览组件
     * @return 是否成功获取题目信息
     */
    bool getProblemInfoFromPreview(OpenJudgePreviewWidget *preview);
    
    /**
     * @brief 从题目 URL 解析题目信息
     * @param url 题目 URL
     * @return 是否成功解析题目信息
     */
    QCoro::Task<bool> getProblemInfoFromUrl(const QUrl &url);

private slots:
    /**
     * @brief 发送按钮点击处理
     */
    void onSendClicked();
    
    /**
     * @brief 清空按钮点击处理
     */
    void onClearClicked();
    
    /**
     * @brief 分析按钮点击处理
     */
    void onAnalyzeClicked();
    
    /**
     * @brief 代码按钮点击处理
     */
    void onCodeClicked();
    
    /**
     * @brief 调试按钮点击处理
     */
    void onDebugClicked();
    
    /**
     * @brief 设置 API 密钥按钮点击处理
     */
    void onSetApiKeyClicked();
    
    /**
     * @brief 消息添加处理
     * @param message 新添加的消息
     */
    void onMessageAdded(const AIMessage &message);
    
    /**
     * @brief API 请求完成处理
     * @param success 是否成功
     * @param response 响应文本或错误信息
     */
    void onRequestCompleted(bool success, const QString &response);
};

#endif // AIASSISTANT_H
