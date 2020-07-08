//
// Created by fruggeri on 7/8/20.
//

#pragma once

#include <unordered_map>
#include <mutex>

template<typename K, typename A>
class Results {
    std::unordered_map<K,A> results;
    std::mutex m_results;

public:
    template<typename V>
    void accumulate(K key, V value) {
        std::lock_guard lg(m_results);
        results[key] += value;
    }

    std::unordered_map<K,A> get_results() {
        return results;
    }
};
