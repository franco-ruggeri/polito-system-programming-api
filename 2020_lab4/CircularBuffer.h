//
// Created by fruggeri on 7/8/20.
//

#pragma once

#include <vector>

template<typename T>
class CircularBuffer {
    std::vector<T> buffer;
    unsigned int head, tail, size;

public:
    CircularBuffer(unsigned int size) : size(size+1), head(size+1), tail(0), buffer(size+1) {}

    void put(T t) {
        buffer[tail++] = t;
        tail %= size;
    }

    T get() {
        head %= size;
        return buffer[head++];
    }

    bool empty() {
        return head % size == tail;
    }

    bool full() {
        return head == tail+1;
    }
};


