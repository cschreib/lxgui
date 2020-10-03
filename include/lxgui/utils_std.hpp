#ifndef LXGUI_UTILS_STD_HPP
#define LXGUI_UTILS_STD_HPP

#include <array>
#include <vector>
#include <algorithm>
#include <iostream>

namespace lxgui {
namespace utils
{
    template<typename C, typename T>
    auto find(C& v, const T& s)
    {
        return std::find(v.begin(), v.end(), s);
    }

    template<typename C, typename T>
    auto find_if(C& v, T&& f)
    {
        return std::find_if(v.begin(), v.end(), std::forward<T>(f));
    }
}
}

namespace std
{
    template<class T, size_t N>
    ostream& operator << (ostream& o, const array<T, N>& a)
    {
        o << "(";
        for (size_t i = 0; i < N; ++i)
        {
            if (i != N-1)
                o << a[i] << ", ";
            else
                o << a[i];
        }
        o << ")";
        return o;
    }

    template<class T>
    ostream& operator << (ostream& o, const vector<T>& a)
    {
        o << "(";
        const size_t N = a.size();
        for (size_t i = 0; i < N; ++i)
        {
            if (i != N-1)
                o << a[i] << ", ";
            else
                o << a[i];
        }
        o << ")";
        return o;
    }
}

#endif
