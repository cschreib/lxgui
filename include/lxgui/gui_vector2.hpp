#ifndef LXGUI_GUI_VECTOR2_HPP
#define LXGUI_GUI_VECTOR2_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <cmath>
#include <type_traits>

namespace lxgui::gui {

template<typename T>
struct vector2 {
    using float_type = std::conditional_t<std::is_floating_point_v<T>, T, double>;

    constexpr vector2() = default;

    constexpr vector2(T m_x, T m_y) noexcept : x(m_x), y(m_y) {}

    template<typename U>
    explicit constexpr vector2(const vector2<U>& m_v) noexcept :
        x(static_cast<T>(m_v.x)), y(static_cast<T>(m_v.y)) {}

    void set(T m_x, T m_y) noexcept {
        x = m_x;
        y = m_y;
    }

    T get_norm_squared() const noexcept {
        return x * x + y * y;
    }

    float_type get_norm() const noexcept {
        return std::sqrt(static_cast<float_type>(get_norm_squared()));
    }

    vector2<float_type> get_unit() const noexcept {
        vector2<float_type> m_vec(static_cast<float_type>(x), static_cast<float_type>(y));
        const typename vector2<float_type>::float_type m_norm = get_norm();
        m_vec.x /= m_norm;
        m_vec.y /= m_norm;
        return m_vec;
    }

    vector2<float_type> get_normal() const noexcept {
        vector2<float_type> m_vec(static_cast<float_type>(-y), static_cast<float_type>(x));
        const typename vector2<float_type>::float_type m_norm = get_norm();
        m_vec.x /= m_norm;
        m_vec.y /= m_norm;
        return m_vec;
    }

    vector2 get_rotated(float_type m_angle) const noexcept {
        vector2<float_type> m_vec(static_cast<float_type>(x), static_cast<float_type>(y));
        vector2<float_type> m_orig = m_vec;

        const float_type ca = std::cos(m_angle);
        const float_type sa = std::sin(m_angle);

        m_vec.x = static_cast<T>(m_orig.x * ca - m_orig.y * sa);
        m_vec.y = static_cast<T>(m_orig.x * sa + m_orig.y * ca);

        return m_vec;
    }

    vector2 get_scaled(const vector2& v) const noexcept {
        vector2 m_vec = *this;
        m_vec.scale(v);
        return m_vec;
    }

    vector2 operator+(const vector2& v) const noexcept {
        return vector2(x + v.x, y + v.y);
    }

    vector2& operator+=(const vector2& v) noexcept {
        x += v.x;
        y += v.y;
        return *this;
    }

    vector2 operator-() const noexcept {
        return vector2(-x, -y);
    }

    vector2 operator-(const vector2& v) const noexcept {
        return vector2(x - v.x, y - v.y);
    }

    vector2& operator-=(const vector2& v) noexcept {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    bool operator==(const vector2& v) const noexcept {
        return (x == v.x) && (y == v.y);
    }

    bool operator!=(const vector2& v) const noexcept {
        return (x != v.x) || (y != v.y);
    }

    vector2 operator*(T m_value) const noexcept {
        return vector2(x * m_value, y * m_value);
    }

    vector2 operator*(const vector2& m_value) const noexcept {
        return vector2(x * m_value.x, y * m_value.y);
    }

    vector2& operator*=(T m_value) noexcept {
        x *= m_value;
        y *= m_value;
        return *this;
    }

    vector2& operator*=(const vector2& m_value) noexcept {
        x *= m_value.x;
        y *= m_value.y;
        return *this;
    }

    vector2 operator/(T m_value) const noexcept {
        return vector2(x / m_value, y / m_value);
    }

    vector2 operator/(const vector2& m_value) const noexcept {
        return vector2(x / m_value.x, y / m_value.y);
    }

    vector2& operator/=(T m_value) noexcept {
        x /= m_value;
        y /= m_value;
        return *this;
    }

    vector2& operator/=(const vector2& m_value) noexcept {
        x /= m_value.x;
        y /= m_value.y;
        return *this;
    }

    T dot(const vector2& v) const noexcept {
        return x * v.x + y * v.y;
    }

    static const vector2 zero;
    static const vector2 unit;
    static const vector2 unit_x;
    static const vector2 unit_y;

    T x = 0, y = 0;
};

template<typename T>
constexpr vector2<T> vector2<T>::zero(0, 0);

template<typename T>
constexpr vector2<T> vector2<T>::unit(1, 1);

template<typename T>
constexpr vector2<T> vector2<T>::unit_x(1, 0);

template<typename T>
constexpr vector2<T> vector2<T>::unit_y(0, 1);

using vector2f  = vector2<float>;
using vector2d  = vector2<double>;
using vector2i  = vector2<std::ptrdiff_t>;
using vector2ui = vector2<std::size_t>;

template<typename T>
vector2<T> operator*(T m_value, const vector2<T>& m_v) noexcept {
    return vector2<T>(m_v.x * m_value, m_v.y * m_value);
}

template<typename O, typename T>
O& operator<<(O& m_stream, const vector2<T>& m_v) {
    return m_stream << m_v.x << ", " << m_v.y;
}

} // namespace lxgui::gui

#endif
