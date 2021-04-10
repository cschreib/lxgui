#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_renderer.hpp"

namespace lxgui {
namespace gui
{

sprite::sprite(const renderer* pRenderer, std::shared_ptr<material> pMat) :
    pRenderer_(pRenderer), mHotSpot_(vector2f::ZERO)
{
    if (!pMat)
        throw gui::exception("gui::sprite", "Cannot construct a sprite from just a null material.");

    mQuad_.mat = pMat;
    fWidth_  = pMat->get_width();
    fHeight_ = pMat->get_height();

    mQuad_.v[0].pos = vector2f(0,         0);
    mQuad_.v[1].pos = vector2f(0+fWidth_, 0);
    mQuad_.v[2].pos = vector2f(0+fWidth_, 0+fHeight_);
    mQuad_.v[3].pos = vector2f(0,         0+fHeight_);

    float u = fWidth_/pMat->get_real_width();
    float v = fHeight_/pMat->get_real_height();

    mQuad_.v[0].uvs = vector2f(0, 0);
    mQuad_.v[1].uvs = vector2f(u, 0);
    mQuad_.v[2].uvs = vector2f(u, v);
    mQuad_.v[3].uvs = vector2f(0, v);
}

sprite::sprite(const renderer* pRenderer, std::shared_ptr<material> pMat, float fWidth, float fHeight) :
    pRenderer_(pRenderer), mHotSpot_(vector2f::ZERO)
{
    mQuad_.mat = pMat;
    fWidth_  = fWidth;
    fHeight_ = fHeight;

    mQuad_.v[0].pos = vector2f(0,         0);
    mQuad_.v[1].pos = vector2f(0+fWidth_, 0);
    mQuad_.v[2].pos = vector2f(0+fWidth_, 0+fHeight_);
    mQuad_.v[3].pos = vector2f(0,         0+fHeight_);

    float u = (pMat ? fWidth_/pMat->get_real_width() : 1.0f);
    float v = (pMat ? fHeight_/pMat->get_real_height() : 1.0f);

    mQuad_.v[0].uvs = vector2f(0, 0);
    mQuad_.v[1].uvs = vector2f(u, 0);
    mQuad_.v[2].uvs = vector2f(u, v);
    mQuad_.v[3].uvs = vector2f(0, v);
}

sprite::sprite(const renderer* pRenderer, std::shared_ptr<material> pMat, float fU, float fV, float fWidth, float fHeight) :
    pRenderer_(pRenderer), mHotSpot_(vector2f::ZERO)
{
    mQuad_.mat = pMat;
    fWidth_  = fWidth;
    fHeight_ = fHeight;

    mQuad_.v[0].pos = vector2f(0,         0);
    mQuad_.v[1].pos = vector2f(0+fWidth_, 0);
    mQuad_.v[2].pos = vector2f(0+fWidth_, 0+fHeight_);
    mQuad_.v[3].pos = vector2f(0,         0+fHeight_);

    float u1 = (pMat ? fU/pMat->get_real_width() : 0.0f);
    float v1 = (pMat ? fV/pMat->get_real_height() : 0.0f);
    float u2 = (pMat ? (fU + fWidth_)/pMat->get_real_width() : 1.0f);
    float v2 = (pMat ? (fV + fHeight_)/pMat->get_real_height() : 1.0f);

    mQuad_.v[0].uvs = vector2f(u1, v1);
    mQuad_.v[1].uvs = vector2f(u2, v1);
    mQuad_.v[2].uvs = vector2f(u2, v2);
    mQuad_.v[3].uvs = vector2f(u1, v2);
}

sprite::sprite(const renderer* pRenderer, const quad& mQuad) : pRenderer_(pRenderer), mQuad_(mQuad),
    mHotSpot_(vector2f::ZERO)
{
    fWidth_ = std::abs((mQuad_.v[2].pos - mQuad_.v[0].pos).x);
    fHeight_ = std::abs((mQuad_.v[2].pos - mQuad_.v[0].pos).y);
}

void sprite::render(float fX, float fY) const
{
    mQuad_.v[0].pos = vector2f(fX,         fY) - mHotSpot_;
    mQuad_.v[1].pos = vector2f(fX+fWidth_, fY) - mHotSpot_;
    mQuad_.v[2].pos = vector2f(fX+fWidth_, fY+fHeight_) - mHotSpot_;
    mQuad_.v[3].pos = vector2f(fX,         fY+fHeight_) - mHotSpot_;

    pRenderer_->render_quad(mQuad_);
}

void sprite::render_ex(float fX, float fY, float fRot, float fHScale, float fVScale) const
{
    float x1 = -mHotSpot_.x*fHScale;
    float x2 = (fWidth_ - mHotSpot_.x)*fHScale;
    float y1 = -mHotSpot_.y*fVScale;
    float y2 = (fHeight_ - mHotSpot_.y)*fVScale;

    if (fRot != 0.0f)
    {
        float cost = cos(fRot);
        float sint = sin(fRot);

        mQuad_.v[0].pos = vector2f(x1*cost - y1*sint + fX, x1*sint + y1*cost + fY);
        mQuad_.v[1].pos = vector2f(x2*cost - y1*sint + fX, x2*sint + y1*cost + fY);
        mQuad_.v[2].pos = vector2f(x2*cost - y2*sint + fX, x2*sint + y2*cost + fY);
        mQuad_.v[3].pos = vector2f(x1*cost - y2*sint + fX, x1*sint + y2*cost + fY);
    }
    else
    {
        mQuad_.v[0].pos = vector2f(x1 + fX, y1 + fY);
        mQuad_.v[1].pos = vector2f(x2 + fX, y1 + fY);
        mQuad_.v[2].pos = vector2f(x2 + fX, y2 + fY);
        mQuad_.v[3].pos = vector2f(x1 + fX, y2 + fY);
    }

    pRenderer_->render_quad(mQuad_);
}

void sprite::render_2v(float fX1, float fY1, float fX3, float fY3)
{
    mQuad_.v[0].pos = vector2f(fX1, fY1);
    mQuad_.v[1].pos = vector2f(fX3, fY1);
    mQuad_.v[2].pos = vector2f(fX3, fY3);
    mQuad_.v[3].pos = vector2f(fX1, fY3);

    pRenderer_->render_quad(mQuad_);
}

void sprite::render_4v(float fX1, float fY1,
                       float fX2, float fY2,
                       float fX3, float fY3,
                       float fX4, float fY4)
{
    mQuad_.v[0].pos = vector2f(fX1, fY1);
    mQuad_.v[1].pos = vector2f(fX2, fY2);
    mQuad_.v[2].pos = vector2f(fX3, fY3);
    mQuad_.v[3].pos = vector2f(fX4, fY4);

    pRenderer_->render_quad(mQuad_);
}

void sprite::render_static() const
{
    pRenderer_->render_quad(mQuad_);
}

void sprite::set_quad(const std::array<vertex,4>& lVertexArray)
{
    mQuad_.v = lVertexArray;
}

void sprite::set_color(const color& mColor, uint uiIndex)
{
    if (uiIndex != uint(-1))
        mQuad_.v[uiIndex].col = mColor;
    else
    {
        mQuad_.v[0].col = mColor;
        mQuad_.v[1].col = mColor;
        mQuad_.v[2].col = mColor;
        mQuad_.v[3].col = mColor;
    }
}

void sprite::set_desaturated(bool bDesaturated)
{
    // Not implemented
}

void sprite::set_blend_mode(blend_mode mBlendMode)
{
    mQuad_.blend = mBlendMode;
}

void sprite::set_hot_spot(const vector2f& mHotSpot)
{
    mHotSpot_ = mHotSpot;
}

void sprite::set_hot_spot(float fX, float fY)
{
    mHotSpot_.x = fX;
    mHotSpot_.y = fY;
}

void sprite::set_texture_rect(const std::array<float,4>& lTextureRect, bool bNormalized)
{
    if (bNormalized)
    {
        mQuad_.v[0].uvs = vector2f(lTextureRect[0], lTextureRect[1]);
        mQuad_.v[1].uvs = vector2f(lTextureRect[2], lTextureRect[1]);
        mQuad_.v[2].uvs = vector2f(lTextureRect[2], lTextureRect[3]);
        mQuad_.v[3].uvs = vector2f(lTextureRect[0], lTextureRect[3]);
    }
    else
    {
        if (!mQuad_.mat)
        {
            throw gui::exception("gui::sprite",
                "Cannot use non-normalised coordinates with null material");
        }

        float fWidth = mQuad_.mat->get_width();
        float fHeight = mQuad_.mat->get_height();

        mQuad_.v[0].uvs = vector2f(lTextureRect[0]/fWidth, lTextureRect[1]/fHeight);
        mQuad_.v[1].uvs = vector2f(lTextureRect[2]/fWidth, lTextureRect[1]/fHeight);
        mQuad_.v[2].uvs = vector2f(lTextureRect[2]/fWidth, lTextureRect[3]/fHeight);
        mQuad_.v[3].uvs = vector2f(lTextureRect[0]/fWidth, lTextureRect[3]/fHeight);
    }
}

void sprite::set_texture_rect(float fX1, float fY1, float fX3, float fY3, bool bNormalized)
{
    if (bNormalized)
    {
        mQuad_.v[0].uvs = vector2f(fX1, fY1);
        mQuad_.v[1].uvs = vector2f(fX3, fY1);
        mQuad_.v[2].uvs = vector2f(fX3, fY3);
        mQuad_.v[3].uvs = vector2f(fX1, fY3);
    }
    else
    {
        if (!mQuad_.mat)
        {
            throw gui::exception("gui::sprite",
                "Cannot use non-normalised coordinates with null material");
        }

        float fWidth = mQuad_.mat->get_width();
        float fHeight = mQuad_.mat->get_height();

        mQuad_.v[0].uvs = vector2f(fX1/fWidth, fY1/fHeight);
        mQuad_.v[1].uvs = vector2f(fX3/fWidth, fY1/fHeight);
        mQuad_.v[2].uvs = vector2f(fX3/fWidth, fY3/fHeight);
        mQuad_.v[3].uvs = vector2f(fX1/fWidth, fY3/fHeight);
    }
}

void sprite::set_texture_coords(const std::array<float,8>& lTextureCoords, bool bNormalized)
{
    if (bNormalized)
    {
        mQuad_.v[0].uvs = vector2f(lTextureCoords[0], lTextureCoords[1]);
        mQuad_.v[1].uvs = vector2f(lTextureCoords[2], lTextureCoords[3]);
        mQuad_.v[2].uvs = vector2f(lTextureCoords[4], lTextureCoords[5]);
        mQuad_.v[3].uvs = vector2f(lTextureCoords[6], lTextureCoords[7]);
    }
    else
    {
        if (!mQuad_.mat)
        {
            throw gui::exception("gui::sprite",
                "Cannot use non-normalised coordinates with null material");
        }

        float fWidth = mQuad_.mat->get_width();
        float fHeight = mQuad_.mat->get_height();

        mQuad_.v[0].uvs = vector2f(lTextureCoords[0]/fWidth, lTextureCoords[1]/fHeight);
        mQuad_.v[1].uvs = vector2f(lTextureCoords[2]/fWidth, lTextureCoords[3]/fHeight);
        mQuad_.v[2].uvs = vector2f(lTextureCoords[4]/fWidth, lTextureCoords[5]/fHeight);
        mQuad_.v[3].uvs = vector2f(lTextureCoords[6]/fWidth, lTextureCoords[7]/fHeight);
    }
}

void sprite::set_texture_coords(float fX1, float fY1, float fX2, float fY2, float fX3, float fY3,
                              float fX4, float fY4, bool bNormalized)
{
    if (bNormalized)
    {
        mQuad_.v[0].uvs = vector2f(fX1, fY1);
        mQuad_.v[1].uvs = vector2f(fX2, fY2);
        mQuad_.v[2].uvs = vector2f(fX3, fY3);
        mQuad_.v[3].uvs = vector2f(fX4, fY4);
    }
    else
    {
        if (!mQuad_.mat)
        {
            throw gui::exception("gui::sprite",
                "Cannot use non-normalised coordinates with null material");
        }

        float fWidth = mQuad_.mat->get_width();
        float fHeight = mQuad_.mat->get_height();

        mQuad_.v[0].uvs = vector2f(fX1/fWidth, fY1/fHeight);
        mQuad_.v[1].uvs = vector2f(fX2/fWidth, fY2/fHeight);
        mQuad_.v[2].uvs = vector2f(fX3/fWidth, fY3/fHeight);
        mQuad_.v[3].uvs = vector2f(fX4/fWidth, fY4/fHeight);
    }
}

void sprite::set_dimensions(float fWidth, float fHeight)
{
    fWidth_ = fWidth;
    fHeight_ = fHeight;
}

float sprite::get_width() const
{
    return fWidth_;
}

float sprite::get_height() const
{
    return fHeight_;
}

color sprite::get_color() const
{
    return mQuad_.v[0].col;
}

blend_mode sprite::get_blend_mode() const
{
    return mQuad_.blend;
}

std::array<float,4> sprite::get_texture_rect() const
{
    std::array<float,4> mRect;
    mRect[0] = mQuad_.v[0].uvs.x;
    mRect[1] = mQuad_.v[0].uvs.y;
    mRect[2] = mQuad_.v[3].uvs.x;
    mRect[3] = mQuad_.v[3].uvs.y;
    return mRect;
}

std::array<float,8> sprite::get_texture_coords(bool bNormalized) const
{
    std::array<float,8> mCoords;
    if (bNormalized)
    {
        for (uint i = 0; i < 4; ++i)
        {
            mCoords[2*i+0] = mQuad_.v[i].uvs.x;
            mCoords[2*i+1] = mQuad_.v[i].uvs.y;
        }
    }
    else
    {
        if (!mQuad_.mat)
        {
            throw gui::exception("gui::sprite",
                "Cannot request non-normalised coordinates with null material");
        }

        float fWidth = mQuad_.mat->get_width();
        float fHeight = mQuad_.mat->get_height();

        for (uint i = 0; i < 4; ++i)
        {
            mCoords[2*i+0] = mQuad_.v[i].uvs.x*fWidth;
            mCoords[2*i+1] = mQuad_.v[i].uvs.y*fHeight;
        }
    }

    return mCoords;
}

}
}
