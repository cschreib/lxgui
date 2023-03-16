#include "lxgui/gui_color.hpp"

#include "lxgui/utils.hpp"
#include "lxgui/utils_string.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

namespace lxgui::gui {

const color color::empty(0.0f, 0.0f, 0.0f, 0.0f);
const color color::white(1.0f, 1.0f, 1.0f);
const color color::black(0.0f, 0.0f, 0.0f);
const color color::red(1.0f, 0.0f, 0.0f);
const color color::green(0.0f, 1.0f, 0.0f);
const color color::blue(0.0f, 0.0f, 1.0f);
const color color::grey(0.5f, 0.5f, 0.5f);

color::color(const std::string& s) {
    std::istringstream ss(s);
    ss >> *this;
}

color::hls color::to_hls() const noexcept {
    float ma = std::max(std::max(r, g), b);
    float mi = std::min(std::min(r, g), b);

    color::hls output{};
    output.a = a;

    if (ma == mi) {
        output.l = ma;
        output.s = 0.0f;
    } else {
        float delta = ma - mi;
        float sum   = ma + mi;

        output.l = 0.5f * sum;
        if (output.l < 0.5f)
            output.s = delta / sum;
        else
            output.s = delta / (2.0f - sum);

        if (ma == r)
            output.h = 60.0f * (g - b) / delta + 0.0f;
        else if (ma == g)
            output.h = 60.0f * (b - r) / delta + 120.0f;
        else
            output.h = 60.0f * (r - g) / delta + 240.0f;

        if (output.h < 0.0f)
            output.h = output.h + 360.0f;
        else if (output.h > 360.0f)
            output.h = output.h - 360.0f;
    }

    return output;
}

color::hsv color::to_hsv() const noexcept {
    color::hsv output{};

    float cmax;
    float cmin;
    if (r > g && r > b) {
        cmax     = r;
        cmin     = g > b ? g : b;
        output.h = 60.0f * std::fmod((g - b) / (cmax - cmin), 6.0f);
    } else if (g > r && g > b) {
        cmax     = g;
        cmin     = r > b ? r : b;
        output.h = 60.0f * ((b - r) / (cmax - cmin) + 2.0f);
    } else if (b > r && b > g) {
        cmax     = b;
        cmin     = r > g ? r : g;
        output.h = 60.0f * ((r - g) / (cmax - cmin) + 4.0f);
    } else {
        output.h = 0.0f;
        output.s = 0.0f;
        output.v = r;
        return output;
    }

    output.s = (cmax - cmin) / cmax;
    output.v = cmax;

    return output;
}

float h2_to_rgb(float v1, float v2, float h) noexcept {
    if (h < 0.0f)
        h = h + 360.0f;
    else if (h > 360.0f)
        h = h - 360.0f;

    if (h < 60.0f)
        return v1 + (v2 - v1) * h / 60.0f;
    else if (h < 180.0f)
        return v2;
    else if (h < 240.0f)
        return v1 + (v2 - v1) * (4.0f - h / 60.0f);
    else
        return v1;
}

color color::from_hls(const hls& hls) noexcept {
    color c;
    c.a = hls.a;

    if (hls.s == 0.0f) {
        c.r = hls.l;
        c.g = hls.l;
        c.b = hls.l;
    } else {
        float v2;
        if (hls.l < 0.5f)
            v2 = hls.l * (1.0f + hls.s);
        else
            v2 = hls.l + hls.s - hls.l * hls.s;

        float v1 = 2.0f * hls.l - v2;

        c.r = h2_to_rgb(v1, v2, hls.h + 120.0f);
        c.g = h2_to_rgb(v1, v2, hls.h);
        c.b = h2_to_rgb(v1, v2, hls.h - 120.0f);
    }

    return c;
}

color color::from_hsv(const hsv& hsv) noexcept {
    const channel t = hsv.s * hsv.v;
    const channel x = t * (1.0f - std::abs(std::fmod(hsv.h / 60.0f, 2.0f) - 1.0f));
    const channel m = hsv.v - t;

    const channel low = std::clamp(m, 0.0f, 1.0f);
    const channel mid = std::clamp(x + m, 0.0f, 1.0f);
    const channel hig = std::clamp(t + m, 0.0f, 1.0f);

    if (hsv.h < 60.0f)
        return color(hig, mid, low, hsv.a);
    else if (hsv.h < 120.0f)
        return color(mid, hig, low, hsv.a);
    else if (hsv.h < 180.0f)
        return color(low, hig, mid, hsv.a);
    else if (hsv.h < 240.0f)
        return color(low, mid, hig, hsv.a);
    else if (hsv.h < 300.0f)
        return color(mid, low, hig, hsv.a);
    else
        return color(hig, low, mid, hsv.a);
}

bool color::operator==(const color& c) const noexcept {
    return (r == c.r && g == c.g && b == c.b && a == c.a);
}

bool color::operator!=(const color& c) const noexcept {
    return (r != c.r || g != c.g || b != c.b || a != c.a);
}

void color::operator+=(const color& c) noexcept {
    r += c.r;
    g += c.g;
    b += c.b;
    a += c.a;
}

void color::operator-=(const color& c) noexcept {
    r -= c.r;
    g -= c.g;
    b -= c.b;
    a -= c.a;
}

void color::operator*=(const color& c) noexcept {
    r *= c.r;
    g *= c.g;
    b *= c.b;
    a *= c.a;
}

void color::operator*=(float f) noexcept {
    r *= f;
    g *= f;
    b *= f;
    a *= f;
}

color operator+(const color& c1, const color& c2) noexcept {
    return color(c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a);
}

color operator-(const color& c1, const color& c2) noexcept {
    return color(c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, c1.a - c2.a);
}

color operator*(const color& c1, const color& c2) noexcept {
    return color(c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, c1.a * c2.a);
}

color operator*(const color& c1, float f) noexcept {
    return color(c1.r * f, c1.g * f, c1.b * f, c1.a);
}

color operator*(float f, const color& c2) noexcept {
    return color(f * c2.r, f * c2.g, f * c2.b, c2.a);
}

std::ostream& operator<<(std::ostream& stream, const color& c) {
    return stream << static_cast<std::size_t>(std::round(255.0f * c.r)) << ", "
                  << static_cast<std::size_t>(std::round(255.0f * c.g)) << ", "
                  << static_cast<std::size_t>(std::round(255.0f * c.b)) << ", "
                  << static_cast<std::size_t>(std::round(255.0f * c.a));
}

std::istream& operator>>(std::istream& stream, color& c) {
    auto pos = stream.tellg();
    char start_char;
    stream >> start_char;
    if (start_char == '#') {
        char h[3];
        h[2] = '\0';
        stream >> h[0] >> h[1];
        c.r = utils::hex_to_uint(h) / 255.0f;
        stream >> h[0] >> h[1];
        c.g = utils::hex_to_uint(h) / 255.0f;
        stream >> h[0] >> h[1];
        c.b = utils::hex_to_uint(h) / 255.0f;

        pos = stream.tellg();
        if (!stream.eof()) {
            stream >> h[0];
            if (std::isalnum(h[0]) != 0 && !stream.eof()) {
                stream >> h[1];
                if (std::isalnum(h[1]) != 0) {
                    c.a = utils::hex_to_uint(h) / 255.0f;
                    return stream;
                }
            }
        }

        stream.seekg(pos);
        c.a = 1.0f;
    } else {
        stream.seekg(pos);
        char delim;
        stream >> c.r >> delim >> c.g >> delim >> c.b >> delim >> c.a;
        c *= 1.0f / 255.0f;
    }

    return stream;
}

} // namespace lxgui::gui
