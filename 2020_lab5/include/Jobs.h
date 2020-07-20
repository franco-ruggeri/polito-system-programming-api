/*
 * Generic thread-safe FIFO queue with support to termination.
 *
 * Author: Franco Ruggeri
 */

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>
#include <stdexcept>

namespace chat_room {
    template<typename T>
    class Jobs {
        std::queue<T> jobs_;
        std::mutex m_jobs_;
        std::condition_variable cv_full_;
        std::condition_variable cv_empty_;
        bool closed_;
        unsigned int max_size_;

        static const unsigned int default_max_size = 1024;

    public:
        Jobs() : max_size_(default_max_size), closed_(false) {}
        Jobs(unsigned int max_size) : max_size_(max_size), closed_(false) {}

        Jobs& operator=(const Jobs& other) {
            this->jobs_ = other.jobs_;
            this->closed_ = other.closed_;
            this->max_size_ = other.max_size_;
            return *this;
        }

        // insert a job in the queue waiting to be processed, or blocks if the queue is full
        void put(T job) {
            std::unique_lock ul(m_jobs_);
            if (closed_) throw std::logic_error("put() called on closed queue");
            cv_full_.wait(ul, [this]() { return jobs_.size() < max_size_; });
            jobs_.push(std::move(job));
            cv_empty_.notify_one();
        }

        // read a job from the queue and remove it, or blocks if the queue is empty
        std::optional<T> get() {
            std::unique_lock ul(m_jobs_);
            cv_empty_.wait(ul, [&]() { return !jobs_.empty() || closed_; });
            if (jobs_.empty()) return std::nullopt;  // no jobs and no producers
            T t = jobs_.front();
            jobs_.pop();
            cv_full_.notify_one();
            return t;
        }

        // close queue, no more jobs can be added
        void close() {
            std::lock_guard lg(m_jobs_);
            closed_ = true;
            cv_empty_.notify_all();  // notify consumers
        }
    };
}
