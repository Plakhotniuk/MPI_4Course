cmake_minimum_required(VERSION 3.17)

project(task1)

set(SOURCES main.cpp MergeSort.hpp)
set(CMAKE_CXX_STANDARD 20)

add_executable(task1 ${SOURCES})
