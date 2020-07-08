//
// Created by fruggeri on 7/8/20.
//

#pragma once

#include <string>

class FileLine {
    std::string filename;
    std::string line;
    unsigned int lineNumber;

public:
    FileLine() {}
    FileLine(std::string filename, std::string line, unsigned int lineNumber);
    std::string getFilename();
    std::string getLine();
    unsigned int getLineNumber();
};


