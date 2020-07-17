//
// Created by fruggeri on 7/13/20.
//

#include <iostream>
#include <thread>
#include <optional>
#include <list>
#include <mutex>
#include <condition_variable>
#include <ncurses.h>
#include "protocol.h"
#include "Socket.h"

std::list<std::string> messages;
std::string command;
bool io_event=false, terminate_now=false, terminate_after_key=false;
std::mutex m_console;
std::condition_variable cv_console;

void refresh_console() {
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
            addstr(m.c_str());
        addstr((">> " + command).c_str());
        refresh();
    }
}

void login(std::shared_ptr<Socket> socket, std::string nickname) {
    // send nickname
    socket->send_line(nickname);

    // receive online users and last messages
    while (true) {
        std::optional<std::string> message = socket->receive_line();
        if (!message || std::regex_match(*message, protocol::error)) {
            std::cerr << "diocane" << std::endl;
            std::cerr << *message << std::endl;
            std::cerr << std::regex_match(*message, protocol::error) << std::endl;
            throw std::runtime_error(message.value_or("connection error"));
        }
        else if (message->empty())
            break;
        messages.push_back(*message + "\n");
    }

    // show
    std::lock_guard lg(m_console);
    io_event = true;
    cv_console.notify_one();
}

// service: send messages to other users
void send_messages(std::shared_ptr<Socket> socket) {
    while (true) {
        char c = getch();
        std::lock_guard lg(m_console);
        if (terminate_after_key) {
            terminate_now = true;
        } else if (c == '\n') {         // end of command
            socket->send_line(command);
            if (std::regex_match(command, protocol::termination)) terminate_now = true;
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
        std::optional<std::string> message;
        message = socket->receive_line();
        if (!message) {
            message = "[info] disconnected by the server, press a key to end";
            terminate_after_key = terminate_receive = true;
        }
        std::lock_guard lg(m_console);
        if (messages.size() >= protocol::max_messages)
            messages.pop_front();
        messages.push_back(*message + "\n");
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

    // parse arguments
    std::string ip_address = argv[1];
    std::string nickname = argv[3];
    int port;
    try {
        port = std::stoi(argv[2]);
    } catch (std::invalid_argument e) {
        std::cerr << "invalid port" << std::endl;
        std::cerr << usage << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // connect to the server
    std::shared_ptr<Socket> socket;
    try {
        socket = std::make_shared<Socket>(ip_address, port);
    } catch (std::runtime_error e) {
        std::cerr << "[info] server down" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // init console
    initscr();
    noecho();
    clear();
    refresh();
    std::atexit(close_console);

    // login and chat
    login(socket, nickname);
    std::thread refresher(refresh_console);
    std::thread sender(send_messages, socket);
    std::thread receiver(receive_messages, socket);

    // terminate
    sender.join();
    receiver.join();
    refresher.join();
}