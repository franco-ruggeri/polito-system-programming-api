//
// Created by fruggeri on 7/2/20.
//

#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include "Serializable.h"

template<typename K, typename V>
class Result: public Serializable<Result<K,V>> {
    K key;
    V value;

    // helper function for binary serialization (specialized for std::string, see below)
    std::size_t getKeySize() { return sizeof(key); }

public:
    Result() {}
    Result(K key, V value): key(key), value(value) {}
    K getKey() const { return key; }
    V getValue() const { return value; }

    friend std::ostream& operator<<(std::ostream& out, const Result& result) {
        out << result.key << " => " << result.value;
        return out;
    }

    pt::ptree buildPTree() const {
        pt::ptree pt;
        pt.put("key", key);
        pt.put("value", value);
        return pt;
    }

    void loadPTree(const pt::ptree& pt) {
        key = pt.get<K>("key");
        value = pt.get<V>("value");
    }

    std::vector<char> serializeBinary() const {
        std::vector<char> tmp, serialized_obj;
        std::size_t total_size = 2*sizeof(std::size_t) + key.size() + sizeof(value);
        serialized_obj = serialize_binary_size(total_size);     // total size
        tmp = serialize_binary_attribute(key);                  // key
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(serialized_obj));
        tmp = serialize_binary_attribute(value);                // value
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(serialized_obj));
        return serialized_obj;
    }

    void deserializeBinary(std::shared_ptr<char[]> serialized_obj) {
        deserializeBinary(serialized_obj.get());
    }

    void deserializeBinary(char *serialized_obj) {
        serialized_obj += sizeof(std::size_t);   // skip total size
        std::pair<K,std::size_t> key_res = deserialize_binary_attribute<K>(serialized_obj);
        key = key_res.first;
        serialized_obj += key_res.second;
        value = deserialize_binary_attribute<V>(serialized_obj).first;
    }
};


