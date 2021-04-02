#ifndef LXGUI_GUI_QUAD2_HPP
#define LXGUI_GUI_QUAD2_HPP

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

        static const quad2 ZERO;

        T left = 0, right = 0, top = 0, bottom = 0;
    };

    template<typename T>
    quad2<T> operator + (const vector2<T>& mOffset, const quad2<T>& mQuad)
    {
        quad2<T> mTmp = mQuad;
        mTmp.left += mOffset.x;
        mTmp.right += mOffset.x;
        mTmp.top += mOffset.y;
        mTmp.bottom += mOffset.y;
        return mTmp;
    }

    template<typename T>
    quad2<T> operator + (const quad2<T>& mQuad, const vector2<T>& mOffset)
    {
        quad2<T> mTmp = mQuad;
        mTmp.left += mOffset.x;
        mTmp.right += mOffset.x;
        mTmp.top += mOffset.y;
        mTmp.bottom += mOffset.y;
        return mTmp;
    }

    template<typename T>
    quad2<T> operator - (const quad2<T>& mQuad, const vector2<T>& mOffset)
    {
        quad2<T> mTmp = mQuad;
        mTmp.left -= mOffset.x;
        mTmp.right -= mOffset.x;
        mTmp.top -= mOffset.y;
        mTmp.bottom -= mOffset.y;
        return mTmp;
    }

    template<typename T>
    const quad2<T> quad2<T>::ZERO(0, 0, 0, 0);

    using quad2f = quad2<float>;
    using quad2i = quad2<int>;
}
}

#endif
