#ifndef LSP_H
#define LSP_H

#include <QJsonObject>
#include <QMutex>
#include <QProcess>
#include <QPromise>
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

struct LSPUri {
    QString uri;

    static LSPUri fromQUrl(const QUrl &url);
    QUrl toQUrl() const;
};

struct LSPTextDocument {
    LSPUri uri;
    Language language;
    std::optional<QString> text = std::nullopt;
    int version = 1;

    QJsonObject toJson() const;
    std::pair<QString, QJsonValue> toEntry() const;
};

struct LSPPosition {
    int line;
    int character;

    void readJson(QJsonObject json);
    QJsonObject toJson() const;
    std::pair<QString, QJsonValue> toEntry() const;
};

struct LSPRange {
    LSPPosition start;
    LSPPosition end;

    void readJson(QJsonObject json);
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
        Snippet = 15,
        Color = 16,
        File = 17,
        Reference = 18,
        Folder = 19,
        EnumMember = 20,
        Constant = 21,
        Struct = 22,
        Event = 23,
        Operator = 24,
        TypeParameter = 25
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

struct DefinitionItem {
    LSPUri uri;
    LSPRange range;
};

struct DefinitionResponse : LSPResponse {
    QList<DefinitionItem> items;
    void parseJson(const QJsonObject &response) override;
};

class LanguageServer : public QObject {
    Q_OBJECT

protected:
    mutable QMutex mutex;
    QProcess *process = nullptr;
    /**
     * @brief send a request to the server
     * @param method the method to call
     * @param payload the payload to send
     */
    void sendRequest(LSPRequestMethod method, const QJsonObject &payload) const;
    /**
     * @brief wait for a response from the server
     * @tparam R the type of the response
     * @return the response
     */
    template<std::derived_from<LSPResponse> R>
    QCoro::Task<R> waitResponse() const;

public:
    virtual QCoro::Task<> start() = 0;
    static QString commentPrefix(Language language);

    QCoro::Task<InitializeResponse> initialize(const QString &rootUri,
                                               const QJsonObject &capabilities) const;
    QCoro::Task<CompletionResponse> completion(const LSPTextDocument &document,
                                               const LSPPosition &position) const;
    QCoro::Task<> didOpen(const LSPTextDocument &document) const;
    QCoro::Task<DefinitionResponse> definition(const LSPTextDocument &document,
                                               const LSPPosition &position) const;
    // TODO: support more functions in LSP
};

class ClangdLanguageServer : public LanguageServer {
    static ClangdLanguageServer *instance;

public:
    static QCoro::Task<ClangdLanguageServer *> getServer();
    QCoro::Task<> start() override;
};

class PylspLanguageServer : public LanguageServer {
    static PylspLanguageServer *instance;

public:
    static QCoro::Task<PylspLanguageServer *> getServer();
    QCoro::Task<> start() override;
};

class LanguageServers {
public:
    static QCoro::Task<LanguageServer *> get(Language language);
};


#endif // LSP_H
