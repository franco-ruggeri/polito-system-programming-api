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

#define MAX_CLIENTS 16
#define MAX_MESSAGES 1024

// TODO: buffered read in Socket.cpp

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
    std::optional<std::string> nickname = socket->receive_line(protocol::max_idle_time);
    if (!nickname) return std::nullopt;     // connection closed by client or timeout

    // check nickname
    std::unique_lock ul_users(m_users);
    if (users.contains(*nickname) || nickname->find(' ') != std::string::npos) {
        socket->send_line(protocol::invalid_nickname);
        return std::nullopt;
    }

    // create and add user
    std::shared_ptr<User> user = std::make_shared<User>(socket, *nickname);
    users[*nickname] = user;
    return user;
}

// service: send messages to other users (+ login and logout)
void send_messages() {
    print("New sender - Hello world!");
    static const std::regex pm_regex("^/private .+ .+$");

    while (true) {
        // login
        std::shared_ptr<Socket> socket = *users_login.get();
        std::optional<std::shared_ptr<User>> opt_u = login(socket);
        if (!opt_u) continue;
        std::shared_ptr<User> user = *opt_u;
        std::string nickname = user->get_nickname();

        // send online users
        std::unique_lock ul_users(m_users);
        std::ostringstream oss;
        oss << "online users:";
        for (const auto& u : users)
            oss << " " << u.first;
        ul_users.unlock();          // before sending, so other threads do not wait uselessly
        socket->send_line(oss.str());

        // send last messages
        std::unique_lock ul_messages(m_messages);
        for (const auto& m : messages)
            socket->send_line(m);
        ul_messages.unlock();

        // launch receive service
        users_receive.put(user);

        while (true) {
            // receive command from user
            std::optional<std::string> command = socket->receive_line(protocol::max_idle_time);
            if (!command) break;    // connection closed by client or timeout

            // correct termination => gracefully terminate
            if (*command == protocol::termination)
                break;

            // send message to other user(s) (produce)
            std::unique_lock ul_users(m_users);
            if (std::regex_match(*command, pm_regex)) {     // private (to one user)
                // get nickname of recipient
                std::istringstream iss(*command);
                std::string recipient;
                iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
                iss >> recipient;

                // check nickname
                if (recipient == nickname || !users.contains(recipient)) {
                    socket->send_line(protocol::invalid_nickname);
                } else {
                    // send message to recipient
                    std::string message;
                    std::getline(iss, message);
                    users[recipient]->send_message(nickname + " (PM):" + message);
                }
            } else {                                        // public (to all users)
                // send message to all
                std::string message = nickname + ": " + *command;
                for (auto& u : users) {
                    if (u.second == user) continue;
                    u.second->send_message(message);
                }
                ul_users.unlock();

                // add message to last messages
                std::unique_lock ul_messages(m_messages);
                if (messages.size() >= MAX_MESSAGES)
                    messages.pop_front();
                messages.push_back(message);
            }
        }
        user->logout();
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
            s->send_line(protocol::max_users_reached);
            print("Connection closed by server");
        } else {
            users_login.put(s);
        }
    }
}
