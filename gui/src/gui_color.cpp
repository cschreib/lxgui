#include "lxgui/gui_color.hpp"
#include <lxgui/utils.hpp>
#include <lxgui/utils_string.hpp>
#include <iostream>
#include <sstream>

namespace gui
{
const color color::EMPTY(0.0f, 0.0f, 0.0f, 0.0f);
const color color::WHITE(1.0f, 1.0f, 1.0f);
const color color::BLACK(0.0f, 0.0f, 0.0f);
const color color::RED  (1.0f, 0.0f, 0.0f);
const color color::GREEN(0.0f, 1.0f, 0.0f);
const color color::BLUE (0.0f, 0.0f, 1.0f);
const color color::GREY (0.5f, 0.5f, 0.5f);

color::color()
{
}

color::color(chanel nr, chanel ng, chanel nb, chanel na) :
    r(nr), g(ng), b(nb), a(na)
{
}

color::color(const std::string& s)
{
    std::istringstream ss(s); ss >> *this;
}

color::hls color::to_hls() const
{
    float ma = std::max(std::max(r, g), b);
    float mi = std::min(std::min(r, g), b);

    hls hls;
    hls.a = a;

    if (ma == mi)
    {
        hls.l = ma;
        hls.s = 0.0f;
    }
    else
    {
        float delta = ma - mi;
        float sum   = ma + mi;

        hls.l = 0.5f*sum;
        if (hls.l < 0.5f)
            hls.s = delta/sum;
        else
            hls.s = delta/(2.0f - sum);

        if      (ma == r) hls.h = 60.0f*(g - b)/delta +   0.0f;
        else if (ma == g) hls.h = 60.0f*(b - r)/delta + 120.0f;
        else              hls.h = 60.0f*(r - g)/delta + 240.0f;

        if      (hls.h < 0.0f)   hls.h = hls.h + 360.0f;
        else if (hls.h > 360.0f) hls.h = hls.h - 360.0f;
    }

    return hls;
}

float h2_to_rgb(float v1, float v2, float h)
{
    if      (h < 0.0f)   h = h + 360.0f;
    else if (h > 360.0f) h = h - 360.0f;

    if      (h < 60.0f)  return v1 + (v2 - v1)*h/60.0f;
    else if (h < 180.0f) return v2;
    else if (h < 240.0f) return v1 + (v2 - v1)*(4.0f - h/60.0f);
    else                 return v1;
}

void color::from_hls(const hls& hls)
{
    a = hls.a;

    if (hls.s == 0.0f)
    {
        r = hls.l; g = hls.l; b = hls.l;
    }
    else
    {
        float v2;
        if (hls.l < 0.5f)
            v2 = hls.l*(1.0f + hls.s);
        else
            v2 = hls.l + hls.s - hls.l*hls.s;

        float v1 = 2.0f*hls.l - v2;

        r = h2_to_rgb(v1, v2, hls.h + 120.0f);
        g = h2_to_rgb(v1, v2, hls.h);
        b = h2_to_rgb(v1, v2, hls.h - 120.0f);
    }
}

color color::luminosity(float f) const
{
    hls hls = to_hls();
    hls.l = f;
    color c; c.from_hls(hls);
    return c;
}

color color::saturation(float f) const
{
    hls hls = to_hls();
    hls.s = f;
    color c; c.from_hls(hls);
    return c;
}

color color::hue(float f) const
{
    hls hls = to_hls();
    hls.h = f;
    color c; c.from_hls(hls);
    return c;
}

bool color::operator == (const color& c) const
{
    return (r == c.r && g == c.g && b == c.b && a == c.a);
}

bool color::operator != (const color& c) const
{
    return (r != c.r || g != c.g || b != c.b || a != c.a);
}

void color::operator += (const color& c)
{
    r += c.r; g += c.g; b += c.b; a += c.a;
}

void color::operator -= (const color& c)
{
    r -= c.r; g -= c.g; b -= c.b; a -= c.a;
}

void color::operator *= (const color& c)
{
    r *= c.r; g *= c.g; b *= c.b; a *= c.a;
}

void color::operator *= (float f)
{
    r *= f; g *= f; b *= f; a *= f;
}

color operator + (const color& c1, const color& c2)
{
    return color(c1.r+c2.r, c1.g+c2.g, c1.b+c2.b, c1.a+c2.a);
}

color operator - (const color& c1, const color& c2)
{
    return color(c1.r-c2.r, c1.g-c2.g, c1.b-c2.b, c1.a-c2.a);
}

color operator * (const color& c1, const color& c2)
{
    return color(c1.r*c2.r, c1.g*c2.g, c1.b*c2.b, c1.a*c2.a);
}

color operator * (const color& c1, float f)
{
    return color(c1.r*f, c1.g*f, c1.b*f, c1.a);
}

color operator * (float f, const color& c2)
{
    return color(f*c2.r, f*c2.g, f*c2.b, c2.a);
}

std::ostream& operator << (std::ostream& mStream, const color& mColor)
{
    return mStream << (uint)255*mColor.r << ", " << (uint)255*mColor.g << ", "
                   << (uint)255*mColor.b << ", " << (uint)255*mColor.a;
}

std::istream& operator >> (std::istream& mStream, color& mColor)
{
    auto pos = mStream.tellg();
    char c; mStream >> c;
    if (c == '#')
    {
        char h[3]; h[2] = '\0';
        mStream >> h[0] >> h[1];
        mColor.r = utils::hex_to_uint(h)/255.0f;
        mStream >> h[0] >> h[1];
        mColor.g = utils::hex_to_uint(h)/255.0f;
        mStream >> h[0] >> h[1];
        mColor.b = utils::hex_to_uint(h)/255.0f;

        pos = mStream.tellg();
        if (!mStream.eof())
        {
            mStream >> h[0];
            if (isalnum(h[0]) && !mStream.eof())
            {
                mStream >> h[1];
                if (isalnum(h[1]))
                {
                    mColor.a = utils::hex_to_uint(h)/255.0f;
                    return mStream;
                }
            }
        }

        mStream.seekg(pos);
        mColor.a = 1.0f;
    }
    else
    {
        mStream.seekg(pos);
        char delim;
        mStream >> mColor.r >> delim >> mColor.g >> delim >> mColor.b >> delim >> mColor.a;
        mColor *= 1.0f/255.0f;
    }

    return mStream;
}
}
