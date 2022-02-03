#include "lxgui/impl/gui_sdl_rendertarget.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"
#include <lxgui/gui_exception.hpp>

#include <iostream>

#include <SDL.h>

namespace lxgui {
namespace gui {
namespace sdl
{
render_target::render_target(SDL_Renderer* pRenderer,
    const vector2ui& mDimensions, material::filter mFilter)
{
    pTexture_ = std::make_shared<sdl::material>(
        pRenderer, mDimensions, true, material::wrap::REPEAT, mFilter
    );
}

void render_target::begin()
{
    if (SDL_SetRenderTarget(pTexture_->get_renderer(), pTexture_->get_render_texture()) != 0)
    {
        throw gui::exception("gui::sdl::render_target", "Could not set current render target.");
    }

    mViewMatrix_ = matrix4f::view(vector2f(get_canvas_dimensions()));
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

std::weak_ptr<sdl::material> render_target::get_material()
{
    return pTexture_;
}

SDL_Texture* render_target::get_render_texture()
{
    return pTexture_->get_render_texture();
}

const matrix4f& render_target::get_view_matrix() const
{
    return mViewMatrix_;
}

void render_target::check_availability(SDL_Renderer* pRenderer)
{
    SDL_RendererInfo mInfo;
    SDL_GetRendererInfo(pRenderer, &mInfo);

    if ((mInfo.flags & SDL_RENDERER_TARGETTEXTURE) == 0)
    {
        throw gui::exception("gui::sdl::render_target",
            "Render targets are not supported by hardware.");
    }
}

}
}
}
