//
// Created by fruggeri on 7/6/20.
//

#include "Pipe.h"
#include "PipeException.h"
#include <unistd.h>

Pipe::Pipe(): readyRead(false), readyWrite(false) {
    if (::pipe(fd) < 0)
        throw PipeException("pipe() failed");
}

Pipe::~Pipe() {
    close();
}

void Pipe::write(const std::vector<char>& content) {
    if (::write(fd[1], reinterpret_cast<const char *>(&content[0]), content.size()) <= 0)
        throw PipeException("write() failed");
    readyWrite = false;
}

void Pipe::read(char *ptr, size_t n) {
    size_t nleft = n;
    ssize_t nread;

    while (nleft > 0) {
        if ((nread = ::read(fd[0], ptr, nleft)) < 0) {
            if (errno == EINTR)
                continue;
            else
                throw PipeException("read() failed");
        } else if (nread == 0) {    // EOF
            throw PipeException("write end closed", true);
        }
        nleft -= nread;
        ptr += nread;
    }
}

std::shared_ptr<char []> Pipe::read() {
    size_t total_size;

    read(reinterpret_cast<char *>(&total_size), sizeof(total_size));
    std::shared_ptr<char[]> ptr(new char[sizeof(total_size) + total_size]);
    for (int i=0; i<sizeof(total_size); i++)    // copy total size here
        ptr[i] = reinterpret_cast<char *>(&total_size)[i];
    read(ptr.get() + sizeof(total_size), total_size);

    readyRead = false;
    return ptr;
}

bool Pipe::isReadyRead() {
    return readyRead;
}

bool Pipe::isReadyWrite() {
    return readyWrite;
}

void Pipe::close() {
    closeRead();
    closeWrite();
}

void Pipe::closeRead() {
    ::close(fd[0]);
    fd[0] = INVALID_FD;
    readyRead = false;
}

void Pipe::closeWrite() {
    ::close(fd[1]);
    fd[1] = INVALID_FD;
    readyWrite = false;
}

bool Pipe::isClose() {
    return fd[0] == INVALID_FD && fd[1] == INVALID_FD;
}

void Pipe::select(const std::vector<std::shared_ptr<Pipe>>& pipes) {
    fd_set rset, wset;

    // prepare sets
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    for (const auto& pipe : pipes) {
        if (pipe->fd[0] != INVALID_FD)
            FD_SET(pipe->fd[0], &rset);
        if (pipe->fd[1] != INVALID_FD)
            FD_SET(pipe->fd[1], &wset);
    }

    // select
    if (::select(FD_SETSIZE, &rset, &wset, NULL, NULL) < 0)
        throw PipeException("select() failed");
    for (auto& pipe : pipes) {
        if (FD_ISSET(pipe->fd[0], &rset))
            pipe->readyRead = true;
        if (FD_ISSET(pipe->fd[1], &wset))
            pipe->readyWrite = true;
    }
}
