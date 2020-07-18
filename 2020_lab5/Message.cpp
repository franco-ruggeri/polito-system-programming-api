//
// Created by fruggeri on 7/17/20.
//

#include "Message.h"

const std::regex Message::r_quit_cmd("^/quit$");                    // /quit
const std::regex Message::r_pm_cmd("^/private ([^\\s]+) (.*)$");    // /private <nickname> <message>
const std::regex Message::r_full_msg("^([\\d]+)(?:$| (.*$))");      // <type> or <type> <message>
const std::regex Message::r_log("^[^\\s]+$");                       // <nickname>
const std::regex Message::r_private("^([^\\s]+) ([^\\s]+) (.*)$");  // <source> <destination> <message>
const std::regex Message::r_public("^([^\\s]+) (.*)$");             // <source> <message>

Message::Message() {}

Message::Message(MessageType type) : type(type) {
    if (type != MessageType::error && type != MessageType::done_last_messages && type != MessageType::quit)
        throw std::logic_error("constructor used with wrong type");
}

Message::Message(MessageType type, std::string message) : type(type), message(message) {
    if (type != MessageType::error && type != MessageType::login && type != MessageType::logout &&
        type != MessageType::online_users)
        throw std::logic_error("constructor used with wrong type");
}

Message::Message(std::string source, std::string message) :
    type(MessageType::public_message), source(source), message(message) {}

Message::Message(std::string source, std::string destination, std::string message) :
    type(MessageType::private_message), source(source), destination(destination), message(message) {}

MessageType Message::get_type() const {
    return type;
}

std::string Message::get_source() const {
    return source;
}

std::string Message::get_destination() const {
    return destination;
}

std::string Message::get_message() const {
    return message;
}

std::string Message::to_console() const {
    std::string m;
    switch (type) {
        case MessageType::error:
            m = "[error] " + message;
            break;
        case MessageType::login:
            m = "[info] " + message + " is now online";
            break;
        case MessageType::logout:
            m = "[info] " + message + " is now offline";
            break;
        case MessageType::online_users:
            m = "[info] online users: " + message;
            break;
        case MessageType::private_message:
            m = "[PM to " + destination + "] " + source + ": " + message;
            break;
        case MessageType::public_message:
            m = source + ": " + message;
            break;
        default:
            throw std::logic_error("to_console() used with wrong type");
    }
    m += '\n';
    return m;
}

std::string Message::to_network() const {
    return std::to_string(static_cast<int>(type)) +
            (source.empty() ? "" : " " + source) +
            (destination.empty() ? "" : " " + destination) +
            " " + message;
}

Message make_message_from_command(std::string source, std::string input) {
    Message message;
    std::smatch match;

    if (std::regex_match(input, match, Message::r_quit_cmd))
        message = Message(MessageType::quit);
    else if (std::regex_match(input, match, Message::r_pm_cmd))
        message = Message(source, match[1], match[2]);
    else    // public message
        message = Message(source, input);
    return message;
}

Message make_message_from_network(std::string input) {
    Message message;
    std::smatch match;
    MessageType type;

    // parse type
    if (!std::regex_match(input, match, Message::r_full_msg))
        throw std::logic_error("invalid message from network: no type field");
    type = static_cast<MessageType>(std::stoi(match[1]));

    // parse rest of message
    input = match[2];
    switch (type) {
        case MessageType::error:
            message = Message(type, input);
            break;
        case MessageType::login:
        case MessageType::logout:
            if (!std::regex_match(input, match, Message::r_log))
                throw std::logic_error("invalid log message from network: invalid nickname");
            message = Message(type, input);
            break;
        case MessageType::online_users:
            message = Message(type, input);
            break;
        case MessageType::done_last_messages:
        case MessageType::quit:
            message = Message(type);
            break;
        case MessageType::private_message:
            if (!std::regex_match(input, match, Message::r_private))
                throw std::logic_error("invalid private message from network: no format <source> <destination> <message>");
            message = Message(match[1], match[2], match[3]);
            break;
        case MessageType::public_message:
            if (!std::regex_match(input, match, Message::r_public))
                throw std::logic_error("invalid public message from network: no format <source> <message>");
            message = Message(match[1], match[2]);
            break;
        default:
            throw std::logic_error("invalid private message from network: unknown type");
    }
    return message;
}
