//
// Created by fruggeri on 6/30/20.
//

#pragma once

#include <string>

class Base {
    std::string name;
protected:
    Base(const std::string& name);
public:
    virtual int mType() const = 0;
    virtual void ls(int indent) const = 0;
    std::string getName() const;
};
