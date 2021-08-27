#ifndef LXGUI_GUI_VECTOR2_HPP
#define LXGUI_GUI_VECTOR2_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>

#include <cmath>
#include <type_traits>

namespace lxgui {
namespace gui
{
    template<typename T>
    struct vector2
    {
        using float_type = std::conditional_t<std::is_floating_point_v<T>, T, double>;

        constexpr vector2() = default;

        constexpr vector2(T mX, T mY) noexcept : x(mX), y(mY) {}

        void set(T mX, T mY) noexcept
        {
            x = mX; y = mY;
        }

        T get_norm_squared() const noexcept
        {
            return x*x + y*y;
        }

        float_type get_norm() const noexcept
        {
            return std::sqrt(static_cast<float_type>(get_norm_squared()));
        }

        vector2<float_type> get_unit() const noexcept
        {
            vector2<float_type> mVec(static_cast<float_type>(x), static_cast<float_type>(y));
            const typename vector2<float_type>::float_type mNorm = get_norm();
            mVec.x /= mNorm;
            mVec.y /= mNorm;
            return mVec;
        }

        vector2<float_type> get_normal() const noexcept
        {
            vector2<float_type> mVec(static_cast<float_type>(-y), static_cast<float_type>(x));
            const typename vector2<float_type>::float_type mNorm = get_norm();
            mVec.x /= mNorm;
            mVec.y /= mNorm;
            return mVec;
        }

        vector2 get_rotated(float_type mAngle) const noexcept
        {
            vector2<float_type> mVec(static_cast<float_type>(x), static_cast<float_type>(y));
            vector2<float_type> mOrig = mVec;

            const float_type ca = std::cos(mAngle);
            const float_type sa = std::sin(mAngle);

            mVec.x = static_cast<T>(mOrig.x*ca - mOrig.y*sa);
            mVec.y = static_cast<T>(mOrig.x*sa + mOrig.y*ca);

            return mVec;
        }

        vector2 get_scaled(const vector2& v) const noexcept
        {
            vector2 mVec = *this;
            mVec.scale(v);
            return mVec;
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

    using vector2f  = vector2<float>;
    using vector2d  = vector2<double>;
    using vector2i  = vector2<int>;
    using vector2ui = vector2<uint>;

    template<typename T>
    vector2<T> operator * (T mValue, const vector2<T>& mV) noexcept
    {
        return vector2<T>(mV.x*mValue, mV.y*mValue);
    }

    template<typename O, typename T>
    O& operator << (O& mStream, const vector2<T>& mV)
    {
        return mStream << mV.x << ", " << mV.y;
    }
}
}

#endif
