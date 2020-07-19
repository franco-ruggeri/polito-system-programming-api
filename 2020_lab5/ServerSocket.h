/*
 * Passive stream socket (for listening).
 *
 * Author: Franco Ruggeri
 */

#pragma once

#include "Socket.h"
#include <memory>
#include <netinet/in.h>

class ServerSocket : private Socket {
    struct sockaddr_in local_address;
    socklen_t local_address_len;
    static const unsigned int backlog;

public:
    ServerSocket(int port);
    std::shared_ptr<Socket> accept();
};


