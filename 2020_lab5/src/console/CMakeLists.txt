cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

find_package(Curses REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(console Console.cpp)

target_link_libraries(console ${CURSES_LIBRARIES})