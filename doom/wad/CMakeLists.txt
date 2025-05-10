cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

project(compress_wad C CXX)

add_executable(compress_wad
        main.cpp
        wad.cpp
)
