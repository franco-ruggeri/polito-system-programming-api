/*
 * Author: Franco Ruggeri
 */

#include "Messages.h"

namespace chat_room {
    Messages::Messages(unsigned int max_messages) : max_messages_(max_messages) {}

    void Messages::push(const Message& message) {
        if (messages_.size() >= max_messages_)
            messages_.pop_front();
        messages_.push_back(message);
    }

    std::list<Message>::iterator Messages::begin() {
        return messages_.begin();
    }

    std::list<Message>::iterator Messages::end() {
        return messages_.end();
    }
}
