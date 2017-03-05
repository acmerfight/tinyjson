cmake_minimum_required(VERSION 3.6)
project(tinyjson)

set(CMAKE_CXX_STANDARD 11)

add_library(tinyjson tinyjson.cpp)
add_executable(tinyjson_test test.cpp)
target_link_libraries(tinyjson_test tinyjson)
