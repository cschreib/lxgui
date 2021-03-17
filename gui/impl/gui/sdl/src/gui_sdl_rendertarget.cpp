#include "lxgui/impl/gui_sdl_rendertarget.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"
#include <lxgui/gui_exception.hpp>

#include <iostream>

#include <SDL.h>

namespace lxgui {
namespace gui {
namespace sdl
{
render_target::render_target(SDL_Renderer* pRenderer, uint uiWidth, uint uiHeight)
{
    pTexture_ = std::make_shared<sdl::material>(
        pRenderer, uiWidth, uiHeight, true, material::wrap::REPEAT, material::filter::NONE
    );
}

void render_target::begin()
{
    if (SDL_SetRenderTarget(pTexture_->get_renderer(), pTexture_->get_render_texture()) != 0)
    {
        throw gui::exception("Could not set current render target");
    }
}

void render_target::end()
{
    SDL_SetRenderTarget(pTexture_->get_renderer(), nullptr);
}

void render_target::clear(const color& mColor)
{
    SDL_SetRenderDrawColor(pTexture_->get_renderer(),
        mColor.r*255, mColor.g*255, mColor.b*255, mColor.a*255);

    SDL_RenderClear(pTexture_->get_renderer());
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

std::weak_ptr<sdl::material> render_target::get_material()
{
    return pTexture_;
}

SDL_Texture* render_target::get_render_texture()
{
    return pTexture_->get_render_texture();
}

void render_target::check_availability(SDL_Renderer* pRenderer)
{
    SDL_RendererInfo mInfo;
    SDL_GetRendererInfo(pRenderer, &mInfo);

    if ((mInfo.flags & SDL_RENDERER_TARGETTEXTURE) == 0)
    {
        throw gui::exception("gui::sdl::material", "Render targets are not supported by hardware.");
    }
}

}
}
}
