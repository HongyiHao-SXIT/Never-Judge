#ifndef AICHAT_H
#define AICHAT_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDateTime>

/**
 * @brief AI 消息类型枚举
 */
enum class AIMessageType {
    USER,       // 用户消息
    ASSISTANT,  // AI 助手消息
    SYSTEM      // 系统消息
};

/**
 * @brief AI 消息结构
 */
struct AIMessage {
    AIMessageType type;    // 消息类型
    QString content;       // 消息内容
    QDateTime timestamp;   // 时间戳
    
    AIMessage(AIMessageType type, const QString &content)
        : type(type), content(content), timestamp(QDateTime::currentDateTime()) {}
};

/**
 * @brief AI 对话管理器类
 * 
 * 负责管理对话历史、上下文和会话状态
 */
class AIChatManager : public QObject {
    Q_OBJECT
    
private:
    QList<AIMessage> messages;  // 对话历史
    int maxContextLength;       // 最大上下文长度
    
    // 单例模式
    explicit AIChatManager(QObject *parent = nullptr);
    static AIChatManager *instance;
    
public:
    /**
     * @brief 获取 AIChatManager 单例
     * @return AIChatManager 实例
     */
    static AIChatManager &getInstance();
    
    /**
     * @brief 添加用户消息
     * @param content 消息内容
     */
    void addUserMessage(const QString &content);
    
    /**
     * @brief 添加助手消息
     * @param content 消息内容
     */
    void addAssistantMessage(const QString &content);
    
    /**
     * @brief 添加系统消息
     * @param content 消息内容
     */
    void addSystemMessage(const QString &content);
    
    /**
     * @brief 获取完整对话历史
     * @return 对话历史列表
     */
    QList<AIMessage> getMessages() const;
    
    /**
     * @brief 获取当前上下文
     * @param maxTokens 最大 token 数
     * @return 格式化的上下文字符串
     */
    QString getContext(int maxTokens = 4096) const;
    
    /**
     * @brief 清空对话历史
     */
    void clearMessages();
    
    /**
     * @brief 生成题目解析提示词
     * @param title 题目标题
     * @param description 题目描述
     * @param inputDesc 输入描述
     * @param outputDesc 输出描述
     * @param sampleInput 样例输入
     * @param sampleOutput 样例输出
     * @return 格式化的提示词
     */
    QString generateProblemAnalysisPrompt(
        const QString &title,
        const QString &description,
        const QString &inputDesc,
        const QString &outputDesc,
        const QString &sampleInput,
        const QString &sampleOutput
    );
    
    /**
     * @brief 生成示例代码提示词
     * @param title 题目标题
     * @param description 题目描述
     * @param inputDesc 输入描述
     * @param outputDesc 输出描述
     * @param sampleInput 样例输入
     * @param sampleOutput 样例输出
     * @return 格式化的提示词
     */
    QString generateCodeExamplePrompt(
        const QString &title,
        const QString &description,
        const QString &inputDesc,
        const QString &outputDesc,
        const QString &sampleInput,
        const QString &sampleOutput
    );
    
    /**
     * @brief 生成代码调试提示词
     * @param title 题目标题
     * @param description 题目描述
     * @param sampleInput 样例输入
     * @param sampleOutput 样例输出
     * @param userCode 用户代码
     * @return 格式化的提示词
     */
    QString generateDebugPrompt(
        const QString &title,
        const QString &description,
        const QString &sampleInput,
        const QString &sampleOutput,
        const QString &userCode
    );
    
signals:
    /**
     * @brief 消息添加信号
     * @param message 新添加的消息
     */
    void messageAdded(const AIMessage &message);
};

#endif // AICHAT_H
