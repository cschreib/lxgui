#include "lxgui/gui_material.hpp"

namespace lxgui {
namespace gui
{

vector2f material::get_canvas_uv(const vector2f& mTextureUV, bool bFromNormalized) const
{
    const quad2f mQuad = get_rect();

    vector2f mPixelUV = mTextureUV;
    if (bFromNormalized)
    {
        mPixelUV.x *= mQuad.width();
        mPixelUV.y *= mQuad.height();
    }

    mPixelUV.x += mQuad.left;
    mPixelUV.y += mQuad.top;

    mPixelUV.x /= get_canvas_width();
    mPixelUV.y /= get_canvas_height();

    return mPixelUV;
}

vector2f material::get_local_uv(const vector2f& mCanvasUV, bool bAsNormalized) const
{
    const quad2f mQuad = get_rect();

    vector2f mPixelUV = mCanvasUV;
    mPixelUV.x *= get_canvas_width();
    mPixelUV.y *= get_canvas_height();

    mPixelUV.x -= mQuad.left;
    mPixelUV.y -= mQuad.top;

    if (bAsNormalized)
    {
        mPixelUV.x /= mQuad.width();
        mPixelUV.y /= mQuad.height();
    }

    return mPixelUV;
}

}
}
