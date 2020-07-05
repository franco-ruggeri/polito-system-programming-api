//
// Created by fruggeri on 7/2/20.
//

#pragma once

#include <vector>
#include <memory>
#include "Serializable.h"

template<typename K, typename V, typename A>
class ReducerInput: public Serializable<ReducerInput<K,V,A>> {
    K key;
    V value;
    A acc;

public:
    ReducerInput() {}
    ReducerInput(K key, V value, A acc): key(key), value(value), acc(acc) {}
    ReducerInput(const ReducerInput& other): key(other.key), value(other.value), acc(other.acc) {}

    K getKey() const { return key; }
    V getValue() const { return value; }
    A getAcc() const { return acc; }

    pt::ptree buildPTree() const {
        pt::ptree pt;
        pt.put("key", key);
        pt.put("value", value);
        pt.put("acc", acc);
        return pt;
    }

    void loadPTree(const pt::ptree& pt) {
        key = pt.get<K>("key");
        value = pt.get<V>("value");
        acc = pt.get<A>("acc");
    }

    std::vector<char> serializeBinary() const {
        std::vector<char> tmp, serialized_obj;
        std::size_t total_size = 3*sizeof(std::size_t) + key.size() + sizeof(value);
        serialized_obj = serialize_binary_size(total_size);     // total size
        tmp = serialize_binary_attribute(key);                  // key
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(serialized_obj));
        tmp = serialize_binary_attribute(value);                // value
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(serialized_obj));
        tmp = serialize_binary_attribute(acc);                  // acc
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
        std::pair<V,std::size_t> value_res = deserialize_binary_attribute<V>(serialized_obj);
        value = value_res.first;
        serialized_obj += value_res.second;
        acc = deserialize_binary_attribute<A>(serialized_obj).first;
    }
};
