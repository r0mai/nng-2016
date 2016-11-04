cmake_minimum_required(VERSION 2.8)

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

set(CXX11_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS} -g")

add_executable(cat cat.cpp solution.cpp)
add_executable(unitTest unitTest.cpp solution.cpp)
target_link_libraries(unitTest
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
