//
// Created by user on 04.03.24.
//
#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>
#include "MergeSort.hpp"



int main() {
    const int arraySize = 1e6;

    int *arr = new int[arraySize];

    // First create an instance of an engine.
    std::random_device rnd_device;
    // Specify the engine and distribution.
    std::mt19937 mersenne_engine {rnd_device()};  // Generates random integers
    std::uniform_int_distribution<int> dist {};

    for(int i = 0; i < arraySize; ++i) {
        arr[i] = dist(mersenne_engine);
    }

    auto start = std::chrono::high_resolution_clock::now();
    SortAlgorithms::mergeSort(arr, 0, arraySize - 1);
    auto sortTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout<<"Time: " << sortTime.count() << ", N: " << arraySize << std::endl;

    delete [] arr;
}
