//
// Created by fruggeri on 7/9/20.
//

#include "protocol.h"

const std::regex protocol::termination("^/quit$");
const std::regex protocol::private_message("^/private .+ .+$");
const std::regex protocol::error("^\\[error\\].*$");
const long protocol::max_idle_time = 60;
const int protocol::max_messages = 32;
