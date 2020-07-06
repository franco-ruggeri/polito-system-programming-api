//
// Created by fruggeri on 7/5/20.
//

#include "Serializable.h"

std::vector<char> serialize_binary_size(std::size_t size) {
    std::ostringstream oss;
    oss.write(reinterpret_cast<char *>(&size), sizeof(size));
    std::string s(oss.str());
    return std::vector<char>(s.begin(), s.end());
}

std::size_t deserialize_binary_size(const char *serialized_size) {
    std::size_t size;
    std::string s(serialized_size, sizeof(size));
    std::istringstream iss(s);
    iss.read(reinterpret_cast<char *>(&size), sizeof(size));
    return size;
}

template<>
std::vector<char> serialize_binary_attribute<std::string>(const std::string& a) {
    std::size_t size_a = a.size();
    std::ostringstream oss;
    oss.write(reinterpret_cast<char *>(&size_a), sizeof(size_a));   // write attribute size
    oss << a;                                                       // write attribute
    std::string s(oss.str());
    return std::vector<char>(s.begin(), s.end());
}

template<>
std::pair<std::string,std::size_t> deserialize_binary_attribute(const char *serialized_a) {
    std::size_t size_a = deserialize_binary_size(serialized_a);     // read attribute size
    serialized_a += sizeof(size_a);
    std::string a(serialized_a, size_a);                            // read attribute
    return std::make_pair(a, size_a+sizeof(size_a));
}
