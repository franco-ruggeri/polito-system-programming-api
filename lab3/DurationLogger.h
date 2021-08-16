//
// Created by fruggeri on 6/25/20.
//

#pragma once

#include <string>
#include <ctime>
#include <iostream>

class DurationLogger {
    std::string name;
    std::clock_t duration;

public:
    DurationLogger(const std::string& name): name(name) {
        this->duration = std::clock();
        std::cout << " >>>>>>>>>>>>>> starting " << this->name << std::endl;
    }

    ~DurationLogger() {
        this->duration = std::clock() - this->duration;
        std::cout << " <<<<<<<<<<<<<<< ending " << this->name << ": " << this->duration << " clock ticks" << std::endl;
    }
};
