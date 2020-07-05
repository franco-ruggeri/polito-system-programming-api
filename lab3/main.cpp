#include <iostream>
#include "Serializable.h"
#include "MapperInput.h"
#include "ReducerInput.h"
#include "Result.h"
#include "DurationLogger.h"

template<typename MapperInputT, typename ReducerInputT, typename ResultT, typename K, typename A, typename M, typename R>
std::vector<ResultT> map_reduce(std::istream& input, M& map_fun, R& reduce_fun) {
    std::unordered_map<K,A> accs;
    MapperInputT mapper_input;

    while (input >> mapper_input) {
        std::vector<ResultT> mapper_results = map_fun(mapper_input);
        for (const auto& mr : mapper_results) {
            ReducerInputT reducer_input(mr.getKey(), mr.getValue(), accs[mr.getKey()]);
            ResultT reducer_result = reduce_fun(reducer_input);
            accs[reducer_result.getKey()] = reducer_result.getValue();
        }
    }


    // map to vector
    std::vector<ResultT> results;
    for (const auto& acc : accs)
        results.push_back(ResultT(acc.first, acc.second));
    return results;
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T> v) {
    for (const auto& entry : v)
        out << entry << std::endl;
    return out;
}

template<typename F1, typename F2>
void measure_serialization(std::istream& input, F1 serialize, F2 deserialize, std::string name) {
    // load
    input.clear();
    input.seekg(0);
    std::vector<MapperInput> x;
    MapperInput mapper_input;
    for (int i=0; i<10; i++) {
        input >> mapper_input;
        x.push_back(mapper_input);
    }

    // serialize
    DurationLogger dl(name);
    std::vector<char> x_v = serialize(x);

    // vector to smart pointer
    std::shared_ptr<char[]> x_p(new char[x_v.size() + 1]);
    for (int i=0; i < x_v.size(); i++)
        x_p[i] = x_v[i];
    x_p[x_v.size()] = 0;

    // deserialize
    x = deserialize(x_p);
    std::cout << x;
}

int main() {
    // open file stream
    std::ifstream input("localhost_access_log.2020.txt");
    if (!input.is_open()) {
        std::cerr << "File not found" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // compare JSON and binary serialization
    measure_serialization(input, serialize_json<MapperInput>, deserialize_json<MapperInput>, "JSON serialization");
    measure_serialization(input, serialize_binary<MapperInput>, deserialize_binary<MapperInput>, "binary serialization");
    std::cout << "\n";

    // reduce for counting
    auto reduce_count = [](const auto& input) {
        return Result(input.getKey(), input.getAcc() + input.getValue());
    };


    /***************
     * Count by ip *
     ***************/
    // prepare file stream
    input.clear();
    input.seekg(0);

    // map
    auto map_count_by_ip = [](const MapperInput& mapper_input) {
        std::istringstream iss(mapper_input.getInput());
        std::string ip;
        if (iss >> ip)
            return std::vector{Result(ip, 1)};
        else
            std::exit(EXIT_FAILURE);
    };

    // run map-reduce
    auto count_by_ip = map_reduce<MapperInput, ReducerInput<std::string,int,int>, Result<std::string,int>, std::string, int>
            (input, map_count_by_ip, reduce_count);
    std::cout << "Count by IP address:\n" << count_by_ip << std::endl;


    /*****************
     * Count by hour *
     *****************/
    // prepare file stream
    input.clear();
    input.seekg(0);

    // map
    auto map_count_by_hour = [](const MapperInput& mapper_input) {
        std::istringstream iss(mapper_input.getInput());
        int hour;

        iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
        iss.ignore(std::numeric_limits<std::streamsize>::max(), ':');
        if (iss >> hour)
            return std::vector{Result(std::to_string(hour), 1)};
        else
            std::exit(EXIT_FAILURE);
    };

    // run map-reduce
    auto count_by_hour = map_reduce<MapperInput, ReducerInput<std::string,int,int>, Result<std::string,int>, std::string, int>
            (input, map_count_by_hour, reduce_count);
    std::cout << "Count by hour:\n" << count_by_hour << std::endl;


    /****************
     * Count by url *
     ****************/
    // prepare file stream
    input.clear();
    input.seekg(0);

    // map
    auto map_count_by_url = [](const MapperInput& mapper_input) {
        std::istringstream iss(mapper_input.getInput());
        std::string url;

        iss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
        iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
        if (iss >> url)
            return std::vector{Result(url, 1)};
        else
            std::exit(EXIT_FAILURE);
    };

    // run map-reduce
    auto count_by_url = map_reduce<MapperInput, ReducerInput<std::string,int,int>, Result<std::string,int>, std::string, int>
            (input, map_count_by_url, reduce_count);
    std::cout << "Count by URL:\n" << count_by_url << std::endl;


    /***********
     * Attacks *
     ***********/
    // prepare file stream
    input.clear();
    input.seekg(0);

    // map
    auto map_attacks = [](const MapperInput& mapper_input) {
        std::istringstream iss(mapper_input.getInput());
        std::string ip;
        int code;

        iss >> ip;
        iss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
        do {
            iss.clear();
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
            iss >> code;
        } while (iss.fail());   // there can be " in the URL

        if (iss) {
            if (code == 400 || code == 404 || code == 405)
                return std::vector{Result(std::to_string(code), ip)};
            else
                return std::vector<Result<std::string,std::string>>{};
        } else {
            std::exit(EXIT_FAILURE);
        }
    };

    // reduce
    auto reduce_attacks = [](const ReducerInput<std::string,std::string,std::string>& input) {
        return Result(input.getKey(),input.getAcc() + (input.getAcc() != "" ? ", " : "") + input.getValue());
    };

    // run map-reduce
    auto attacks = map_reduce<MapperInput, ReducerInput<std::string,std::string,std::string>,
                                Result<std::string,std::string>, std::string, std::string>
            (input, map_attacks, reduce_attacks);
    std::cout << "Attacks:\n" << attacks << std::endl;

    return 0;
}
