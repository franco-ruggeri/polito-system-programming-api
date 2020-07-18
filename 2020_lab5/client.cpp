//
// Created by fruggeri on 7/13/20.
//

#include <iostream>
#include <thread>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <ncurses.h>
#include "Socket.h"
#include "Message.h"
#include "Messages.h"

#define MAX_MESSAGES 32

Messages messages(MAX_MESSAGES);
std::string command;
bool io_event=false, terminate_now=false, terminate_after_key=false;
std::mutex m_console;
std::condition_variable cv_console;

void refresh_console(const std::string& nickname) {
    while (true) {
        std::unique_lock ul(m_console);

        // wait I/O event
        cv_console.wait(ul, []() { return io_event; });
        if (terminate_now) break;
        io_event = false;

        // update screen
        clear();
        refresh();
        for (const auto& m : messages)
            addstr(m.to_console().c_str());
        addstr((">> " + command).c_str());
        refresh();
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
    if (message.get_type() != MessageType::online_users) throw std::logic_error("expected online users");
    messages.push(message);

    // receive last messages
    while (true) {
        opt_m = socket->receive_line();
        if (!opt_m) throw std::runtime_error("connection error");
        message = make_message_from_network(*opt_m);
        MessageType type = message.get_type();
        if (type == MessageType::done_last_messages) break;
        if (type != MessageType::public_message) throw std::logic_error("expected last messages");
        messages.push(message);
    }

    // show
    std::lock_guard lg(m_console);
    io_event = true;
    cv_console.notify_one();
}

// service: send messages to other users
void send_messages(std::shared_ptr<Socket> socket, std::string nickname) {
    while (true) {
        char c = getch();
        std::lock_guard lg(m_console);
        if (terminate_after_key) {
            terminate_now = true;
        } else if (c == '\n') {         // end of command
            Message message = make_message_from_command(nickname, command);
            socket->send_line(message.to_network());
            if (message.get_type() == MessageType::quit) terminate_now = true;
            else messages.push(message);
            command.clear();
        } else if (c == 127) {          // delete character
            if (!command.empty())
                command.pop_back();
        } else {                        // insert character
            command.push_back(c);
        }
        io_event = true;
        cv_console.notify_one();
        if (terminate_now) break;
    }
}

// service: receive messages from other users
void receive_messages(std::shared_ptr<Socket> socket) {
    bool terminate_receive = false;
    while (!terminate_receive) {
        std::optional<std::string> opt_m = socket->receive_line();
        std::lock_guard lg(m_console);
        if (!opt_m) {
            messages.push(Message(MessageType::error, "disconnected by the server, press a key to terminate"));
            terminate_after_key = terminate_receive = true;
        } else {
            messages.push(make_message_from_network(*opt_m));
        }
        io_event = true;
        cv_console.notify_one();
    }
}

void close_console() {
    endwin();
}

int main(int argc, char **argv) {
    const std::string usage = std::string{} + "usage: " + argv[0] + " ip_address port nickname";

    if (argc < 4) {
        std::cerr << usage << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string ip_address = argv[1];

    // parse nickname
    std::string nickname = argv[3];
    if (nickname.find(' ') != std::string::npos) {
        std::cerr << "[error] invalid nickname, no whitespace allowed" << std::endl;
        std::cerr << usage << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // parse port
    int port;
    try {
        port = std::stoi(argv[2]);
    } catch (std::invalid_argument e) {
        std::cerr << "[error] invalid port" << std::endl;
        std::cerr << usage << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // init console
    initscr();
    noecho();
    clear();
    refresh();
    addstr("[info] connecting to the server...\n");
    refresh();
    std::atexit(close_console);

    // connect to the server
    std::shared_ptr<Socket> socket;
    try {
        socket = std::make_shared<Socket>(ip_address, port);
    } catch (std::runtime_error e) {
        addstr((std::string("[error]") + e.what() + "\n").c_str());
        std::exit(EXIT_FAILURE);
    }

    // login
    try {
        login(socket, nickname);
    } catch (std::exception e) {
        addstr((std::string("[error]") + e.what() + "\n").c_str());
        std::exit(EXIT_FAILURE);
    }

    // chat
    std::thread refresher(refresh_console, nickname);
    std::thread sender(send_messages, socket, nickname);
    std::thread receiver(receive_messages, socket);

    // terminate
    sender.join();
    receiver.join();
    refresher.join();
}