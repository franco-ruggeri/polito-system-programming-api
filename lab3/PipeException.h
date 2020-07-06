//
// Created by fruggeri on 7/6/20.
//

#pragma once

#include <exception>
#include <string>

class PipeException: public std::exception {
    std::string explanation;
    bool eof;

public:
    PipeException(std::string explanation): explanation(explanation), eof(false) {}
    PipeException(std::string explanation, bool eof): explanation(explanation), eof(eof) {}
    const char *what() const noexcept override { return explanation.c_str(); }
    bool isEOF() { return eof; }
};
