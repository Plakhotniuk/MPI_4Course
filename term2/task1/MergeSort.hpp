//
// Created by user on 04.03.24.
//

#ifndef MERGESORT_HPP
#define MERGESORT_HPP
#include "Merge.hpp"

namespace SortAlgorithms {
    // C++ program for Merge Sort

    // begin is for left index and end is
    // right index of the sub-array
    // of arr to be sorted */
    template<typename T>
    void mergeSort(T *array,
                   int const begin,
                   int const end) {
        // Returns recursively
        if (begin >= end)
            return;

        auto mid = begin + (end - begin) / 2;
        mergeSort(array, begin, mid);
        mergeSort(array, mid + 1, end);
        MergeAlgorithms::merge(array, begin, mid, end);
    }

    template<typename T>
    void mergeSortParallel(T *array,
                           int const begin,
                           int const end, const int nt) {
        // Returns recursively
        if (begin >= end)
            return;

        if (nt < 2) {
            mergeSort(array, begin, end);
            return;
        }

        auto mid = begin + (end - begin) / 2;
        #pragma omp parallel num_threads(2)
        {
            #pragma omp sections
            {
                #pragma omp section
                mergeSortParallel(array, begin, mid, nt / 2);
                #pragma omp section
                mergeSortParallel(array, mid + 1, end, nt - nt / 2);
            }
        }
        MergeAlgorithms::merge(array, begin, mid, end);
    }
}

#endif //MERGESORT_HPP
