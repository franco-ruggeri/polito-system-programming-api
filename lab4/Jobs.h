//
// Created by fruggeri on 7/8/20.
//

#pragma once

#include <queue>
#include <mutex>

template<typename T>
class Jobs {
    std::queue<T> jobs;
    std::mutex mJobs;
    std::condition_variable cvEmpty;
    bool closed = false;

public:
    // insert a job in the queue waiting to be processed, or blocks if the queue is full
    void put(T job) {
        std::lock_guard lg(mJobs);
        jobs.push(job);
        cvEmpty.notify_one();
    }

    // read a job from the queue and remove it, or blocks if the queue is empty
    T get() {
        std::unique_lock ul(mJobs);
        cvEmpty.wait(ul, [&]() { return !jobs.empty(); });
        T t = jobs.front();
        jobs.pop();
        return t;
    }

    bool isEmpty() {
        std::lock_guard lg(mJobs);
        return jobs.empty();
    }

    bool isClosed() {
        return closed;
    }

    void close() {
        closed = true;  // before notifying!
        cvEmpty.notify_all();
    }
};
