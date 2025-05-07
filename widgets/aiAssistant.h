#ifndef AIASSISTANTWIDGET_H
#define AIASSISTANTWIDGET_H

#include <QDockWidget>
#include <QString>
#include <QCoro/Task>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QScrollBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QUrl>
#include "../ide/aiChat.h"
#include "../web/aiClient.h"
#include "../widgets/preview.h"
#include "../web/problemCrawler.h"

// 前向声明
class CodeTabWidget;

class AIAssistantWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit AIAssistantWidget(CodeTabWidget* codeTab, QWidget *parent = nullptr);
    ~AIAssistantWidget() override;

    void setProblemInfo(const QString &title, const QString &description,
                         const QString &inputDesc, const QString &outputDesc,
                         const QString &sampleInput, const QString &sampleOutput);
    void setUserCode(const QString &code);

    // 显示当前题目信息
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

    // 主动获取题目信息
    bool tryGetProblemInfo();

private:
    void setupUI();
    void connectSignals();
    void displayMessage(const AIMessage &message);
    void displayMarkdown(const QString &text, bool isUser);
    void setProgressVisible(bool visible);
    QString extractCodeFromMarkdown(const QString &markdown);
    QString getCurrentCode() const;
    void insertGeneratedCode(CodeTabWidget *codeTabWidget);
    bool getProblemInfoFromPreview(OpenJudgePreviewWidget *preview);
    QCoro::Task<bool> getProblemInfoFromUrl(const QUrl &url);
    QString getFullProblemDescription() const; // 获取完整的题目描述

    // 发送AI请求的辅助函数
    void sendAIRequest(const QString &prompt, const QString &requestType);

    // 调试日志辅助函数
    void logDebug(const QString &message) const;
    void logCurrentProblemInfo() const; // 记录当前题目信息

    // UI组件
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

    // 数据成员
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
