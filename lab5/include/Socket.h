/*
 * Stream socket.
 *
 * Author: Franco Ruggeri
 */

#pragma once

#include <netinet/in.h>
#include <string>
#include <optional>

namespace chat_room {
    class Socket {
        int socket_fd_;
        struct sockaddr_in remote_address_;
        socklen_t remote_address_len_;

        Socket(int socket_fd, struct sockaddr_in remote_address, socklen_t remote_address_len);
        Socket(const Socket& other) = delete;
        Socket& operator=(const Socket& other) = delete;
        void close();

        friend class ServerSocket;
    public:
        static const int invalid_socket;

        Socket();                                   // open socket
        Socket(std::string ip_address, int port);   // open and connect socket
        Socket(Socket&& other);
        ~Socket();
        Socket& operator=(Socket&& other);
        std::string remote_address() const;
        std::optional<std::string> receive_line();
        std::optional<std::string> receive_line(long timeout);
        void send_line(const std::string& message);
    };
}
