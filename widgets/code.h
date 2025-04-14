#ifndef CODE_EDIT_H
#define CODE_EDIT_H

#include <QPlainTextEdit>

#include "../ide/highlighter.h"
#include "../ide/project.h"
#include "fileTree.h"

class WelcomeWidget : public QWidget {
    Q_OBJECT
    void setup();

public:
    explicit WelcomeWidget(QWidget *parent);
};

class LineNumberArea;

class CodeEditWidget : public QPlainTextEdit {
    friend class LineNumberArea;

    Q_OBJECT

    LangFileInfo file;
    Highlighter *highlighter;
    bool modified;
    QWidget *lineNumberArea;

    void setup();
    /** Highlight the code between the given lines */
    void highlightCode(int line1, int col1, int line2, int col2, QColor color);
    // friend functions for line number area
    int LNAWidth() const;
    void LNAEvent(const QPaintEvent *event) const;

private slots:
    /** Font setter for configs */
    void onSetFont(const QJsonValue &value);
    /** Adapt the viewport margins */
    void adaptViewport();
    /** Update the line number area when the content changes */
    void updateLineNumberArea(const QRect &rect, int dy);
    /** Highlight the line where the cursor is */
    void highlightLine();
    /** What to do when the text is modified */
    void onTextChanged();
    /** update the cursor position (and tell it to highlighter) */
    void updateCursorPosition() const;


signals:
    void modify();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;

public:
    CodeEditWidget(const QString &filename, QWidget *parent);

    const LangFileInfo &getFile() const;

    QString getTabText() const;
    /* Read the file content and display it */
    void readFile();
    /* Save the file content to the file */
    void saveFile();
    /* Check if the content is modified, if so, ask for save */
    bool askForSave();
};

class CodeTabWidget : public QTabWidget {
    Q_OBJECT

    void setup();

    /* Add a welcome widget */
    void welcome();
    /* Add a code edit widget for the given file */
    void addCodeEdit(const QString &filePath);
    /* Check if the file is opened, if so, remove it */
    void checkRemoveCodeEdit(const QString &filename);

public slots:
    /* Handle file operations (from file tree) */
    void handleFileOperation(const QString &filename, FileOperation operation);
    /* Remove the code edit widget at the given index */
    void removeCodeEdit(int index);
    /* Ask the user before removing the code edit widget */
    void removeCodeEditRequested(int index);
    /* What to do when a widget is modified */
    void widgetModified(int index);
    /* What to do when current tab changed */
    void onCurrentTabChanged(int index) const;

public:
    explicit CodeTabWidget(QWidget *parent);

    void clearAll();
    LangFileInfo currentFile() const;
    CodeEditWidget *curEdit() const;
    CodeEditWidget *editAt(int index) const;
    void save();
};

#endif // CODE_EDIT_H
