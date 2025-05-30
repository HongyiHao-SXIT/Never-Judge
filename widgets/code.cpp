#include <QFile>
#include <QMessageBox>
#include <QPainter>
#include <QVBoxLayout>

#include "../ide/highlighter.h"
#include "../ide/lsp.h"
#include "../util/file.h"
#include "code.h"

#include <QThread>
#include <QTimer>

#include "footer.h"
#include "icon.h"

/* Completion list */

CompletionList::CompletionList(CodeEditWidget *codeEdit) :
    QListWidget(codeEdit), codeEdit(codeEdit) {
    setWindowFlags(Qt::Popup);
    setSelectionMode(SingleSelection);
    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    hide();

    connect(this, &QListWidget::itemClicked, this, &CompletionList::onItemClicked);
}

void CompletionList::updateHeight() {
    const int itemHeight = sizeHintForRow(0);
    // at maximum 8 items
    const int visibleItems = qMin(count(), 8);

    int totalHeight = visibleItems * itemHeight + 2 * frameWidth();

    setFixedHeight(totalHeight);
    setFixedWidth(400);
}

void CompletionList::onItemClicked(const QListWidgetItem *item) {
    emit completionSelected(item->data(Qt::UserRole).toString());
}

void CompletionList::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
        QListWidget::keyPressEvent(e);
    } else if (e->key() == Qt::Key_Tab) {
        if (currentItem()) {
            emit completionSelected(currentItem()->data(Qt::UserRole).toString());
            hide();
        }
    } else if (e->key() == Qt::Key_Escape) {
        hide();
    } else {
        if (e->key() == Qt::Key_Return) {
            hide();
        }
        codeEdit->keyPressEvent(e);
    }
}

void CompletionList::readCompletions(const CompletionResponse &response) {
    codeEdit->requireCompletion = response.incomplete;
    completions = response.items;
}

void CompletionList::update(const QString &curWord) {
    clear();
    for (const auto &item: completions) {
        if (item.insertText == curWord) {
            return; // The word is finished and do not give completions
        }
    }
    for (const auto &item: completions) {
        if (item.insertText.startsWith(curWord)) {
            addCompletionItem(item);
        }
    }
}

void CompletionList::addCompletionItem(const CompletionItem &item) {
    auto *listItem = new QListWidgetItem(this);

    auto *widget = new QWidget(this);

    auto *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 2, 5, 2);

    auto *textLabel = new QLabel(item.label, this);
    textLabel->setMaximumWidth(300);
    textLabel->setFont(font());
    layout->addWidget(textLabel);
    layout->addStretch();

    static const QMap<CompletionItem::ItemKind, QString> kindIconMap = {
            {CompletionItem::Class, "code/class"},
            {CompletionItem::Function, "code/function"},
            {CompletionItem::Interface, "code/class"},
            {CompletionItem::Field, "code/variable"},
            {CompletionItem::Variable, "code/variable"},
            {CompletionItem::Module, "code/module"},
            {CompletionItem::Function, "code/function"},
            {CompletionItem::Keyword, "code/keyword"},
            {CompletionItem::File, "code/file"},
            {CompletionItem::Struct, "code/class"},
            {CompletionItem::Enum, "code/enum"},
            {CompletionItem::Reference, "code/variable"},
            {CompletionItem::Property, "code/variable"},
    };

    auto *iconBtn = new IconPushButton(this);
    auto iconPath = kindIconMap.value(item.kind, "code/text");
    iconBtn->setIconFromResName(iconPath);
    iconBtn->setDisabled(true);
    iconBtn->setStyleSheet(loadText("qss/completion.css"));
    layout->addWidget(iconBtn);

    widget->setLayout(layout);

    listItem->setSizeHint(widget->sizeHint());
    setItemWidget(listItem, widget);

    listItem->setData(Qt::UserRole, item.insertText);
}

void CompletionList::display() {
    setCurrentRow(0);
    show();
    setFocus();
    updateHeight();
}

/* Line number area */

LineNumberArea::LineNumberArea(CodeEditWidget *codeEdit) : QWidget(codeEdit), codeEdit(codeEdit) {}

int LineNumberArea::getWidth() const {
    int digits = 1;
    int max = qMax(1, codeEdit->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    // at least 3 digits width
    int fontWidth = codeEdit->fontMetrics().horizontalAdvance(QLatin1Char('9')) * qMax(digits, 3);
    int marginWidth = L_MARGIN + R_MARGIN;
    return fontWidth + marginWidth;
}

QSize LineNumberArea::sizeHint() const { return {getWidth(), 0}; }

void LineNumberArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.fillRect(event->rect(), QColor(0x252526));

    QTextBlock block = codeEdit->firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(
            codeEdit->blockBoundingGeometry(block).translated(codeEdit->contentOffset()).top());
    int bottom = top + static_cast<int>(codeEdit->blockBoundingRect(block).height());

    painter.setFont(codeEdit->font());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);

            if (blockNumber == codeEdit->textCursor().blockNumber()) {
                painter.setPen(QColor(0xFFFFFF));
            } else {
                painter.setPen(QColor(0x858585));
            }

            painter.drawText(0, top, this->width() - R_MARGIN, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(codeEdit->blockBoundingRect(block).height());
        ++blockNumber;
    }
};

const int LineNumberArea::L_MARGIN = 5;
const int LineNumberArea::R_MARGIN = 5;

/* Welcome widget */

WelcomeWidget::WelcomeWidget(QWidget *parent) : QWidget(parent) { setup(); }

void WelcomeWidget::setup() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(30);

    auto *logoLabel = new QLabel(this);
    logoLabel->setText(loadText("logo.txt"));

    QFont logoFont("Consolas", 9);
    logoLabel->setFont(logoFont);
    logoLabel->setObjectName("logo");

    auto *shortcutLabel = new QLabel(this);
    shortcutLabel->setText("<p style='font-size: 16px; color: #D4D4D4;'>Ctrl+N 新建文件</p>"
                           "<p style='font-size: 16px; color: #D4D4D4;'>Ctrl+O 打开项目</p>"
                           "<p style='font-size: 16px; color: #D4D4D4;'>Ctrl+S 保存文件</p>"
                           "<p style='font-size: 16px; color: #D4D4D4;'>Ctrl+R 运行代码</p>");
    shortcutLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addStretch();
    mainLayout->addWidget(logoLabel);
    mainLayout->addWidget(shortcutLabel);
    mainLayout->addStretch();

    setLayout(mainLayout);
}


/* Code plain text edit widget */

CodeEditWidget::CodeEditWidget(const QString &filename, QWidget *parent) :
    QPlainTextEdit(parent), server(nullptr), modified(false), requireCompletion(true) {
    lna = new LineNumberArea(this);
    cl = new CompletionList(this);
    file = LangFileInfo(filename);
    highlighter = HighlighterFactory::getHighlighter(file.language(), document());

    readFile();
    setup();
    adaptViewport();

    connect(this, &CodeEditWidget::setupFinished, this, &CodeEditWidget::onSetupFinished);
    connect(this, &CodeEditWidget::blockCountChanged, this, &CodeEditWidget::adaptViewport);
    connect(this, &CodeEditWidget::updateRequest, this, &CodeEditWidget::updateLineNumberArea);
    connect(this, &CodeEditWidget::cursorPositionChanged, this, &CodeEditWidget::highlightLine);
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditWidget::onTextChanged);
    connect(cl, &CompletionList::completionSelected, this, &CodeEditWidget::insertCompletion);
    connect(this, &CodeEditWidget::toggleComment, this, &CodeEditWidget::onToggleComment);
    connect(this, &CodeEditWidget::jumpToDefinition, this, &CodeEditWidget::askForDefinition);

    emit setupFinished();
}

QCoro::Task<> CodeEditWidget::onSetupFinished() {
    if (highlighter) {
        highlighter->parseDocument();
    }
    server = co_await LanguageServers::get(file.language());
    if (server == nullptr) {
        co_return;
    }
    auto response = co_await server->initialize(file.path(), {});
    if (!response.ok) {
        qWarning() << "Server of language" << langName(file.language()) << "initialized failed";
    }
    co_await server->didOpen({LSPUri::fromQUrl(file.filePath()), file.language(), toPlainText()});
    co_return;
}

void CodeEditWidget::setup() {
    Configs::bindHotUpdateOn(this, "codeFont", &CodeEditWidget::onSetFont);
    Configs::instance().manuallyUpdate("codeFont");
}

void CodeEditWidget::onSetFont(const QJsonValue &fontJson) {
    QJsonObject obj = fontJson.toObject();
    QFont font;
    font.setFamily(obj["family"].toString());
    font.setPointSize(obj["size"].toInt());
    setTabStopDistance(3 * font.pointSize());
    setFont(font);
    lna->setFont(font);

    QFont sFont(font);
    sFont.setPointSize(font.pointSize() - 3); // smaller than edit
    cl->setFont(sFont);
}

void CodeEditWidget::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);

    auto cr = contentsRect();
    lna->setGeometry(QRect(cr.left(), cr.top(), lna->getWidth(), cr.height()));
}


void CodeEditWidget::keyPressEvent(QKeyEvent *e) {
    QPlainTextEdit::keyPressEvent(e);
    if (e->key() == Qt::Key_Slash && e->modifiers() & Qt::ControlModifier) {
        emit toggleComment();
        return;
    }
    updateCursorPosition();
}

void CodeEditWidget::mousePressEvent(QMouseEvent *e) {
    QPlainTextEdit::mousePressEvent(e);
    if (e->button() == Qt::LeftButton && e->modifiers() & Qt::ControlModifier) {
        emit jumpToDefinition();
    }
}

void CodeEditWidget::updateCursorPosition() const {
    if (auto *highlighter = document()->findChild<QSyntaxHighlighter *>()) {
        if (auto *hl = dynamic_cast<Highlighter *>(highlighter)) {
            hl->setCursorPosition(textCursor().position(), textCursor().block());
        }
    }
}

void CodeEditWidget::adaptViewport() { setViewportMargins(lna->getWidth(), 0, 0, 0); }

QCoro::Task<> CodeEditWidget::askForCompletion() const {
    if (!server) {
        co_return;
    }

    // Debounce mechanism
    static QDateTime lastRequestTime;
    // Skip if user requested recently (within 300 ms)
    QDateTime now = QDateTime::currentDateTime();
    if (lastRequestTime.isValid() && lastRequestTime.msecsTo(now) < 300) {
        lastRequestTime = now;
        co_return;
    }
    lastRequestTime = now;

    auto cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    auto word = cursor.selectedText();
    if (word.isEmpty()) {
        co_return;
    }

    if (modified) {
        co_await server->didOpen(
                {LSPUri::fromQUrl(file.filePath()), file.language(), toPlainText()});
    }

    auto completion = co_await server->completion({LSPUri::fromQUrl(file.filePath())},
                                                  {cursor.blockNumber(), cursor.columnNumber()});
    for (const auto &item: completion.items) {
        if (item.insertText == word) {
            co_return; // The word is finished and do not give completions
        }
    }

    cl->readCompletions(completion);
    auto rect = cursorRect();
    auto pos = mapToGlobal(QPoint(rect.right(), rect.bottom()));
    cl->move(pos);

    co_return;
}

void CodeEditWidget::updateCompletionList() {
    auto cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    auto word = cursor.selectedText();
    if (word.isEmpty()) {
        cl->hide();
        requireCompletion = true;
        return;
    }
    cl->update(word);
    if (cl->count() != 0) {
        cl->display();
    } else {
        cl->hide();
    }
}

void CodeEditWidget::insertCompletion(const QString &completion) {
    auto cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    cursor.removeSelectedText();
    cursor.insertText(completion);
    cl->hide();
    setFocus();
    requireCompletion = true;
}

void CodeEditWidget::onToggleComment() {
    QString prefix = LanguageServer::commentPrefix(file.language());
    if (prefix.isEmpty()) {
        return;
    }
    auto insertPrefix = prefix + " "; // add a space after the comment prefix

    QTextCursor cursor = textCursor();
    int originPosInBlock = cursor.positionInBlock();
    int originPos = cursor.position();
    int originBlock = cursor.blockNumber();

    int startPos = cursor.selectionStart();
    int endPos = cursor.selectionEnd();
    cursor.setPosition(startPos);
    int startLine = cursor.blockNumber();
    cursor.setPosition(endPos);
    int endLine = cursor.blockNumber();

    bool comment = false;
    int commentIndex = INT_MAX;

    cursor.setPosition(startPos);
    for (int i = startLine; i <= endLine; i++) {
        cursor.movePosition(QTextCursor::StartOfLine);
        QString lineText = cursor.block().text();
        if (!lineText.trimmed().startsWith(prefix)) {
            comment = true;
        }
        int firstNonSpace = 0;
        while (firstNonSpace < lineText.length() && lineText.at(firstNonSpace).isSpace()) {
            firstNonSpace++;
        }
        int index = firstNonSpace < lineText.length() ? firstNonSpace : 0;
        commentIndex = qMin(commentIndex, index);
    }

    cursor.beginEditBlock();

    qsizetype movement = 0;
    cursor.setPosition(startPos);
    for (int i = startLine; i <= endLine; i++) {
        cursor.movePosition(QTextCursor::StartOfLine);

        QString lineText = cursor.block().text();
        if (comment) {
            // comment
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, commentIndex);
            cursor.insertText(insertPrefix);
        } else {
            // uncomment
            cursor.movePosition(QTextCursor::StartOfLine);
            auto index = lineText.indexOf(prefix);
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,
                                static_cast<int>(index));
            auto n = lineText.trimmed().startsWith(insertPrefix) ? insertPrefix.length()
                                                                 : prefix.length();
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, static_cast<int>(n));
            if (i == originBlock && index <= originPosInBlock) {
                movement = insertPrefix.length();
            }
            cursor.removeSelectedText();
        }
        if (i != endLine) {
            cursor.movePosition(QTextCursor::NextBlock);
        }
    }

    cursor.endEditBlock();

    cursor.setPosition(originPos);
    if (comment) {
        cursor.movePosition(QTextCursor::NextBlock);
    } else {
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, static_cast<int>(movement));
    }
    setTextCursor(cursor);
}

QCoro::Task<> CodeEditWidget::askForDefinition() {
    QTextCursor cursor = textCursor();

    if (modified) {
        co_await server->didOpen(
                {LSPUri::fromQUrl(file.filePath()), file.language(), toPlainText()});
    }

    auto definition = co_await server->definition({LSPUri::fromQUrl(file.filePath())},
                                                  {cursor.blockNumber(), cursor.columnNumber()});
    if (definition.items.isEmpty()) {
        co_return;
    }
    // just use the first element for test here
    auto start = definition.items[0].range.start, end = definition.items[0].range.end;
    emit jumpTo(definition.items[0].uri.toQUrl(), start.line, start.character, end.line,
                end.character);
}

void CodeEditWidget::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy) {
        lna->scroll(0, dy);
    } else {
        lna->update(0, rect.y(), lna->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        adaptViewport();
    }
}

const LangFileInfo &CodeEditWidget::getFile() const { return file; }

QString CodeEditWidget::getTabText() const { return file.fileName(); };

void CodeEditWidget::highlightLine() {
    QList<QTextEdit::ExtraSelection> selections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(0x222222).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }
    setExtraSelections(selections);
}

#define MAX_BUFFER_SIZE (1024 * 1024)

void CodeEditWidget::readFile() {
    QFile check(file.filePath());
    if (!check.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "CodeEditWidget::readFile: Failed to open file " << file.filePath();
        return;
    }

    // Check if the file is binary
    QTextStream in(&check);
    in.setAutoDetectUnicode(true);
    int lineCnt = 0;
    while (!in.atEnd()) {
        auto line = in.readLine();
        if (++lineCnt > 50) {
            break;
        }
        for (const auto &c: line) {
            if (!c.isPrint() && !c.isSpace() && !c.isPunct()) {
                if (c.category() != QChar::Mark_NonSpacing && // Mn
                    c.category() != QChar::Symbol_Other && // So
                    c.category() != QChar::Other_Surrogate && // Cs
                    c.category() != QChar::Other_Format) {
                    setReadOnly(true);
                    lna->setVisible(false);
                    setPlainText(tr("文件格式不支持"));
                    check.close();
                    return;
                }
            }
        }
    }

    QFile read(file.filePath());
    if (!read.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "CodeEditWidget::readFile: Failed to open file " << file.filePath();
        return;
    }
    QString buffer;
    if (read.size() > MAX_BUFFER_SIZE) {
        buffer = tr("文件过大，无法在编辑器内打开");
        lna->setVisible(false);
        setReadOnly(true);
    } else {
        buffer = read.readAll();
    }
    setPlainText(buffer);
    check.close();
}

void CodeEditWidget::saveFile() {
    QFile qfile(file.filePath());
    if (!qfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误",
                             tr("文件 %1 保存失败, 请检查用户权限！").arg(file.filePath()));
        return;
    }
    modified = false;
    qfile.write(toPlainText().toUtf8());
    qfile.close();
}

bool CodeEditWidget::askForSave() {
    if (!modified) {
        return true;
    }
    // if the content is modified, ask for save
    QMessageBox::StandardButton reply =
            QMessageBox::question(this, tr("保存文件"), tr("文件已修改，是否保存？"),
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
        saveFile();
    } else if (reply == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

void CodeEditWidget::cursorMoveTo(int startLine, int startChar, int endLine, int endChar) {
    QTextDocument *doc = document();

    startLine = qBound(0, startLine, doc->blockCount() - 1);
    endLine = qBound(0, endLine, doc->blockCount() - 1);

    QTextBlock startBlock = doc->findBlockByNumber(startLine);
    QTextBlock endBlock = doc->findBlockByNumber(endLine);
    startChar = qBound(0, startChar, startBlock.length() - 1);
    endChar = qBound(0, endChar, endBlock.length() - 1);

    QTextCursor cursor(doc);
    cursor.setPosition(startBlock.position() + startChar);
    if (startLine != endLine || startChar != endChar) {
        cursor.setPosition(endBlock.position() + endChar, QTextCursor::KeepAnchor);
    }

    setTextCursor(cursor);
    ensureCursorVisible();
}


QCoro::Task<> CodeEditWidget::onTextChanged() {
    // This is a hack!
    // If highlighter has not changed the text, we should not emit modify signal
    if (highlighter && highlighter->textNotChanged) {
        highlighter->textNotChanged = false;
        co_return;
    }

    if (!modified) {
        modified = true;
        emit modify();
    }
    if (requireCompletion) {
        co_await askForCompletion();
    }
    updateCompletionList();
    co_return;
}

/* Code tab widget */

CodeTabWidget::CodeTabWidget(QWidget *parent) : QTabWidget(parent) {
    setup();
    welcome();
    connect(this, &QTabWidget::tabCloseRequested, this, &CodeTabWidget::removeCodeEditRequested);
    connect(this, &QTabWidget::currentChanged, this, &CodeTabWidget::onCurrentTabChanged);
}

void CodeTabWidget::setProject(Project *project) {
    this->project = project;
    clearAll();
}

void CodeTabWidget::clearAll() {
    for (int i = count() - 1; i >= 0; --i) {
        removeTab(i);
    }
    welcome();
}

void CodeTabWidget::setup() {
    setTabsClosable(true);
    setMovable(true);
    setStyleSheet(loadText("qss/code.css"));
}

CodeEditWidget *CodeTabWidget::curEdit() const {
    return qobject_cast<CodeEditWidget *>(currentWidget());
}
CodeEditWidget *CodeTabWidget::editAt(int index) const {
    return qobject_cast<CodeEditWidget *>(widget(index));
}

void CodeTabWidget::welcome() { addTab(new WelcomeWidget(this), "欢迎"); }

CodeEditWidget *CodeTabWidget::addCodeEdit(const QString &filePath) {
    // find if the file is already opened
    for (int i = 0; i < count(); ++i) {
        auto *edit = editAt(i);
        if (edit && edit->getFile().filePath() == filePath) {
            setCurrentIndex(i); // switch to the existing tab
            return edit;
        }
    }

    auto *edit = new CodeEditWidget(filePath, this);
    int index;
    {
        QMutexLocker locker(&tabMutex);
        index = addTab(edit, edit->getTabText());
    }
    connect(edit, &CodeEditWidget::modify, this, [this, index] { widgetModified(index); });
    connect(edit, &CodeEditWidget::jumpTo, this, &CodeTabWidget::jumpTo);
    setCurrentIndex(index);
    return edit;
}

void CodeTabWidget::checkRemoveCodeEdit(const QString &filename) {
    for (int i = 0; i < count(); ++i) {
        auto *edit = editAt(i);
        if (edit && edit->getFile().filePath() == filename) {
            removeCodeEdit(i);
            return;
        }
    }
}

void CodeTabWidget::handleFileOperation(const QString &filename, FileOperation operation) {
    switch (operation) {
        case OPEN:
            addCodeEdit(filename);
            break;
        case RENAME:
        case DELETE:
            checkRemoveCodeEdit(filename);
        default:
            break;
    }
}

void CodeTabWidget::removeCodeEditRequested(int index) {
    if (index < 0 || index >= count())
        return;

    auto *edit = editAt(index);
    if (!edit || edit->askForSave()) {
        removeCodeEdit(index);
    }
}

void CodeTabWidget::widgetModified(int index) {
    QMutexLocker locker(&tabMutex);
    // add a * after the title
    setTabText(index, editAt(index)->getTabText() + " *");
}

void CodeTabWidget::removeCodeEdit(int index) {
    if (index < 0 || index >= count())
        return;

    QWidget *w = widget(index);
    removeTab(index);
    w->deleteLater();

    if (count() == 0) {
        welcome();
    }
}


LangFileInfo CodeTabWidget::currentFile() const {
    if (auto *edit = curEdit()) {
        return edit->getFile();
    }
    return LangFileInfo::empty();
}

void CodeTabWidget::save() {
    if (auto *edit = curEdit()) {
        edit->saveFile();
        // recover the tab title
        QMutexLocker locker(&tabMutex);
        setTabText(currentIndex(), edit->getTabText());
    }
}

void CodeTabWidget::onCurrentTabChanged(int) const {
    const auto *edit = curEdit();
    FooterWidget::instance().setFileLabel(edit ? edit->getFile().filePath() : "");
}

void CodeTabWidget::jumpTo(const QUrl &url, int startLine, int startChar, int endLine,
                           int endChar) {
    auto edit = addCodeEdit(url.url());
    edit->cursorMoveTo(startLine, startChar, endLine, endChar);
}
