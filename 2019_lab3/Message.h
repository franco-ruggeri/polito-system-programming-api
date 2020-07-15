//
// Created by fruggeri on 7/15/20.
//

#pragma once

#include <memory>
#include "Symbol.h"

class SharedEditor;

class Message {
public:
    typedef enum {
        insert, erase
    } MessageType;

    Message(std::shared_ptr<SharedEditor> source, Symbol symbol, MessageType type);
    std::shared_ptr<SharedEditor> get_source() const;
    MessageType get_type() const;
    Symbol get_symbol() const;

private:
    std::shared_ptr<SharedEditor> source;
    Symbol symbol;
    MessageType type;
};
