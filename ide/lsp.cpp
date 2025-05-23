#include "lsp.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <qcoreapplication.h>
#include <qcoro/qcoroprocess.h>

// FIXME: this is linux only...?

LSPUri LSPUri::fromQUrl(const QUrl &url) { return {QString("file://%1").arg(url.toEncoded())}; }

QUrl LSPUri::toQUrl() const { return QUrl::fromPercentEncoding(uri.mid(7).toUtf8()); }

QJsonObject LSPTextDocument::toJson() const {
    static QMap<Language, QString> languageMap{
            {Language::C, "c"}, {Language::CPP, "cpp"}, {Language::PYTHON, "python"}};

    QJsonObject obj;
    obj["uri"] = uri.uri;
    obj["version"] = version;
    obj["languageId"] = languageMap.value(language, "");
    if (text.has_value()) {
        obj["text"] = text.value();
    }
    return obj;
}

QPair<QString, QJsonValue> LSPTextDocument::toEntry() const { return {"textDocument", toJson()}; }

void LSPPosition::readJson(QJsonObject json) {
    line = json["line"].toInt();
    character = json["character"].toInt();
}

void LSPRange::readJson(QJsonObject json) {
    start.readJson(json["start"].toObject());
    end.readJson(json["end"].toObject());
}

QJsonObject LSPPosition::toJson() const { return {{"line", line}, {"character", character}}; }

QPair<QString, QJsonValue> LSPPosition::toEntry() const { return {"position", toJson()}; }

void InitializeResponse::parseJson(const QJsonObject &response) {
    // check the necessary keys
    ok = response.contains("id") && response.contains("jsonrpc") && response.contains("result");
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

void DefinitionResponse::parseJson(const QJsonObject &response) {
    items.clear();
    if (!response.contains("result")) {
        return;
    }
    // qDebug() << response;
    const auto &jsonItems = response["result"].toArray();
    for (auto jsonItem: jsonItems) {
        auto obj = jsonItem.toObject();
        LSPUri uri(obj["uri"].toString());
        auto rangeObj = obj["range"].toObject();
        LSPRange range{};
        range.readJson(rangeObj);
        items.emplace_back(uri, range);
    }
}

bool isNotification(LSPRequestMethod method) { return method == DidOpen; }

QString LanguageServer::commentPrefix(Language language) {
    switch (language) {
        case Language::C:
        case Language::CPP:
            return "//";
        case Language::PYTHON:
            return "#";
        default:
            return "";
    }
}

/* Language Server */
const QMap<LSPRequestMethod, QString> methodMap = {
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


void LanguageServer::sendRequest(LSPRequestMethod method, const QJsonObject &payload) const {
    static int requestId = 0;

    QJsonObject request;
    request["jsonrpc"] = "2.0";
    if (!isNotification(method)) {
        request["id"] = ++requestId;
    }
    request["method"] = methodMap[method];
    request["params"] = payload;

    // write the request to the process stdin
    QJsonDocument doc(request);
    auto data = doc.toJson().replace("\n", "");
    auto header = QString("Content-Length: %1\r\n\r\n").arg(QString::number(data.size()));
    auto content = header.toUtf8() + data;
    // qDebug() << content;
    process->write(content);

    // TODO: return the request ID and make sure of the corresponding between request and response
}

template<std::derived_from<LSPResponse> R>
QCoro::Task<R> LanguageServer::waitResponse() const {
    auto pw = qCoro(process); // Coroutine wrapper for QProcess
    co_await pw.waitForReadyRead(3000);
    while (true) {
        QByteArray line = co_await pw.readLine();
        if (line.startsWith("Content-Length:")) {
            int length = line.mid(16).toInt();

            // Skip empty line
            line = co_await pw.readLine();
            if (line.startsWith("Content-Type:")) {
                // pylsp has the content type line
                co_await pw.readLine();
            }

            QByteArray data = co_await pw.read(length);
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

QCoro::Task<> LanguageServer::didOpen(const LSPTextDocument &document) const {
    QJsonObject payload = {document.toEntry()};
    sendRequest(DidOpen, payload);
    co_return;
}

QCoro::Task<CompletionResponse> LanguageServer::completion(const LSPTextDocument &document,
                                                           const LSPPosition &position) const {
    QJsonObject payload = {document.toEntry(), position.toEntry()};
    sendRequest(Completion, payload);
    auto response = co_await waitResponse<CompletionResponse>();
    co_return response;
}

QCoro::Task<DefinitionResponse> LanguageServer::definition(const LSPTextDocument &document,
                                                           const LSPPosition &position) const {
    QJsonObject payload = {document.toEntry(), position.toEntry()};
    sendRequest(Definition, payload);
    auto response = co_await waitResponse<DefinitionResponse>();
    co_return response;
};

ClangdLanguageServer *ClangdLanguageServer::instance = nullptr;

QCoro::Task<ClangdLanguageServer *> ClangdLanguageServer::getServer() {
    if (instance == nullptr) {
        instance = new ClangdLanguageServer();
        qDebug() << "ClangdLanguageServer: Server created";
        co_await instance->start();
    }
    co_return instance;
}

QCoro::Task<> ClangdLanguageServer::start() {
    QString serverName = "clangd";
    QStringList serverParams = {"--log=verbose"};
    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::SeparateChannels);
    co_await qCoro(process).start(serverName, serverParams);
    co_return;
}

PylspLanguageServer *PylspLanguageServer::instance = nullptr;

QCoro::Task<PylspLanguageServer *> PylspLanguageServer::getServer() {
    if (instance == nullptr) {
        instance = new PylspLanguageServer();
        qDebug() << "PylspLanguageServer: Server created";
        co_await instance->start();
    }
    co_return instance;
}

QCoro::Task<> PylspLanguageServer::start() {
    // TODO: use pyright later?
    QString serverName = "pylsp";
    QStringList serverParams = {"-vv"};
    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::SeparateChannels);
    co_await qCoro(process).start(serverName, serverParams);
    co_return;
}

QCoro::Task<LanguageServer *> LanguageServers::get(Language language) {
    switch (language) {
        case Language::C:
        case Language::CPP:
            co_return co_await ClangdLanguageServer::getServer();
        case Language::PYTHON:
            co_return co_await PylspLanguageServer::getServer();
        default:
            co_return nullptr;
    }
}
