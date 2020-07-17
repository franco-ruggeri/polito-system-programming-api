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

bool Symbol::operator<(const Symbol& other) {
    return this->position < other.position || (this->position == other.position && this->site_id < other.site_id);
}

bool Symbol::operator==(const Symbol &other) {
    return this->site_id == other.site_id && this->site_counter == other.site_counter &&
            this->position == other.position;
}
