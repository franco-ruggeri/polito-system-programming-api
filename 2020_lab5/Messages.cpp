/*
 * Author: Franco Ruggeri
 */

#include "Messages.h"

Messages::Messages(unsigned int max_messages) : max_messages(max_messages) {}

void Messages::push(const Message& message) {
    if (messages.size() >= max_messages)
        messages.pop_front();
    messages.push_back(message);
}

std::list<Message>::iterator Messages::begin() {
    return messages.begin();
}

std::list<Message>::iterator Messages::end() {
    return messages.end();
}
