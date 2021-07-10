#include "lxgui/gui_atlas_default.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_vertex.hpp"

namespace lxgui {
namespace gui
{

atlas_page_default::atlas_page_default(const renderer& mRenderer, material::filter mFilter) :
    atlas_page(mFilter), mRenderer_(mRenderer)
{
    uint uiSize = mRenderer_.get_texture_atlas_page_size();
    pTarget_ = mRenderer_.create_render_target(uiSize, uiSize, mFilter);
}

std::shared_ptr<gui::material> atlas_page_default::add_material_(const gui::material& mMat,
    const quad2f& mLocation)
{
    mRenderer_.begin(pTarget_);
    mRenderer_.set_view(matrix4f::view(
        vector2f(pTarget_->get_canvas_width(), pTarget_->get_canvas_height())));

    std::vector<std::array<vertex,4>> mQuads;
    mQuads.emplace_back();
    auto& mQuad = mQuads.back();

    const vector2f mUVs = mMat.get_canvas_uv(vector2f(1.0f, 1.0f), true);

    mQuad[0].pos = mLocation.top_left();
    mQuad[1].pos = mLocation.top_right();
    mQuad[2].pos = mLocation.bottom_right();
    mQuad[3].pos = mLocation.bottom_left();

    mQuad[0].uvs = vector2f(0.0f,   0.0f);
    mQuad[1].uvs = vector2f(mUVs.x, 0.0f);
    mQuad[2].uvs = vector2f(mUVs.x, mUVs.y);
    mQuad[3].uvs = vector2f(0.0f,   mUVs.y);

    mRenderer_.render_quads(&mMat, mQuads);

    mRenderer_.end();

    return mRenderer_.create_material(pTarget_, mLocation);
}

float atlas_page_default::get_width() const
{
    return pTarget_->get_canvas_width();
}

float atlas_page_default::get_height() const
{
    return pTarget_->get_canvas_height();
}


atlas_default::atlas_default(const renderer& mRenderer, material::filter mFilter) : gui::atlas(mRenderer, mFilter) {}

std::unique_ptr<gui::atlas_page> atlas_default::create_page_() const
{
    return std::make_unique<atlas_page_default>(mRenderer_, mFilter_);
}


}
}
