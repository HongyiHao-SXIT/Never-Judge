#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include "project.h"

#include <QJsonObject>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <tree_sitter/api.h>

struct HighlightRule {
    QString pattern = nullptr;
    QTextCharFormat strFormat;
};

struct Query {
    TSQuery *query = nullptr;
    TSQueryCursor *cursor = nullptr;
    QTextCharFormat strFormat;
};

struct QueryResult {
    QList<QPair<int, int>> strRanges;
    QTextCharFormat *strFormat = nullptr;
};

class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

    // Tree-sitter members
    const TSLanguage *language;
    QString langName;
    TSTree *lastTree = nullptr;
    TSParser *parser = nullptr;
    QList<Query> queries;
    QList<QueryResult> results;
    static QList<HighlightRule> rules;

    void parseDocument();
    static int byteToCharPosition(uint32_t bytePos, const QByteArray &utf8, const QString &text);
    void highlightBlock(const QString &text) override;

private slots:
    void onContentsChanged(int, int, int);
    void readRules(const QJsonValue& jsonRules);

public:
    Highlighter(const TSLanguage *language, QString langName, QTextDocument *parent) ;
    ~Highlighter() override;
    static QPair<TSLanguage *, QString> toTSLanguage(Language language);
};

class HighlighterFactory {
public:
    static Highlighter *getHighlighter(Language language, QTextDocument *parent);
};

#endif // HIGHLIGHTER_H
