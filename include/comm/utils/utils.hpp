//
// Created by ga78cat on 12.03.2021.
//

#ifndef PINAKOTHEKDRAWING_UTILS_HPP
#define PINAKOTHEKDRAWING_UTILS_HPP

#include <algorithm>
#include <cassert>
#include <map>
#include <utility>
#include <vector>

template<typename T>
inline int intRound(const T a) {
    return lround(a);
}

template<typename T>
inline T fastMin(const T a, const T b) {
    return (a < b ? a : b);
}

template<typename T>
inline bool equal(const T a, const T b, double tol = 1e-9) {
    return (abs(a - b) < tol);
}

template<typename T>
inline bool less(const T a, const T b, double tol = 1e-9) {
    return (b - a) > tol;
}

template<typename T>
inline bool lessEqual(const T a, const T b, double tol = 1e-9) {
    return (b - a) > -tol;
}

template<typename T>
inline bool greater(const T a, const T b, double tol = 1e-9) {
    return (a - b) > tol;
}

template<typename T>
inline bool greaterEqual(const T a, const T b, double tol = 1e-9) {
    return (a - b) > -tol;
}

template<typename T>
inline T median(std::vector<T> a) {
    int n = a.size();
    // Applying nth_element on n/2th index
    std::nth_element(a.begin(), a.begin() + n / 2, a.end());
    T median = a[n / 2];

    if (n % 2 == 0) {
        // Applying nth_element on (n-1)/2 th index
        std::nth_element(a.begin(), a.begin() + (n - 1) / 2, a.end());
        median = (a[(n - 1) / 2] + median) / 2.0;
    }
    return median;
}

template<typename T>
inline T average(std::vector<T> a) {
    T avg;
    for (const T & val : a) {
        avg += val;
    }
    avg /= (double) a.size();
    return avg;
}

template<class T1, class T2>
bool mapContains(const std::map<T1, T2> &container, const T1 &key, T2 *value = nullptr) {
    const auto &data = container.find(key);
    if (data != container.end()) {
        if (value != nullptr) {
            *value = data->second;
        }
        return true;
    }
    return false;
}

template<class T1, class T2>
bool mapGetIfContains(const std::map<T1, T2> &container, const T1 &key, T2 &value) {
    return mapContains(container, key, &value);
}

template<class T1, class T2>
T2 mapGet(const std::map<T1, T2> &container, const T1 &key) {
    T2 value;
    if (!mapContains(container, key, &value)) {
        assert(false);
    }
    return value;
}

#endif //PINAKOTHEKDRAWING_UTILS_HPP
