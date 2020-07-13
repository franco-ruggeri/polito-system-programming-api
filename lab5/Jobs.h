//
// Created by fruggeri on 7/8/20.
//

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>
#include <stdexcept>

template<typename T>
class Jobs {
    std::queue<T> jobs;
    std::mutex m_jobs;
    std::condition_variable cv_full;
    std::condition_variable cv_empty;
    bool closed;
    unsigned int max_size;

    static const unsigned int default_max_size = 1024;

public:
    Jobs() : max_size(default_max_size), closed(false) {}
    Jobs(unsigned int max_size) : max_size(max_size), closed(false) {}

    Jobs& operator=(const Jobs& other) {
        this->jobs = other.jobs;
        this->closed = other.closed;
        this->max_size = other.max_size;
        return *this;
    }

    // insert a job in the queue waiting to be processed, or blocks if the queue is full
    void put(T job) {
        std::unique_lock ul(m_jobs);
        if (closed) throw std::logic_error("put() called on closed queue");
        cv_full.wait(ul, [&]() { return jobs.size() < max_size; });
        jobs.push(std::move(job));
        cv_empty.notify_one();
    }

    // read a job from the queue and remove it, or blocks if the queue is empty
    std::optional<T> get() {
        std::unique_lock ul(m_jobs);
        cv_empty.wait(ul, [&]() { return !jobs.empty() || closed; });
        if (jobs.empty()) return std::nullopt;  // no jobs and no producers
        T t = jobs.front();
        jobs.pop();
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
