cmake_minimum_required(VERSION 3.27)
project(Torrent-RD-PC)

set(CMAKE_CXX_STANDARD 20)

add_executable(Torrent_RD_PC main.cpp
        RdFile.cpp
        RdFile.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -municode")