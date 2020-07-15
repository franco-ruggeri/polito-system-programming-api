//
// Created by fruggeri on 7/9/20.
//

#pragma once

#include <regex>

namespace protocol {
    extern const std::regex termination;
    extern const std::regex private_message;
    extern const std::regex error;
    extern const long max_idle_time;
    extern const int max_messages;
}
