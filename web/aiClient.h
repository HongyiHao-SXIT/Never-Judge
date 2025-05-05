#ifndef AICLIENT_H
#define AICLIENT_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <expected>
#include <qcoro/qcoronetworkreply.h>
#include <qcorotask.h>

/**
 * @brief DeepSeek API 客户端类
 * 
 * 负责与 DeepSeek API 通信，发送请求和处理响应
 */
class AIClient : public QObject {
    Q_OBJECT

private:
    QString apiKey;
    QString apiEndpoint;
    QNetworkAccessManager nam;

    // 单例模式
    explicit AIClient(QObject *parent = nullptr);
    static AIClient *instance;

public:
    /**
     * @brief 获取 AIClient 单例
     * @return AIClient 实例
     */
    static AIClient &getInstance();

    /**
     * @brief 设置 API 密钥
     * @param key DeepSeek API 密钥
     */
    void setApiKey(const QString &key);

    /**
     * @brief 检查是否已设置 API 密钥
     * @return 是否已设置 API 密钥
     */
    bool hasApiKey() const;

    /**
     * @brief 向 DeepSeek API 发送请求
     * @param prompt 提示词
     * @param maxTokens 最大生成 token 数
     * @param temperature 温度参数
     * @return 成功返回响应文本，失败返回错误信息
     */
    QCoro::Task<std::expected<QString, QString>> sendRequest(
        const QString &prompt,
        int maxTokens = 2048,
        double temperature = 0.7
    );

    /**
     * @brief 构造 API 请求 JSON
     * @param prompt 提示词
     * @param maxTokens 最大生成 token 数
     * @param temperature 温度参数
     * @return 请求 JSON 对象
     */
    QJsonObject buildRequestJson(
        const QString &prompt,
        int maxTokens,
        double temperature
    );

signals:
    /**
     * @brief API 调用完成信号
     * @param success 是否成功
     * @param response 响应文本或错误信息
     */
    void requestCompleted(bool success, const QString &response);
};

#endif // AICLIENT_H
