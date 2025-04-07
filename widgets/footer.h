#ifndef FOOTER_H
#define FOOTER_H

#include <QLabel>
#include <QWidget>

class FooterWidget : public QFrame {
    Q_OBJECT

    QLabel *fileLabel;
    // QLabel *reminderLabel;

    void setup();
    explicit FooterWidget(QWidget *parent = nullptr);

public:
    // Use singleton for global access
    static FooterWidget &instance();
    void clear() const;
    void setFileLabel(const QString &text) const;
};

#endif // FOOTER_H
