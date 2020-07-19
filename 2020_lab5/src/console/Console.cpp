/*
 * Author: Franco Ruggeri
 */

#include "Console.h"
#include <ncurses.h>

Console::Console() {
    initscr();
    noecho();
    ::clear();
    ::refresh();
}

Console::~Console() {
    endwin();
}

void Console::write(const std::string &message) {
    addstr(message.c_str());
}

void Console::write_line(const std::string &message) {
    addstr((message + "\n").c_str());
}

void Console::write_line(const Message &message) {
    addstr((message.to_console() + "\n").c_str());
}

void Console::clear() {
    ::clear();
    ::refresh();
}

void Console::refresh() {
    ::refresh();
}

char Console::read_char() {
    return getch();
}

void Console::close() {
    endwin();
}
