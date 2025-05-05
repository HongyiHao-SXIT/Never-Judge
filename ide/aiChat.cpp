#include "aiChat.h"

// 初始化静态实例指针
AIChatManager* AIChatManager::instance = nullptr;

AIChatManager::AIChatManager(QObject *parent) : QObject(parent) {
    maxContextLength = 10; // 默认保留最近10条消息作为上下文
}

AIChatManager &AIChatManager::getInstance() {
    if (!instance) {
        instance = new AIChatManager();
    }
    return *instance;
}

void AIChatManager::addUserMessage(const QString &content) {
    AIMessage message(AIMessageType::USER, content);
    messages.append(message);
    emit messageAdded(message);
}

void AIChatManager::addAssistantMessage(const QString &content) {
    AIMessage message(AIMessageType::ASSISTANT, content);
    messages.append(message);
    emit messageAdded(message);
}

void AIChatManager::addSystemMessage(const QString &content) {
    AIMessage message(AIMessageType::SYSTEM, content);
    messages.append(message);
    emit messageAdded(message);
}

QList<AIMessage> AIChatManager::getMessages() const {
    return messages;
}

QString AIChatManager::getContext(int maxTokens) const {
    // 简单实现，实际应用中可能需要更复杂的 token 计数
    QString context;
    
    // 从最新的消息开始，向前收集上下文
    for (int i = messages.size() - 1; i >= 0; i--) {
        const AIMessage &msg = messages[i];
        QString role;
        
        switch (msg.type) {
            case AIMessageType::USER:
                role = "用户";
                break;
            case AIMessageType::ASSISTANT:
                role = "助手";
                break;
            case AIMessageType::SYSTEM:
                role = "系统";
                break;
        }
        
        QString messageText = role + ": " + msg.content + "\n\n";
        
        // 简单估计 token 数量（中文每字约1-2个token）
        // 如果添加这条消息会超出限制，就停止添加
        if (context.length() + messageText.length() > maxTokens * 2) {
            break;
        }
        
        context = messageText + context;
    }
    
    return context;
}

void AIChatManager::clearMessages() {
    messages.clear();
}

QString AIChatManager::generateProblemAnalysisPrompt(
    const QString &title,
    const QString &description,
    const QString &inputDesc,
    const QString &outputDesc,
    const QString &sampleInput,
    const QString &sampleOutput
) {
    return QString(
        "你是一位编程助手，请分析以下编程题目并给出解题思路：\n\n"
        "题目：%1\n"
        "描述：%2\n"
        "输入：%3\n"
        "输出：%4\n"
        "样例输入：%5\n"
        "样例输出：%6\n\n"
        "请提供以下内容：\n"
        "1. 题目理解和分析\n"
        "2. 解题思路和算法\n"
        "3. 时间复杂度和空间复杂度分析\n"
        "4. 可能的边界情况和处理方法"
    ).arg(title, description, inputDesc, outputDesc, sampleInput, sampleOutput);
}

QString AIChatManager::generateCodeExamplePrompt(
    const QString &title,
    const QString &description,
    const QString &inputDesc,
    const QString &outputDesc,
    const QString &sampleInput,
    const QString &sampleOutput
) {
    return QString(
        "你是一位编程助手，请根据以下编程题目生成示例代码：\n\n"
        "题目：%1\n"
        "描述：%2\n"
        "输入：%3\n"
        "输出：%4\n"
        "样例输入：%5\n"
        "样例输出：%6\n\n"
        "请生成一个完整、高效、易于理解的C++解决方案。包括：\n"
        "1. 必要的注释说明算法思路\n"
        "2. 输入输出处理\n"
        "3. 主要算法实现\n"
        "4. 边界情况处理"
    ).arg(title, description, inputDesc, outputDesc, sampleInput, sampleOutput);
}

QString AIChatManager::generateDebugPrompt(
    const QString &title,
    const QString &description,
    const QString &sampleInput,
    const QString &sampleOutput,
    const QString &userCode
) {
    return QString(
        "你是一位编程助手，请帮助调试以下代码：\n\n"
        "题目：%1\n"
        "描述：%2\n"
        "样例输入：%3\n"
        "样例输出：%4\n\n"
        "用户代码：\n```cpp\n%5\n```\n\n"
        "请分析代码中可能存在的问题：\n"
        "1. 逻辑错误\n"
        "2. 边界情况处理\n"
        "3. 算法实现问题\n"
        "4. 输入输出处理错误\n"
        "5. 提供修复建议"
    ).arg(title, description, sampleInput, sampleOutput, userCode);
}
