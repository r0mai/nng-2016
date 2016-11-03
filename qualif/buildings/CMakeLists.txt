cmake_minimum_required(VERSION 2.8)

project(nng-2016-buildings)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11")

add_executable(buildings
    main.cpp
    calculate2.cpp
)
