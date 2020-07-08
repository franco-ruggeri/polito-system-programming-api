#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <mutex>
#include <regex>
#include "Jobs.h"
#include "FileLine.h"

#define N_PRODUCERS 10
#define N_CONSUMERS 10

namespace fs = std::filesystem;

Jobs<fs::path> file_jobs(1);
Jobs<FileLine> line_jobs(N_PRODUCERS);
std::mutex m_output;

void print(std::string msg) {
    std::lock_guard lg(m_output);
    std::cout << msg << std::endl;
}

void produce() {
    std::regex txt_regex(".+\\.txt");

    while (true) {
        // get file
        std::optional<fs::path> file = file_jobs.get();
        if (!file)
            break;

        // check if file has extension .txt
        std::string filename = file->filename().string();
        if (!std::regex_match(filename, txt_regex))
            continue;

        // produce lines
        std::ifstream ifs(*file);
        std::string line;
        unsigned int line_number = 1;
        while (std::getline(ifs, line))
            line_jobs.put(FileLine(filename, line, line_number++));
    }
}

void consume(std::regex regex) {
    std::sregex_iterator r_end;

    while (true) {
        // get line
        std::optional<FileLine> fl = line_jobs.get();
        if (!fl)
            break;
        std::string filename = fl->getFilename();
        std::string line = fl->getLine();
        unsigned int line_number = fl->getLineNumber();

        // consume line
        std::sregex_iterator r_begin(line.begin(), line.end(), regex);
        for (std::sregex_iterator it=r_begin; it != r_end; it++) {
            std::string output("filename: " + filename + " --- line: " + std::to_string(line_number) + " --- match: " + it->str());
            print(output);
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " directory regex" << std::endl;
        std::cerr << "example of regex for IP address: "
                  << "((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // launch producer
    std::vector<std::thread> producers;
    for (int i=0; i < N_PRODUCERS; i++)
        producers.emplace_back(produce);

    // launch consumers
    std::vector<std::thread> consumers;
    for (int i=0; i < N_CONSUMERS; i++)
        consumers.emplace_back(consume, std::regex(argv[2]));

    // produce filenames
    for (const auto& entry : fs::directory_iterator(argv[1]))
        file_jobs.put(entry.path());
    file_jobs.close();

    // wait
    for (auto& p : producers)
        p.join();
    line_jobs.close();
    for (auto& c : consumers)
        c.join();

    return 0;
}
