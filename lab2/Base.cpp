//
// Created by fruggeri on 6/30/20.
//

#include "Base.h"

Base::Base(const std::string& name): name(name) {}

std::string Base::getName() const { return name; }
