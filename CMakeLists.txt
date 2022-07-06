cmake_minimum_required(VERSION 3.5)

project(simpleini LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(tests/googletest)

add_executable(simpleini-tests
    simpleini/simpleini.h
    tests/main.cpp
    tests/simpleini-tests.cpp
    tests/readwrite-tests.cpp
)

target_link_libraries(simpleini-tests
    gtest
)

target_include_directories(simpleini-tests PRIVATE
    simpleini
)
