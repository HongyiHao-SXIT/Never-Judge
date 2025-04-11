#ifndef FOOTER_H
#define FOOTER_H

#include <QLabel>
#include <QProgressBar>

class FooterWidget;

class ProgressBarTask {
    friend class FooterWidget;

    unsigned int id;
    QString text;
    int max;

    explicit ProgressBarTask(QString  text, int max);
};

class FooterWidget : public QFrame {
    Q_OBJECT

    QLabel *fileLabel;
    QLabel *reminderLabel;
    QProgressBar *progressBar;
    int curTaskId; // set -1 to be no task

    void setup();
    explicit FooterWidget(QWidget *parent = nullptr);

public:
    // Use singleton for global access
    static FooterWidget &instance();
    void clear() const;
    void setFileLabel(const QString &text) const;

    /** Generate a new task. */
    static ProgressBarTask newTask(const QString &text = "", int max = 100);
    /** Update the progress bar with the current task. Here newText is optional */
    void updateTask(const ProgressBarTask &task, int value, const QString &newText = nullptr);
    /** Finish the current task. */
    void finishTask(const ProgressBarTask &task);
};

#endif // FOOTER_H
