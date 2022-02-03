#ifndef LXGUI_GUI_BOUNDS2_HPP
#define LXGUI_GUI_BOUNDS2_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/gui_vector2.hpp"

namespace lxgui {
namespace gui
{
    template<typename T>
    struct bounds2
    {
        constexpr bounds2() = default;

        constexpr bounds2(T mLeft, T mRight, T mTop, T mBottom) noexcept :
            left(mLeft), right(mRight), top(mTop), bottom(mBottom) {}

        constexpr bounds2(const vector2<T>& mCenter, const vector2<T>& mSize) noexcept :
            left(mCenter.x - mSize.x/2), right(left + mSize.x),
            top(mCenter.y - mSize.y/2), bottom(top + mSize.y) {}

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

        vector2<T> dimensions() const noexcept
        {
            return vector2<T>(width(), height());
        }

        bool contains(const vector2<T>& mPoint) const
        {
            return mPoint.x >= left && mPoint.x < right && mPoint.y >= top && mPoint.y < bottom;
        }

        bool overlaps(const bounds2<T>& mQuad) const
        {
            auto range_overlaps = [](T mMin1, T mMax1, T mMin2, T mMax2)
            {
                return (mMin1 >= mMin2 && mMin1 < mMax2) || (mMin2 >= mMin1 && mMin2 < mMax1);
            };

            return range_overlaps(left, right, mQuad.left, mQuad.right) &&
                   range_overlaps(top, bottom, mQuad.top, mQuad.bottom);
        }

        bool operator == (const bounds2<T>& mQuad) const
        {
            return left == mQuad.left && right == mQuad.right &&
                   top == mQuad.top && bottom == mQuad.bottom;
        }

        bool operator != (const bounds2<T>& mQuad) const
        {
            return left != mQuad.left || right != mQuad.right ||
                   top != mQuad.top || bottom != mQuad.bottom;
        }

        static const bounds2 ZERO;

        T left = 0, right = 0, top = 0, bottom = 0;
    };

    template<typename T>
    bounds2<T> operator + (const vector2<T>& mOffset, const bounds2<T>& mQuad) noexcept
    {
        bounds2<T> mTmp = mQuad;
        mTmp.left += mOffset.x;
        mTmp.right += mOffset.x;
        mTmp.top += mOffset.y;
        mTmp.bottom += mOffset.y;
        return mTmp;
    }

    template<typename T>
    bounds2<T> operator + (const bounds2<T>& mQuad, const vector2<T>& mOffset) noexcept
    {
        bounds2<T> mTmp = mQuad;
        mTmp.left += mOffset.x;
        mTmp.right += mOffset.x;
        mTmp.top += mOffset.y;
        mTmp.bottom += mOffset.y;
        return mTmp;
    }

    template<typename T>
    bounds2<T> operator - (const bounds2<T>& mQuad, const vector2<T>& mOffset) noexcept
    {
        bounds2<T> mTmp = mQuad;
        mTmp.left -= mOffset.x;
        mTmp.right -= mOffset.x;
        mTmp.top -= mOffset.y;
        mTmp.bottom -= mOffset.y;
        return mTmp;
    }

    template<typename T>
    bounds2<T> operator * (const bounds2<T>& mQuad, T fScale) noexcept
    {
        bounds2<T> mTmp = mQuad;
        mTmp.left *= fScale;
        mTmp.right *= fScale;
        mTmp.top *= fScale;
        mTmp.bottom *= fScale;
        return mTmp;
    }

    template<typename T>
    bounds2<T> operator * (T fScale, const bounds2<T>& mQuad) noexcept
    {
        bounds2<T> mTmp = mQuad;
        mTmp.left *= fScale;
        mTmp.right *= fScale;
        mTmp.top *= fScale;
        mTmp.bottom *= fScale;
        return mTmp;
    }

    template<typename T>
    bounds2<T> operator / (const bounds2<T>& mQuad, T fScale) noexcept
    {
        bounds2<T> mTmp = mQuad;
        mTmp.left /= fScale;
        mTmp.right /= fScale;
        mTmp.top /= fScale;
        mTmp.bottom /= fScale;
        return mTmp;
    }

    template<typename T>
    const bounds2<T> bounds2<T>::ZERO(0, 0, 0, 0);

    using bounds2f = bounds2<float>;
    using bounds2i = bounds2<int>;

    template<typename O, typename T>
    O& operator << (O& mStream, const bounds2<T>& mQuad)
    {
        return mStream << "(" << mQuad.top_left() << "), (" << mQuad.bottom_right() << ")";
    }
}
}

#endif
