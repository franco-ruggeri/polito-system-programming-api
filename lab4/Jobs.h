//
// Created by fruggeri on 7/8/20.
//

#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>
#include <stdexcept>
#include "CircularBuffer.h"

template<typename T>
class Jobs {
    CircularBuffer<T> jobs;
    std::mutex m_jobs;
    std::condition_variable cv_full;
    std::condition_variable cv_empty;
    bool closed;
    static const unsigned int default_size = 1024;

public:
    Jobs() : jobs(default_size), closed(false) {}

    // insert a job in the queue waiting to be processed, or blocks if the queue is full
    void put(T job) {
        std::unique_lock ul(m_jobs);
        if (closed) throw std::logic_error("put() called on closed queue");
        cv_full.wait(ul, [this]() { return !jobs.full(); });
        jobs.put(job);
        cv_empty.notify_one();
    }

    // read a job from the queue and remove it, or blocks if the queue is empty
    std::optional<T> get() {
        std::unique_lock ul(m_jobs);
        cv_empty.wait(ul, [&]() { return !jobs.empty() || closed; });
        if (jobs.empty()) return std::nullopt;  // no jobs and no producers
        T t = jobs.get();
        cv_full.notify_one();
        return t;
    }

    // close queue, no more jobs can be added
    void close() {
        std::lock_guard lg(m_jobs);
        closed = true;
        cv_empty.notify_all();  // notify consumers
    }
};
