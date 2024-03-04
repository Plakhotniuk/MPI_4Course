//
// Created by user on 04.03.24.
//

#ifndef MERGESORT_HPP
#define MERGESORT_HPP

namespace SortAlgorithms {
    void mergeSortRecursive(std::array<int> arr, int na) {
        if (na < 2) return;
        mergeSortRecursive(arr, na / 2);
        mergeSortRecursive(arr + na / 2, na - na / 2);
        int *b = new int[na];
        std::merge(a, a + na / 2, b, na / 2, na − na / 2);
        memcpy(a, b, s i z e o f(i n t) ∗ na);
        delete [] b;
    }
}

#endif //MERGESORT_HPP
