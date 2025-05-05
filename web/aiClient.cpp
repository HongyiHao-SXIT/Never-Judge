#include "aiClient.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonArray>
#include <QDebug>

#include "../util/file.h"

// 初始化静态实例指针
AIClient* AIClient::instance = nullptr;

AIClient::AIClient(QObject *parent) : QObject(parent) {
    // 从配置中加载 API 密钥和端点
    apiEndpoint = "https://api.deepseek.com/v1/chat/completions";

    // 尝试从配置中加载 API 密钥
    apiKey = Configs::instance().get("aiAssistant.apiKey").toString();
}

AIClient &AIClient::getInstance() {
    if (!instance) {
        instance = new AIClient();
    }
    return *instance;
}

void AIClient::setApiKey(const QString &key) {
    apiKey = key;

    // 保存 API 密钥到配置
    Configs::instance().set("aiAssistant.apiKey", key);
}

bool AIClient::hasApiKey() const {
    return !apiKey.isEmpty();
}

QJsonObject AIClient::buildRequestJson(const QString &prompt, int maxTokens, double temperature) {
    QJsonObject message;
    message["role"] = "user";
    message["content"] = prompt;

    QJsonArray messages;
    messages.append(message);

    QJsonObject json;
    json["model"] = "deepseek-coder";  // 使用 DeepSeek Coder 模型
    json["messages"] = messages;
    json["max_tokens"] = maxTokens;
    json["temperature"] = temperature;

    return json;
}

QCoro::Task<std::expected<QString, QString>> AIClient::sendRequest(
    const QString &prompt,
    int maxTokens,
    double temperature
) {
    if (!hasApiKey()) {
        co_return std::unexpected("API 密钥未设置");
    }

    QNetworkRequest request;
    request.setUrl(QUrl(apiEndpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    QJsonObject requestJson = buildRequestJson(prompt, maxTokens, temperature);
    QByteArray requestData = QJsonDocument(requestJson).toJson();

    QNetworkReply *reply = co_await nam.post(request, requestData);

    if (reply->error()) {
        QString errorMsg = reply->errorString();
        delete reply;
        co_return std::unexpected(errorMsg);
    }

    QByteArray responseData = reply->readAll();
    delete reply;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        co_return std::unexpected("JSON 解析错误: " + parseError.errorString());
    }

    QJsonObject responseJson = doc.object();

    // 检查是否有错误
    if (responseJson.contains("error")) {
        QString errorMsg = responseJson["error"].toObject()["message"].toString();
        co_return std::unexpected(errorMsg);
    }

    // 提取生成的文本
    QJsonArray choices = responseJson["choices"].toArray();
    if (choices.isEmpty()) {
        co_return std::unexpected("API 返回的响应中没有内容");
    }

    QString responseText = choices[0].toObject()["message"].toObject()["content"].toString();
    emit requestCompleted(true, responseText);

    co_return responseText;
}
