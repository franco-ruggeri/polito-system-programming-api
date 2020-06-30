//
// Created by fruggeri on 6/28/20.
//

#pragma once

#include "Base.h"
#include "File.h"
#include <string>
#include <memory>
#include <unordered_map>

class Directory: public Base {
    std::weak_ptr<Directory> parent;
    std::weak_ptr<Directory> self;
    std::unordered_map<std::string, std::shared_ptr<Base>> children;
    static std::shared_ptr<Directory> root;
    static const int TYPE = 0;

    bool check_name(const std::string &name) const;
    Directory(const std::string& name, std::weak_ptr<Directory> parent);

public:
    static std::shared_ptr<Directory> makeDirectory(const std::string& name, std::weak_ptr<Directory> parent);
    static std::shared_ptr<Directory> getRoot();

    virtual int mType() const override;
    virtual void ls(int indent) const override;

    std::shared_ptr<Base> get(const std::string& name) const;
    std::shared_ptr<Directory> getDir(const std::string& name) const;
    std::shared_ptr<File> getFile(const std::string& name) const;

    std::shared_ptr<Directory> addDirectory(const std::string& name);
    std::shared_ptr<File> addFile(const std::string& name, uintmax_t size);

    bool remove(const std::string& name);
};
