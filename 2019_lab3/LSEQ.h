//
// Created by fruggeri on 7/16/20.
//

#pragma once

#include <vector>

class LSEQ {
    enum class Strategy : bool {
        boundary_plus=true, boundary_minus=false
    };

    static const int begin;     // first possible child of a node, reserved
    static const int end;       // last possible child of a node, reserved
    static const unsigned int boundary;
    std::vector<Strategy> strategies;

public:
    static std::vector<int> get_begin();
    static std::vector<int> get_end();
    std::vector<int> get_between(std::vector<int> prev, std::vector<int> next);
};


