//
// Created by fruggeri on 6/30/20.
//

#pragma once

#include "Base.h"
#include <memory>

class File: public Base {
    uintmax_t size;
    static const int TYPE = 1;

    File(const std::string& name, uintmax_t size);

public:
    static std::shared_ptr<File> makeFile(const std::string& name, uintmax_t size);

    int mType() const override;
    void ls(int indent) const override;

    uintmax_t getSize() const;
};
