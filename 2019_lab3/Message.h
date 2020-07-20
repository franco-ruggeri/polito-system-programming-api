//
// Created by fruggeri on 7/15/20.
//

#pragma once

#include <memory>
#include "Symbol.h"
#include "MessageType.h"

class SharedEditor;

class Message {
    std::shared_ptr<SharedEditor> source;
    Symbol symbol;
    MessageType type;

public:
    Message(std::shared_ptr<SharedEditor> source, const Symbol& symbol, MessageType type);
    std::shared_ptr<SharedEditor> get_source() const;
    MessageType get_type() const;
    Symbol get_symbol() const;
};
