#include "footer.h"

#include <QLabel>

FooterWidget::FooterWidget(QWidget *parent) : QWidget(parent) { setup(); }

void FooterWidget::setup() {
    setFixedHeight(20);
    auto label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setText("Footer");
    label->setStyleSheet("color: #999999");
}
