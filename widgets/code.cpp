#include <QFile>
#include <QMessageBox>
#include <QPainter>
#include <utility>

#include "../ide/highlighter.h"
#include "code.h"

#include "footer.h"

/* Welcome widget */

WelcomeWidget::WelcomeWidget(QWidget *parent) : QPlainTextEdit(parent) { setup(); }

void WelcomeWidget::setup() { setReadOnly(true); }

/* Code plain text edit widget */

class LineNumberArea : public QWidget {
    CodeEditWidget *codeEdit;

public:
    static const int L_MARGIN;
    static const int R_MARGIN;

    explicit LineNumberArea(CodeEditWidget *codeEdit) : QWidget(codeEdit), codeEdit(codeEdit) {}

    QSize sizeHint() const override { return {codeEdit->LNAWidth(), 0}; }

protected:
    void paintEvent(QPaintEvent *event) override { codeEdit->LNAEvent(event); };
};

const int LineNumberArea::L_MARGIN = 5;
const int LineNumberArea::R_MARGIN = 5;

QFont CodeEditWidget::font("Consolas", 15);

CodeEditWidget::CodeEditWidget(const QString &filename, QWidget *parent) : QPlainTextEdit(parent) {
    lineNumberArea = new LineNumberArea(this);
    file = FileInfo(filename);
    modified = false;
    highlighter = HighlighterFactory::getHighlighter(file.getLanguage(), this->document());

    readFile();
    setup();
    adaptViewport();

    connect(this, &CodeEditWidget::blockCountChanged, this, &CodeEditWidget::adaptViewport);
    connect(this, &CodeEditWidget::updateRequest, this, &CodeEditWidget::updateLineNumberArea);
    connect(this, &CodeEditWidget::cursorPositionChanged, this, &CodeEditWidget::highlightLine);
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditWidget::onTextChanged);
}

void CodeEditWidget::setup() {
    setFont(font);
    // Set tab size to 4 spaces
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);
}

void CodeEditWidget::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), LNAWidth(), cr.height()));
}

void CodeEditWidget::adaptViewport() { setViewportMargins(LNAWidth(), 0, 0, 0); }

void CodeEditWidget::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy) {
        lineNumberArea->scroll(0, dy);
    } else {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        adaptViewport();
    }
}

const FileInfo &CodeEditWidget::getFile() const { return file; }

QString CodeEditWidget::getTabText() const { return file.fileName(); };

int CodeEditWidget::LNAWidth() const {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int fontWidth = fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    int marginWidth = LineNumberArea::L_MARGIN + LineNumberArea::R_MARGIN;
    return fontWidth + marginWidth;
}

void CodeEditWidget::LNAEvent(QPaintEvent *event) const {
    QPainter painter(lineNumberArea);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::lightGray);
            painter.drawText(-LineNumberArea::L_MARGIN, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditWidget::highlightLine() {
    QList<QTextEdit::ExtraSelection> selections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor("#222222").lighter(160);
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
    QFile qfile(file.filePath());
    if (!qfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "CodeEditWidget::readFile: Failed to open file " << file.filePath();
        return;
    }
    // check if the file is binary
    auto bytes = qfile.read(100);
    for (const auto &byte: bytes) {
        if (byte < 0x20 && byte != '\n' && byte != '\r' && byte != '\t') {
            setReadOnly(true);
            setPlainText(tr("文件格式不支持"));
            qfile.close();
            return;
        }
    }

    QString buffer;
    if (qfile.size() > MAX_BUFFER_SIZE) {
        buffer = tr("文件过大，无法在编辑器内打开");
        setReadOnly(true);
    } else {
        buffer = qfile.readAll();
    }
    setPlainText(buffer);
    qfile.close();
}

void CodeEditWidget::saveFile() {
    QFile qfile(file.filePath());
    if (!qfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "CodeEditWidget::saveFile: Failed to open file " << file.filePath();
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
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("保存文件"), tr("文件已修改，是否保存？"),
                                                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
        saveFile();
    } else if (reply == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

void CodeEditWidget::onTextChanged() {
    if (!modified) {
        modified = true;
        emit modify();
    }
}

/* Code tab widget */

CodeTabWidget::CodeTabWidget(QWidget *parent) : QTabWidget(parent) {
    setup();
    welcome();
    connect(this, &QTabWidget::currentChanged, this, &CodeTabWidget::onCurrentTabChanged);
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
    connect(this, &QTabWidget::tabCloseRequested, this, &CodeTabWidget::removeCodeEditRequested);
}

CodeEditWidget *CodeTabWidget::curEdit() const { return qobject_cast<CodeEditWidget *>(currentWidget()); }
CodeEditWidget *CodeTabWidget::editAt(int index) const { return qobject_cast<CodeEditWidget *>(widget(index)); }

void CodeTabWidget::welcome() { addTab(new WelcomeWidget(this), "欢迎"); }

void CodeTabWidget::addCodeEdit(const QString &filePath) {
    // find if the file is already opened
    for (int i = 0; i < count(); ++i) {
        auto *edit = editAt(i);
        if (edit && edit->getFile().filePath() == filePath) {
            setCurrentIndex(i); // switch to the existing tab
            return;
        }
    }

    auto *edit = new CodeEditWidget(filePath, this);
    int index = addTab(edit, edit->getTabText());
    connect(edit, &CodeEditWidget::modify, this, [this, index] { widgetModified(index); });
    setCurrentIndex(index);
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


FileInfo CodeTabWidget::currentFile() const {
    if (auto *edit = curEdit()) {
        return edit->getFile();
    }
    return FileInfo::empty();
}

void CodeTabWidget::save() {
    if (auto *edit = curEdit()) {
        edit->saveFile();
        // recover the tab title
        setTabText(currentIndex(), edit->getTabText());
    }
}

void CodeTabWidget::onCurrentTabChanged(int) const {
    const auto *edit = curEdit();
    FooterWidget::instance().setFileLabel(edit ? edit->getFile().filePath() : "");
}
