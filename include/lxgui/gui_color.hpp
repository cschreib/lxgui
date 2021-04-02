#ifndef LXGUI_GUI_COLOR_HPP
#define LXGUI_GUI_COLOR_HPP

#include <iosfwd>
#include <string>

namespace lxgui {
namespace gui
{
    /// A simple color class (float RGBA)
    class color
    {
    public :

        using channel = float;

        struct hls
        {
            channel h, l, s, a;
        };

        constexpr color() = default;
        constexpr color(channel nr, channel ng, channel nb, channel na = 1.0f) noexcept :
            r(nr), g(ng), b(nb), a(na) {}
        explicit color(const std::string& s);

        hls  to_hls() const noexcept;
        void from_hls(const hls& hls) noexcept;

        color luminosity(float f) const noexcept;
        color saturation(float f) const noexcept;
        color hue(float f) const noexcept;

        bool operator == (const color& c) const noexcept;
        bool operator != (const color& c) const noexcept;

        void operator += (const color& c) noexcept;
        void operator -= (const color& c) noexcept;
        void operator *= (const color& c) noexcept;
        void operator *= (float f) noexcept;

        channel r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

        static const color EMPTY;
        static const color WHITE;
        static const color BLACK;
        static const color RED;
        static const color GREEN;
        static const color BLUE;
        static const color GREY;
    };

    color operator + (const color& c1, const color& c2) noexcept;
    color operator - (const color& c1, const color& c2) noexcept;
    color operator * (const color& c1, const color& c2) noexcept;
    color operator * (const color& c1, float f) noexcept;
    color operator * (float f, const color& c2) noexcept;

    std::ostream& operator << (std::ostream& mStream, const color& mColor);
    std::istream& operator >> (std::istream& mStream, color& mColor);
}
}

#endif
