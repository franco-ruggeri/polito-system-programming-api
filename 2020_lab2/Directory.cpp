//
// Created by fruggeri on 6/28/20.
//

#include "Directory.h"
#include <iostream>

std::shared_ptr<Directory> Directory::root{nullptr};

Directory::Directory(const std::string& name, std::shared_ptr<Directory> parent): Base(name), parent(parent) {}

std::shared_ptr<Directory> makeDirectory(const std::string& name, std::shared_ptr<Directory> parent) {
    std::shared_ptr<Directory> dir{new Directory(name, parent)};
    dir->self = dir;
    return dir;
}

std::shared_ptr<Directory> Directory::getRoot() {
    if (!root)
        root = makeDirectory("/", std::shared_ptr<Directory>(nullptr));
    return root;
}

int Directory::mType() const { return Directory::TYPE; }

void Directory::ls(int indent) const {
    // name
    for (int i=0; i<indent; i++)
        std::cout << " ";
    std::cout << "[+] " << getName() << std::endl;

    // recursion
    for (auto &c : children)
        c.second->ls(indent+4);
}

std::shared_ptr<Base> Directory::get(const std::string& name) const {
    if (name == ".")
        return self.lock();
    if (name == "..")
        return parent.lock();
    auto c = children.find(name);
    return c != children.end() ? c->second : nullptr;
}

std::shared_ptr<Directory> Directory::getDir(const std::string &name) const {
    return dynamic_pointer_cast<Directory>(get(name));
}

std::shared_ptr<File> Directory::getFile(const std::string &name) const {
    return dynamic_pointer_cast<File>(get(name));
}

std::shared_ptr<Directory> Directory::addDirectory(const std::string& name) {
    if (name == "." || name == ".." || children.find(name) != children.end())
        return nullptr;
    auto child = makeDirectory(name, this->self.lock());
    children.insert(std::make_pair(name, child));
    return child;
}

std::shared_ptr<File> Directory::addFile(const std::string &name, uintmax_t size) {
    if (name == "." || name == ".." || children.find(name) != children.end())
        return nullptr;
    auto child = File::makeFile(name, size);
    children.insert(std::make_pair(name, child));
    return child;
}

bool Directory::remove(const std::string &name) {
    if (name == "." || name == ".." || children.find(name) == children.end())
        return false;
    children.erase(name);
    return true;
}
