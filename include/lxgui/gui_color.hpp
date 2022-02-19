#ifndef LXGUI_GUI_COLOR_HPP
#define LXGUI_GUI_COLOR_HPP

#include "lxgui/lxgui.hpp"

#include <iosfwd>
#include <string>

namespace lxgui::gui {

/// A simple color class (float RGBA)
class color {
public:
    using channel = float;

    struct hls {
        channel h, l, s, a;
    };

    struct hsv {
        channel h, s, v, a;
    };

    constexpr color() = default;
    constexpr color(channel nr, channel ng, channel nb, channel na = 1.0f) noexcept :
        r(nr), g(ng), b(nb), a(na) {}
    explicit color(const std::string& s);

    hls to_hls() const noexcept;
    hsv to_hsv() const noexcept;

    bool operator==(const color& c) const noexcept;
    bool operator!=(const color& c) const noexcept;

    void operator+=(const color& c) noexcept;
    void operator-=(const color& c) noexcept;
    void operator*=(const color& c) noexcept;
    void operator*=(float f) noexcept;

    channel r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

    static const color empty;
    static const color white;
    static const color black;
    static const color red;
    static const color green;
    static const color blue;
    static const color grey;

    static constexpr color
    from_bytes(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255u) noexcept {
        return color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    static color from_hls(const hls& hls) noexcept;
    static color from_hsv(const hsv& hsv) noexcept;
};

color operator+(const color& c1, const color& c2) noexcept;
color operator-(const color& c1, const color& c2) noexcept;
color operator*(const color& c1, const color& c2) noexcept;
color operator*(const color& c1, float f) noexcept;
color operator*(float f, const color& c2) noexcept;

std::ostream& operator<<(std::ostream& stream, const color& c);
std::istream& operator>>(std::istream& stream, color& c);

} // namespace lxgui::gui

#endif
