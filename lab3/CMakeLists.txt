cmake_minimum_required(VERSION 3.16)
project(lab3)

set(CMAKE_CXX_STANDARD 20)

add_executable(lab3 main.cpp Serializable.cpp Serializable.h MapperInput.h ReducerInput.h Result.h DurationLogger.h Pipe.cpp Pipe.h PipeException.h)
