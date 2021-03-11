cmake_minimum_required(VERSION 3.5)
project(example)

add_definitions("-std=c++14")

include_directories(${CMAKE_CURRENT_LIST_DIR}/../../include)

add_executable(example main.cpp)