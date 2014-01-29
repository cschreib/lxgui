#ifndef GUI_GL_MATRIX4_HPP
#define GUI_GL_MATRIX4_HPP

#include <lxgui/gui_vector2.hpp>
#include <initializer_list>
#include <iosfwd>

namespace gui {
namespace gl
{
    class matrix4
    {
    public :

        typedef float T;

        matrix4();
        matrix4(std::initializer_list<T> mList);
        matrix4(T* mat);

        inline T& operator () (size_t i, size_t j)
        {
            return data[j][i];
        }

        inline const T& operator () (size_t i, size_t j) const
        {
            return data[j][i];
        }

        inline T& operator () (size_t i)
        {
            return data[i/4][i%4];
        }

        inline const T& operator () (size_t i) const
        {
            return data[i/4][i%4];
        }

        void make_translation(const vector2f& dx);
        void make_scaling(const vector2f& scale);
        void make_rotation(float rot);
        void make_transformation(const vector2f& dx, const vector2f& scale, float rot);

        void transpose();
        void invert();

        static matrix4 translation(const vector2f& dx);
        static matrix4 scaling(const vector2f& scale);
        static matrix4 rotation(float rot);
        static matrix4 transformation(const vector2f& dx, const vector2f& scale, float rot);

        static matrix4 transpose(const matrix4& m);
        static matrix4 invert(const matrix4& m);

        matrix4  operator + (const matrix4& m);
        matrix4  operator - (const matrix4& m);
        matrix4  operator * (const matrix4& m);
        vector2f operator * (const vector2f& v);

        T data[4][4];

        static const matrix4 IDENTITY;
    };

    std::ostream& operator << (std::ostream& o, const matrix4& m);
}
}

#endif
