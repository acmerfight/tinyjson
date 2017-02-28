cmake_minimum_required(VERSION 3.6)
project(tinyjson)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES tinyjson.cpp tinyjson.h)
add_library(tinyjson ${SOURCE_FILES})