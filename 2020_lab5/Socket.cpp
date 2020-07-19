/*
 * Author: Franco Ruggeri
 */

#include "Socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <sys/select.h>
#include <ctime>
#include <climits>

const int Socket::invalid_socket = -1;

Socket::Socket(int socket_fd, struct sockaddr_in remote_address, socklen_t remote_address_len) :
        socket_fd(socket_fd), remote_address(remote_address), remote_address_len(remote_address_len) {}

Socket::Socket() {
    socket_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0)
        throw std::runtime_error("socket() failed");
}

Socket::Socket(std::string ip_address, int port) : Socket() {
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(port);
    remote_address_len = sizeof(remote_address);
    if (::inet_pton(AF_INET, ip_address.c_str(), &remote_address.sin_addr) <= 0)
        throw std::runtime_error("inet_pton() failed");
    if (::connect(socket_fd, reinterpret_cast<struct sockaddr*>(&remote_address), remote_address_len) < 0)
        throw std::runtime_error("connect() failed");
}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) {
    this->socket_fd = other.socket_fd;
    this->remote_address = other.remote_address;
    this->remote_address_len = other.remote_address_len;
    other.socket_fd = invalid_socket;
    std::memset(&other.remote_address, 0, other.remote_address_len);
    other.remote_address_len = 0;
}

Socket& Socket::operator=(Socket &&other) {
    if (this != &other) {
        this->close();
        this->socket_fd = other.socket_fd;
        this->remote_address = other.remote_address;
        this->remote_address_len = other.remote_address_len;
        other.socket_fd = invalid_socket;
        std::memset(&other.remote_address, 0, other.remote_address_len);
        other.remote_address_len = 0;
    }
    return *this;
}

void Socket::close() {
    if (this->socket_fd != invalid_socket) {
        ::close(this->socket_fd);
        socket_fd = invalid_socket;
    }
}

std::string Socket::get_remote_address() const {
    char name[16];
    if (inet_ntop(AF_INET, &remote_address.sin_addr, name, sizeof(name)) == nullptr)
        throw std::logic_error("inet_ntop() failed, remote address not set properly");
    return std::string(name) + ":" + std::to_string(ntohs(remote_address.sin_port));
}

std::optional<std::string> Socket::receive_line() {
    return receive_line(LONG_MAX);
}

std::optional<std::string> Socket::receive_line(long timeout) {
    std::string line;
    char c;
    int n;
    fd_set rset;
    struct timeval to;
    time_t time_start, time_end;

    time_start = std::time(nullptr);
    FD_ZERO(&rset);

    while (true) {
        // check timeout
        time_end = std::time(nullptr);
        if (time_end - time_start > timeout)
            return std::nullopt;

        // wait data
        to = {timeout - (time_end-time_start), 0};
        FD_SET(socket_fd, &rset);
        n = ::select(FD_SETSIZE, &rset, nullptr, nullptr, &to);
        if (n == 0) return std::nullopt;    //timeout

        // read data
        n = ::recv(socket_fd, &c, sizeof(char), 0);
        if (n < 0) {
            if (errno == EINTR) continue;   // interrupted by signal, repeat
            else throw std::runtime_error("recv() failed");
        } else if (n == 0) {                // EOF, remote end closed
            return std::nullopt;
        }

        if (c == '\r') continue;
        if (c == '\n') break;
        line += c;
    }
    return line;
}

void Socket::send_line(const std::string& message) {
    std::string line = message + '\n';
    const char *ptr = line.c_str();
    size_t n_left = line.size();
    ssize_t n_written;

    while (n_left > 0) {
        n_written = ::send(socket_fd, ptr, n_left, MSG_NOSIGNAL);     // for SIGPIPE
        if (n_written < 0) {
            if (errno == EINTR) continue;   // interrupted by signal, repeat
            else throw std::runtime_error("send() failed");
        }
        n_left -= n_written;
        ptr += n_written;
    }
}
