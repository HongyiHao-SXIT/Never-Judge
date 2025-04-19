#include "lsp.h"

#include <QJsonArray>
#include <qcoreapplication.h>
#include <qcoro/qcoroprocess.h>

QJsonObject LSPTextDocument::toJson() const {
    QJsonObject obj;
    obj["uri"] = uri;
    if (text.has_value()) {
        obj["text"] = text.value();
    }
    return obj;
}

std::pair<QString, QJsonValue> LSPTextDocument::toEntry() const { return {"textDocument", toJson()}; }

QJsonObject LSPPosition::toJson() const { return {{"line", line}, {"character", character}}; }

std::pair<QString, QJsonValue> LSPPosition::toEntry() const { return {"position", toJson()}; }

void InitializeResponse::parseJson(const QJsonObject &response) {
    // check the necessary keys
    ok = response.contains("id") && response.contains("jsonrpc") && response.contains("result");
}

void DidOpenResponse::parseJson(const QJsonObject &response) {
    ok = response.contains("result") && response["result"].isNull();
}


void CompletionResponse::parseJson(const QJsonObject &response) {
    auto result = response["result"].toObject();
    incomplete = result["isIncomplete"].toBool();
    const auto &jsonItems = result["items"].toArray();
    for (auto jsonItem: jsonItems) {
        auto item = jsonItem.toObject();
        auto label = item["label"].toString();
        auto kind = static_cast<CompletionItem::ItemKind>(item["kind"].toInt());
        auto sortText = item["sortText"].toString();
        auto insertText = item["insertText"].toString();
        items.emplace_back(label, kind, sortText, insertText);
    }
}

/* Language Server */

void LanguageServer::sendRequest(LSPRequestMethod method, const QJsonObject &payload) const {
    static const QMap<LSPRequestMethod, QString> methodMap = {
            {Initialize, "initialize"},
            {Shutdown, "shutdown"},
            {DidOpen, "textDocument/didOpen"},
            {Completion, "textDocument/completion"},
            {Definition, "textDocument/definition"},
            {Hover, "textDocument/hover"},
            {References, "textDocument/references"},
            {Formatting, "textDocument/formatting"},
            {Rename, "textDocument/rename"},
            {PublishDiagnostics, "textDocument/publishDiagnostics"},
            {DocumentSymbol, "textDocument/documentSymbol"}};
    static int requestId = 0;

    QJsonObject request;
    request["jsonrpc"] = "2.0";
    request["id"] = ++requestId;
    request["method"] = methodMap[method];
    request["params"] = payload;

    // write the request to the process stdin
    QJsonDocument doc(request);
    auto data = doc.toJson().replace("\n", "");
    auto header = QString("Content-Length: %1\r\n\r\n").arg(QString::number(data.size()));
    auto content = header.toUtf8() + data;
    // qDebug() << content;
    process->write(content);
}
template<std::derived_from<LSPResponse> R>
QCoro::Task<R> LanguageServer::waitResponse() const {
    while (true) {
        co_await qCoro(process).waitForReadyRead(3000);
        QByteArray line = process->readLine();

        if (line.startsWith("Content-Length:")) {
            int length = line.mid(16).toInt();
            process->readLine(); // Skip content-type line
            process->readLine(); // Skip empty line
            QByteArray data = process->read(length);
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject json = doc.object();

            if (!json.contains("id")) {
                // skip notifications
                // if (json["method"] == "textDocument/publishDiagnostics") {
                // qDebug() << "Received diagnostics:" << json["params"];
                // }
                continue;
            }

            R response;
            if (json.contains("error")) {
                qWarning() << "Response error:" << json["error"];
            } else {
                response.parseJson(json);
            }
            co_return response;
        }
    }
}

QCoro::Task<InitializeResponse> LanguageServer::initialize(const QString &rootUri,
                                                           const QJsonObject &capabilities) const {
    QJsonObject payload = {
            {"processId", QCoreApplication::applicationPid()},
            {"rootUri", rootUri},
            {"capabilities", capabilities},
    };
    sendRequest(Initialize, payload);
    auto response = co_await waitResponse<InitializeResponse>();
    co_return response;
}

QCoro::Task<DidOpenResponse> LanguageServer::didOpen(const LSPTextDocument &document) const {
    QJsonObject payload = {document.toEntry()};
    sendRequest(DidOpen, payload);
    auto response = co_await waitResponse<DidOpenResponse>();
    co_return response;
}

QCoro::Task<CompletionResponse> LanguageServer::completion(const LSPTextDocument &document,
                                                           const LSPPosition &position) const {
    QJsonObject payload = {document.toEntry(), position.toEntry()};
    sendRequest(Completion, payload);
    auto response = co_await waitResponse<CompletionResponse>();
    co_return response;
}

PythonLanguageServer *PythonLanguageServer::instance = nullptr;

QCoro::Task<PythonLanguageServer *> PythonLanguageServer::getServer() {
    if (instance == nullptr) {
        instance = new PythonLanguageServer();
        co_await instance->start();
    }
    co_return instance;
}

QCoro::Task<> PythonLanguageServer::start() {
    QString serverName = "pylsp";
    QStringList serverParams = {"-vv"};
    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::SeparateChannels);
    co_await qCoro(process).start(serverName, serverParams);
    co_return;
}

QCoro::Task<LanguageServer *> LanguageServers::get(Language language) {
    switch (language) {
        case Language::PYTHON:
            co_return co_await PythonLanguageServer::getServer();
        default:
            co_return nullptr;
    }
}
