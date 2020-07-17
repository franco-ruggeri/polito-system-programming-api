cmake_minimum_required(VERSION 3.16)
project(2019_lab3)

set(CMAKE_CXX_STANDARD 20)

add_executable(2019_lab3 main.cpp NetworkServer.cpp NetworkServer.h SharedEditor.cpp SharedEditor.h Message.cpp Message.h Symbol.cpp Symbol.h LSEQ.cpp LSEQ.h)
