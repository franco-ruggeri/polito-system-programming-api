#include <iostream>
#include <functional>
#include <queue>
#include "Serializable.h"
#include "MapperInput.h"
#include "ReducerInput.h"
#include "Result.h"
#include "DurationLogger.h"
#include "Pipe.h"
#include "PipeException.h"

template<typename MapperInputT, typename ReducerInputT, typename ResultT, typename K, typename A, typename M, typename R>
std::vector<ResultT> map_reduce_single_process(std::istream& input, M& map_fun, R& reduce_fun) {
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

template<typename MapperInputT, typename ReducerInputT, typename ResultT, typename K, typename A, typename M, typename R>
std::vector<ResultT> map_reduce_multi_process_synchronous(std::istream& input, M& map_fun, R& reduce_fun) {
    std::unordered_map<K,A> accs;
    MapperInputT mapper_input;
    ReducerInputT reducer_input;
    std::vector<ResultT> mapper_results;
    ResultT reducer_result;
    std::shared_ptr<char[]> read_ptr;
    std::vector<char> write_v;
    Pipe pipe_cm, pipe_mc, pipe_cr, pipe_rc;
    pid_t pid;

    /**********
     * Mapper *
     **********/
    pid = fork();
    if (!pid) {
        while (true) {
            try {
                read_ptr = pipe_cm.read();
                mapper_input.deserializeBinary(read_ptr);
                mapper_results = map_fun(mapper_input);
                write_v = serialize_binary(mapper_results);
                pipe_mc.write(write_v);
            } catch (PipeException e) {
                if (e.isEOF())
                    std::exit(EXIT_SUCCESS);    // input terminated => all done for the mapper
                else throw;
            }
        }
    } else if (pid < 0) {
        throw std::runtime_error("error - fork() failed");
    }

    /***********
     * Reducer *
     ***********/
    pid = fork();
    if (!pid) {
        while (true) {
            try {
                read_ptr = pipe_cr.read();
                reducer_input.deserializeBinary(read_ptr);
                reducer_result = reduce_fun(reducer_input);
                write_v = reducer_result.serializeBinary();
                pipe_rc.write(write_v);
            } catch (PipeException e) {
                if (e.isEOF())
                    std::exit(EXIT_SUCCESS);    // input terminated => all done for the reducer
                else throw;
            }
        }
    } else if (pid < 0) {
        throw std::runtime_error("error - fork() failed");
    }

    /***************
     * Coordinator *
     ***************/
    while (input >> mapper_input) {
        // communicate with mapper
        write_v = mapper_input.serializeBinary();
        pipe_cm.write(write_v);
        read_ptr = pipe_mc.read();
        mapper_results = deserialize_binary<ResultT>(read_ptr);

        // communicate with reducer
        for (const auto& mr : mapper_results) {
            reducer_input = ReducerInputT(mr.getKey(), mr.getValue(), accs[mr.getKey()]);
            write_v = reducer_input.serializeBinary();
            pipe_cr.write(write_v);
            read_ptr = pipe_rc.read();
            reducer_result.deserializeBinary(read_ptr);
            accs[reducer_result.getKey()] = reducer_result.getValue();
        }
    }

    // map to vector
    std::vector<ResultT> results;
    for (const auto& acc : accs)
        results.push_back(ResultT(acc.first, acc.second));
    return results;
}

template<typename MapperInputT, typename ReducerInputT, typename ResultT, typename K, typename A, typename M, typename R>
std::vector<ResultT> map_reduce_multi_process_multiplexed(std::istream& input, M& map_fun, R& reduce_fun) {
    std::unordered_map<K,A> accs;
    MapperInputT mapper_input;
    ReducerInputT reducer_input;
    std::queue<ResultT> mapper_results;
    std::vector<ResultT> mapper_new_results;
    ResultT result;
    K key;
    std::shared_ptr<char[]> read_ptr;
    std::vector<char> write_v;
    std::vector<Pipe> pipes(4);     // 0 -> cm, 1 -> mc, 2 -> cr, 3 -> rc
    pid_t pid;

    /**********
     * Mapper *
     **********/
    pid = fork();
    if (!pid) {
        while (true) {
            pipes[0].closeWrite();
            pipes[1].closeRead();
            pipes[2].close();
            pipes[3].close();

            try {
                read_ptr = pipes[0].read();
                mapper_input.deserializeBinary(read_ptr);
                mapper_new_results = map_fun(mapper_input);
                for (const auto& mr : mapper_new_results) {
                    write_v = mr.serializeBinary();
                    pipes[1].write(write_v);
                }
            } catch (PipeException e) {
                if (e.isEOF()) std::exit(EXIT_SUCCESS);    // input terminated => all done for the mapper
                else throw;
            }
        }
    } else if (pid < 0) {
        throw std::runtime_error("error - fork() failed");
    }

    /***********
     * Reducer *
     ***********/
    pid = fork();
    if (!pid) {
        while (true) {
            pipes[0].close();
            pipes[1].close();
            pipes[2].closeWrite();
            pipes[3].closeRead();

            try {
                read_ptr = pipes[2].read();
                result.deserializeBinary(read_ptr);
                key = result.getKey();
                reducer_input = ReducerInputT(key, result.getValue(), accs[key]);
                result = reduce_fun(reducer_input);
                accs[key] = result.getValue();
                write_v = result.serializeBinary();
                pipes[3].write(write_v);
            } catch (PipeException e) {
                if (e.isEOF()) std::exit(EXIT_SUCCESS);    // input terminated => all done for the reducer
                else throw;
            }
        }
    } else if (pid < 0) {
        throw std::runtime_error("error - fork() failed");
    }

    /***************
     * Coordinator *
     ***************/
    pipes[0].closeRead();
    pipes[1].closeWrite();
    pipes[2].closeRead();
    pipes[3].closeWrite();

    while (std::any_of(pipes.begin(), pipes.end(), [](Pipe& p) { return !p.isClose(); })) {
        Pipe::select(pipes);

        // send work to mapper
        if (pipes[0].isReadyWrite()) {
            input >> mapper_input;
            if (!input) {
                pipes[0].close();   // all done for mapper
            } else {
                write_v = mapper_input.serializeBinary();
                pipes[0].write(write_v);
            }
        }

        // get (one!) result from mapper
        if (pipes[1].isReadyRead()) {
            try {
                read_ptr = pipes[1].read();
                result.deserializeBinary(read_ptr);
                mapper_results.push(result);
            } catch (PipeException e) {
                if (e.isEOF()) pipes[1].close();
                else throw;
            }
        }

        // send work to reducer
        if (pipes[2].isReadyWrite()) {
            if (!mapper_results.empty()) {
                result = mapper_results.front();
                mapper_results.pop();
                write_v = result.serializeBinary();
                pipes[2].write(write_v);
            } else if (pipes[0].isClose() && pipes[1].isClose()) {
                pipes[2].close();   // all done for reducer
            }
        }

        // get result from reducer
        if (pipes[3].isReadyRead()) {
            try {
                read_ptr = pipes[3].read();
                result.deserializeBinary(read_ptr);
                accs[result.getKey()] = result.getValue();
            } catch (PipeException e) {
                if (e.isEOF()) pipes[3].close();
                else throw;
            }
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

//    // compare JSON and binary serialization
//    measure_serialization(input, serialize_json<MapperInput>, deserialize_json<MapperInput>, "JSON serialization");
//    measure_serialization(input, serialize_binary<MapperInput>, deserialize_binary<MapperInput>, "binary serialization");
//    std::cout << "\n";
//
//    DurationLogger dl("main - MapReduce");
//
//    /***************
//     * Count by ip *
//     ***************/
    // reduce for counting
    auto reduce_count = [](const auto& input) {
        return Result(input.getKey(), input.getAcc() + input.getValue());
    };
//
//    // prepare file stream
//    input.clear();
//    input.seekg(0);
//
//    // map
//    auto map_count_by_ip = [](const MapperInput& mapper_input) {
//        std::istringstream iss(mapper_input.getInput());
//        std::string ip;
//        if (iss >> ip)
//            return std::vector{Result(ip, 1)};
//        else
//            std::exit(EXIT_FAILURE);
//    };
//
//    // run map-reduce
//    auto count_by_ip = map_reduce_multi_process_multiplexed
//            <MapperInput, ReducerInput<std::string, int, int>, Result<std::string, int>, std::string, int>
//            (input, map_count_by_ip, reduce_count);
//    std::cout << "Count by IP address:\n" << count_by_ip << std::endl;


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
    auto count_by_hour = map_reduce_multi_process_multiplexed
            <MapperInput, ReducerInput<std::string, int, int>, Result<std::string, int>, std::string, int>
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
    auto count_by_url = map_reduce_multi_process_multiplexed
            <MapperInput, ReducerInput<std::string, int, int>, Result<std::string, int>, std::string, int>
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
    auto attacks = map_reduce_multi_process_multiplexed
            <MapperInput, ReducerInput<std::string, std::string, std::string>,
            Result<std::string, std::string>, std::string, std::string>
            (input, map_attacks, reduce_attacks);
    std::cout << "Attacks:\n" << attacks;

    return 0;
}
