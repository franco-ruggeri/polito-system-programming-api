//
// Created by fruggeri on 6/25/20.
//

#pragma once

#include <string>
#include <ctime>

class DurationLogger {
    std::string name;
    std::clock_t duration;

public:
    DurationLogger(const std::string& name);
    ~DurationLogger();
};
