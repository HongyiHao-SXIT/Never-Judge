#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QJsonObject>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <qcorotask.h>
#include <tree_sitter/api.h>

#include "language.h"

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
    QTextCharFormat strFormat;
};

class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

    // Tree-sitter members
    const TSLanguage *language;
    QString langName;
    TSTree *tree = nullptr;
    TSParser *parser = nullptr;

    QList<Query> queries;
    QList<QueryResult> results;

    bool parsing;

    int currentCursorPos = -1;
    QTextBlock lastBlock;
    TSQuery *bracketQuery = nullptr;
    TSQueryCursor *bracketCursor = nullptr;

    static int byteToCharPosition(uint32_t bytePos, const QByteArray &utf8);
    void highlightBlock(const QString &text) override;
    void setupBracketQuery();
    void highlightBracketPairs(const QString &text);
    static QTextCharFormat matchFormat(QTextCharFormat format);

private slots:
    void onContentsChanged(int, int, int);
    void readRules(const QJsonValue &jsonRules);

public:
    mutable bool textNotChanged = true;
    Highlighter(const TSLanguage *language, QString langName, QTextDocument *parent);
    ~Highlighter() override;
    static QPair<TSLanguage *, QString> toTSLanguage(Language language);
    QCoro::Task<> parseDocument();
    void setCursorPosition(int pos, const QTextBlock &block);
};
class HighlighterFactory {
public:
    static Highlighter *getHighlighter(Language language, QTextDocument *parent);
};

#endif // HIGHLIGHTER_H
