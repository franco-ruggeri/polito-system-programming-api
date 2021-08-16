#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <unordered_map>
#include <array>
#include "Jobs.h"
#include "Results.h"

#define N_MAPPERS 10
#define N_REDUCERS 10
#define DEBUG 0

namespace fs = std::filesystem;

template<typename K, typename V, typename A, typename M, typename R>
void map_reduce(std::istream& input, M& map_fun, R& reduce_fun) {
    Jobs<std::string> line_jobs;
    Jobs<std::pair<K,V>> result_jobs;
    Results<K,A> results;

    // launch mappers
    std::vector<std::thread> mappers;
    for (int i=0; i < N_MAPPERS; i++)
        mappers.emplace_back(map_fun, std::ref(line_jobs), std::ref(result_jobs));

    // launch reducers
    std::vector<std::thread> reducers;
    for (int i=0; i < N_REDUCERS; i++)
        reducers.emplace_back(reduce_fun, std::ref(result_jobs), std::ref(results));

    // produce lines for mappers
    std::string line;
    while (std::getline(input, line))
        line_jobs.put(line);
    line_jobs.close();

    // wait
    for (auto& m : mappers)
        m.join();
    result_jobs.close();
    for (auto& r : reducers)
        r.join();

    // output
    for (const auto& r : results.get_results())
        std::cout << r.first << " => " << r.second << std::endl;
}

int main() {
    // open file stream
    std::ifstream input("localhost_access_log.2020.txt");
    if (!input.is_open()) {
        std::cerr << "File not found" << std::endl;
        std::exit(EXIT_FAILURE);
    }


    /***************
     * Count by ip *
     ***************/
    auto map_count_by_ip = [](Jobs<std::string>& line_jobs, Jobs<std::pair<std::string,int>>& result_jobs) {
        while (true) {
            std::optional<std::string> line = line_jobs.get();
            if (!line) break;
            std::istringstream iss(*line);
            std::string ip;
            if (iss >> ip) result_jobs.put(std::make_pair(ip, 1));
            else std::cerr << "invalid line" << std::endl;
        }
    };

    auto reduce_count_by_ip = [](Jobs<std::pair<std::string,int>>& result_jobs, Results<std::string,int>& results) {
        while (true) {
            std::optional<std::pair<std::string,int>> result = result_jobs.get();
            if (!result) break;
            results.accumulate(result->first, result->second);
        }
    };

    std::cout << "Count by IP address:" << std::endl;
    map_reduce<std::string, int, int>(input, map_count_by_ip, reduce_count_by_ip);
    std::cout << "\n";


    /*****************
     * Count by hour *
     *****************/
    input.clear();
    input.seekg(0);

    auto map_count_by_hour = [](Jobs<std::string>& line_jobs, Jobs<std::pair<int,int>>& result_jobs) {
        while (true) {
            std::optional<std::string> line = line_jobs.get();
            if (!line) break;
            std::istringstream iss(*line);
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            iss.ignore(std::numeric_limits<std::streamsize>::max(), ':');
            int hour;
            if (iss >> hour) result_jobs.put(std::make_pair(hour, 1));
            else std::cerr << "invalid line" << std::endl;
        }
    };

    auto reduce_count_by_hour = [](Jobs<std::pair<int,int>>& result_jobs, Results<int,int>& results) {
        while (true) {
            std::optional<std::pair<int,int>> result = result_jobs.get();
            if (!result) break;
            results.accumulate(result->first, result->second);
        }
    };

    std::cout << "Count by hour:\n";
    map_reduce<int, int, int>(input, map_count_by_hour, reduce_count_by_hour);
    std::cout << "\n";


    /****************
     * Count by url *
     ****************/
    input.clear();
    input.seekg(0);

    auto map_count_by_url = [](Jobs<std::string>& line_jobs, Jobs<std::pair<std::string,int>>& result_jobs) {
        while (true) {
            std::optional<std::string> line = line_jobs.get();
            if (!line) break;
            std::istringstream iss(*line);
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
            iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
            std::string url;
            if (iss >> url) result_jobs.put(std::make_pair(url, 1));
            else std::cerr << "invalid line" << std::endl;
        }
    };

    auto reduce_count_by_url = [](Jobs<std::pair<std::string,int>>& result_jobs, Results<std::string,int>& results) {
        while (true) {
            std::optional<std::pair<std::string,int>> result = result_jobs.get();
            if (!result) break;
            results.accumulate(result->first, result->second);
        }
    };

    std::cout << "Count by URL:\n";
    map_reduce<std::string, int, int>(input, map_count_by_url, reduce_count_by_url);
    std::cout << "\n";


    /***********
     * Attacks *
     ***********/
    input.clear();
    input.seekg(0);

    auto map_attacks = [](Jobs<std::string>& line_jobs, Jobs<std::pair<int,std::string>>& result_jobs) {
        while (true) {
            std::optional<std::string> line = line_jobs.get();
            if (!line) break;
            std::istringstream iss(*line);
            std::string ip;
            int code;

            // IP address
            iss >> ip;
            if (!iss) {
                std::cerr << "invalid line" << std::endl;
                continue;
            }

            // code
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
            do {
                iss.clear();
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
                iss >> code;
            } while (iss.fail());   // there can be " in the URL
            if (!iss) {
                std::cerr << "invalid line" << std::endl;
                continue;
            }

            // add result
            if (code == 400 || code == 404 || code == 405)
                result_jobs.put(std::make_pair(code, ip));
        }
    };

    auto reduce_attacks = [](Jobs<std::pair<int,std::string>>& result_jobs, Results<int,std::string>& results) {
        while (true) {
            std::optional<std::pair<int,std::string>> result = result_jobs.get();
            if (!result) break;
            results.accumulate(result->first, result->second + ", ");
        }
    };

    std::cout << "Attacks:\n";
    map_reduce<int, std::string, std::string>(input, map_attacks, reduce_attacks);
    std::cout << "\n";


#if DEBUG
    /********
     * Test *
     ********/
    // test correct_count_by_hour by hour
    std::array<int,24> correct_count_by_hour;
    std::fill(correct_count_by_hour.begin(), correct_count_by_hour.end(), 0);
    input.clear();
    input.seekg(0);
    std::string line;
    while (std::getline(input, line)) {
        std::istringstream iss(line);
        iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
        iss.ignore(std::numeric_limits<std::streamsize>::max(), ':');
        int hour;
        iss >> hour;
        correct_count_by_hour[hour]++;
    }
    std::cout << "Correct count by hour:\n";
    for (int i=0; i<24; i++)
        std::cout << i << " => " << correct_count_by_hour[i] << std::endl;
#endif

    return 0;
}