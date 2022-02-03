#ifndef LXGUI_GUI_MATRIX4_HPP
#define LXGUI_GUI_MATRIX4_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/gui_vector2.hpp"
#include <initializer_list>
#include <iosfwd>

namespace lxgui {
namespace gui
{
    struct matrix4f
    {
        using element_type = float;

        matrix4f() noexcept = default;
        matrix4f(std::initializer_list<element_type> mList) noexcept;
        explicit matrix4f(const element_type* mat) noexcept;

        element_type& operator () (std::size_t row, std::size_t col) noexcept
        {
            return data[col+row*4];
        }

        element_type operator () (std::size_t row, std::size_t col) const noexcept
        {
            return data[col+row*4];
        }

        element_type& operator () (std::size_t i) noexcept
        {
            return data[i];
        }

        element_type operator () (std::size_t i) const noexcept
        {
            return data[i];
        }

        void transpose() noexcept;
        void invert() noexcept;

        static matrix4f translation(const vector2f& dx) noexcept;
        static matrix4f scaling(const vector2f& scale) noexcept;
        static matrix4f rotation(float rot) noexcept;
        static matrix4f transformation(const vector2f& dx, const vector2f& scale, float rot) noexcept;
        static matrix4f view(const vector2f& window) noexcept;
        static matrix4f view(const vector2f& window, const vector2f& center) noexcept;

        static matrix4f transpose(const matrix4f& m) noexcept;
        static matrix4f invert(const matrix4f& m) noexcept;

        element_type data[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

        static const matrix4f IDENTITY;
    };

    matrix4f operator + (const matrix4f& m1, const matrix4f& m2) noexcept;
    matrix4f operator - (const matrix4f& m1, const matrix4f& m2) noexcept;
    matrix4f operator * (const matrix4f& m1, const matrix4f& m2) noexcept;
    vector2f operator * (const matrix4f& m,  const vector2f& v) noexcept;
    vector2f operator * (const vector2f& v,  const matrix4f& m) noexcept;

    std::ostream& operator << (std::ostream& o, const matrix4f& m);
}
}

#endif
