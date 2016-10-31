cmake_minimum_required(VERSION 2.8)

project(nng-2016-buildings)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++11 -g -DLOCAL")

add_executable(buildings main.cpp)
