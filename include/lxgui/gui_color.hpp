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

        typedef float chanel;

        struct hls
        {
            chanel h, l, s, a;
        };

        color() = default;
        color(chanel nr, chanel ng, chanel nb, chanel na = 1.0f);
        explicit color(const std::string& s);

        hls  to_hls() const;
        void from_hls(const hls& hls);

        color luminosity(float f) const;
        color saturation(float f) const;
        color hue(float f) const;

        bool operator == (const color& c) const;
        bool operator != (const color& c) const;

        void operator += (const color& c);
        void operator -= (const color& c);
        void operator *= (const color& c);
        void operator *= (float f);

        chanel r, g, b, a;

        static const color EMPTY;
        static const color WHITE;
        static const color BLACK;
        static const color RED;
        static const color GREEN;
        static const color BLUE;
        static const color GREY;
    };

    color operator + (const color& c1, const color& c2);
    color operator - (const color& c1, const color& c2);
    color operator * (const color& c1, const color& c2);
    color operator * (const color& c1, float f);
    color operator * (float f, const color& c2);

    std::ostream& operator << (std::ostream& mStream, const color& mColor);
    std::istream& operator >> (std::istream& mStream, color& mColor);
}
}

#endif
