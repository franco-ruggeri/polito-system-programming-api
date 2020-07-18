//
// Created by fruggeri on 7/9/20.
//

#pragma once

#include <string>
#include <mutex>
#include <unordered_set>
#include "Socket.h"
#include "Jobs.h"
#include "Message.h"

class User {
    std::shared_ptr<Socket> socket;
    std::string nickname;
    Jobs<Message> incoming_messages;

public:
    User(std::shared_ptr<Socket> socket, std::string nickname);
    std::shared_ptr<Socket> get_socket();
    std::string get_nickname();
    void send_message(const Message& message);
    std::optional<Message> receive_message();
    void logout();
};


