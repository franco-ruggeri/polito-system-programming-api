/*
 * User participating in the chat room.
 *
 * Author: Franco Ruggeri
 */

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
    Jobs<Message> inbox;

public:
    User(std::shared_ptr<Socket> socket, std::string nickname);
    std::shared_ptr<Socket> get_socket();
    std::string get_nickname();
    void send_message(const Message& message);  // put the message in the user's inbox
    std::optional<Message> receive_message();   // get a message from the user's inbox
    void logout();
};
