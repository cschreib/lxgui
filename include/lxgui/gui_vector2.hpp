#ifndef LXGUI_GUI_VECTOR2_HPP
#define LXGUI_GUI_VECTOR2_HPP

#include <cmath>
#include <iosfwd>
#include <lxgui/utils.hpp>

namespace lxgui {
namespace gui
{
    template<typename T>
    struct vector2
    {
        constexpr vector2() = default;

        constexpr vector2(T mX, T mY) noexcept : x(mX), y(mY) {}

        void set(T mX, T mY) noexcept
        {
            x = mX; y = mY;
        }

        T get_norm() const noexcept
        {
            return T(sqrt(x*x + y*y));
        }

        T get_norm_squared() const noexcept
        {
            return x*x + y*y;
        }

        void normalize() noexcept
        {
            T mNorm = T(sqrt(x*x + y*y));
            x = x/mNorm;
            y = y/mNorm;
        }

        vector2 get_unit() const noexcept
        {
            T mNorm = T(sqrt(x*x + y*y));
            return vector2(x/mNorm, y/mNorm);
        }

        void rotate(const float& fAngle) noexcept
        {
            vector2 p;

            double ca = cos(fAngle), sa = sin(fAngle);

            p.x = x*ca - y*sa;
            p.y = x*sa + y*ca;

            x = p.x;
            y = p.y;
        }

        vector2 get_rotated(const float& fAngle) const noexcept
        {
            double ca = cos(fAngle), sa = sin(fAngle);
            return vector2(x*ca - y*sa, x*sa + y*ca);
        }

        void scale(const vector2& v) noexcept
        {
            x *= v.x;
            y *= v.y;
        }

        vector2 get_scale(const vector2& v) const noexcept
        {
            return vector2(x*v.x, y*v.y);
        }

        vector2 operator + (const vector2& v)  const noexcept
        {
            return vector2(x + v.x, y + v.y);
        }
        void operator += (const vector2& v) noexcept
        {
            x += v.x; y += v.y;
        }

        vector2 operator - () const noexcept
        {
            return vector2(-x, -y);
        }

        vector2 operator - (const vector2& v) const noexcept
        {
            return vector2(x - v.x, y - v.y);
        }
        void operator -= (const vector2& v) noexcept
        {
            x -= v.x; y -= v.y;
        }

        bool operator == (const vector2& v) const noexcept
        {
            return (x == v.x) && (y == v.y);
        }
        bool operator != (const vector2& v) const noexcept
        {
            return (x != v.x) || (y != v.y);
        }

        vector2 operator * (T mValue) const noexcept
        {
            return vector2(x*mValue, y*mValue);
        }

        void operator *= (T mValue) noexcept
        {
            x *= mValue;  y *= mValue;
        }

        vector2 operator / (T mValue) const noexcept
        {
            return vector2(x/mValue, y/mValue);
        }

        void operator /= (T mValue) noexcept
        {
            x /= mValue;  y /= mValue;
        }

        T operator * (const vector2& v) const noexcept
        {
            return x*v.x + y*v.y;
        }

        static const vector2 ZERO;
        static const vector2 UNIT;
        static const vector2 X;
        static const vector2 Y;

        T x = 0, y = 0;
    };

    template<typename T>
    constexpr vector2<T> vector2<T>::ZERO(0, 0);

    template<typename T>
    constexpr vector2<T> vector2<T>::UNIT(1, 1);

    template<typename T>
    constexpr vector2<T> vector2<T>::X(1, 0);

    template<typename T>
    constexpr vector2<T> vector2<T>::Y(0, 1);

    using vector2f = vector2<float>;
    using vector2i  = vector2<int>;
    using vector2ui = vector2<uint>;

    template<typename T>
    vector2<T> operator * (T mValue, const vector2<T>& mV) noexcept
    {
        return vector2<T>(mV.x*mValue, mV.y*mValue);
    }

    template<typename T>
    std::ostream& operator << (std::ostream& mStream, const vector2<T>& mV)
    {
        return mStream << mV.x << ", " << mV.y;
    }
}
}

#endif
