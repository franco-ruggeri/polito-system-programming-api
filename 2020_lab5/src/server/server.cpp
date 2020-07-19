/*
 * Concurrent server for chat room. It uses 4 thread pools:
 * - Login: logs in connecting user.
 * - Send: send messages from user to the others.
 * - Receive: receive messages from others.
 * - Logout: logs out disconnected user.
 *
 * Since thread pools are used, there is a maximum number of online users.
 * The settings are indicative and should be tuned accordingly to the available resources.
 *
 * Author: Franco Ruggeri
 */

#include <iostream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ServerSocket.h"
#include "Socket.h"
#include "Jobs.h"
#include "User.h"
#include "Messages.h"

#define N_LOGIN_THREADS 2
#define N_LOGOUT_THREADS 1
#define MAX_CLIENTS 32
#define MAX_IDLE_TIME 60
#define MAX_MESSAGES 32

// TODO: namespace + underscore

Jobs<std::shared_ptr<Socket>> users_login(MAX_CLIENTS);         // producer=main, consumer=login
Jobs<std::shared_ptr<User>> users_send(MAX_CLIENTS);            // producer=login, consumer=send
Jobs<std::shared_ptr<User>> users_receive(MAX_CLIENTS);         // producer=login, consumer=receive
Jobs<std::shared_ptr<User>> users_logout(MAX_CLIENTS);          // producer=send, consumer=logout
std::unordered_map<std::string,std::shared_ptr<User>> online_users;
Messages last_messages(MAX_MESSAGES);
std::mutex m_log, m_users, m_messages;
std::condition_variable cv_users;

void log(std::string msg) {
    std::lock_guard lg(m_log);
    std::clog << msg << std::endl;
}

void send_error(std::shared_ptr<Socket> socket, const std::string& what_arg) {
    Message message(MessageType::error, what_arg);
    socket->send_line(message.to_network());
}

std::shared_ptr<User> login() {
    while (true) {
        std::shared_ptr<Socket> socket = *users_login.get();
        std::shared_ptr<User> user;
        Message message;
        std::string what_arg;

        try {
            // receive nickname
            std::optional<std::string> opt_m = socket->receive_line(MAX_IDLE_TIME);
            if (!opt_m) continue;
            message = make_message_from_network(*opt_m);
            if (message.get_type() != MessageType::login) send_error(socket, "expected login");
            std::string nickname = message.get_message();

            // check nickname
            std::unique_lock ul_users(m_users);
            bool correct_nickname = false;
            if (online_users.contains(nickname)) send_error(socket, "nickname already used");
            else if (nickname.find(' ') != std::string::npos) send_error(socket, "nickname with whitespaces");
            else correct_nickname = true;
            if (!correct_nickname) continue;

            // create and add user
            user = std::make_shared<User>(socket, nickname);
            online_users[nickname] = user;

            // wait if limit of users reached
            cv_users.wait(ul_users, []() { return online_users.size() < MAX_CLIENTS; });

            // prepare online users and send login message to others
            message = Message(MessageType::login, nickname);
            std::ostringstream oss;
            for (const auto &u : online_users) {
                if (u.second == user) continue;
                u.second->send_message(message);
                oss << u.first << " ";
            }
            ul_users.unlock();

            // send online users to new user
            message = Message(MessageType::online_users, oss.str());
            socket->send_line(message.to_network());

            // send last messages to new user
            std::unique_lock ul_messages(m_messages);
            for (const auto &m : last_messages)
                socket->send_line(m.to_network());
            ul_messages.unlock();
            message = Message(MessageType::done_last_messages);
            socket->send_line(message.to_network());

        } catch (const std::exception& e) {
            log(std::string("during login: ") + e.what());
            continue;   // next user (robust server)
        }

        // start send and receive services (blocking)
        users_send.put(user);
        users_receive.put(user);
    }
}

void logout() {
    while (true) {
        std::shared_ptr<User> user = *users_logout.get();
        std::string nickname = user->get_nickname();

        // logout
        user->logout();

        // erase user
        std::lock_guard lg(m_users);
        online_users.erase(nickname);

        // send logout message to others
        Message message(MessageType::logout, nickname);
        for (const auto& u : online_users)
            u.second->send_message(message);

        // notify spot for new user
        cv_users.notify_one();
    }
}

void send_private_message(std::shared_ptr<User> user, Message message) {
    std::shared_ptr<Socket> socket = user->get_socket();
    std::string destination = message.get_destination();
    std::string source = message.get_source();

    std::unique_lock ul_users(m_users);
    if (destination == source) send_error(socket, "you cannot send PM to yourself");
    else if (source != user->get_nickname()) send_error(socket, "no source spoofing thanks ;)");
    else if (!online_users.contains(destination)) send_error(socket, destination + " is not online");
    else online_users[destination]->send_message(message);
}

void send_public_message(std::shared_ptr<User> user, const Message& message) {
    // send to others
    std::unique_lock ul_users(m_users);
    for (auto &u : online_users)
        if (u.second != user)
            u.second->send_message(message);
    ul_users.unlock();

    // add to last messages
    std::unique_lock ul_messages(m_messages);
    last_messages.push(message);
}

void send_messages() {
    log("new sender - hello world!");

    while (true) {
        // get user
        std::shared_ptr<User> user = *users_send.get();
        std::shared_ptr<Socket> socket = user->get_socket();
        bool next = false;

        // serve user: send messages to other users
        while (!next) {
            // receive command from user
            std::optional<std::string> opt_m = socket->receive_line(MAX_IDLE_TIME);
            if (!opt_m) break;
            try {
                Message message = make_message_from_network(*opt_m);
                MessageType type = message.get_type();

                // execute command
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
                        send_error(socket, "invalid type");
                        next = true;
                        break;
                }
            } catch (const std::exception& e) {
                log(std::string("while sending: ") + e.what());
                break;   // logout and next user (robust server)
            }
        }

        // start logout service
        users_logout.put(user);
    }
}

void receive_messages() {
    log("new receiver - hello world!");

    while (true) {
        // get user
        std::shared_ptr<User> user = *users_receive.get();
        std::shared_ptr<Socket> socket = user->get_socket();

        // serve user: receive messages from other users
        while (true) {
            // receive message from other user
            std::optional<Message> opt_m = user->receive_message();
            if (!opt_m) break;      // user logged out
            Message message = *opt_m;

            // send message to user
            try {
                socket->send_line(message.to_network());
            } catch (...) {
                // error (e.g. broken pipe due to connection closed by client)
                // nothing to do here... just serve next user
                break;
            }
        }
    }
}

int main(int argc, char **argv) {
    const std::string usage = std::string{} + "usage: " + argv[0] + " port";

    if (argc < 2) {
        std::cout << usage << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // parse port
    int port;
    try {
        port = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "invalid port" << std::endl;
        std::cout << usage << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // launch thread for login
    for (int i=0; i<N_LOGIN_THREADS; i++) {
        std::thread t(login);
        t.detach();
    }

    // launch thread for logout
    for (int i=0; i<N_LOGOUT_THREADS; i++) {
        std::thread t(logout);
        t.detach();
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

    // listen
    ServerSocket server_socket(port);
    while (true) {
        // accept
        std::shared_ptr<Socket> socket;
        try {
            socket = server_socket.accept();
        } catch (const std::runtime_error& e) {
            log(e.what());
            continue;
        }

        // start login service
        std::unique_lock ul_users(m_users);
        users_login.put(socket);
        log("connection accepted from " + socket->get_remote_address());
    }
}
