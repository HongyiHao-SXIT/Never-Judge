#include "footer.h"

#include <QLabel>
#include <qboxlayout.h>
#include <utility>

#include "../util/file.h"

#define TASK_FREE (-1)

ProgressBarTask::ProgressBarTask(QString text, int max) : text(std::move(text)), max(max) {
    static int id = 0;
    this->id = ++id;
}

FooterWidget::FooterWidget(QWidget *parent) : QFrame(parent), curTaskId(TASK_FREE) {
    fileLabel = new QLabel(this);
    progressBar = new QProgressBar(this);
    reminderLabel = new QLabel(this);
    setup();
}

void FooterWidget::setup() {
    auto *layout = new QHBoxLayout(this);

    setFixedHeight(30);
    layout->setContentsMargins(10, 0, 10, 0);
    fileLabel->setStyleSheet("color: #999999");

    reminderLabel->setText("");
    progressBar->setMaximumWidth(250);
    progressBar->setMinimumWidth(150);
    progressBar->setVisible(false);

    layout->addWidget(fileLabel);
    layout->addStretch(1);
    layout->addWidget(reminderLabel);
    layout->addWidget(progressBar);

    setLayout(layout);

    setStyleSheet(loadText("qss/footer.css"));
}

FooterWidget &FooterWidget::instance() {
    static FooterWidget instance;
    return instance;
}

void FooterWidget::clear() const { fileLabel->setText(""); }

void FooterWidget::setFileLabel(const QString &text) const { fileLabel->setText(text); }

ProgressBarTask FooterWidget::newTask(const QString &text, int max) { return ProgressBarTask(text, max); }

void FooterWidget::updateTask(const ProgressBarTask &task, int value, const QString &newText) {
    if (curTaskId == TASK_FREE || curTaskId == task.id) {
        // accept the update
        reminderLabel->setText(newText == nullptr ? task.text : newText);
        progressBar->setVisible(true);
        progressBar->setMaximum(task.max);
        progressBar->setValue(value);
        curTaskId = task.id;
    }
    // else we ignore the update request
}

void FooterWidget::finishTask(const ProgressBarTask &task) {
    if (curTaskId == task.id) {
        progressBar->setVisible(false);
        curTaskId = TASK_FREE;
        reminderLabel->setText("");
    }

    // TODO: Keep a queue here for other tasks
}
