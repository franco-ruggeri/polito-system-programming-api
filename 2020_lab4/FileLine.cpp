//
// Created by fruggeri on 7/8/20.
//

#include "FileLine.h"

FileLine::FileLine(std::string filename, std::string line, unsigned int lineNumber) :
                    filename(filename), line(line), lineNumber(lineNumber) {}

std::string FileLine::getFilename() {
    return filename;
}

std::string FileLine::getLine() {
    return line;
}

unsigned int FileLine::getLineNumber() {
    return lineNumber;
}
