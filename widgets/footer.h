#ifndef FOOTER_H
#define FOOTER_H

#include <QLabel>
#include <QWidget>

class FooterWidget : public QWidget {
    Q_OBJECT

    QLabel *fileLabel;
    QLabel *reminderLabel;

    void setup();

public:
    explicit FooterWidget(QWidget *parent = nullptr);
};

#endif // FOOTER_H
