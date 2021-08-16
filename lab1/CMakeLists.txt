cmake_minimum_required(VERSION 3.16)
project(lab1)

set(CMAKE_CXX_STANDARD 20)

add_executable(lab1 main.cpp DurationLogger.cpp DurationLogger.h Message.cpp Message.h MessageStore.cpp MessageStore.h)
