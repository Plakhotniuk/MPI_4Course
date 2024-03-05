//
// Created by user on 05.03.24.
//

#ifndef MERGE_HPP
#define MERGE_HPP

namespace MergeAlgorithms {
    // Merges two subarrays of array[].
    // First subarray is arr[begin..mid]
    // Second subarray is arr[mid+1..end]
    template<typename T>
    void merge(T *array, int const left,
               int const mid, int const right) {
        auto const subArrayOne = mid - left + 1;
        auto const subArrayTwo = right - mid;

        // Create temp arrays
        auto *leftArray = new T[subArrayOne],
                *rightArray = new T[subArrayTwo];

        // Copy data to temp arrays leftArray[]
        // and rightArray[]
        for (auto i = 0; i < subArrayOne; i++)
            leftArray[i] = array[left + i];
        for (auto j = 0; j < subArrayTwo; j++)
            rightArray[j] = array[mid + 1 + j];

        // Initial index of first sub-array
        // Initial index of second sub-array
        auto indexOfSubArrayOne = 0,
                indexOfSubArrayTwo = 0;

        // Initial index of merged array
        int indexOfMergedArray = left;

        // Merge the temp arrays back into
        // array[left..right]
        while (indexOfSubArrayOne < subArrayOne &&
               indexOfSubArrayTwo < subArrayTwo) {
            if (leftArray[indexOfSubArrayOne] <=
                rightArray[indexOfSubArrayTwo]) {
                array[indexOfMergedArray] =
                        leftArray[indexOfSubArrayOne];
                indexOfSubArrayOne++;
            } else {
                array[indexOfMergedArray] =
                        rightArray[indexOfSubArrayTwo];
                indexOfSubArrayTwo++;
            }
            indexOfMergedArray++;
        }

        // Copy the remaining elements of
        // left[], if there are any
        while (indexOfSubArrayOne < subArrayOne) {
            array[indexOfMergedArray] =
                    leftArray[indexOfSubArrayOne];
            indexOfSubArrayOne++;
            indexOfMergedArray++;
        }

        // Copy the remaining elements of
        // right[], if there are any
        while (indexOfSubArrayTwo < subArrayTwo) {
            array[indexOfMergedArray] =
                    rightArray[indexOfSubArrayTwo];
            indexOfSubArrayTwo++;
            indexOfMergedArray++;
        }
        delete [] leftArray;
        delete [] rightArray;
    }
}

#endif //MERGE_HPP
