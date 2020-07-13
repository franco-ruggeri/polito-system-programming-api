//
// Created by fruggeri on 7/9/20.
//

#pragma once

#include <string>
#include <mutex>
#include <unordered_set>
#include "Socket.h"
#include "Jobs.h"

class User {
    std::shared_ptr<Socket> socket;
    std::string nickname;
    Jobs<std::string> incoming_messages;

public:
    User(std::shared_ptr<Socket> socket, std::string nickname);
    std::shared_ptr<Socket> get_socket();
    std::string get_nickname();
    void send_message(const std::string& message);
    std::optional<std::string> receive_message();
    void logout();
};


