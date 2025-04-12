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

    int currentCursorPos = -1;
    TSQuery *bracketQuery = nullptr;
    TSQueryCursor *bracketCursor = nullptr;

    void parseDocument();
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
    void setCursorPosition(int pos);
};
class HighlighterFactory {
public:
    static Highlighter *getHighlighter(Language language, QTextDocument *parent);
};

#endif // HIGHLIGHTER_H
