#ifndef CODE_EDIT_H
#define CODE_EDIT_H

#include <QListWidget>
#include <QPlainTextEdit>
#include <qcorotask.h>

#include "../ide/highlighter.h"
#include "../ide/lsp.h"
#include "../ide/project.h"
#include "fileTree.h"

class CodeEditWidget;

class CompletionList : public QListWidget {
    CodeEditWidget *codeEdit;
    QList<CompletionItem> completions;

    Q_OBJECT

    void onItemClicked(const QListWidgetItem *item);
    void updateHeight();
    void addCompletionItem(const CompletionItem &item);

protected:
    void keyPressEvent(QKeyEvent *e) override;

private slots:

signals:
    void completionSelected(const QString &completion);

public:
    explicit CompletionList(CodeEditWidget *codeEdit);
    void readCompletions(const CompletionResponse &response);
    void update(const QString &curWord);
    void display();
};

class LineNumberArea : public QWidget {
    CodeEditWidget *codeEdit;

public:
    static const int L_MARGIN;
    static const int R_MARGIN;

    explicit LineNumberArea(CodeEditWidget *codeEdit);
    int getWidth() const;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
};

class WelcomeWidget : public QWidget {
    Q_OBJECT
    void setup();

public:
    explicit WelcomeWidget(QWidget *parent);
};

class CodeEditWidget : public QPlainTextEdit {
    Q_OBJECT

    friend class LineNumberArea;
    friend class CompletionList;

    LangFileInfo file;
    Highlighter *highlighter;
    LanguageServer *server;
    CompletionList *cl;
    LineNumberArea *lna;

    bool modified;
    bool requireCompletion;

    void setup();

private slots:
    /** Async initialization */
    QCoro::Task<> onSetupFinished();
    /** Font setter for configs */
    void onSetFont(const QJsonValue &value);
    /** Adapt the viewport margins */
    void adaptViewport();
    /** Update the line number area when the content changes */
    void updateLineNumberArea(const QRect &rect, int dy);
    /** Highlight the line where the cursor is */
    void highlightLine();
    /** What to do when the text is modified */
    QCoro::Task<> onTextChanged();
    /** Update the cursor position (and tell it to highlighter) */
    void updateCursorPosition() const;
    /** Ask the language server for completion */
    QCoro::Task<> askForCompletion() const;
    /** Update the completion list */
    void updateCompletionList();
    /** Insert the given completion */
    void insertCompletion(const QString &completion);
    /** Toggle the comment (if available) */
    void onToggleComment();
    /** Ask the language server for definition */
    QCoro::Task<> askForDefinition();

signals:
    void setupFinished();
    void modify();
    void toggleComment();
    void jumpToDefinition();
    void jumpTo(QUrl url, int startLine, int startChar, int endLine, int endChar);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;

public:
    explicit CodeEditWidget(const QString &filename, QWidget *parent = nullptr);

    const LangFileInfo &getFile() const;
    QString getTabText() const;
    /** Read the file content and display it */
    void readFile();
    /** Save the file content to the file */
    void saveFile();
    /** Check if the content is modified, if so, ask for save */
    bool askForSave();
    /** Move the cursor to the given position */
    void cursorMoveTo(int startLine, int startChar, int endLine, int endChar);
};

class CodeTabWidget : public QTabWidget {
    Q_OBJECT

    Project *project = nullptr;
    QMutex tabMutex;

    void setup();
    /** Add a welcome widget */
    void welcome();
    /** Add a code edit widget for the given file */
    CodeEditWidget *addCodeEdit(const QString &filePath);
    /** Check if the file is opened, if so, remove it */
    void checkRemoveCodeEdit(const QString &filename);

public slots:
    /** Handle file operations (from the file tree) */
    void handleFileOperation(const QString &filename, FileOperation operation);
    /** Remove the code edit widget at the given index */
    void removeCodeEdit(int index);
    /** Ask the user before removing the code edit widget */
    void removeCodeEditRequested(int index);
    /** What to do when a widget is modified */
    void widgetModified(int index);
    /** What to do when the current tab changed */
    void onCurrentTabChanged(int index) const;
    /** Jump to the given range */
    void jumpTo(const QUrl &url, int startLine, int startChar, int endLine, int endChar);

public:
    explicit CodeTabWidget(QWidget *parent);
    void setProject(Project *project);
    void clearAll();
    LangFileInfo currentFile() const;
    CodeEditWidget *curEdit() const;
    CodeEditWidget *editAt(int index) const;
    void save();
};

#endif // CODE_EDIT_H
