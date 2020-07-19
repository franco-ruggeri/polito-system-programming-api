/*
 * Container of Message objects. It has a limited size. When a new message is pushed and the container is full,
 * the oldest one gets discarded (FIFO).
 *
 * Author: Franco Ruggeri
 */

#pragma once

#include <list>
#include "Message.h"

class Messages {
    std::list<Message> messages;
    const unsigned int max_messages;

public:
    Messages(unsigned int max_messages);
    void push(const Message& message);      // add message and discards oldest one if full
    std::list<Message>::iterator begin();
    std::list<Message>::iterator end();
};
