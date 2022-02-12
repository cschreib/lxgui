#include "lxgui/impl/gui_sdl_atlas.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"
#include "lxgui/utils_string.hpp"

#include <SDL.h>
#include <SDL_image.h>

namespace lxgui::gui::sdl {

atlas_page::atlas_page(renderer& m_renderer, material::filter m_filter) :
    gui::atlas_page(m_filter), m_renderer_(m_renderer) {
    // Set filtering
    if (SDL_SetHint(
            SDL_HINT_RENDER_SCALE_QUALITY, m_filter == material::filter::none ? "0" : "1") ==
        SDL_FALSE) {
        throw gui::exception("gui::sdl::atlas_page", "Could not set filtering hint");
    }

    ui_size_ = m_renderer_.get_texture_atlas_page_size();

    p_texture_ = SDL_CreateTexture(
        m_renderer_.get_sdl_renderer(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
        ui_size_, ui_size_);

    if (p_texture_ == nullptr) {
        throw gui::exception(
            "gui::sdl::material", "Could not create texture with dimensions " +
                                      utils::to_string(ui_size_) + " x " +
                                      utils::to_string(ui_size_) + ".");
    }
}

atlas_page::~atlas_page() {
    if (p_texture_)
        SDL_DestroyTexture(p_texture_);
}

std::shared_ptr<gui::material>
atlas_page::add_material_(const gui::material& m_mat, const bounds2f& m_location) {
    const sdl::material& m_sdl_mat = static_cast<const sdl::material&>(m_mat);

    std::size_t      ui_texture_pitch = 0;
    const ub32color* p_texture_pixels = nullptr;

    try {
        p_texture_pixels = m_sdl_mat.lock_pointer(&ui_texture_pitch);

        SDL_Rect m_update_rect;
        m_update_rect.x = m_location.left;
        m_update_rect.y = m_location.top;
        m_update_rect.w = m_location.width();
        m_update_rect.h = m_location.height();

        if (SDL_UpdateTexture(p_texture_, &m_update_rect, p_texture_pixels, ui_texture_pitch * 4) !=
            0) {
            throw gui::exception("sdl::atlas_page", "Failed to upload texture to atlas.");
        }

        m_sdl_mat.unlock_pointer();
        p_texture_pixels = nullptr;
    } catch (...) {
        if (p_texture_pixels)
            m_sdl_mat.unlock_pointer();
        throw;
    }

    return std::make_shared<sdl::material>(
        m_renderer_.get_sdl_renderer(), p_texture_, m_location, m_filter_);
}

float atlas_page::get_width_() const {
    return ui_size_;
}

float atlas_page::get_height_() const {
    return ui_size_;
}

atlas::atlas(renderer& m_renderer, material::filter m_filter) :
    gui::atlas(m_renderer, m_filter), m_sdl_renderer_(m_renderer) {}

std::unique_ptr<gui::atlas_page> atlas::create_page_() {
    return std::make_unique<sdl::atlas_page>(m_sdl_renderer_, m_filter_);
}

} // namespace lxgui::gui::sdl
