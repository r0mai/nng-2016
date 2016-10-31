cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(Boost 1.56 REQUIRED)
find_package(LibGMP)
include_directories(${Boost_INCLUDE_DIRS})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CXX11_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS} -O3")

add_executable(tiling tiling.cpp)
target_link_libraries(tiling ${Boost_LIBRARIES} gmp)
