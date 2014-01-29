#include "lxgui/impl/gui_gl_matrix4.hpp"
#include <iostream>

namespace gui {
namespace gl
{
matrix4 build_identity()
{
    matrix4 mId;
    for (uint i = 0; i < 4; ++i)
    for (uint j = 0; j < 4; ++j)
        mId(i, j) = (i == j ? 1 : 0);

    return mId;
}

const matrix4 matrix4::IDENTITY = build_identity();

matrix4::matrix4()
{
}

matrix4::matrix4(std::initializer_list<T> mList)
{
    int i = 0;
    for (const T* p = mList.begin(); p != mList.end() && i < 16; ++p)
    {
        i = p-mList.begin();
        (*this)(i/4,i%4) = *p;
    }
}

matrix4::matrix4(T* mat)
{
    for (int i = 0; i < 16; ++i)
        (*this)(i/4,i%4) = mat[i];
}

void matrix4::make_translation(const vector2f& dx)
{
    (*this)(0,0) = 1.0f; (*this)(0,1) = 0.0f; (*this)(0,2) = 0.0f; (*this)(0,3) = dx.x;
    (*this)(1,0) = 0.0f; (*this)(1,1) = 1.0f; (*this)(1,2) = 0.0f; (*this)(1,3) = dx.y;
    (*this)(2,0) = 0.0f; (*this)(2,1) = 0.0f; (*this)(2,2) = 1.0f; (*this)(2,3) = 0.0f;
    (*this)(3,0) = 0.0f; (*this)(3,1) = 0.0f; (*this)(3,2) = 0.0f; (*this)(3,3) = 1.0f;
}

void matrix4::make_scaling(const vector2f& scale)
{
    (*this)(0,0) = scale.x; (*this)(0,1) = 0.0f;    (*this)(0,2) = 0.0f; (*this)(0,3) = 0.0f;
    (*this)(1,0) = 0.0f;    (*this)(1,1) = scale.y; (*this)(1,2) = 0.0f; (*this)(1,3) = 0.0f;
    (*this)(2,0) = 0.0f;    (*this)(2,1) = 0.0f;    (*this)(2,2) = 1.0f; (*this)(2,3) = 0.0f;
    (*this)(3,0) = 0.0f;    (*this)(3,1) = 0.0f;    (*this)(3,2) = 0.0f; (*this)(3,3) = 1.0f;
}

void matrix4::make_rotation(float rot)
{
    float co = cos(rot), si = sin(rot);

    (*this)(0,0) = co;   (*this)(0,1) = -si;  (*this)(0,2) = 0.0f; (*this)(0,3) = 0.0f;
    (*this)(1,0) = si;   (*this)(1,1) = co;   (*this)(1,2) = 0.0f; (*this)(1,3) = 0.0f;
    (*this)(2,0) = 0.0f; (*this)(2,1) = 0.0f; (*this)(2,2) = 1.0f; (*this)(2,3) = 0.0f;
    (*this)(3,0) = 0.0f; (*this)(3,1) = 0.0f; (*this)(3,2) = 0.0f; (*this)(3,3) = 1.0f;
}

void matrix4::make_transformation(const vector2f& dx, const vector2f& scale, float rot)
{
    float co = cos(rot), si = sin(rot);

    (*this)(0,0) = scale.x*co;   (*this)(0,1) = -scale.y*si; (*this)(0,2) = 0.0f; (*this)(0,3) = dx.x;
    (*this)(1,0) = scale.x*si;   (*this)(1,1) = scale.y*co;  (*this)(1,2) = 0.0f; (*this)(1,3) = dx.y;
    (*this)(2,0) = 0.0f;         (*this)(2,1) = 0.0f;        (*this)(2,2) = 1.0f; (*this)(2,3) = 0.0f;
    (*this)(3,0) = 0.0f;         (*this)(3,1) = 0.0f;        (*this)(3,2) = 0.0f; (*this)(3,3) = 1.0f;
}

void matrix4::transpose()
{
    T fTemp;
    for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
    {
        if (i != j)
        {
            fTemp = (*this)(i,j);
            (*this)(i,j) = (*this)(j,i);
            (*this)(j,i) = fTemp;
        }
    }
}

void matrix4::invert()
{
    *this = invert(*this);
}

matrix4 matrix4::translation(const vector2f& dx)
{
    matrix4 m; m.make_translation(dx);
    return m;
}

matrix4 matrix4::scaling(const vector2f& scale)
{
    matrix4 m; m.make_scaling(scale);
    return m;
}

matrix4 matrix4::rotation(float rot)
{
    matrix4 m; m.make_rotation(rot);
    return m;
}

matrix4 matrix4::transformation(const vector2f& dx, const vector2f& scale, float rot)
{
    matrix4 m; m.make_transformation(dx, scale, rot);
    return m;
}

matrix4 matrix4::transpose(const matrix4& m)
{
    float tmp[16] = {
        m(0,0), m(1,0), m(2,0), m(3,0),
        m(0,1), m(1,1), m(2,1), m(3,1),
        m(0,2), m(1,2), m(2,2), m(3,2),
        m(0,3), m(1,3), m(2,3), m(3,3)
    };

    return tmp;
}

matrix4 matrix4::invert(const matrix4& m)
{
    T m00 = m(0,0), m01 = m(0,1), m02 = m(0,2), m03 = m(0,3);
    T m10 = m(1,0), m11 = m(1,1), m12 = m(1,2), m13 = m(1,3);
    T m20 = m(2,0), m21 = m(2,1), m22 = m(2,2), m23 = m(2,3);
    T m30 = m(3,0), m31 = m(3,1), m32 = m(3,2), m33 = m(3,3);

    T v0 = m20*m31 - m21*m30;
    T v1 = m20*m32 - m22*m30;
    T v2 = m20*m33 - m23*m30;
    T v3 = m21*m32 - m22*m31;
    T v4 = m21*m33 - m23*m31;
    T v5 = m22*m33 - m23*m32;

    T t00 =  (v5*m11 - v4*m12 + v3*m13);
    T t10 = -(v5*m10 - v2*m12 + v1*m13);
    T t20 =  (v4*m10 - v2*m11 + v0*m13);
    T t30 = -(v3*m10 - v1*m11 + v0*m12);

    T invDet = 1.0f/(t00*m00 + t10*m01 + t20*m02 + t30*m03);

    T d00 = t00*invDet;
    T d10 = t10*invDet;
    T d20 = t20*invDet;
    T d30 = t30*invDet;

    T d01 = -(v5*m01 - v4*m02 + v3*m03)*invDet;
    T d11 =  (v5*m00 - v2*m02 + v1*m03)*invDet;
    T d21 = -(v4*m00 - v2*m01 + v0*m03)*invDet;
    T d31 =  (v3*m00 - v1*m01 + v0*m02)*invDet;

    v0 = m10*m31 - m11*m30;
    v1 = m10*m32 - m12*m30;
    v2 = m10*m33 - m13*m30;
    v3 = m11*m32 - m12*m31;
    v4 = m11*m33 - m13*m31;
    v5 = m12*m33 - m13*m32;

    T d02 =  (v5*m01 - v4*m02 + v3*m03)*invDet;
    T d12 = -(v5*m00 - v2*m02 + v1*m03)*invDet;
    T d22 =  (v4*m00 - v2*m01 + v0*m03)*invDet;
    T d32 = -(v3*m00 - v1*m01 + v0*m02)*invDet;

    v0 = m21*m10 - m20*m11;
    v1 = m22*m10 - m20*m12;
    v2 = m23*m10 - m20*m13;
    v3 = m22*m11 - m21*m12;
    v4 = m23*m11 - m21*m13;
    v5 = m23*m12 - m22*m13;

    T d03 = -(v5*m01 - v4*m02 + v3*m03)*invDet;
    T d13 =  (v5*m00 - v2*m02 + v1*m03)*invDet;
    T d23 = -(v4*m00 - v2*m01 + v0*m03)*invDet;
    T d33 =  (v3*m00 - v1*m01 + v0*m02)*invDet;

    float tmp[16] = {
        d00, d01, d02, d03,
        d10, d11, d12, d13,
        d20, d21, d22, d23,
        d30, d31, d32, d33
    };

    return tmp;
}

matrix4 matrix4::operator + (const matrix4& m)
{
    matrix4 r;

    r(0,0) = (*this)(0,0) + m(0,0);
    r(0,1) = (*this)(0,1) + m(0,1);
    r(0,2) = (*this)(0,2) + m(0,2);
    r(0,3) = (*this)(0,3) + m(0,3);

    r(1,0) = (*this)(1,0) + m(1,0);
    r(1,1) = (*this)(1,1) + m(1,1);
    r(1,2) = (*this)(1,2) + m(1,2);
    r(1,3) = (*this)(1,3) + m(1,3);

    r(2,0) = (*this)(2,0) + m(2,0);
    r(2,1) = (*this)(2,1) + m(2,1);
    r(2,2) = (*this)(2,2) + m(2,2);
    r(2,3) = (*this)(2,3) + m(2,3);

    r(3,0) = (*this)(3,0) + m(3,0);
    r(3,1) = (*this)(3,1) + m(3,1);
    r(3,2) = (*this)(3,2) + m(3,2);
    r(3,3) = (*this)(3,3) + m(3,3);

    return r;
}


matrix4 matrix4::operator - (const matrix4& m)
{
    matrix4 r;

    r(0,0) = (*this)(0,0) - m(0,0);
    r(0,1) = (*this)(0,1) - m(0,1);
    r(0,2) = (*this)(0,2) - m(0,2);
    r(0,3) = (*this)(0,3) - m(0,3);

    r(1,0) = (*this)(1,0) - m(1,0);
    r(1,1) = (*this)(1,1) - m(1,1);
    r(1,2) = (*this)(1,2) - m(1,2);
    r(1,3) = (*this)(1,3) - m(1,3);

    r(2,0) = (*this)(2,0) - m(2,0);
    r(2,1) = (*this)(2,1) - m(2,1);
    r(2,2) = (*this)(2,2) - m(2,2);
    r(2,3) = (*this)(2,3) - m(2,3);

    r(3,0) = (*this)(3,0) - m(3,0);
    r(3,1) = (*this)(3,1) - m(3,1);
    r(3,2) = (*this)(3,2) - m(3,2);
    r(3,3) = (*this)(3,3) - m(3,3);

    return r;
}

matrix4 matrix4::operator * (const matrix4& m)
{
    matrix4 r;

    r(0,0) = (*this)(0,0)*m(0,0) + (*this)(0,1)*m(1,0) + (*this)(0,2)*m(2,0) + (*this)(0,3)*m(3,0);
    r(0,1) = (*this)(0,0)*m(0,1) + (*this)(0,1)*m(1,1) + (*this)(0,2)*m(2,1) + (*this)(0,3)*m(3,1);
    r(0,2) = (*this)(0,0)*m(0,2) + (*this)(0,1)*m(1,2) + (*this)(0,2)*m(2,2) + (*this)(0,3)*m(3,2);
    r(0,3) = (*this)(0,0)*m(0,3) + (*this)(0,1)*m(1,3) + (*this)(0,2)*m(2,3) + (*this)(0,3)*m(3,3);

    r(1,0) = (*this)(1,0)*m(0,0) + (*this)(1,1)*m(1,0) + (*this)(1,2)*m(2,0) + (*this)(1,3)*m(3,0);
    r(1,1) = (*this)(1,0)*m(0,1) + (*this)(1,1)*m(1,1) + (*this)(1,2)*m(2,1) + (*this)(1,3)*m(3,1);
    r(1,2) = (*this)(1,0)*m(0,2) + (*this)(1,1)*m(1,2) + (*this)(1,2)*m(2,2) + (*this)(1,3)*m(3,2);
    r(1,3) = (*this)(1,0)*m(0,3) + (*this)(1,1)*m(1,3) + (*this)(1,2)*m(2,3) + (*this)(1,3)*m(3,3);

    r(2,0) = (*this)(2,0)*m(0,0) + (*this)(2,1)*m(1,0) + (*this)(2,2)*m(2,0) + (*this)(2,3)*m(3,0);
    r(2,1) = (*this)(2,0)*m(0,1) + (*this)(2,1)*m(1,1) + (*this)(2,2)*m(2,1) + (*this)(2,3)*m(3,1);
    r(2,2) = (*this)(2,0)*m(0,2) + (*this)(2,1)*m(1,2) + (*this)(2,2)*m(2,2) + (*this)(2,3)*m(3,2);
    r(2,3) = (*this)(2,0)*m(0,3) + (*this)(2,1)*m(1,3) + (*this)(2,2)*m(2,3) + (*this)(2,3)*m(3,3);

    r(3,0) = (*this)(3,0)*m(0,0) + (*this)(3,1)*m(1,0) + (*this)(3,2)*m(2,0) + (*this)(3,3)*m(3,0);
    r(3,1) = (*this)(3,0)*m(0,1) + (*this)(3,1)*m(1,1) + (*this)(3,2)*m(2,1) + (*this)(3,3)*m(3,1);
    r(3,2) = (*this)(3,0)*m(0,2) + (*this)(3,1)*m(1,2) + (*this)(3,2)*m(2,2) + (*this)(3,3)*m(3,2);
    r(3,3) = (*this)(3,0)*m(0,3) + (*this)(3,1)*m(1,3) + (*this)(3,2)*m(2,3) + (*this)(3,3)*m(3,3);

    return r;
}

vector2f matrix4::operator*(const vector2f& v)
{
    vector2f r;

    T fInvW = 1.0f/((*this)(3,0)*v.x + (*this)(3,1)*v.y + (*this)(3,3));

    r.x = ((*this)(0,0)*v.x + (*this)(0,1)*v.y + (*this)(0,3))*fInvW;
    r.y = ((*this)(1,0)*v.x + (*this)(1,1)*v.y + (*this)(1,3))*fInvW;

    return r;
}

std::ostream& operator << (std::ostream& o, const matrix4& m)
{
    return o << "(" << m(0,0) << ", " << m(0,1) << ", " << m(0,2) << ", " << m(0,3) << ")\n"
      << "(" << m(1,0) << ", " << m(1,1) << ", " << m(1,2) << ", " << m(1,3) << ")\n"
      << "(" << m(2,0) << ", " << m(2,1) << ", " << m(2,2) << ", " << m(2,3) << ")\n"
      << "(" << m(3,0) << ", " << m(3,1) << ", " << m(3,2) << ", " << m(3,3) << ")\n";
}
}
}
