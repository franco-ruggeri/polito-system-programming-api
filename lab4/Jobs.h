//
// Created by fruggeri on 7/8/20.
//

#pragma once

#include <queue>
#include <mutex>
#include <atomic>
#include <optional>

template<typename T>
class Jobs {
    std::queue<T> jobs;
    std::mutex m_jobs;
    std::condition_variable cv_full;
    std::condition_variable cv_empty;
    std::atomic<int> active_producers;
    static const long max_size = 1024;

    bool is_closed() {
        return active_producers.load() == 0;
    }

public:
    Jobs(int n_producers) : active_producers(n_producers) {}

    // insert a job in the queue waiting to be processed, or blocks if the queue is full
    void put(T job) {
        std::unique_lock ul(m_jobs);
        cv_full.wait(ul, [&]() { return jobs.size() < max_size; });
        jobs.push(job);
        cv_empty.notify_one();
    }

    // read a job from the queue and remove it, or blocks if the queue is empty
    std::optional<T> get() {
        std::unique_lock ul(m_jobs);
        cv_empty.wait(ul, [&]() { return !jobs.empty() || is_closed(); });
        if (jobs.empty()) return std::nullopt;  // no jobs and no producers
        T t = jobs.front();
        jobs.pop();
        cv_full.notify_one();
        return t;
    }

    // unregister active producer
    void close() {
        active_producers--;     // before notifying!
        cv_empty.notify_all();
    }
};
