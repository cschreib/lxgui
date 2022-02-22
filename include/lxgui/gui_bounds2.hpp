#ifndef LXGUI_GUI_BOUNDS2_HPP
#define LXGUI_GUI_BOUNDS2_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/lxgui.hpp"

namespace lxgui::gui {

/**
 * \brief Holds 2D bounds of a region.
 * \details The bounds are stored as left, right, top, and bottom coordinates.
 */
template<typename T>
struct bounds2 {
    constexpr bounds2() = default;

    constexpr bounds2(T l, T r, T t, T b) noexcept : left(l), right(r), top(t), bottom(b) {}

    constexpr bounds2(const vector2<T>& center, const vector2<T>& size) noexcept :
        left(center.x - size.x / 2),
        right(left + size.x),
        top(center.y - size.y / 2),
        bottom(top + size.y) {}

    void set(T l, T r, T t, T b) noexcept {
        left   = l;
        right  = r;
        top    = t;
        bottom = b;
    }

    vector2<T> top_left() const noexcept {
        return vector2<T>(left, top);
    }

    vector2<T> top_right() const noexcept {
        return vector2<T>(right, top);
    }

    vector2<T> bottom_right() const noexcept {
        return vector2<T>(right, bottom);
    }

    vector2<T> bottom_left() const noexcept {
        return vector2<T>(left, bottom);
    }

    vector2<T> center() const noexcept {
        return vector2<T>((left + right) / 2, (top + bottom) / 2);
    }

    T width() const noexcept {
        return right - left;
    }

    T height() const noexcept {
        return bottom - top;
    }

    vector2<T> dimensions() const noexcept {
        return vector2<T>(width(), height());
    }

    bool contains(const vector2<T>& point) const {
        return point.x >= left && point.x < right && point.y >= top && point.y < bottom;
    }

    bool overlaps(const bounds2<T>& quad) const {
        auto range_overlaps = [](T min1, T max1, T min2, T max2) {
            return (min1 >= min2 && min1 < max2) || (min2 >= min1 && min2 < max1);
        };

        return range_overlaps(left, right, quad.left, quad.right) &&
               range_overlaps(top, bottom, quad.top, quad.bottom);
    }

    bool operator==(const bounds2<T>& quad) const {
        return left == quad.left && right == quad.right && top == quad.top && bottom == quad.bottom;
    }

    bool operator!=(const bounds2<T>& quad) const {
        return left != quad.left || right != quad.right || top != quad.top || bottom != quad.bottom;
    }

    static const bounds2 zero;

    T left = 0, right = 0, top = 0, bottom = 0;
};

template<typename T>
bounds2<T> operator+(const vector2<T>& offset, const bounds2<T>& quad) noexcept {
    bounds2<T> tmp = quad;
    tmp.left += offset.x;
    tmp.right += offset.x;
    tmp.top += offset.y;
    tmp.bottom += offset.y;
    return tmp;
}

template<typename T>
bounds2<T> operator+(const bounds2<T>& quad, const vector2<T>& offset) noexcept {
    bounds2<T> tmp = quad;
    tmp.left += offset.x;
    tmp.right += offset.x;
    tmp.top += offset.y;
    tmp.bottom += offset.y;
    return tmp;
}

template<typename T>
bounds2<T> operator-(const bounds2<T>& quad, const vector2<T>& offset) noexcept {
    bounds2<T> tmp = quad;
    tmp.left -= offset.x;
    tmp.right -= offset.x;
    tmp.top -= offset.y;
    tmp.bottom -= offset.y;
    return tmp;
}

template<typename T>
bounds2<T> operator*(const bounds2<T>& quad, T scale) noexcept {
    bounds2<T> tmp = quad;
    tmp.left *= scale;
    tmp.right *= scale;
    tmp.top *= scale;
    tmp.bottom *= scale;
    return tmp;
}

template<typename T>
bounds2<T> operator*(T scale, const bounds2<T>& quad) noexcept {
    bounds2<T> tmp = quad;
    tmp.left *= scale;
    tmp.right *= scale;
    tmp.top *= scale;
    tmp.bottom *= scale;
    return tmp;
}

template<typename T>
bounds2<T> operator/(const bounds2<T>& quad, T scale) noexcept {
    bounds2<T> tmp = quad;
    tmp.left /= scale;
    tmp.right /= scale;
    tmp.top /= scale;
    tmp.bottom /= scale;
    return tmp;
}

template<typename T>
const bounds2<T> bounds2<T>::zero(0, 0, 0, 0);

/// Holds 2D bounds of a region (as floats).
using bounds2f = bounds2<float>;
/// Holds 2D bounds of a region (as signed integers).
using bounds2i = bounds2<int>;

template<typename O, typename T>
O& operator<<(O& stream, const bounds2<T>& quad) {
    return stream << "(" << quad.top_left() << "), (" << quad.bottom_right() << ")";
}

} // namespace lxgui::gui

#endif
