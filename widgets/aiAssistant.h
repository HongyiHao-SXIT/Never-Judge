#ifndef AIASSISTANTWIDGET_H
#define AIASSISTANTWIDGET_H

#include <QApplication>
#include <QCoro/Task>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QMutex>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QString>
#include <QTextBrowser>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include "../ide/aiChat.h"
#include "../web/aiClient.h"
#include "../web/problemCrawler.h"
#include "../widgets/preview.h"

// Forward declaration
class CodeTabWidget;

class AIAssistantWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit AIAssistantWidget(CodeTabWidget* codeTab, QWidget *parent = nullptr);
    ~AIAssistantWidget() override;

    void setUserCode(const QString &code) const;

    // Display current problem information
    void showCurrentProblemInfo();

private slots:
    void onSendClicked();
    void onAnalyzeClicked();
    void onCodeClicked();
    void onDebugClicked();
    void onClearClicked();
    void onInsertCodeClicked();
    void onSetApiKeyClicked();
    void onShowProblemClicked();
    void onMessageAdded(const AIMessage &message);
    void onRequestCompleted(bool success, const QString &response);

    // Actively get problem information
    bool tryGetProblemInfo();

private:
    void setupUI();
    void connectSignals();
    void displayMessage(const AIMessage &message);
    void displayMarkdown(const QString &text, bool isUser);
    void setProgressVisible(bool visible) const;
    static QString extractCodeFromMarkdown(const QString &markdown);
    QString getCurrentCode() const;
    void insertGeneratedCode(CodeTabWidget *codeTabWidget);
    bool getProblemInfoFromPreview(OpenJudgePreviewWidget *preview);
    QCoro::Task<bool> getProblemInfoFromUrl(const QUrl &url);
    QString getFullProblemDescription() const;

    // Helper function to send AI request
    static void sendAIRequest(const QString &prompt, const QString &requestType);

    // Debug logging helper functions
    static void logDebug(const QString &message) ;
    void logCurrentProblemInfo() const;

    // UI components
    QWidget *mainWidget = nullptr;
    QVBoxLayout *mainLayout = nullptr;
    QHBoxLayout *buttonLayout = nullptr;
    QHBoxLayout *settingsLayout = nullptr;
    QLabel *apiKeyLabel = nullptr;

    QTextBrowser *conversationView = nullptr;
    QTextEdit *userInput = nullptr;
    QPushButton *sendButton = nullptr;
    QPushButton *clearButton = nullptr;
    QPushButton *analyzeButton = nullptr;
    QPushButton *codeButton = nullptr;
    QPushButton *debugButton = nullptr;
    QPushButton *insertCodeButton = nullptr;
    QPushButton *showProblemButton = nullptr;
    QPushButton *setApiKeyButton = nullptr;
    QProgressBar *progressBar = nullptr;

    // Data members
    CodeTabWidget* m_codeTab = nullptr;
    QString currentTitle;
    QString currentDescription;
    QString currentInputDesc;
    QString currentOutputDesc;
    QString currentSampleInput;
    QString currentSampleOutput;
    QString currentUserCode;
    QString generatedCode;
    QString historyMarkdown;
};

#endif // AIASSISTANTWIDGET_H
