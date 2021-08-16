/*
 * Client for chat room. It uses 3 threads:
 * - Refresher: waits for I/O events and updates the console, such that input and output are not mixed.
 * - Sender: send user's commands to the server.
 * - Receiver: receive other users' messages.
 *
 * Author: Franco Ruggeri
 */

#include <iostream>
#include <thread>
#include <optional>
#include <mutex>
#include <condition_variable>
#include "Socket.h"
#include "Message.h"
#include "Messages.h"
#include "Console.h"

using namespace chat_room;

const unsigned int max_messages = 32;

Console console;
std::string command;
Messages last_messages(max_messages);
bool io_event=false, terminate_now=false, terminate_after_key=false;
std::mutex m;
std::condition_variable cv;

void refresh_console() {
    while (true) {
        std::unique_lock ul(m);

        // wait I/O event
        cv.wait(ul, []() { return io_event; });
        if (terminate_now) break;
        io_event = false;

        // refresh console
        console.clear();
        for (const auto& m : last_messages)
            console.write_line(m);
        console.write(">> " + command);
        console.refresh();
    }
}

void login(std::shared_ptr<Socket> socket, const std::string& nickname) {
    Message message;
    std::optional<std::string> opt_m;

    // send nickname
    message = Message(MessageType::login, nickname);
    socket->send_line(message.to_network());

    // receive online users
    opt_m = socket->receive_line();
    if (!opt_m) throw std::runtime_error("connection error");
    message = make_message_from_network(*opt_m);
    if (message.type() != MessageType::online_users) throw std::logic_error("expected online users");
    last_messages.push(message);

    // receive last messages
    while (true) {
        opt_m = socket->receive_line();
        if (!opt_m) throw std::runtime_error("connection error");
        message = make_message_from_network(*opt_m);
        MessageType type = message.type();
        if (type == MessageType::done_last_messages) break;
        if (type != MessageType::public_message) throw std::logic_error("expected last messages");
        last_messages.push(message);
    }

    // show
    std::lock_guard lg(m);
    io_event = true;
    cv.notify_one();
}

void send_message(const std::shared_ptr<Socket>& socket, std::string nickname) {
    Message message = make_message_from_command(nickname, command);
    socket->send_line(message.to_network());
    if (message.type() == MessageType::quit) terminate_now = true;
    else last_messages.push(message);
    command.clear();
}

void send_messages(std::shared_ptr<Socket> socket, const std::string& nickname) {
    while (true) {
        char c = console.read_char();
        std::lock_guard lg(m);

        if (terminate_after_key) {
            terminate_now = true;
        } else {
            switch (c) {
                case '\n':
                    send_message(socket, nickname);
                    break;
                case 127:   // delete character
                    if (!command.empty())
                        command.pop_back();
                    break;
                default:
                    command.push_back(c);
                    break;
            }
        }

        io_event = true;
        cv.notify_one();
        if (terminate_now) break;
    }
}

void receive_messages(std::shared_ptr<Socket> socket) {
    Message message;
    bool terminate_receive = false;

    while (!terminate_receive) {
        std::optional<std::string> opt_m = socket->receive_line();
        std::lock_guard lg(m);
        if (!opt_m) {
            message = Message(MessageType::error, "disconnected by the server, press a key to terminate");
            terminate_after_key = terminate_receive = true;
        } else {
            message = make_message_from_network(*opt_m);
        }

        last_messages.push(message);
        io_event = true;
        cv.notify_one();
    }
}

void error(Console& console, const std::string& message) {
    console.close();
    std::cerr << message << std::endl;
    std::exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    const std::string usage = std::string{} + "usage: " + argv[0] + " ip_address port nickname";

    if (argc < 4)
        error(console, usage);

    // get IP address
    std::string ip_address = argv[1];

    // parse nickname
    std::string nickname = argv[3];
    if (nickname.find(' ') != std::string::npos)
        error(console, "[error] invalid nickname, no whitespace allowed");

    // parse port
    int port;
    try {
        port = std::stoi(argv[2]);
    } catch (const std::invalid_argument& e) {
        error(console, "[error] invalid port");
    }

    // connect to the server
    console.write("[info] connecting to the server...");
    console.refresh();
    std::shared_ptr<Socket> socket;
    try {
        socket = std::make_shared<Socket>(ip_address, port);
        login(socket, nickname);
    } catch (const std::exception& e) {
        error(console, std::string("[error] ") + e.what());
    }

    // chat
    std::thread refresher(refresh_console);
    std::thread sender(send_messages, socket, nickname);
    std::thread receiver(receive_messages, socket);

    // terminate
    sender.join();
    receiver.join();
    refresher.join();
}