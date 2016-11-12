cmake_minimum_required(VERSION 2.8)

project(nng-2016-creep)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(SFML 2 REQUIRED system window graphics)
include_directories(${SFML_INCLUDE_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -g -O2 -D_DEBUG -DFONT_ASCII -DCOLOR_ANSI -DFAST_FWD=20")

add_executable(gcreep creep2.cc gui/Game.cpp Model.cpp MC.cpp)
target_link_libraries(gcreep ${SFML_LIBRARIES})

add_executable(creep creep.cc)
