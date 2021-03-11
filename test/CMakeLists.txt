cmake_minimum_required(VERSION 3.5)
project(scorpion-headers-test VERSION 1.0.0 LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_definitions("-std=c++14")

include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

add_executable(${PROJECT_NAME} main.cpp
    src/StateMachineTests.cpp)

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
