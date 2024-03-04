//
// Created by user on 04.03.24.
//
#include <algorithm>
#include <functional>
#include <random>
#include <ranges>
#include "MergeSort.hpp"
#include <iostream>


int main(int argc, char *argv[]) {
    const int arraySize = 10;

    // fill the vectors with random numbers
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<> dis();
    std::vector<int> arr(arraySize);
    std::generate(arr.begin(), arr.end(), std::bind(dis, std::ref(mt)));


    auto print_elem = [](auto const e) { std::cout << e << '\n'; };

}
