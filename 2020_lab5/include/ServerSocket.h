/*
 * Passive stream socket (for listening).
 *
 * Author: Franco Ruggeri
 */

#pragma once

#include "Socket.h"
#include <memory>
#include <netinet/in.h>

namespace chat_room {
    class ServerSocket : private Socket {
        struct sockaddr_in local_address_;
        socklen_t local_address_len_;
        static const unsigned int backlog;

    public:
        ServerSocket(int port);
        std::shared_ptr<Socket> accept();
    };
}
