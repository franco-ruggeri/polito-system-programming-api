//
// Created by fruggeri on 6/25/20.
//

#include <tuple>
#include "MessageStore.h"

void MessageStore::reallocate(int dim) {
    auto *new_mess = new Message[dim];
    int free_pos = 0;
    for (int i=0; i<this->dim; i++)
        if (this->messages[i].getId() != Message::INVALID_ID)
            new_mess[free_pos++] = std::move(this->messages[i]);    // move!
    delete[] this->messages;
    this->messages = new_mess;
    this->dim = dim;
}

int MessageStore::find_message_pos(long id) const {
    for (int i=0; i<this->dim; i++)
        if (this->messages[i].getId() == id)
            return i;
    return -1;
}

MessageStore::MessageStore(int n): dim(n), n(n), messages(new Message[n]) {}

MessageStore::~MessageStore() {
    delete[] messages;
}

void MessageStore::add(Message m) {
    long id = m.getId();
    int pos;

    if (id == Message::INVALID_ID)
        return;

    // search position and add
    pos = this->find_message_pos(id);                   // id present => overwrite
    if (pos == -1) {
        pos = find_message_pos(Message::INVALID_ID);    // id not present => first free position
        if (pos == -1) {                                // full => reallocate and take first free position
            pos = this->dim;
            this->reallocate(this->dim + this->n);
        }
    }
    this->messages[pos] = std::move(m);
}

std::optional<Message> MessageStore::get(long id) const {
    int pos = this->find_message_pos(id);
    return pos == -1 ? std::nullopt : std::make_optional(this->messages[pos]);
}

bool MessageStore::remove(long id) {
    int pos = this->find_message_pos(id);
    if (pos == -1)
        return false;
    this->messages[pos] = Message{};
    return true;
}

std::tuple<int, int> MessageStore::stats() const {
    int valid=0, invalid=0;

    for (int i=0; i<this->dim; i++)
        if (this->messages[i].getId() == Message::INVALID_ID)
            invalid++;
        else
            valid++;

    return std::make_tuple(valid, invalid);
}

void MessageStore::compact() {
    int valid = std::get<0>(this->stats());
    int new_dim = valid % n == 0 ? valid : (valid/n + 1) * n;
    this->reallocate(new_dim);
}
