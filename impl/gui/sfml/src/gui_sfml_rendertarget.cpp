#include "lxgui/impl/gui_sfml_rendertarget.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"

#include <iostream>

namespace lxgui {
namespace gui {
namespace sfml
{
render_target::render_target(const vector2ui& mDimensions, material::filter mFilter)
{
    pTexture_ = std::make_shared<sfml::material>(
        mDimensions, true, material::wrap::REPEAT, mFilter
    );

    pRenderTexture_ = pTexture_->get_render_texture();
}

void render_target::begin()
{
}

void render_target::end()
{
    pRenderTexture_->display();
}

void render_target::clear(const color& mColor)
{
    pRenderTexture_->clear(sf::Color(mColor.r*255, mColor.g*255, mColor.b*255, mColor.a*255));
}

bounds2f render_target::get_rect() const
{
    return pTexture_->get_rect();
}

vector2ui render_target::get_canvas_dimensions() const
{
    return pTexture_->get_canvas_dimensions();
}

bool render_target::set_dimensions(const vector2ui& mDimensions)
{
    return pTexture_->set_dimensions(mDimensions);
}

std::weak_ptr<sfml::material> render_target::get_material()
{
    return pTexture_;
}

sf::RenderTexture* render_target::get_render_texture()
{
    return pRenderTexture_;
}
}
}
}
