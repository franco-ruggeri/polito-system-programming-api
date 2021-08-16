/*
 * Advanced console using ncurses.
 *
 * Author: fruggeri
 */

#pragma once

#include <string>
#include "Message.h"

namespace chat_room {
    class Console {
    public:
        Console();
        ~Console();
        void write(const std::string& message);
        void write_line(const std::string& message);
        void write_line(const Message& message);
        void clear();
        void refresh();
        char read_char();
        void close();
    };
}
