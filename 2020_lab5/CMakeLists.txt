cmake_minimum_required(VERSION 3.16)
project(2020_lab5)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(Curses REQUIRED)

add_executable(server server.cpp Socket.cpp Socket.h ServerSocket.cpp ServerSocket.h Jobs.h User.cpp User.h Message.cpp Message.h MessageType.h Messages.cpp Messages.h)
add_executable(client client.cpp Socket.cpp Socket.h Message.cpp Message.h MessageType.h Messages.cpp Messages.h)

target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT} ${CURSES_LIBRARIES})
