#ifndef CMD_H
#define CMD_H

#include "language.h"

/**
 * An easy way to run commands in the terminal.
 */
class Command {
    QString cmd;
public:
    Command(QString cmd);
    QString text() const;
    Command merge(const Command &other) const;

    static Command clearScreen();
    static Command changeDirectory(const QString &dir);
    static Command runFile(const LangFileInfo &file);
};

#endif // CMD_H
