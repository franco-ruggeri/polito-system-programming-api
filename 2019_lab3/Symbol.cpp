//
// Created by fruggeri on 7/15/20.
//

#include "Symbol.h"

Symbol::Symbol(char value, int site_id, int site_counter, const std::vector<int>& position) :
                value(value), site_id(site_id), site_counter(site_counter), position(position) {}

char Symbol::get_value() const {
    return value;
}

std::vector<int> Symbol::get_position() const {
    return position;
}

int Symbol::get_site_id() const {
    return site_id;
}

int Symbol::get_site_counter() const {
    return site_counter;
}
