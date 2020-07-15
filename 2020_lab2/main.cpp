#include <iostream>
#include "Directory.h"
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    // example of usage
    auto root = Directory::getRoot();
    auto alpha = root->addDirectory("alpha");
    alpha->addDirectory("beta")->addFile("beta1", 100);
    alpha->getDir("beta")->addFile("beta2", 200);
    alpha->getDir("..")->ls(0);
    alpha->remove("beta");
    root->ls(0);

    // clear
    root->remove("alpha");
    std::cout << "\n\n";

    // Q12 - test with real directory
    for(auto& entry: fs::recursive_directory_iterator(fs::current_path())) {
        std::string path{entry.path()};
        path = path.substr(1);     // remove initial '/'
        std::shared_ptr<Directory> cur_dir = Directory::getRoot();

        // navigate or create path
        int idx;
        while ( (idx = path.find("/")) != std::string::npos) {
            std::string name = path.substr(0, idx);
            std::shared_ptr<Directory> new_dir = cur_dir->addDirectory(name);
            if (!new_dir)   // already present
                new_dir = cur_dir->getDir(name);
            cur_dir = new_dir;
            path = path.substr(idx+1);
        }

        // create file or dir
        std::string name = entry.path().filename();
        if (entry.is_regular_file())
            cur_dir->addFile(name, entry.file_size());
        else
            cur_dir->addDirectory(name);
    }
    root->ls(0);

    return 0;
}
