//
// Created by fruggeri on 7/15/20.
//

#include "Message.h"

Message::Message(std::shared_ptr<SharedEditor> source, Symbol symbol, MessageType type) :
                source(source), symbol(symbol), type(type) {}

std::shared_ptr<SharedEditor> Message::get_source() const {
    return source;
}

MessageType Message::get_type() const {
    return type;
}

Symbol Message::get_symbol() const {
    return symbol;
}
