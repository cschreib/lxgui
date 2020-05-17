#ifndef GUI_QUAD2_HPP
#define GUI_QUAD2_HPP

#include "lxgui/gui_vector2.hpp"

namespace lxgui {
namespace gui
{
    template<class T>
    class quad2
    {
    public :

        quad2() = default;

        quad2(T mLeft, T mRight, T mTop, T mBottom) : left(mLeft), right(mRight), top(mTop), bottom(mBottom)
        {
        }

        void set(T mLeft, T mRight, T mTop, T mBottom)
        {
            left = mLeft; right = mRight; top = mTop; bottom = mBottom;
        }

        vector2<T> top_left() const
        {
            return vector2<T>(left, top);
        }

        vector2<T> top_right() const
        {
            return vector2<T>(right, top);
        }

        vector2<T> bottom_right() const
        {
            return vector2<T>(right, bottom);
        }

        vector2<T> bottom_left() const
        {
            return vector2<T>(left, bottom);
        }

        vector2<T> center() const
        {
            return vector2<T>((left + right)/2, (top + bottom)/2);
        }

        T width() const
        {
            return right - left;
        }

        T height() const
        {
            return bottom - top;
        }

        static const quad2 ZERO;

        T left = 0, right = 0, top = 0, bottom = 0;
    };

    template<class T>
    quad2<T> operator + (const vector2<T>& mOffset, const quad2<T>& mQuad)
    {
        quad2<T> mTmp = mQuad;
        mTmp.left += mOffset.x;
        mTmp.right += mOffset.x;
        mTmp.top += mOffset.y;
        mTmp.bottom += mOffset.y;
        return mTmp;
    }

    template<class T>
    quad2<T> operator + (const quad2<T>& mQuad, const vector2<T>& mOffset)
    {
        quad2<T> mTmp = mQuad;
        mTmp.left += mOffset.x;
        mTmp.right += mOffset.x;
        mTmp.top += mOffset.y;
        mTmp.bottom += mOffset.y;
        return mTmp;
    }

    template<class T>
    quad2<T> operator - (const quad2<T>& mQuad, const vector2<T>& mOffset)
    {
        quad2<T> mTmp = mQuad;
        mTmp.left -= mOffset.x;
        mTmp.right -= mOffset.x;
        mTmp.top -= mOffset.y;
        mTmp.bottom -= mOffset.y;
        return mTmp;
    }

    template<class T>
    const quad2<T> quad2<T>::ZERO(0, 0, 0, 0);

    typedef quad2<float> quad2f;
    typedef quad2<int>   quad2i;
}
}

#endif
