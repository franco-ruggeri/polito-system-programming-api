#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <regex>
#include "Jobs.h"
#include "FileLine.h"

#define N_CONSUMER 10

namespace fs = std::filesystem;

Jobs<FileLine> jobs;

void produce(fs::path path) {
    std::regex txt_regex(".+\\.txt");

    for (const auto& entry : fs::directory_iterator(path)) {
        // open file stream
        std::ifstream ifs(entry.path());

        // check if file has extension .txt
        std::string filename = entry.path().filename().string();
        if (!std::regex_match(filename, txt_regex))
            continue;

        // produce jobs
        std::string line;
        unsigned int line_number = 1;
        while (std::getline(ifs, line))
            jobs.put(FileLine(filename, line, line_number++));
    }

    jobs.close();
}

void consume(std::regex regex) {
    std::sregex_iterator r_end;

    while (!jobs.is_closed() || !jobs.empty()) {
        // get job
        FileLine fl = jobs.get();
        std::string filename = fl.getFilename();
        std::string line = fl.getLine();
        unsigned int line_number = fl.getLineNumber();

        // consume job
        std::sregex_iterator r_begin(line.begin(), line.end(), regex);
        for (std::sregex_iterator it=r_begin; it != r_end; it++) {
            std::string output("filename: " + filename + " --- line: " + std::to_string(line_number) + " --- match: " + it->str() + "\n");
            std::cout << output;
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " directory regex" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // launch producer
    std::thread producer(produce, fs::path(argv[1]));

    // launch consumers
    std::vector<std::thread> consumers;
    for (int i=0; i<N_CONSUMER; i++)
        consumers.emplace_back(consume, std::regex(argv[2]));

    // wait
    producer.join();
    for (auto& c : consumers)
        c.join();

    return 0;
}
