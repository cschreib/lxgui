#ifndef UTILS_HPP
#define UTILS_HPP

#include <cmath>
#define foreach(iter, cont) for (iter = (cont).begin(); iter != (cont).end(); ++(iter))

typedef unsigned long ulong;
typedef unsigned int  uint;
typedef unsigned char uchar;

namespace math
{
    #ifdef MSVC
    template<typename T>
    inline bool isinf(T x)
    {
        return !_finite(x);
    }
    #else
    using std::isinf;
    #endif
}

#endif
