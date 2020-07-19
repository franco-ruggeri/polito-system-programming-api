/*
 * Author: Franco Ruggeri
 */

#include "User.h"

User::User(std::shared_ptr<Socket> socket, std::string nickname) : socket(socket), nickname(nickname) {}

std::shared_ptr<Socket> User::get_socket() {
    return socket;
}

std::string User::get_nickname() {
    return nickname;
}

void User::send_message(const Message& message) {
    inbox.put(message);
}

std::optional<Message> User::receive_message() {
    return inbox.get();
}

void User::logout() {
    inbox.close();
}
