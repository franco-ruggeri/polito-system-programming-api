/*
 * Author: Franco Ruggeri
 */

#include "ServerSocket.h"
#include <sys/socket.h>
#include <cstring>
#include <stdexcept>

const unsigned int ServerSocket::backlog = 1024;

ServerSocket::ServerSocket(int port) {
    std::memset(&local_address, 0, sizeof(local_address));
    local_address.sin_family = AF_INET;
    local_address.sin_port = htons(port);
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address_len = sizeof(local_address);

    if (::bind(socket_fd, reinterpret_cast<struct sockaddr *>(&local_address), local_address_len) < 0)
        throw std::runtime_error("bind() failed");
    if (::listen(socket_fd, backlog) < 0)
        throw std::runtime_error("listen() failed");
}

std::shared_ptr<Socket> ServerSocket::accept() {
    struct sockaddr_in remote_address;
    socklen_t remote_address_len = sizeof(remote_address);

    int fd = ::accept(socket_fd, reinterpret_cast<struct sockaddr *>(&remote_address), &remote_address_len);
    if (fd < 0) throw std::runtime_error("accept() failed");

    return std::shared_ptr<Socket>(new Socket(fd, remote_address, remote_address_len));
}
