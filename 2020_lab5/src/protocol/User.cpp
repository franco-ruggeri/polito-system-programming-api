/*
 * Author: Franco Ruggeri
 */

#include "User.h"

namespace chat_room {
    User::User(std::shared_ptr<Socket> socket, std::string nickname) : socket_(socket), nickname_(nickname) {}

    std::shared_ptr<Socket> User::socket() {
        return socket_;
    }

    std::string User::nickname() {
        return nickname_;
    }

    void User::send_message(const Message& message) {
        inbox_.put(message);
    }

    std::optional<Message> User::receive_message() {
        return inbox_.get();
    }

    void User::logout() {
        inbox_.close();
    }
}
