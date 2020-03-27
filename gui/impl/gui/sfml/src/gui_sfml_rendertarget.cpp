#include "lxgui/impl/gui_sfml_rendertarget.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"

#include <iostream>

namespace gui {
namespace sfml
{
render_target::render_target(uint uiWidth, uint uiHeight)
{
    pTexture_ = utils::refptr<sfml::material>(new sfml::material(
        uiWidth, uiHeight, true, material::wrap::REPEAT, material::filter::NONE
    ));

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

uint render_target::get_width() const
{
    return pTexture_->get_width();
}

uint render_target::get_height() const
{
    return pTexture_->get_height();
}

uint render_target::get_real_width() const
{
    return pTexture_->get_real_width();
}

uint render_target::get_real_height() const
{
    return pTexture_->get_real_height();
}

bool render_target::set_dimensions(uint uiWidth, uint uiHeight)
{
    return pTexture_->set_dimensions(uiWidth, uiHeight);
}

utils::wptr<sfml::material> render_target::get_material()
{
    return pTexture_;
}

sf::RenderTexture* render_target::get_render_texture()
{
    return pRenderTexture_;
}
}
}
