#include "lxgui/gui_matrix4.hpp"
#include <iostream>

namespace lxgui {
namespace gui
{
matrix4f build_identity()
{
    matrix4f mId;
    for (uint row = 0u; row < 4u; ++row)
    for (uint col = 0u; col < 4u; ++col)
        mId(row, col) = (row == col ? 1.0f : 0.0f);

    return mId;
}

const matrix4f matrix4f::IDENTITY = build_identity();

matrix4f::matrix4f(std::initializer_list<element_type> mList)
{
    const uint length = std::min<std::size_t>(mList.size(), 16u);
    std::copy(mList.begin(), mList.begin() + length, data);
}

matrix4f::matrix4f(const element_type* mat)
{
    std::copy(mat, mat + 16u, data);
}

matrix4f matrix4f::translation(const vector2f& dx)
{
    return {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        dx.x, dx.y, 0.0f, 1.0f
    };
}

matrix4f matrix4f::scaling(const vector2f& scale)
{
    return {
        scale.x, 0.0f,    0.0f, 0.0f,
        0.0f,    scale.y, 0.0f, 0.0f,
        0.0f,    0.0f,    1.0f, 0.0f,
        0.0f,    0.0f,    0.0f, 1.0f
    };
}

matrix4f matrix4f::rotation(float rot)
{
    float co = cos(rot), si = sin(rot);

    return {
        co,   si,   0.0f, 0.0f,
        -si,  co,   0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

matrix4f matrix4f::transformation(const vector2f& dx, const vector2f& scale, float rot)
{
    float co = cos(rot), si = sin(rot);

    return {
        scale.x*co,   scale.y*si,   0.0f, 0.0f,
        -scale.x*si,  scale.y*co,   0.0f, 0.0f,
        0.0f,         0.0f,         1.0f, 0.0f,
        scale.x*dx.x, scale.y*dx.y, 0.0f, 1.0f
    };
}

matrix4f matrix4f::view(const vector2f& window)
{
    return {
        2.0f/window.x, 0.0f,          0.0f, 0.0f,
        0.0f,          2.0f/window.y, 0.0f, 0.0f,
        -1.0f,         -1.0f,         1.0f, 0.0f,
        -1.0f,         -1.0f,         0.0f, 1.0f
    };
}

matrix4f matrix4f::view(const vector2f& window, const vector2f& center)
{
    return translation(vector2f(window.x/2, window.y/2) - center)*view(window);
}

void matrix4f::transpose()
{
    for (uint row = 0; row < 4; ++row)
    for (uint col = 0; col < 4; ++col)
    {
        std::swap((*this)(row,col), (*this)(col,row));
    }
}

void matrix4f::invert()
{
    *this = invert(*this);
}

matrix4f matrix4f::transpose(const matrix4f& m)
{
    matrix4f n = m;
    n.transpose();
    return n;
}

matrix4f matrix4f::invert(const matrix4f& m)
{
    element_type m00 = m(0,0), m01 = m(0,1), m02 = m(0,2), m03 = m(0,3);
    element_type m10 = m(1,0), m11 = m(1,1), m12 = m(1,2), m13 = m(1,3);
    element_type m20 = m(2,0), m21 = m(2,1), m22 = m(2,2), m23 = m(2,3);
    element_type m30 = m(3,0), m31 = m(3,1), m32 = m(3,2), m33 = m(3,3);

    element_type v0 = m20*m31 - m21*m30;
    element_type v1 = m20*m32 - m22*m30;
    element_type v2 = m20*m33 - m23*m30;
    element_type v3 = m21*m32 - m22*m31;
    element_type v4 = m21*m33 - m23*m31;
    element_type v5 = m22*m33 - m23*m32;

    element_type t00 =  (v5*m11 - v4*m12 + v3*m13);
    element_type t10 = -(v5*m10 - v2*m12 + v1*m13);
    element_type t20 =  (v4*m10 - v2*m11 + v0*m13);
    element_type t30 = -(v3*m10 - v1*m11 + v0*m12);

    element_type invDet = 1.0f/(t00*m00 + t10*m01 + t20*m02 + t30*m03);

    element_type d00 = t00*invDet;
    element_type d10 = t10*invDet;
    element_type d20 = t20*invDet;
    element_type d30 = t30*invDet;

    element_type d01 = -(v5*m01 - v4*m02 + v3*m03)*invDet;
    element_type d11 =  (v5*m00 - v2*m02 + v1*m03)*invDet;
    element_type d21 = -(v4*m00 - v2*m01 + v0*m03)*invDet;
    element_type d31 =  (v3*m00 - v1*m01 + v0*m02)*invDet;

    v0 = m10*m31 - m11*m30;
    v1 = m10*m32 - m12*m30;
    v2 = m10*m33 - m13*m30;
    v3 = m11*m32 - m12*m31;
    v4 = m11*m33 - m13*m31;
    v5 = m12*m33 - m13*m32;

    element_type d02 =  (v5*m01 - v4*m02 + v3*m03)*invDet;
    element_type d12 = -(v5*m00 - v2*m02 + v1*m03)*invDet;
    element_type d22 =  (v4*m00 - v2*m01 + v0*m03)*invDet;
    element_type d32 = -(v3*m00 - v1*m01 + v0*m02)*invDet;

    v0 = m21*m10 - m20*m11;
    v1 = m22*m10 - m20*m12;
    v2 = m23*m10 - m20*m13;
    v3 = m22*m11 - m21*m12;
    v4 = m23*m11 - m21*m13;
    v5 = m23*m12 - m22*m13;

    element_type d03 = -(v5*m01 - v4*m02 + v3*m03)*invDet;
    element_type d13 =  (v5*m00 - v2*m02 + v1*m03)*invDet;
    element_type d23 = -(v4*m00 - v2*m01 + v0*m03)*invDet;
    element_type d33 =  (v3*m00 - v1*m01 + v0*m02)*invDet;

    return {
        d00, d01, d02, d03,
        d10, d11, d12, d13,
        d20, d21, d22, d23,
        d30, d31, d32, d33
    };
}

matrix4f operator + (const matrix4f& m1, const matrix4f& m2)
{
    matrix4f r;

    for (uint i = 0; i < 16; ++i)
        r.data[i] = m1.data[i] + m2.data[i];

    return r;
}


matrix4f operator - (const matrix4f& m1, const matrix4f& m2)
{
    matrix4f r;

    for (uint i = 0; i < 16; ++i)
        r.data[i] = m1.data[i] - m2.data[i];

    return r;
}

matrix4f operator * (const matrix4f& m1, const matrix4f& m2)
{
    matrix4f r;

    r(0,0) = m1(0,0)*m2(0,0) + m1(0,1)*m2(1,0) + m1(0,2)*m2(2,0) + m1(0,3)*m2(3,0);
    r(0,1) = m1(0,0)*m2(0,1) + m1(0,1)*m2(1,1) + m1(0,2)*m2(2,1) + m1(0,3)*m2(3,1);
    r(0,2) = m1(0,0)*m2(0,2) + m1(0,1)*m2(1,2) + m1(0,2)*m2(2,2) + m1(0,3)*m2(3,2);
    r(0,3) = m1(0,0)*m2(0,3) + m1(0,1)*m2(1,3) + m1(0,2)*m2(2,3) + m1(0,3)*m2(3,3);

    r(1,0) = m1(1,0)*m2(0,0) + m1(1,1)*m2(1,0) + m1(1,2)*m2(2,0) + m1(1,3)*m2(3,0);
    r(1,1) = m1(1,0)*m2(0,1) + m1(1,1)*m2(1,1) + m1(1,2)*m2(2,1) + m1(1,3)*m2(3,1);
    r(1,2) = m1(1,0)*m2(0,2) + m1(1,1)*m2(1,2) + m1(1,2)*m2(2,2) + m1(1,3)*m2(3,2);
    r(1,3) = m1(1,0)*m2(0,3) + m1(1,1)*m2(1,3) + m1(1,2)*m2(2,3) + m1(1,3)*m2(3,3);

    r(2,0) = m1(2,0)*m2(0,0) + m1(2,1)*m2(1,0) + m1(2,2)*m2(2,0) + m1(2,3)*m2(3,0);
    r(2,1) = m1(2,0)*m2(0,1) + m1(2,1)*m2(1,1) + m1(2,2)*m2(2,1) + m1(2,3)*m2(3,1);
    r(2,2) = m1(2,0)*m2(0,2) + m1(2,1)*m2(1,2) + m1(2,2)*m2(2,2) + m1(2,3)*m2(3,2);
    r(2,3) = m1(2,0)*m2(0,3) + m1(2,1)*m2(1,3) + m1(2,2)*m2(2,3) + m1(2,3)*m2(3,3);

    r(3,0) = m1(3,0)*m2(0,0) + m1(3,1)*m2(1,0) + m1(3,2)*m2(2,0) + m1(3,3)*m2(3,0);
    r(3,1) = m1(3,0)*m2(0,1) + m1(3,1)*m2(1,1) + m1(3,2)*m2(2,1) + m1(3,3)*m2(3,1);
    r(3,2) = m1(3,0)*m2(0,2) + m1(3,1)*m2(1,2) + m1(3,2)*m2(2,2) + m1(3,3)*m2(3,2);
    r(3,3) = m1(3,0)*m2(0,3) + m1(3,1)*m2(1,3) + m1(3,2)*m2(2,3) + m1(3,3)*m2(3,3);

    return r;
}

vector2f operator * (const matrix4f& m, const vector2f& v)
{
    vector2f r;

    const float fInvW = 1.0f/(m(3,0)*v.x + m(3,1)*v.y + m(3,3));

    r.x = (m(0,0)*v.x + m(0,1)*v.y + m(0,3))*fInvW;
    r.y = (m(1,0)*v.x + m(1,1)*v.y + m(1,3))*fInvW;

    return r;
}

vector2f operator * (const vector2f& v, const matrix4f& m)
{
    vector2f r;

    const float fInvW = 1.0f/(m(0,3)*v.x + m(1,3)*v.y + m(3,3));

    r.x = (m(0,0)*v.x + m(1,0)*v.y + m(3,0))*fInvW;
    r.y = (m(0,1)*v.x + m(1,1)*v.y + m(3,1))*fInvW;

    return r;
}

std::ostream& operator << (std::ostream& o, const matrix4f& m)
{
    return o << "(" << m(0,0) << ", " << m(0,1) << ", " << m(0,2) << ", " << m(0,3) << ")\n"
      << "(" << m(1,0) << ", " << m(1,1) << ", " << m(1,2) << ", " << m(1,3) << ")\n"
      << "(" << m(2,0) << ", " << m(2,1) << ", " << m(2,2) << ", " << m(2,3) << ")\n"
      << "(" << m(3,0) << ", " << m(3,1) << ", " << m(3,2) << ", " << m(3,3) << ")\n";
}
}
}
