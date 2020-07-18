//
// Created by fruggeri on 7/18/20.
//

#pragma once

#include <list>
#include "Message.h"

class Messages {
    std::list<Message> messages;
    const unsigned int max_messages;

public:
    Messages(unsigned int max_messages);
    void push(const Message& message);
    std::list<Message>::iterator begin();
    std::list<Message>::iterator end();
};
