#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>


// T must support operator<
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

    T get(size_t i) {
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

struct IntWrapper {
    int val;
    IntWrapper(int val) {
        this->val = val;
    }
    bool operator<(const IntWrapper& other) const {
        return val < other.val;
    }
    bool operator>(const IntWrapper& other) const {
        // use < to implement >
        return other < *this;
    }
    bool operator==(const IntWrapper& other) const {
        return !(val < other.val) && !(other.val < val);
    }

};

int main() {
    CompressedArray<IntWrapper> arr(10);
    arr.set(0, IntWrapper(1));
    arr.set(1, IntWrapper(1));
    arr.set(2, IntWrapper(1));
    arr.set(3, IntWrapper(2));
    arr.set(4, IntWrapper(2));
    arr.set(5, IntWrapper(2));
    arr.set(6, IntWrapper(3));
    arr.set(7, IntWrapper(3));
    arr.set(8, IntWrapper(3));
    arr.set(9, IntWrapper(3));

    // print arr data
    std::cout << "data: ";
    for (auto& i : arr.data) {
        std::cout << i.val << " ";
    }
    std::cout << std::endl;

    // print arr compressed
    std::cout << "compressed: ";
    for (auto& i : arr.compressed) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    

    std::cout << arr.get(0).val << std::endl;
    std::cout << arr.get(1).val << std::endl;
    std::cout << arr.get(2).val << std::endl;
    std::cout << arr.get(3).val << std::endl;
    std::cout << arr.get(4).val << std::endl;
    std::cout << arr.get(5).val << std::endl;
    std::cout << arr.get(6).val << std::endl;
    std::cout << arr.get(7).val << std::endl;
    std::cout << arr.get(8).val << std::endl;
    std::cout << arr.get(9).val << std::endl;

}



