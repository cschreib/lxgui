#ifndef LXGUI_GUI_QUAD2_HPP
#define LXGUI_GUI_QUAD2_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_vector2.hpp"

namespace lxgui {
namespace gui
{
    template<typename T>
    struct quad2
    {
        constexpr quad2() = default;

        constexpr quad2(T mLeft, T mRight, T mTop, T mBottom) noexcept :
            left(mLeft), right(mRight), top(mTop), bottom(mBottom) {}

        void set(T mLeft, T mRight, T mTop, T mBottom) noexcept
        {
            left = mLeft; right = mRight; top = mTop; bottom = mBottom;
        }

        vector2<T> top_left() const noexcept
        {
            return vector2<T>(left, top);
        }

        vector2<T> top_right() const noexcept
        {
            return vector2<T>(right, top);
        }

        vector2<T> bottom_right() const noexcept
        {
            return vector2<T>(right, bottom);
        }

        vector2<T> bottom_left() const noexcept
        {
            return vector2<T>(left, bottom);
        }

        vector2<T> center() const noexcept
        {
            return vector2<T>((left + right)/2, (top + bottom)/2);
        }

        T width() const noexcept
        {
            return right - left;
        }

        T height() const noexcept
        {
            return bottom - top;
        }

        bool contains(const vector2<T>& mPoint) const
        {
            return mPoint.x >= left && mPoint.x < right && mPoint.y >= top && mPoint.y < bottom;
        }

        bool overlaps(const quad2<T>& mQuad) const
        {
            auto range_overlaps = [](T mMin1, T mMax1, T mMin2, T mMax2)
            {
                return (mMin1 >= mMin2 && mMin1 < mMax2) || (mMin2 >= mMin1 && mMin2 < mMax1);
            };

            return range_overlaps(left, right, mQuad.left, mQuad.right) &&
                   range_overlaps(top, bottom, mQuad.top, mQuad.bottom);
        }

        static const quad2 ZERO;

        T left = 0, right = 0, top = 0, bottom = 0;
    };

    template<typename T>
    quad2<T> operator + (const vector2<T>& mOffset, const quad2<T>& mQuad) noexcept
    {
        quad2<T> mTmp = mQuad;
        mTmp.left += mOffset.x;
        mTmp.right += mOffset.x;
        mTmp.top += mOffset.y;
        mTmp.bottom += mOffset.y;
        return mTmp;
    }

    template<typename T>
    quad2<T> operator + (const quad2<T>& mQuad, const vector2<T>& mOffset) noexcept
    {
        quad2<T> mTmp = mQuad;
        mTmp.left += mOffset.x;
        mTmp.right += mOffset.x;
        mTmp.top += mOffset.y;
        mTmp.bottom += mOffset.y;
        return mTmp;
    }

    template<typename T>
    quad2<T> operator - (const quad2<T>& mQuad, const vector2<T>& mOffset) noexcept
    {
        quad2<T> mTmp = mQuad;
        mTmp.left -= mOffset.x;
        mTmp.right -= mOffset.x;
        mTmp.top -= mOffset.y;
        mTmp.bottom -= mOffset.y;
        return mTmp;
    }

    template<typename T>
    quad2<T> operator * (const quad2<T>& mQuad, T fScale) noexcept
    {
        quad2<T> mTmp = mQuad;
        mTmp.left *= fScale;
        mTmp.right *= fScale;
        mTmp.top *= fScale;
        mTmp.bottom *= fScale;
        return mTmp;
    }

    template<typename T>
    quad2<T> operator * (T fScale, const quad2<T>& mQuad) noexcept
    {
        quad2<T> mTmp = mQuad;
        mTmp.left *= fScale;
        mTmp.right *= fScale;
        mTmp.top *= fScale;
        mTmp.bottom *= fScale;
        return mTmp;
    }

    template<typename T>
    quad2<T> operator / (const quad2<T>& mQuad, T fScale) noexcept
    {
        quad2<T> mTmp = mQuad;
        mTmp.left /= fScale;
        mTmp.right /= fScale;
        mTmp.top /= fScale;
        mTmp.bottom /= fScale;
        return mTmp;
    }

    template<typename T>
    const quad2<T> quad2<T>::ZERO(0, 0, 0, 0);

    using quad2f = quad2<float>;
    using quad2i = quad2<int>;

    template<typename O, typename T>
    O& operator << (O& mStream, const quad2<T>& mQuad)
    {
        return mStream << "(" << mQuad.top_left() << "), (" << mQuad.bottom_right() << ")";
    }
}
}

#endif
