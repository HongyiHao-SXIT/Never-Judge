#ifndef AIASSISTANTWIDGET_H
#define AIASSISTANTWIDGET_H

#include <QWidget>       // <-- Add back: Needed for QWidget base class
#include <QString>       // <-- Add back: Needed for QString members
#include <QCoro/Task>    // <-- Add back: Needed for QCoro::Task<> return types
#include <QJsonObject>
#include <QJsonArray>

// Forward declarations
class QPushButton;
class QTextBrowser;
class QPlainTextEdit;
class QProgressBar;
class AIClient;
class AIChatManager;
class CodeTabWidget;

class AIAssistantWidget : public QWidget {
    Q_OBJECT

public:
    explicit AIAssistantWidget(CodeTabWidget* codeTab, QWidget *parent = nullptr);
    ~AIAssistantWidget() override;

    void setProblemInfo(const QString &title, const QString &description,
                         const QString &inputDesc, const QString &outputDesc,
                         const QString &sampleInput, const QString &sampleOutput);
    void setUserCode(const QString &code);

private slots:
    QCoro::Task<> onSendClicked();
    QCoro::Task<> onAnalyzeClicked();
    QCoro::Task<> onCodeClicked();
    QCoro::Task<> onDebugClicked();

    void onClearClicked();
    void onInsertCodeClicked();
    void onSetApiKeyClicked();

private:
    void setupUI();
    void connectSignals();
    void displayMarkdown(const QString &text, bool isUser);
    void setProgressVisible(bool visible);
    QString extractCodeFromMarkdown(const QString &markdown);

    QTextBrowser *conversationView = nullptr;
    QPlainTextEdit *userInput = nullptr;
    QPushButton *sendButton = nullptr;
    QPushButton *clearButton = nullptr;
    QPushButton *analyzeButton = nullptr;
    QPushButton *codeButton = nullptr;
    QPushButton *debugButton = nullptr;
    QPushButton *insertCodeButton = nullptr;
    QPushButton *setApiKeyButton = nullptr;
    QProgressBar *progressBar = nullptr;

    AIClient *aiClient = nullptr;
    AIChatManager *aiChatManager = nullptr;
    CodeTabWidget* m_codeTab = nullptr;
    QString currentTitle;
    QString currentDescription;
    QString currentInputDesc;
    QString currentOutputDesc;
    QString currentSampleInput;
    QString currentSampleOutput;
    QString currentUserCode;
    QString generatedCode;
};

#endif // AIASSISTANTWIDGET_H
