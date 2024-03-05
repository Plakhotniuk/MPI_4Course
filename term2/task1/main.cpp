//
// Created by user on 04.03.24.
//
#include <algorithm>
#include <functional>
#include <random>
#include "MergeSort.hpp"
#include <vector>
#include <iostream>
#include <iterator>


int main(int argc, char *argv[]) {
    const int arraySize = 10;

    int *arr = new int[arraySize];

    // First create an instance of an engine.
    std::random_device rnd_device;
    // Specify the engine and distribution.
    std::mt19937 mersenne_engine {rnd_device()};  // Generates random integers
    std::uniform_int_distribution<int> dist {};

    for(int i = 0; i < arraySize; ++i) {
        arr[i] = dist(mersenne_engine);
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;

    SortAlgorithms::mergeSort(arr, 0, arraySize - 1);

    for(int i = 0; i < arraySize; ++i) {
        std::cout << arr[i] << " ";
    }

    delete [] arr;
}
