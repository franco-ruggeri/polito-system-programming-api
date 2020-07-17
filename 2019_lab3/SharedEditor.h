//
// Created by fruggeri on 7/15/20.
//

#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Symbol.h"
#include "Message.h"
#include "LSEQ.h"

class NetworkServer;

class SharedEditor {
    std::weak_ptr<SharedEditor> self;
    NetworkServer& server;
    int site_id;
    int site_counter;
    std::vector<Symbol> symbols;
    LSEQ pos_allocator;

    SharedEditor(NetworkServer& server);                                                // named constructor idiom
    void remote_insert(const Message& message);
    void remote_erase(const Message& message);

public:
    static std::shared_ptr<SharedEditor> make_shared_editor(NetworkServer& server);     // named constructor idiom
    void local_insert(int index, char value);
    void local_erase(int index);
    void process(const Message& message);
    std::string to_string();
};
