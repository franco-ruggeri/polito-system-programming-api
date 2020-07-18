//
// Created by fruggeri on 7/13/20.
//

#include <iostream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include "ServerSocket.h"
#include "Jobs.h"
#include "User.h"
#include "Message.h"
#include "Messages.h"

#define MAX_CLIENTS 32
#define MAX_IDLE_TIME 60
#define MAX_MESSAGES 32

// TODO: code refactor (comments, declaration at the beginning, more functions)

Jobs<std::shared_ptr<Socket>> users_login(MAX_CLIENTS);
Jobs<std::shared_ptr<User>> users_receive(MAX_CLIENTS);
std::unordered_map<std::string,std::shared_ptr<User>> users;
Messages messages(MAX_MESSAGES);
std::mutex m_output, m_users, m_messages;

void print(std::string msg) {
    std::lock_guard lg(m_output);
    std::clog << msg << std::endl;
}

std::shared_ptr<User> login(std::shared_ptr<Socket> socket) {
    Message message;
    std::optional<std::string> opt_m;
    std::string nickname;

    // receive nickname
    opt_m = socket->receive_line(MAX_IDLE_TIME);
    if (!opt_m) throw std::runtime_error("connection error");
    message = make_message_from_network(*opt_m);
    if (message.get_type() != MessageType::login) throw std::logic_error("expected login");
    nickname = message.get_message();

    // check nickname
    std::unique_lock ul_users(m_users);
    if (users.contains(nickname)) {
        message = Message(MessageType::error, "nickname already used");
        socket->send_line(message.to_network());
        throw std::runtime_error("nickname already used");
    }

    // create and add user
    std::shared_ptr<User> user = std::make_shared<User>(socket, nickname);
    users[nickname] = user;

    // send online users and login message
    message = Message(MessageType::login, nickname);
    std::ostringstream oss;
    for (const auto& u : users) {
        if (u.second != user) {
            u.second->send_message(message);
            oss << u.first << " ";
        }
    }
    ul_users.unlock();
    message = Message(MessageType::online_users, oss.str());
    socket->send_line(message.to_network());

    // send last messages
    std::unique_lock ul_messages(m_messages);
    for (const auto& m : messages)
        socket->send_line(m.to_network());
    ul_messages.unlock();
    message = Message(MessageType::done_last_messages);
    socket->send_line(message.to_network());

    return user;
}

void logout(std::shared_ptr<User> user) {
    user->logout();
    std::string nickname = user->get_nickname();
    std::lock_guard lg(m_users);
    users.erase(nickname);
    Message message(MessageType::logout, nickname);
    for (const auto& u : users)
        u.second->send_message(message);
}

void send_private_message(std::shared_ptr<User> user, Message message) {
    std::unique_lock ul_users(m_users);

    // check message
    std::string destination = message.get_destination();
    std::string source = message.get_source();
    if (destination == source)
        message = Message(MessageType::error, "you cannot send PM to yourself");
    else if (source != user->get_nickname())
        message = Message(MessageType::error, "no source spoofing thanks ;)");
    else if (!users.contains(destination))
        message = Message(MessageType::error, destination + " is not online");

    // send message
    MessageType type = message.get_type();
    if (type == MessageType::error) user->send_message(message);
    else users[destination]->send_message(message);
}

void send_public_message(std::shared_ptr<User> user, const Message& message) {
    // send to all users
    std::unique_lock ul_users(m_users);
    for (auto &u : users)
        if (u.second != user)
            u.second->send_message(message);
    ul_users.unlock();

    // add message to last messages
    std::unique_lock ul_messages(m_messages);
    messages.push(message);
}

// service: send messages to other users (+ login and logout)
void send_messages() {
    print("new sender - hello world!");

    while (true) {
        std::shared_ptr<User> user;

        try {
            // login
            std::shared_ptr<Socket> socket = *users_login.get();
            user = login(socket);
            std::string nickname = user->get_nickname();

            // launch receive service
            users_receive.put(user);

            bool next = false;
            while (!next) {
                // receive command from user
                std::optional<std::string> opt_m = socket->receive_line(MAX_IDLE_TIME);
                if (!opt_m) break;
                Message message = make_message_from_network(*opt_m);
                MessageType type = message.get_type();

                switch (type) {
                    case MessageType::quit:
                        next = true;
                        break;
                    case MessageType::private_message:
                        send_private_message(user, message);
                        break;
                    case MessageType::public_message:
                        send_public_message(user, message);
                        break;
                    default:
                        message = Message(MessageType::error, "wrong type");
                        next = true;
                        break;
                }
            }
        } catch (...) {
            // error (e.g. broken pipe due to connection closed by client or protocol not respected by client)
            // nothing to do here... logout the user and serve next one
        }

        // logout
        if (user) logout(user);
    }
}

// service: receive messages from other users
void receive_messages() {
    print("new receiver - hello world!");

    while (true) {
        // get user
        std::shared_ptr<User> user = *users_receive.get();
        std::shared_ptr<Socket> socket = user->get_socket();

        while (true) {
            // receive message from other user
            std::optional<Message> opt_m = user->receive_message();
            if (!opt_m) break;      // user logged out
            Message message = *opt_m;

            // send message to user
            try {
                socket->send_line(message.to_network());
            } catch (...) {
                // error (e.g. broken pipe due to connection closed by client or protocol not respected by client)
                // nothing to do here... just serve next user
                break;
            }
        }
    }
}

int main(int argc, char **argv) {
    const std::string usage = std::string{} + "usage: " + argv[0] + " port";

    if (argc < 2) {
        std::cerr << usage << std::endl;
        std::exit(EXIT_FAILURE);
    }

    int port;
    try {
        port = std::stoi(argv[1]);
    } catch (std::invalid_argument e) {
        std::cerr << "invalid port" << std::endl;
        std::cerr << usage << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // launch thread pool of receivers (receive messages from other users, consumers)
    for (int i=0; i<MAX_CLIENTS; i++) {
        std::thread t(receive_messages);
        t.detach();
    }

    // launch thread pool of senders (send messages to other users, producers)
    for (int i=0; i<MAX_CLIENTS; i++) {
        std::thread t(send_messages);
        t.detach();
    }

    // accept sockets
    ServerSocket ss(port);
    while (true) {
        std::shared_ptr<Socket> s;

        // accept connection
        try {
            s = ss.accept();
        } catch (std::runtime_error e) {
            print(e.what());
            continue;
        }

        // serve connection (concurrently)
        std::unique_lock ul_users(m_users);
        users_login.put(s);
        print("connection accepted from " + s->get_remote_address());
    }
}
