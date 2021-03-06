//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 12.03.2021.
//

#ifndef COMM_MAPUTILS_HPP
#define COMM_MAPUTILS_HPP

#include <cassert>
#include <map>

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

#endif //COMM_MAPUTILS_HPP
