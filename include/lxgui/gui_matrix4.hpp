#ifndef LXGUI_GUI_MATRIX4_HPP
#define LXGUI_GUI_MATRIX4_HPP

#include <lxgui/gui_vector2.hpp>
#include <initializer_list>
#include <iosfwd>

namespace lxgui {
namespace gui
{
    struct matrix4f
    {
        using element_type = float;

        matrix4f() = default;
        matrix4f(std::initializer_list<element_type> mList);
        explicit matrix4f(element_type* mat);

        element_type& operator () (uint row, uint col)
        {
            return data[col+row*4];
        }

        element_type operator () (uint row, uint col) const
        {
            return data[col+row*4];
        }

        element_type& operator () (uint i)
        {
            return data[i];
        }

        element_type operator () (uint i) const
        {
            return data[i];
        }

        void transpose();
        void invert();

        static matrix4f translation(const vector2f& dx);
        static matrix4f scaling(const vector2f& scale);
        static matrix4f rotation(float rot);
        static matrix4f transformation(const vector2f& dx, const vector2f& scale, float rot);
        static matrix4f view(const vector2f& window);
        static matrix4f view(const vector2f& window, const vector2f& center);

        static matrix4f transpose(const matrix4f& m);
        static matrix4f invert(const matrix4f& m);

        element_type data[16];

        static const matrix4f IDENTITY;
    };

    matrix4f operator + (const matrix4f& m1, const matrix4f& m2);
    matrix4f operator - (const matrix4f& m1, const matrix4f& m2);
    matrix4f operator * (const matrix4f& m1, const matrix4f& m2);
    vector2f operator * (const matrix4f& m,  const vector2f& v);
    vector2f operator * (const vector2f& v,  const matrix4f& m);

    std::ostream& operator << (std::ostream& o, const matrix4f& m);
}
}

#endif
