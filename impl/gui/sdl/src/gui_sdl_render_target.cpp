#include "lxgui/impl/gui_sdl_render_target.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <algorithm>
#include <iostream>

namespace lxgui::gui::sdl {

render_target::render_target(
    SDL_Renderer* rdr, const vector2ui& dimensions, material::filter filt) {
    texture_ = std::make_shared<sdl::material>(rdr, dimensions, true, material::wrap::repeat, filt);
}

void render_target::begin() {
    if (SDL_SetRenderTarget(texture_->get_renderer(), texture_->get_render_texture()) != 0) {
        throw gui::exception("gui::sdl::render_target", "Could not set current render target.");
    }

    view_matrix_ = matrix4f::view(vector2f(get_canvas_dimensions()));
}

void render_target::end() {
    SDL_SetRenderTarget(texture_->get_renderer(), nullptr);
}

void render_target::clear(const color& c) {
    SDL_SetRenderDrawColor(texture_->get_renderer(), c.r * 255, c.g * 255, c.b * 255, c.a * 255);

    SDL_RenderClear(texture_->get_renderer());
}

bounds2f render_target::get_rect() const {
    return texture_->get_rect();
}

vector2ui render_target::get_canvas_dimensions() const {
    return texture_->get_canvas_dimensions();
}

bool render_target::set_dimensions(const vector2ui& dimensions) {
    return texture_->set_dimensions(dimensions);
}

void render_target::save_to_file(std::string filename) const {
    SDL_Renderer* renderer = texture_->get_renderer();
    SDL_Texture*  target   = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture_->get_render_texture());

    SDL_Surface* surface = SDL_CreateRGBSurface(
        0, texture_->get_rect().width(), texture_->get_rect().height(), 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);

    // De-multiply alpha
    color32* pixel_data = reinterpret_cast<color32*>(surface->pixels);

    const std::size_t area = surface->w * surface->h;
    for (std::size_t i = 0; i < area; ++i) {
        float a = static_cast<float>(pixel_data[i].a) / 255.0f;
        if (a > 0.0f) {
            pixel_data[i].r = static_cast<unsigned char>(
                std::clamp(static_cast<float>(pixel_data[i].r) / a, 0.0f, 255.0f));
            pixel_data[i].g = static_cast<unsigned char>(
                std::clamp(static_cast<float>(pixel_data[i].g) / a, 0.0f, 255.0f));
            pixel_data[i].b = static_cast<unsigned char>(
                std::clamp(static_cast<float>(pixel_data[i].b) / a, 0.0f, 255.0f));
        }
    }

    IMG_SavePNG(surface, filename.c_str());
    SDL_FreeSurface(surface);

    SDL_SetRenderTarget(renderer, target);
}

std::weak_ptr<sdl::material> render_target::get_material() {
    return texture_;
}

SDL_Texture* render_target::get_render_texture() {
    return texture_->get_render_texture();
}

const matrix4f& render_target::get_view_matrix() const {
    return view_matrix_;
}

void render_target::check_availability(SDL_Renderer* rdr) {
    SDL_RendererInfo info;
    SDL_GetRendererInfo(rdr, &info);

    if ((info.flags & SDL_RENDERER_TARGETTEXTURE) == 0) {
        throw gui::exception(
            "gui::sdl::render_target", "Render targets are not supported by hardware.");
    }
}

} // namespace lxgui::gui::sdl
