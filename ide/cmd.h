#ifndef CMD_H
#define CMD_H

#include "../ide/project.h"

/**
 * An easy way to run commands in the terminal.
 * TODO: This is designed for Linux only currently.
 */
class Command {
    QString cmd;

public:
    Command();
    Command(QString cmd);
    QString text() const;
    Command merge(const Command &other) const;

    static Command clearScreen();
    static Command changeDirectory(const QString &dir);
    static Command runFile(const FileInfo &file);
};

#endif // CMD_H
