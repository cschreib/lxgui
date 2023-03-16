#ifndef LXGUI_UTILS_STD_HPP
#define LXGUI_UTILS_STD_HPP

#include "lxgui/lxgui.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

namespace lxgui::utils {

template<typename C, typename T>
auto find(C& v, const T& s) {
    return std::find(v.begin(), v.end(), s);
}

template<typename C, typename T>
auto find_if(C& v, T&& f) {
    return std::find_if(v.begin(), v.end(), std::forward<T>(f));
}

} // namespace lxgui::utils

namespace std {

template<class T, std::size_t N>
ostream& operator<<(ostream& o, const array<T, N>& a) {
    o << "(";
    for (std::size_t i = 0; i < N; ++i) {
        if (i != N - 1)
            o << a[i] << ", ";
        else
            o << a[i];
    }
    o << ")";
    return o;
}

template<class T>
ostream& operator<<(ostream& o, const vector<T>& a) {
    o << "(";
    const std::size_t n = a.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (i != n - 1)
            o << a[i] << ", ";
        else
            o << a[i];
    }
    o << ")";
    return o;
}

} // namespace std

#endif
