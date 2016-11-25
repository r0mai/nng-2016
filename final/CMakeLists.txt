cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11")

file(GLOB_RECURSE sources client/*.cpp client/*.h client/*.hpp)

add_executable(bees ${sources})
