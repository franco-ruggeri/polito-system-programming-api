//
// Created by fruggeri on 7/5/20.
//

#pragma once

#include <vector>
#include <memory>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

template<typename T>
class Serializable {
    Serializable() {}
    friend T;

public:
    /*********************************
     * JSON serialization with boost *
     *********************************/

    pt::ptree buildPTree() const {
        return static_cast<T const&>(*this).getPTree();
    }

    void loadPTree(const pt::ptree& pt) {
        return static_cast<T const&>(*this).loadPTree(pt);
    }

    std::vector<char> serializeJSON() const {
        pt::ptree pt = static_cast<T const&>(*this).buildPTree();
        std::ostringstream oss;
        pt::write_json(oss, pt);
        std::string s(oss.str());
        return std::vector<char>(s.begin(), s.end());
    }

    void serializeJSON(pt::ptree& array) const {
        pt::ptree pt = static_cast<T const&>(*this).buildPTree();
        array.push_back(std::make_pair("", pt));
    }

    void deserializeJSON(std::shared_ptr<char[]> serialized_obj) {
        std::string s{serialized_obj.get()};
        std::istringstream iss(s);
        pt::ptree pt;
        pt::read_json(iss, pt);
        deserializeJSON(pt);
    }

    void deserializeJSON(const pt::ptree& pt) {
        static_cast<T&>(*this).loadPTree(pt);
    }


    /************************
     * Binary serialization *
     ************************/

    std::vector<char> serializeBinary() const {
        return static_cast<T const&>(*this).serializeBinary();
    }

    void deserializeBinary(std::shared_ptr<char[]> serialized_obj) {
        static_cast<T&>(*this).deserializeBinary(serialized_obj);
    }

    void deserializeBinary(char *serialized_obj) {
        static_cast<T&>(*this).deserializeBinary(serialized_obj);
    }
};

/********************************
 * JSON serialization of arrays *
 ********************************/

template<typename T>
std::vector<char> serialize_json(const std::vector<T>& objs) {
    pt::ptree root, array;
    for (auto& obj : objs)
        obj.serializeJSON(array);
    root.add_child("array", array);
    std::ostringstream oss;
    pt::write_json(oss, root);
    std::string s(oss.str());
    return std::vector<char>(s.begin(), s.end());
}

template<typename T>
std::vector<T> deserialize_json(std::shared_ptr<char[]> serialized_objs) {
    std::string s{serialized_objs.get()};
    std::istringstream iss(s);
    pt::ptree root;
    pt::read_json(iss, root);
    std::vector<T> objs;
    for (const auto& child : root.get_child("array")) {
        pt::ptree pt = child.second;
        T obj;
        obj.deserializeJSON(pt);
        objs.push_back(obj);
    }
    return objs;
}


/**************************************
 * Binary serialization of attributes *
 **************************************/

std::vector<char> serialize_binary_size(std::size_t size);
std::size_t deserialize_binary_size(char *serialized_size);

template<typename A>
std::vector<char> serialize_binary_attribute(const A& a) {
    std::size_t size_a = sizeof(a);
    std::ostringstream oss;
    oss.write(reinterpret_cast<char *>(&size_a), sizeof(size_a));   // write attribute size
    oss.write(reinterpret_cast<const char *>(&a), size_a);          // write attribute
    std::string s(oss.str());
    return std::vector<char>(s.begin(), s.end());
}

template<typename A>
std::pair<A, std::size_t> deserialize_binary_attribute(char *serialized_a) {
    A a;
    std::size_t size_a = sizeof(A);                                 // attribute size
    serialized_a += sizeof(size_a);                                 // skip size (assumed same as sizeof(A))
    std::string s(serialized_a, size_a);
    std::istringstream iss(s);
    iss.read(reinterpret_cast<char *>(&a), size_a);                 // read attribute
    return std::make_pair(a, size_a+sizeof(size_a));
}


/**********************************
 * Binary serialization of arrays *
 **********************************/

template<typename T>
std::vector<char> serialize_binary(const std::vector<T>& objs) {
    std::vector<char> serialized_objs;
    serialized_objs = serialize_binary_size(objs.size());   // number of objects
    for (const auto& obj : objs) {                          // objects
        std::vector<char> serialized_obj = obj.serializeBinary();
        std::copy(serialized_obj.begin(), serialized_obj.end(), std::back_inserter(serialized_objs));
    }
    return serialized_objs;
}

template<typename T>
std::vector<T> deserialize_binary(std::shared_ptr<char[]> serialized_objs) {
    std::vector<T> objs;
    char *ptr = serialized_objs.get();
    std::size_t n_objs = deserialize_binary_size(ptr);      // number of objects
    ptr += sizeof(n_objs);
    for (int i=0; i<n_objs; i++) {                          // objects
        std::size_t size_obj = deserialize_binary_size(ptr);
        T t;
        t.deserializeBinary(ptr);
        objs.push_back(t);
        ptr += sizeof(size_obj) + size_obj;
    }
    return objs;
}