#include "lxgui/gui_material.hpp"

namespace lxgui {
namespace gui
{

material::material(bool bIsAtlas) : bIsAtlas_(bIsAtlas)
{
}

vector2f material::get_canvas_uv(const vector2f& mTextureUV, bool bFromNormalized) const
{
    const bounds2f mQuad = get_rect();

    vector2f mPixelUV = mTextureUV;
    if (bFromNormalized)
        mPixelUV *= mQuad.dimensions();

    mPixelUV += mQuad.top_left();
    mPixelUV /= vector2f(get_canvas_dimensions());

    return mPixelUV;
}

vector2f material::get_local_uv(const vector2f& mCanvasUV, bool bAsNormalized) const
{
    const bounds2f mQuad = get_rect();

    vector2f mPixelUV = mCanvasUV;
    mPixelUV *= vector2f(get_canvas_dimensions());
    mPixelUV -= mQuad.top_left();

    if (bAsNormalized)
        mPixelUV /= mQuad.dimensions();

    return mPixelUV;
}

bool material::is_in_atlas() const
{
    return bIsAtlas_;
}

}
}
