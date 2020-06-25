//
// Created by fruggeri on 6/25/20.
//

#pragma once

#include <optional>
#include "Message.h"

class MessageStore {
    Message *messages;
    int dim;            // current dimension
    const int n;        // increment of dimension when full

    void reallocate(int dim);
    int find_message_pos(long id) const;

public:
    MessageStore(int n);
    ~MessageStore();

    // insert message or overwrite the previous one if a message with the same id is already present
    // interface modified wrt the instructions to leverage the move constructor
    void add(Message m);

    // return the message with the specified id if present
    std::optional<Message> get(long id) const;

    // delete the message with the specified id if present
    bool remove(long id);

    // return number of valid messages and number of free positions (invalid messages)
    std::tuple<int, int> stats() const;

    // compact array and reduce dimension to the minimum multiple of n
    void compact();
};
