#include "footer.h"

#include <QLabel>
#include <qboxlayout.h>

FooterWidget::FooterWidget(QWidget *parent) : QFrame(parent) {
    fileLabel = new QLabel(this);
    // reminderLabel = new QLabel(this);
    setup();
}

void FooterWidget::setup() {
    auto layout = new QHBoxLayout(this);

    setFixedHeight(20);
    layout->setContentsMargins(5, 0, 5, 0);
    fileLabel->setText("Footer");
    fileLabel->setStyleSheet("color: #999999");
    layout->addWidget(fileLabel);

    setLayout(layout);
}

FooterWidget &FooterWidget::instance() {
    static FooterWidget instance;
    return instance;
}

void FooterWidget::clear() const {
    fileLabel->setText("");
}

void FooterWidget::setFileLabel(const QString &text) const {
    fileLabel->setText(text);
}
