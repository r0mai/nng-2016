cmake_minimum_required(VERSION 2.8)

set(CXX11_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS}")

add_executable(cat cat.cpp solution.cpp)
