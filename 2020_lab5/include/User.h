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

namespace chat_room {
    class User {
        std::shared_ptr<Socket> socket_;
        std::string nickname_;
        Jobs<Message> inbox_;

    public:
        User(std::shared_ptr<Socket> socket, std::string nickname);
        std::shared_ptr<Socket> socket() const;
        std::string nickname() const;
        void send_message(const Message& message);  // put the message in the user's inbox
        std::optional<Message> receive_message();   // get a message from the user's inbox
        void logout();
    };
}
