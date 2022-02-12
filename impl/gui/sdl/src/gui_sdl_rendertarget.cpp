#include "lxgui/impl/gui_sdl_rendertarget.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"

#include <SDL.h>
#include <iostream>

namespace lxgui::gui::sdl {

render_target::render_target(
    SDL_Renderer* p_renderer, const vector2ui& m_dimensions, material::filter m_filter) {
    p_texture_ = std::make_shared<sdl::material>(
        p_renderer, m_dimensions, true, material::wrap::repeat, m_filter);
}

void render_target::begin() {
    if (SDL_SetRenderTarget(p_texture_->get_renderer(), p_texture_->get_render_texture()) != 0) {
        throw gui::exception("gui::sdl::render_target", "Could not set current render target.");
    }

    m_view_matrix_ = matrix4f::view(vector2f(get_canvas_dimensions()));
}

void render_target::end() {
    SDL_SetRenderTarget(p_texture_->get_renderer(), nullptr);
}

void render_target::clear(const color& m_color) {
    SDL_SetRenderDrawColor(
        p_texture_->get_renderer(), m_color.r * 255, m_color.g * 255, m_color.b * 255, m_color.a * 255);

    SDL_RenderClear(p_texture_->get_renderer());
}

bounds2f render_target::get_rect() const {
    return p_texture_->get_rect();
}

vector2ui render_target::get_canvas_dimensions() const {
    return p_texture_->get_canvas_dimensions();
}

bool render_target::set_dimensions(const vector2ui& m_dimensions) {
    return p_texture_->set_dimensions(m_dimensions);
}

std::weak_ptr<sdl::material> render_target::get_material() {
    return p_texture_;
}

SDL_Texture* render_target::get_render_texture() {
    return p_texture_->get_render_texture();
}

const matrix4f& render_target::get_view_matrix() const {
    return m_view_matrix_;
}

void render_target::check_availability(SDL_Renderer* p_renderer) {
    SDL_RendererInfo m_info;
    SDL_GetRendererInfo(p_renderer, &m_info);

    if ((m_info.flags & SDL_RENDERER_TARGETTEXTURE) == 0) {
        throw gui::exception(
            "gui::sdl::render_target", "Render targets are not supported by hardware.");
    }
}

} // namespace lxgui::gui::sdl
