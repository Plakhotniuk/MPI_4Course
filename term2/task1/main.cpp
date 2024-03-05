//
// Created by user on 04.03.24.
//
#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include "MergeSort.hpp"
#include "omp.h"

#include <string>


int main(int argc, char* argv[]) {


    const int arraySize = 30e6;
    const int nt = std::stoi(argv[1]);

    int *arr = new int[arraySize];

    // First create an instance of an engine.
    std::random_device rnd_device;
    // Specify the engine and distribution.
    std::mt19937 mersenne_engine{rnd_device()}; // Generates random integers
    std::uniform_int_distribution<int> dist{0, 10};

    for (int i = 0; i < arraySize; ++i) {
        arr[i] = dist(mersenne_engine);
    }

    double t = omp_get_wtime();
    SortAlgorithms::mergeSortParallel(arr, 0, arraySize - 1, nt);
    t = omp_get_wtime() - t;


    std::fstream file;
    file.open("time.txt", std::ios_base::app);
    file << t << " " << nt << std::endl;

    delete [] arr;
}
