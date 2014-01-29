#ifndef GUI_VECTOR2_HPP
#define GUI_VECTOR2_HPP

#include <cmath>
#include <iosfwd>
#include <lxgui/utils.hpp>

namespace gui
{
    template<class T>
    class vector2
    {
    public :

        vector2()
        {
        }

        vector2(T mX, T mY) : x(mX), y(mY)
        {
        }

        void set(T mX, T mY)
        {
            x = mX; y = mY;
        }

        T get_norm() const
        {
            return T(sqrt(x*x + y*y));
        }

        T get_norm_squared() const
        {
            return x*x + y*y;
        }

        void normalize()
        {
            T mNorm = T(sqrt(x*x + y*y));
            x = x/mNorm;
            y = y/mNorm;
        }

        vector2 get_unit() const
        {
            T mNorm = T(sqrt(x*x + y*y));
            return vector2(x/mNorm, y/mNorm);
        }

        void rotate(const float& fAngle)
        {
            vector2 p;

            double ca = cos(fAngle), sa = sin(fAngle);

            p.x = x*ca - y*sa;
            p.y = x*sa + y*ca;

            x = p.x;
            y = p.y;
        }

        vector2 get_rotated(const float& fAngle) const
        {
            double ca = cos(fAngle), sa = sin(fAngle);
            return vector2(x*ca - y*sa, x*sa + y*ca);
        }

        void scale(const vector2& v)
        {
            x *= v.x;
            y *= v.y;
        }

        vector2 get_scale(const vector2& v) const
        {
            return vector2(x*v.x, y*v.y);
        }

        vector2 operator + (const vector2& v)  const
        {
            return vector2(x + v.x, y + v.y);
        }
        void operator += (const vector2& v)
        {
            x += v.x; y += v.y;
        }

        vector2 operator - () const
        {
            return vector2(-x, -y);
        }

        vector2 operator - (const vector2& v) const
        {
            return vector2(x - v.x, y - v.y);
        }
        void operator -= (const vector2& v)
        {
            x -= v.x; y -= v.y;
        }

        bool operator == (const vector2& v) const
        {
            return (x == v.x) && (y == v.y);
        }
        bool operator != (const vector2& v) const
        {
            return (x != v.x) || (y != v.y);
        }

        vector2 operator * (T mValue) const
        {
            return vector2(x*mValue, y*mValue);
        }

        void operator *= (T mValue)
        {
            x *= mValue;  y *= mValue;
        }

        vector2 operator / (T mValue) const
        {
            return vector2(x/mValue, y/mValue);
        }

        void operator /= (T mValue)
        {
            x /= mValue;  y /= mValue;
        }

        T operator * (const vector2& v) const
        {
            return x*v.x + y*v.y;
        }

        static const vector2 ZERO;
        static const vector2 UNIT;
        static const vector2 X;
        static const vector2 Y;

        T x, y;
    };

    template<class T>
    const vector2<T> vector2<T>::ZERO(0, 0);

    template<class T>
    const vector2<T> vector2<T>::UNIT(1, 1);

    template<class T>
    const vector2<T> vector2<T>::X(1, 0);

    template<class T>
    const vector2<T> vector2<T>::Y(0, 1);

    typedef vector2<float> vector2f;
    typedef vector2<int>   vector2i;
    typedef vector2<uint>  vector2ui;

    template<class T>
    vector2<T> operator * (T mValue, const vector2<T>& mV)
    {
        return vector2<T>(mV.x*mValue, mV.y*mValue);
    }

    template<class T>
    std::ostream& operator << (std::ostream& mStream, const vector2<T>& mV)
    {
        return mStream << mV.x << ", " << mV.y;
    }
}

#endif
