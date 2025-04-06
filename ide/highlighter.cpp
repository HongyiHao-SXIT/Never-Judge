#include "highlighter.h"
#include <QJsonArray>
#include <QLibrary>
#include <utility>

#include "../util/file.h"

QList<HighlightRule> Highlighter::rules{};

// TODO: optimize the rule memory use
Highlighter::Highlighter(const TSLanguage *language, QString langName, QTextDocument *parent) :
    QSyntaxHighlighter(parent), language(language), langName(std::move(langName)) {
    parser = ts_parser_new();
    ts_parser_set_language(parser, language);

    queries.clear();
    readRules(ConfigManager::instance().getAll());

    connect(&ConfigManager::instance(), &ConfigManager::configChanged, this, &Highlighter::readRules);
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
        qWarning() << "Failed to load tree-sitter library:" << tsLib.errorString();
        return {nullptr, ""};
    }

    auto languageFn = reinterpret_cast<TSLanguage *(*) ()>(tsLib.resolve(("tree_sitter_" + name).toUtf8()));
    if (!languageFn) {
        qWarning() << "Failed to resolve tree-sitter language function:" << tsLib.errorString();
        return {nullptr, ""};
    }

    return {languageFn(), name};
}

Highlighter::~Highlighter() {
    disconnect(document(), &QTextDocument::contentsChange, this, &Highlighter::onContentsChanged);
    if (lastTree) {
        ts_tree_delete(lastTree);
    }
    for (auto &[query, cursor, rule]: queries) {
        if (cursor)
            ts_query_cursor_delete(cursor);
        if (query)
            ts_query_delete(query);
    }
    if (parser)
        ts_parser_delete(parser);
}

void Highlighter::highlightBlock(const QString &text) {
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
                setFormat(highlightStart, highlightEnd - highlightStart, *result.strFormat);
            }
        }
    }
}

void Highlighter::onContentsChanged(int, int, int) { parseDocument(); }

void Highlighter::readRules(QJsonObject settings) {
    static auto key = "highlightRules";
    if (!settings.contains(key) || !settings[key].isArray()) {
        qWarning() << "No highlight rules found in settings.";
        return;
    }
    rules.clear();
    auto arrays = settings.value(key).toArray();
    for (const auto &array: arrays) {
        auto obj = array.toObject();
        if (!obj.contains("pattern")) {
            qWarning() << "Invalid highlight rule format on key: " << key;
            continue;
        }
        if (obj.contains("language")) {
            // skip other languages' rules
            auto languageJSON = obj["language"];
            auto languages = languageJSON.isArray() ? languageJSON.toArray() : QJsonArray{languageJSON};
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
            rules.append({pattern, format});
        }
    }

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
        queries.append({query, cursor, format});
    }
}

void Highlighter::parseDocument() {

    QString docContent = document()->toPlainText();
    QByteArray utf8Content = docContent.toUtf8();

    // FIXME: use old tree here for better performance

    auto tree = ts_parser_parse_string(parser, lastTree, utf8Content.constData(), utf8Content.size());
    // lastTree = tree;  // If uncomment the code, it would not work!

    TSNode root = ts_tree_root_node(tree);
    results.clear();

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
                int startPos = byteToCharPosition(startByte, utf8Content, docContent);
                int endPos = byteToCharPosition(endByte, utf8Content, docContent);
                strRanges.emplace_back(startPos, endPos);
            }
        }
        results.append({strRanges, &format});
    }

    if (lastTree && lastTree != tree) {
        ts_tree_delete(lastTree);
    }

    rehighlight();
}

int Highlighter::byteToCharPosition(uint32_t bytePos, const QByteArray &utf8, const QString &text) {
    int charPos = 0;

    for (int currentByte = 0; charPos < text.length() && currentByte < bytePos; charPos++) {
        QChar c = text[charPos];
        currentByte += c.unicode() <= 0x7F ? 1 : c.unicode() <= 0x7FF ? 2 : c.isSurrogate() ? 4 : 3;
    }

    return charPos;
}

Highlighter *HighlighterFactory::getHighlighter(Language language, QTextDocument *parent) {
    auto [tsLanguage, name] = Highlighter::toTSLanguage(language);
    return tsLanguage == nullptr ? nullptr : new Highlighter(tsLanguage, name, parent);
};
