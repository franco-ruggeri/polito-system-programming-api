//
// Created by fruggeri on 7/8/20.
//

#pragma once

#include <queue>
#include <mutex>
#include <atomic>

template<typename T>
class Jobs {
    std::queue<T> jobs;
    std::mutex m_jobs;
    std::condition_variable cv_full;
    std::condition_variable cv_empty;
    std::atomic<bool> closed;
    static const long max_size = 1024;

public:
    Jobs(): closed(false) {}

    // insert a job in the queue waiting to be processed, or blocks if the queue is full
    void put(T job) {
        std::unique_lock ul(m_jobs);
        cv_full.wait(ul, [&]() { return jobs.size() < max_size; });
        jobs.push(job);
        cv_empty.notify_one();
    }

    // read a job from the queue and remove it, or blocks if the queue is empty
    T get() {
        std::unique_lock ul(m_jobs);
        cv_empty.wait(ul, [&]() { return !jobs.empty(); });
        T t = jobs.front();
        jobs.pop();
        cv_full.notify_one();
        return t;
    }

    bool empty() {
        std::lock_guard lg(m_jobs);
        return jobs.empty();
    }

    bool is_closed() {
        return closed.load();
    }

    void close() {
        closed.store(true);     // before notifying!
        cv_empty.notify_all();
    }
};
