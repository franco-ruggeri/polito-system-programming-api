//
// Created by fruggeri on 7/9/20.
//

#include "User.h"

User::User(std::shared_ptr<Socket> socket, std::string nickname) : socket(socket), nickname(nickname) {}

std::shared_ptr<Socket> User::get_socket() {
    return socket;
}

std::string User::get_nickname() {
    return nickname;
}

void User::send_message(const Message& message) {
    incoming_messages.put(message);
}

std::optional<Message> User::receive_message() {
    return incoming_messages.get();
}

void User::logout() {
    incoming_messages.close();
}
