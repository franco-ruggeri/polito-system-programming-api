/*
 * Message for the chat room application protocol.
 *
 * Author: Franco Ruggeri
 */

#pragma once

#include <string>
#include <regex>
#include "MessageType.h"

namespace chat_room {
    class Message {
        MessageType type_;
        std::string source_, destination_, message_;
        static const std::regex r_quit_cmd, r_pm_cmd, r_full_msg, r_log, r_private, r_public;

    public:
        Message();
        Message(MessageType type);                                                  // info message with no payload
        Message(MessageType type, std::string message);                             // info message with payload
        Message(std::string source, std::string message);                           // public message
        Message(std::string source, std::string destination, std::string message);  // private message

        MessageType type() const;
        std::string source() const;
        std::string destination() const;
        std::string message() const;

        std::string to_console() const;     // visualization format (for client)
        std::string to_network() const;     // network format (serialization)

        friend Message make_message_from_command(std::string source_nickname, std::string input);
        friend Message make_message_from_network(std::string input);
    };

    Message make_message_from_command(std::string source_nickname, std::string input);      // from user command
    Message make_message_from_network(std::string input);                                   // from network
}
