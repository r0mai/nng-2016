cmake_minimum_required(VERSION 2.8)

project(nng-2016-creep)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -O2 -D_DEBUG -DFONT_ASCII -DCOLOR_ANSI")

add_executable(creep creep.cc)
add_executable(creep2 creep2.cc)
add_executable(kerrigen kerrigan.cpp)
