//
// Created by fruggeri on 7/15/20.
//

#pragma once

#include <vector>

class Symbol {
    char value;
    int site_id, site_counter;
    std::vector<int> position;
public:
    Symbol(char value, int site_id, int site_counter, const std::vector<int>& position);
    bool operator<(const Symbol& other);
    bool operator==(const Symbol& other);
    char get_value() const;
    std::vector<int> get_position() const;
    int get_site_id() const;
    int get_site_counter() const;
};
