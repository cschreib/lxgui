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

    constexpr vector2(T nx, T ny) noexcept : x(nx), y(ny) {}

    template<typename U>
    explicit constexpr vector2(const vector2<U>& v) noexcept :
        x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) {}

    void set(T nx, T ny) noexcept {
        x = nx;
        y = ny;
    }

    T get_norm_squared() const noexcept {
        return x * x + y * y;
    }

    float_type get_norm() const noexcept {
        return std::sqrt(static_cast<float_type>(get_norm_squared()));
    }

    vector2<float_type> get_unit() const noexcept {
        vector2<float_type> vec(static_cast<float_type>(x), static_cast<float_type>(y));
        const typename vector2<float_type>::float_type norm = get_norm();
        vec.x /= norm;
        vec.y /= norm;
        return vec;
    }

    vector2<float_type> get_normal() const noexcept {
        vector2<float_type> vec(static_cast<float_type>(-y), static_cast<float_type>(x));
        const typename vector2<float_type>::float_type norm = get_norm();
        vec.x /= norm;
        vec.y /= norm;
        return vec;
    }

    vector2 get_rotated(float_type angle) const noexcept {
        vector2<float_type> vec(static_cast<float_type>(x), static_cast<float_type>(y));
        vector2<float_type> orig = vec;

        const float_type ca = std::cos(angle);
        const float_type sa = std::sin(angle);

        vec.x = static_cast<T>(orig.x * ca - orig.y * sa);
        vec.y = static_cast<T>(orig.x * sa + orig.y * ca);

        return vec;
    }

    vector2 get_scaled(const vector2& v) const noexcept {
        vector2 vec = *this;
        vec.scale(v);
        return vec;
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

    vector2 operator*(T value) const noexcept {
        return vector2(x * value, y * value);
    }

    vector2 operator*(const vector2& value) const noexcept {
        return vector2(x * value.x, y * value.y);
    }

    vector2& operator*=(T value) noexcept {
        x *= value;
        y *= value;
        return *this;
    }

    vector2& operator*=(const vector2& value) noexcept {
        x *= value.x;
        y *= value.y;
        return *this;
    }

    vector2 operator/(T value) const noexcept {
        return vector2(x / value, y / value);
    }

    vector2 operator/(const vector2& value) const noexcept {
        return vector2(x / value.x, y / value.y);
    }

    vector2& operator/=(T value) noexcept {
        x /= value;
        y /= value;
        return *this;
    }

    vector2& operator/=(const vector2& value) noexcept {
        x /= value.x;
        y /= value.y;
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
vector2<T> operator*(T value, const vector2<T>& v) noexcept {
    return vector2<T>(v.x * value, v.y * value);
}

template<typename O, typename T>
O& operator<<(O& stream, const vector2<T>& v) {
    return stream << v.x << ", " << v.y;
}

} // namespace lxgui::gui

#endif
