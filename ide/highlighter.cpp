#include "highlighter.h"
#include <QJsonArray>
#include <QLibrary>
#include <qcorotimer.h>
#include <utility>

#include "../util/file.h"

// TODO: optimize the rule memory use
Highlighter::Highlighter(const TSLanguage *language, QString langName, QTextDocument *parent) :
    QSyntaxHighlighter(parent), language(language), langName(std::move(langName)), parsing(false) {
    parser = ts_parser_new();
    ts_parser_set_language(parser, language);
    queries.clear();
    Configs::bindHotUpdateOn(this, "highlightRules", &Highlighter::readRules);
    Configs::instance().manuallyUpdate("highlightRules");
    setupBracketQuery();
    connect(document(), &QTextDocument::contentsChange, this, &Highlighter::onContentsChanged);
}

QPair<TSLanguage *, QString> Highlighter::toTSLanguage(Language language) {
    QString name;
    switch (language) {
        case Language::C:
            name = "c";
            break;
        case Language::CPP:
            name = "cpp";
            break;
        case Language::PYTHON:
            name = "python";
            break;
        default:
            return {nullptr, ""};
    }

    QLibrary tsLib("tree-sitter-" + name);
    if (!tsLib.load()) {
        return {nullptr, ""};
    }

    auto languageFn =
            reinterpret_cast<TSLanguage *(*) ()>(tsLib.resolve(("tree_sitter_" + name).toUtf8()));
    if (!languageFn) {
        qWarning() << "Failed to resolve tree-sitter language function:" << tsLib.errorString();
        return {nullptr, ""};
    }

    return {languageFn(), name};
}

Highlighter::~Highlighter() {
    disconnect(document(), &QTextDocument::contentsChange, this, &Highlighter::onContentsChanged);
    if (tree) {
        ts_tree_delete(tree);
    }
    for (auto &[query, cursor, rule]: queries) {
        if (cursor)
            ts_query_cursor_delete(cursor);
        if (query)
            ts_query_delete(query);
    }
    if (parser)
        ts_parser_delete(parser);
    if (bracketCursor)
        ts_query_cursor_delete(bracketCursor);
    if (bracketQuery)
        ts_query_delete(bracketQuery);
}

void Highlighter::setupBracketQuery() {
    const char *queryPattern = "(\"(\" @leftb \")\" @right) "
                               "(\"(\" @leftb \")\" @right) "
                               "(\"{\" @leftb \"}\" @right) "
                               "(\"[\" @leftb \"]\" @right)";

    uint32_t errorOffset;
    TSQueryError errorType;
    bracketQuery =
            ts_query_new(language, queryPattern, strlen(queryPattern), &errorOffset, &errorType);

    if (!bracketQuery) {
        qWarning() << "Failed to create bracket query for language" << langName << "at offset"
                   << errorOffset << "with error" << errorType;
        return;
    }

    bracketCursor = ts_query_cursor_new();
}

void Highlighter::setCursorPosition(int pos, const QTextBlock &block) {
    currentCursorPos = pos;
    if (lastBlock.isValid()) {
        rehighlightBlock(lastBlock);
    }
    rehighlightBlock(block);
    lastBlock = block;
}

void Highlighter::highlightBlock(const QString &text) {
    QTextCharFormat f;
    f.setFontWeight(QFont::Normal);
    setFormat(0, text.length(), f);

    for (auto &result: results) {
        int blockPos = currentBlock().position();
        auto blockLen = text.length();
        auto blockEnd = blockPos + blockLen;

        for (const auto &[start, end]: result.strRanges) {
            if (start >= blockEnd || end <= blockPos)
                continue;

            int highlightStart = qMax(start - blockPos, 0);
            int highlightEnd = qMin(end - blockPos, blockLen);

            if (highlightStart < highlightEnd) {
                setFormat(highlightStart, highlightEnd - highlightStart, result.strFormat);
            }
        }
    }
    highlightBracketPairs(text);
    textNotChanged = true;
}

QTextCharFormat Highlighter::matchFormat(QTextCharFormat format) {
    format.setFontWeight(QFont::Bold);
    // format.setForeground(QColor(0xFF0000));
    return format;
}

void Highlighter::highlightBracketPairs(const QString &text) {
     if (!bracketQuery || !bracketCursor || currentCursorPos == -1)
         return;

     int blockPos = currentBlock().position();
     int cursorPosInBlock = currentCursorPos - blockPos - 1;
     if (cursorPosInBlock < 0 || cursorPosInBlock >= text.length())
         return;

     QChar cursorChar = text.at(cursorPosInBlock);
     if (cursorChar != '(' && cursorChar != ')' && cursorChar != '{' && cursorChar != '}' &&
         cursorChar != '[' && cursorChar != ']') {
         return;
     }

     auto utf8Content = document()->toPlainText().toUtf8();

     TSNode root = ts_tree_root_node(tree);
     ts_query_cursor_exec(bracketCursor, bracketQuery, root);

     TSQueryMatch match;

     while (ts_query_cursor_next_match(bracketCursor, &match)) {
         uint32_t leftPos = 0, rightPos = 0;
         bool hasLeft = false, hasRight = false;

         for (uint32_t i = 0; i < match.capture_count; ++i) {
             uint32_t id = match.captures[i].index;
             TSNode node = match.captures[i].node;
             uint32_t length = 5;
             auto name = ts_query_capture_name_for_id(bracketQuery, id, &length);
             if (!name) {
                 continue;
             }
             if (strcmp(name, "leftb") == 0) {
                 leftPos = ts_node_start_byte(node);
                 hasLeft = true;
             } else if (strcmp(name, "right") == 0) {
                 rightPos = ts_node_start_byte(node);
                 hasRight = true;
             }
         }

         if (hasLeft && hasRight) {
             int leftCharPos = byteToCharPosition(leftPos, utf8Content);
             int rightCharPos = byteToCharPosition(rightPos, utf8Content);

             int left = leftCharPos - blockPos;
             int right = rightCharPos - blockPos;

             if (cursorPosInBlock == left || cursorPosInBlock == right) {
                 // FIXME: Bracket cross lines?
                 // QTextBlock block = currentBlock();
                 // bool thisLine = true;
                 //
                 // while (leftCharPos - block.position() < 0) {
                 //     block = block.previous();
                 //     thisLine = false;
                 // }
                 // block = currentBlock();
                 // while (rightCharPos - block.position() >= block.length()) {
                 //     block = block.next();
                 //     thisLine = false;
                 // }

                 setFormat(left, 1, matchFormat(format(left)));
                 setFormat(right, 1, matchFormat(format(right)));
                 break;
             }
         }
     }
}


void Highlighter::onContentsChanged(int, int, int) { parseDocument(); }

void Highlighter::readRules(const QJsonValue &jsonRules) {
    if (!jsonRules.isArray()) {
        qDebug() << "readRules: HighlightRules is not an array";
        return;
    }

    QList<HighlightRule> rules;

    // read highlight rules from json
    for (const auto &array: jsonRules.toArray()) {
        auto obj = array.toObject();
        if (!obj.contains("pattern")) {
            qWarning() << "Invalid highlight rule format on: " << array.toString();
            continue;
        }
        if (obj.contains("language")) {
            // skip other languages' rules
            auto languageJSON = obj["language"];
            auto languages =
                    languageJSON.isArray() ? languageJSON.toArray() : QJsonArray{languageJSON};
            bool found = false;
            for (const auto &lang: languages) {
                if (lang.toString() == langName) {
                    found = true;
                    break;
                }
            }
            if (!found)
                continue;
        }

        QTextCharFormat format;
        if (obj.contains("foreground")) {
            QString color = obj["foreground"].toString();
            format.setForeground(QColor(color));
        }
        if (obj.contains("background")) {
            QString color = obj["background"].toString();
            format.setBackground(QColor(color));
        }
        if (obj.contains("style")) {
            auto styles = obj["style"].toString().split(" ", Qt::SkipEmptyParts);
            for (const auto &style: styles) {
                if (style == "bold") {
                    format.setFontWeight(QFont::Bold);
                } else if (style == "italic") {
                    format.setFontItalic(true);
                } else if (style == "underline") {
                    format.setFontUnderline(true);
                } else if (style == "strikeout") {
                    format.setFontStrikeOut(true);
                }
            }
        }

        auto patternsJSON = obj["pattern"];
        // if patterns is not an array, convert it to an array with one element
        auto patterns = patternsJSON.isArray() ? patternsJSON.toArray() : QJsonArray{patternsJSON};

        for (const auto &p: patterns) {
            auto pattern = p.toString();
            rules.emplace_back(pattern, format);
        }
    }

    // create queries from rules
    for (auto &[pattern, format]: rules) {
        // We still do not support error messages
        uint32_t errorOffset;
        TSQueryError errorType;
        const char *ptn = pattern.toUtf8().constData();
        auto query = ts_query_new(language, ptn, strlen(ptn), &errorOffset, &errorType);
        if (query == nullptr) {
            continue;
        }
        auto cursor = ts_query_cursor_new();
        queries.emplace_back(query, cursor, std::move(format));
    }
}

QCoro::Task<> Highlighter::parseDocument() {
    if (parsing) {
        co_return;
    }

    auto utf8Content = document()->toPlainText().toUtf8();
    // TODO: use old tree here for better performance
    parsing = true;
    if (tree) {
        ts_tree_delete(tree);
    }
    tree = ts_parser_parse_string(parser, nullptr, utf8Content.constData(), utf8Content.size());

    TSNode root = ts_tree_root_node(tree);
    results.clear();

    int batchSize = 10000;
    int cnt = 0;

    for (auto &[query, cursor, format]: queries) {
        ts_query_cursor_exec(cursor, query, root);

        TSQueryMatch match;
        QList<QPair<int, int>> strRanges;
        while (ts_query_cursor_next_match(cursor, &match)) {
            for (uint32_t i = 0; i < match.capture_count; ++i) {
                TSNode node = match.captures[i].node;
                uint32_t startByte = ts_node_start_byte(node);
                uint32_t endByte = ts_node_end_byte(node);

                // Convert byte offsets to character positions
                int startPos = byteToCharPosition(startByte, utf8Content);
                int endPos = byteToCharPosition(endByte, utf8Content);
                strRanges.emplace_back(startPos, endPos);
            }
            if (++cnt % batchSize == 0) {
                co_await QCoro::sleepFor(std::chrono::milliseconds(100));
            }
        }

        results.emplace_back(strRanges, format);
    }
    rehighlight();
    parsing = false;
    co_return;
}

int Highlighter::byteToCharPosition(uint32_t bytePos, const QByteArray &utf8) {
    static const QByteArray *utf8Ptr = nullptr;
    static QList<int> byteOffsets = {}; // charPos -> bytePos

    if (utf8Ptr != &utf8) {
        // build cache
        byteOffsets.clear();
        for (int byteOffset = 0; byteOffset < utf8.size();) {
            byteOffsets.append(byteOffset);
            uchar ch = utf8[byteOffset];

            if (ch < 0x80)
                byteOffset += 1;
            else if ((ch & 0xE0) == 0xC0)
                byteOffset += 2;
            else if ((ch & 0xF0) == 0xE0)
                byteOffset += 3;
            else if ((ch & 0xF8) == 0xF0)
                byteOffset += 4;
        }
        utf8Ptr = &utf8;
    }

    // use binary search with complexity O(log n)
    auto it = std::ranges::upper_bound(byteOffsets, bytePos);
    return static_cast<int>(it - byteOffsets.begin() - 1);
}

Highlighter *HighlighterFactory::getHighlighter(Language language, QTextDocument *parent) {
    auto [tsLanguage, name] = Highlighter::toTSLanguage(language);
    return tsLanguage == nullptr ? nullptr : new Highlighter(tsLanguage, name, parent);
};
