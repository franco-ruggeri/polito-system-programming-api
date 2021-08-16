//
// Created by fruggeri on 7/6/20.
//

#pragma once

#include "Serializable.h"
#include <vector>

#define INVALID_FD -1

class Pipe {
    int fd[2];
    bool readyRead, readyWrite;

    Pipe(const Pipe& other) = delete;
    Pipe& operator=(const Pipe& other) = delete;
    void read(char *ptr, size_t n);

public:
    Pipe();
    ~Pipe();
    void write(const std::vector<char>& content);
    std::shared_ptr<char[]> read();
    bool isReadyRead();
    bool isReadyWrite();
    void close();
    void closeRead();
    void closeWrite();
    bool isClose();
    static void select(const std::vector<std::shared_ptr<Pipe>>& pipes);
};


