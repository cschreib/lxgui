#ifndef LXGUI_GUI_BOUNDS2_HPP
#define LXGUI_GUI_BOUNDS2_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/lxgui.hpp"

namespace lxgui::gui {

template<typename T>
struct bounds2 {
    constexpr bounds2() = default;

    constexpr bounds2(T m_left, T m_right, T m_top, T m_bottom) noexcept :
        left(m_left), right(m_right), top(m_top), bottom(m_bottom) {}

    constexpr bounds2(const vector2<T>& m_center, const vector2<T>& m_size) noexcept :
        left(m_center.x - m_size.x / 2),
        right(left + m_size.x),
        top(m_center.y - m_size.y / 2),
        bottom(top + m_size.y) {}

    void set(T m_left, T m_right, T m_top, T m_bottom) noexcept {
        left   = m_left;
        right  = m_right;
        top    = m_top;
        bottom = m_bottom;
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

    bool contains(const vector2<T>& m_point) const {
        return m_point.x >= left && m_point.x < right && m_point.y >= top && m_point.y < bottom;
    }

    bool overlaps(const bounds2<T>& m_quad) const {
        auto range_overlaps = [](T m_min1, T m_max1, T m_min2, T m_max2) {
            return (m_min1 >= m_min2 && m_min1 < m_max2) || (m_min2 >= m_min1 && m_min2 < m_max1);
        };

        return range_overlaps(left, right, m_quad.left, m_quad.right) &&
               range_overlaps(top, bottom, m_quad.top, m_quad.bottom);
    }

    bool operator==(const bounds2<T>& m_quad) const {
        return left == m_quad.left && right == m_quad.right && top == m_quad.top &&
               bottom == m_quad.bottom;
    }

    bool operator!=(const bounds2<T>& m_quad) const {
        return left != m_quad.left || right != m_quad.right || top != m_quad.top ||
               bottom != m_quad.bottom;
    }

    static const bounds2 zero;

    T left = 0, right = 0, top = 0, bottom = 0;
};

template<typename T>
bounds2<T> operator+(const vector2<T>& m_offset, const bounds2<T>& m_quad) noexcept {
    bounds2<T> m_tmp = m_quad;
    m_tmp.left += m_offset.x;
    m_tmp.right += m_offset.x;
    m_tmp.top += m_offset.y;
    m_tmp.bottom += m_offset.y;
    return m_tmp;
}

template<typename T>
bounds2<T> operator+(const bounds2<T>& m_quad, const vector2<T>& m_offset) noexcept {
    bounds2<T> m_tmp = m_quad;
    m_tmp.left += m_offset.x;
    m_tmp.right += m_offset.x;
    m_tmp.top += m_offset.y;
    m_tmp.bottom += m_offset.y;
    return m_tmp;
}

template<typename T>
bounds2<T> operator-(const bounds2<T>& m_quad, const vector2<T>& m_offset) noexcept {
    bounds2<T> m_tmp = m_quad;
    m_tmp.left -= m_offset.x;
    m_tmp.right -= m_offset.x;
    m_tmp.top -= m_offset.y;
    m_tmp.bottom -= m_offset.y;
    return m_tmp;
}

template<typename T>
bounds2<T> operator*(const bounds2<T>& m_quad, T scale) noexcept {
    bounds2<T> m_tmp = m_quad;
    m_tmp.left *= scale;
    m_tmp.right *= scale;
    m_tmp.top *= scale;
    m_tmp.bottom *= scale;
    return m_tmp;
}

template<typename T>
bounds2<T> operator*(T scale, const bounds2<T>& m_quad) noexcept {
    bounds2<T> m_tmp = m_quad;
    m_tmp.left *= scale;
    m_tmp.right *= scale;
    m_tmp.top *= scale;
    m_tmp.bottom *= scale;
    return m_tmp;
}

template<typename T>
bounds2<T> operator/(const bounds2<T>& m_quad, T scale) noexcept {
    bounds2<T> m_tmp = m_quad;
    m_tmp.left /= scale;
    m_tmp.right /= scale;
    m_tmp.top /= scale;
    m_tmp.bottom /= scale;
    return m_tmp;
}

template<typename T>
const bounds2<T> bounds2<T>::zero(0, 0, 0, 0);

using bounds2f = bounds2<float>;
using bounds2i = bounds2<int>;

template<typename O, typename T>
O& operator<<(O& m_stream, const bounds2<T>& m_quad) {
    return m_stream << "(" << m_quad.top_left() << "), (" << m_quad.bottom_right() << ")";
}

} // namespace lxgui::gui

#endif
