//
// Created by fruggeri on 7/15/20.
//

#pragma once

#include <vector>
#include <queue>
#include <memory>
#include "SharedEditor.h"
#include "Message.h"

class NetworkServer {
    std::vector<std::shared_ptr<SharedEditor>> editors;
    std::queue<Message> messages;
    static int next_editor_id;
public:
    int connect(std::shared_ptr<SharedEditor> shared_editor);
    void disconnect(std::shared_ptr<SharedEditor> shared_editor);
    void send(Message message);
    void dispatch_messages();
};
