//
// Created by fruggeri on 6/30/20.
//

#include "File.h"
#include <iostream>

File::File(const std::string &name, uintmax_t size): Base(name), size(size) {}

std::shared_ptr<File> File::makeFile(const std::string &name, uintmax_t size) {
    return std::shared_ptr<File>{new File(name, size)};
}

int File::mType() const { return File::TYPE; }

void File::ls(int indent) const {
    for (int i=0; i<indent; i++)
        std::cout << " ";
    std::cout << getName() << " " << size << std::endl;
}

uintmax_t File::getSize() const { return size; }
