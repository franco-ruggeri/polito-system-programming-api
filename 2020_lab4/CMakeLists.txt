cmake_minimum_required(VERSION 3.16)
project(2020_lab4)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_executable(part1 part1.cpp Jobs.h FileLine.cpp FileLine.h CircularBuffer.h)
add_executable(part2 part2.cpp Jobs.h CircularBuffer.h Results.h)

target_link_libraries(part1 ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(part2 ${CMAKE_THREAD_LIBS_INIT})
