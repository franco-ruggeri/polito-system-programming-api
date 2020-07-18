//
// Created by fruggeri on 7/17/20.
//

#pragma once

#include <string>
#include <regex>
#include "MessageType.h"

class Message {
    MessageType type;
    std::string source, destination, message;
    static const std::regex r_quit_cmd, r_pm_cmd, r_full_msg, r_log, r_private, r_public;

public:
    Message();
    Message(MessageType type);                                                  // info message with no payload
    Message(MessageType type, std::string message);                             // info message with payload
    Message(std::string source, std::string message);                           // public message
    Message(std::string source, std::string destination, std::string message);  // private message

    MessageType get_type() const;
    std::string get_source() const;
    std::string get_destination() const;
    std::string get_message() const;

    std::string to_console() const;
    std::string to_network() const;

    friend Message make_message_from_command(std::string source_nickname, std::string input);
    friend Message make_message_from_network(std::string input);
};

Message make_message_from_command(std::string source_nickname, std::string input);      // from user command
Message make_message_from_network(std::string input);                                   // from network
