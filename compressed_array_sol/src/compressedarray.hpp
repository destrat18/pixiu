#pragma once

#include <vector>
#include <algorithm>


template <typename T>
struct CompressedArray {
    size_t len;
    size_t last_element_set = -1;

    // stores monotonic sequence of unique values
    std::vector<T> data;
    // stores the index of start of the data[i] in the original array
    std::vector<int> compressed;

    CompressedArray(size_t len) {
        this->len = len;
    } 

    CompressedArray() {
        this->len = 0;
    }

    T get(size_t i) {
        // std::cout << "in get " << std::endl;
        // std::cout << "trying to get " << i << std::endl;

        if (i >= len || i < 0) { 
            throw std::out_of_range("Index out of range");
        }

        // find the index of the first element in data that is less than or equal to i
        auto it = std::upper_bound(compressed.begin(), compressed.end(), i);
        if (it == compressed.begin()) {
            return data[0];
        } else {
            return data[it - compressed.begin() - 1];
        }
    }


    void set(size_t i, T val) {
        // std::cout << "in set " << std::endl;
        // std::cout << "trying to set " << i << std::endl;

        if (i == 0) {
            data.push_back(val);
            compressed.push_back(0);
            last_element_set = 0;
            return;
        }

        if (i != last_element_set + 1) {
            throw std::out_of_range("Index out of range");
        }

        last_element_set++;

        if (val > data.back()) {
            data.push_back(val);
            compressed.push_back(last_element_set);
            return;
        } else if (!(val < data.back()) && !(data.back() < val)) {
            // do nothing
            return;
        } else {
            throw std::invalid_argument("Value is less than the last element");
        }
    }    
};
