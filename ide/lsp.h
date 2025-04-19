#ifndef LSP_H
#define LSP_H

#include <QJsonObject>
#include <QMutex>
#include <QProcess>
#include <qcorotask.h>

#include "language.h"

/* Basic request and response */

enum LSPRequestMethod {
    Initialize,
    Shutdown,
    DidOpen,
    Completion,
    Definition,
    Hover,
    References,
    Formatting,
    Rename,
    PublishDiagnostics,
    DocumentSymbol
};

struct LSPTextDocument {
    QString uri;
    std::optional<QString> text = std::nullopt;

    QJsonObject toJson() const;
    std::pair<QString, QJsonValue> toEntry() const;
};

struct LSPPosition {
    int line;
    int character;

    QJsonObject toJson() const;
    std::pair<QString, QJsonValue> toEntry() const;
};

struct LSPResponse {
    virtual ~LSPResponse() = default;
    virtual void parseJson(const QJsonObject &response) = 0;
};

/* Server implements */

struct InitializeResponse : LSPResponse {
    bool ok = false;
    void parseJson(const QJsonObject &response) override;
};

struct ShutdownResponse : LSPResponse {
    void parseJson(const QJsonObject &response) override;
};

struct DidOpenResponse : LSPResponse {
    bool ok;
    void parseJson(const QJsonObject &response) override;
};

struct CompletionItem {
    enum ItemKind {
        Text = 1,
        Method = 2,
        Function = 3,
        Constructor = 4,
        Field = 5,
        Variable = 6,
        Class = 7,
        Interface = 8,
        Module = 9,
        Property = 10,
        Unit = 11,
        Value = 12,
        Enum = 13,
        Keyword = 14,
        Snippet = 15
    };

    QString label;
    ItemKind kind;
    QString sortText;
    QString insertText;
};

struct CompletionResponse : LSPResponse {
    bool incomplete;
    QList<CompletionItem> items;
    void parseJson(const QJsonObject &response) override;
};

class LanguageServer : public QObject {
    Q_OBJECT

protected:
    mutable QMutex mutex;
    QProcess *process = nullptr;
    void sendRequest(LSPRequestMethod method, const QJsonObject &payload) const;
    template<std::derived_from<LSPResponse> R>
    QCoro::Task<R> waitResponse() const;

public:
    virtual QCoro::Task<> start() = 0;
    QCoro::Task<InitializeResponse> initialize(const QString &rootUri,
                                               const QJsonObject &capabilities) const;
    QCoro::Task<CompletionResponse> completion(const LSPTextDocument &document,
                                               const LSPPosition &position) const;
    QCoro::Task<DidOpenResponse> didOpen(const LSPTextDocument &document) const;
    // TODO: support more functions in LSP
};

class PythonLanguageServer : public LanguageServer {
    static PythonLanguageServer *instance;

public:
    static QCoro::Task<PythonLanguageServer *> getServer();
    QCoro::Task<> start() override;
};

class LanguageServers {
public:
    static QCoro::Task<LanguageServer *> get(Language language);
};


#endif // LSP_H
