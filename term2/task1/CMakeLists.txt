cmake_minimum_required(VERSION 3.17)

project(task1)

set(SOURCES main.cpp MergeSort.hpp Merge.hpp)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp ")

add_executable(task1 ${SOURCES})
