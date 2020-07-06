//
// Created by fruggeri on 7/2/20.
//

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include "Serializable.h"

class MapperInput: public Serializable<MapperInput> {
    std::string input;

public:
    std::string getInput() const { return input; }

    friend std::istream& operator>>(std::istream& in, MapperInput& mapperInput) {
        std::getline(in, mapperInput.input);
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const MapperInput& mapperInput) {
        out << mapperInput.input;
        return out;
    }

    pt::ptree buildPTree() const {
        pt::ptree pt;
        pt.put("input", input);
        return pt;
    }

    void loadPTree(const pt::ptree& pt) {
        input = pt.get<std::string>("input");
    }

    std::vector<char> serializeBinary() const {
        std::size_t total_size = sizeof(std::size_t) + input.size();    // input size + input
        std::vector<char> serialized_obj = serialize_binary_size(total_size);
        std::vector<char> tmp = serialize_binary_attribute(input);
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(serialized_obj));
        return serialized_obj;
    }

    void deserializeBinary(std::shared_ptr<char[]> serialized_obj) {
        deserializeBinary(serialized_obj.get());
    }

    void deserializeBinary(const char *serialized_obj) {
        serialized_obj += sizeof(std::size_t);   // skip total size
        input = deserialize_binary_attribute<std::string>(serialized_obj).first;
    }
};
