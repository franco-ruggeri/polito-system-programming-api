//
// Created by fruggeri on 7/13/20.
//

#include <iostream>
#include <unordered_map>
#include <list>
#include <thread>
#include <mutex>
#include <regex>
#include "protocol.h"
#include "ServerSocket.h"
#include "Jobs.h"
#include "User.h"

#define MAX_CLIENTS 32

Jobs<std::shared_ptr<Socket>> users_login;
Jobs<std::shared_ptr<User>> users_receive;
std::unordered_map<std::string,std::shared_ptr<User>> users;
std::list<std::string> messages;
std::mutex m_output, m_users, m_messages;

void print(std::string msg) {
    std::lock_guard lg(m_output);
    std::cout << msg << std::endl;
}

std::optional<std::shared_ptr<User>> login(std::shared_ptr<Socket> socket) {
    // receive nickname
    std::optional<std::string> nickname = socket->receive_line(protocol::max_idle_time);
    if (!nickname) return std::nullopt;     // connection closed by client or timeout

    // check nickname
    std::unique_lock ul_users(m_users);
    if (users.contains(*nickname) || nickname->find(' ') != std::string::npos) {
        socket->send_line("error: invalid nickname (already used or with spaces)");
        return std::nullopt;
    }

    // create and add user
    std::shared_ptr<User> user = std::make_shared<User>(socket, *nickname);
    users[*nickname] = user;

    // send online users and login message
    std::string message = "[info] " + *nickname + " is now online";
    std::ostringstream oss;
    oss << "[info] " << (users.size() == 1 ? "nobody online" : "online users:");
    for (const auto& u : users) {
        if (u.second != user) {
            u.second->send_message(message);
            oss << " " << u.first;
        }
    }
    ul_users.unlock();
    socket->send_line(oss.str());

    // send last messages
    std::unique_lock ul_messages(m_messages);
    for (const auto& m : messages)
        socket->send_line(m);
    ul_messages.unlock();
    socket->send_line("");  // empty line => end of last messages

    return user;
}

void logout(std::shared_ptr<User> user) {
    user->logout();
    std::lock_guard lg(m_users);
    std::string nickname = user->get_nickname();
    users.erase(nickname);
    for (const auto& u : users)
        u.second->send_message("[info] " + nickname + " logged out");
}

// service: send messages to other users (+ login and logout)
void send_messages() {
    print("New sender - Hello world!");

    while (true) {
        // login
        std::shared_ptr<Socket> socket = *users_login.get();
        std::optional<std::shared_ptr<User>> opt_u = login(socket);
        if (!opt_u) continue;
        std::shared_ptr<User> user = *opt_u;
        std::string nickname = user->get_nickname();

        // launch receive service
        users_receive.put(user);

        while (true) {
            // receive command from user
            std::optional<std::string> command = socket->receive_line(protocol::max_idle_time);

            // connection closed by client or timeout or correct termination => close socket => serve to next user
            if (!command || std::regex_match(*command, protocol::termination)) break;

            // empty command => skip
            if (command->empty()) continue;

            // send message to other user(s) (produce)
            std::unique_lock ul_users(m_users);
            if (std::regex_match(*command, protocol::private_message)) {     // private (to one user)
                // get nickname of recipient
                std::istringstream iss(*command);
                std::string recipient;
                iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
                iss >> recipient;

                // check nickname
                if (recipient == nickname || !users.contains(recipient)) {
                    socket->send_line("error: " + recipient + " is not online");
                } else {
                    // send message to recipient
                    std::string message;
                    std::getline(iss, message);
                    users[recipient]->send_message( "(PM) " + nickname + ":" + message);
                    user->send_message("(PM to " + recipient + ") me:" + message);
                }
            } else {                                                        // public (to all users)
                // send message to all
                std::string message = nickname + ": " + *command;
                for (auto& u : users) {
                    if (u.second == user) continue;
                    u.second->send_message(message);
                }
                ul_users.unlock();
                user->send_message("me: " + *command);

                // add message to last messages
                std::unique_lock ul_messages(m_messages);
                if (messages.size() >= protocol::max_messages)
                    messages.pop_front();
                messages.push_back(message);
            }
        }

        // logout
        logout(user);
    }
}

// service: receive messages from other users
void receive_messages() {
    print("New receiver - Hello world!");

    while (true) {
        // get user
        std::optional<std::shared_ptr<User>> opt_u = users_receive.get();
        if (!opt_u) throw std::logic_error("Error: optional does not contain user, in receiver");
        std::shared_ptr<User> user = *opt_u;
        std::shared_ptr<Socket> socket = user->get_socket();

        while (true) {
            // receive message from other user
            std::optional<std::string> message = user->receive_message();
            if (!message) break;      // user logged out

            // send message to user (consume)
            socket->send_line(*message);
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " port" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (!port) {
        std::cerr << "invalid port" << std::endl;
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
        std::shared_ptr<Socket> s = ss.accept();
        print("Connection accepted from " + s->get_remote_address());

        std::unique_lock ul_users(m_users);
        if (users.size() >= MAX_CLIENTS) {
            s->send_line("error: limit of users reached... retry later");
            print("Connection closed by server");
        } else {
            users_login.put(s);
        }
    }
}
