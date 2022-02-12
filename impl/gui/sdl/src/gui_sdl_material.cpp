#include "lxgui/impl/gui_sdl_material.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"
#include "lxgui/utils_string.hpp"

#include <SDL.h>
#include <SDL_image.h>

namespace lxgui::gui::sdl {

int material::get_premultiplied_alpha_blend_mode() {
    static const SDL_BlendMode m_blend = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
        SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);

    return (int)m_blend;
}

material::material(
    SDL_Renderer*    p_renderer,
    const vector2ui& m_dimensions,
    bool             b_render_target,
    wrap             m_wrap,
    filter           m_filter) :
    gui::material(false), p_renderer_(p_renderer), b_is_owner_(true) {
    SDL_RendererInfo m_info;
    if (SDL_GetRendererInfo(p_renderer, &m_info) != 0) {
        throw gui::exception("gui::sdl::material", "Could not get renderer information.");
    }

    if (m_info.max_texture_width != 0) {
        if (m_dimensions.x > static_cast<std::size_t>(m_info.max_texture_width) ||
            m_dimensions.y > static_cast<std::size_t>(m_info.max_texture_height)) {
            throw gui::exception(
                "gui::sdl::material", "Texture dimensions not supported by hardware: (" +
                                          utils::to_string(m_dimensions.x) + " x " +
                                          utils::to_string(m_dimensions.y) + ").");
        }
    }

    // Set filtering
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, m_filter == filter::none ? "0" : "1") ==
        SDL_FALSE) {
        throw gui::exception("gui::sdl::material", "Could not set filtering hint");
    }

    p_texture_ = SDL_CreateTexture(
        p_renderer, SDL_PIXELFORMAT_ABGR8888,
        b_render_target ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STREAMING, m_dimensions.x,
        m_dimensions.y);

    if (p_texture_ == nullptr) {
        throw gui::exception(
            "gui::sdl::material", "Could not create " +
                                      std::string(b_render_target ? "render target" : "texture") +
                                      " with dimensions " + utils::to_string(m_dimensions.x) +
                                      " x " + utils::to_string(m_dimensions.y) + ".");
    }

    int    i_canvas_width = 0, i_canvas_height = 0, i_access = 0;
    Uint32 ui_texture_format = 0;
    SDL_QueryTexture(p_texture_, &ui_texture_format, &i_access, &i_canvas_width, &i_canvas_height);

    m_dimensions_       = m_dimensions;
    m_canvas_dimensions_ = vector2ui(i_canvas_width, i_canvas_height);
    m_wrap_             = m_wrap;
    m_filter_           = m_filter;
    b_render_target_     = b_render_target;

    m_rect_ = bounds2f(0, m_dimensions_.x, 0, m_dimensions_.y);
}

material::material(
    SDL_Renderer*      p_renderer,
    const std::string& s_file_name,
    bool               b_pre_multiplied_alpha_supported,
    wrap               m_wrap,
    filter             m_filter) :
    gui::material(false), p_renderer_(p_renderer), b_is_owner_(true) {
    // Load file
    SDL_Surface* p_surface = IMG_Load(s_file_name.c_str());
    if (p_surface == nullptr) {
        throw gui::exception("gui::sdl::material", "Could not load image file " + s_file_name + ".");
    }

    // Convert to RGBA 32bit
    SDL_Surface* p_converted_surface =
        SDL_ConvertSurfaceFormat(p_surface, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(p_surface);
    if (p_converted_surface == NULL) {
        throw gui::exception(
            "gui::sdl::material", "Could convert image file " + s_file_name + " to RGBA format.");
    }

    // Pre-multiply alpha
    if (b_pre_multiplied_alpha_supported)
        premultiply_alpha(p_converted_surface);

    const std::size_t ui_width  = p_converted_surface->w;
    const std::size_t ui_height = p_converted_surface->h;

    // Set filtering
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, m_filter == filter::none ? "0" : "1") ==
        SDL_FALSE) {
        throw gui::exception("gui::sdl::material", "Could not set filtering hint");
    }

    // Create streamable texture
    p_texture_ = SDL_CreateTexture(
        p_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, static_cast<int>(ui_width),
        static_cast<int>(ui_height));
    if (p_texture_ == nullptr) {
        SDL_FreeSurface(p_converted_surface);
        throw gui::exception(
            "gui::sdl::material", "Could not create texture with dimensions " +
                                      utils::to_string(ui_width) + " x " +
                                      utils::to_string(ui_height) + ".");
    }

    // Copy data into the texture
    std::size_t      ui_pitch        = 0;
    ub32color*       p_texture_pixels = lock_pointer(&ui_pitch);
    const ub32color* p_surface_pixels_start =
        reinterpret_cast<const ub32color*>(p_converted_surface->pixels);
    const ub32color* p_surface_pixels_end = p_surface_pixels_start + ui_pitch * ui_height;
    std::copy(p_surface_pixels_start, p_surface_pixels_end, p_texture_pixels);
    unlock_pointer();

    int    i_canvas_width = 0, i_canvas_height = 0, i_access = 0;
    Uint32 ui_texture_format = 0;
    SDL_QueryTexture(p_texture_, &ui_texture_format, &i_access, &i_canvas_width, &i_canvas_height);

    m_dimensions_       = vector2ui(ui_width, ui_height);
    m_canvas_dimensions_ = vector2ui(i_canvas_width, i_canvas_height);
    m_wrap_             = m_wrap;
    m_filter_           = m_filter;
    b_render_target_     = false;

    m_rect_ = bounds2f(0, m_dimensions_.x, 0, m_dimensions_.y);
}

material::material(
    SDL_Renderer* p_renderer, SDL_Texture* p_texture, const bounds2f& m_rect, filter m_filter) :
    gui::material(true),
    p_renderer_(p_renderer),
    m_rect_(m_rect),
    m_filter_(m_filter),
    p_texture_(p_texture),
    b_is_owner_(false) {
    int    i_canvas_width = 0, i_canvas_height = 0, i_access = 0;
    Uint32 ui_texture_format = 0;
    SDL_QueryTexture(p_texture_, &ui_texture_format, &i_access, &i_canvas_width, &i_canvas_height);

    m_dimensions_       = vector2ui(m_rect_.dimensions());
    m_canvas_dimensions_ = vector2ui(i_canvas_width, i_canvas_height);
}

material::~material() noexcept {
    if (p_texture_ && b_is_owner_)
        SDL_DestroyTexture(p_texture_);
}

void material::set_wrap(wrap m_wrap) {
    if (!b_is_owner_) {
        throw gui::exception(
            "gui::sdl::material", "A material in an atlas cannot change its wrapping mode.");
    }

    m_wrap_ = m_wrap;
}

material::wrap material::get_wrap() const {
    return m_wrap_;
}

void material::set_filter(filter m_filter) {
    if (!b_is_owner_) {
        throw gui::exception(
            "gui::sdl::material", "A material in an atlas cannot change its filtering.");
    }

    m_filter_ = m_filter;
}

material::filter material::get_filter() const {
    return m_filter_;
}

void material::premultiply_alpha(SDL_Surface* p_surface) {
    ub32color* p_pixel_data = reinterpret_cast<ub32color*>(p_surface->pixels);

    const std::size_t ui_area = p_surface->w * p_surface->h;
    for (std::size_t i = 0; i < ui_area; ++i) {
        float a = p_pixel_data[i].a / 255.0f;
        p_pixel_data[i].r *= a;
        p_pixel_data[i].g *= a;
        p_pixel_data[i].b *= a;
    }
}

bounds2f material::get_rect() const {
    return m_rect_;
}

vector2ui material::get_canvas_dimensions() const {
    return m_canvas_dimensions_;
}

bool material::uses_same_texture(const gui::material& m_other) const {
    return p_texture_ == static_cast<const sdl::material&>(m_other).p_texture_;
}

bool material::set_dimensions(const vector2ui& m_dimensions) {
    if (!b_is_owner_) {
        throw gui::exception("gui::sdl::material", "A material in an atlas cannot be resized.");
    }

    if (!b_render_target_)
        return false;

    SDL_RendererInfo m_info;
    if (SDL_GetRendererInfo(p_renderer_, &m_info) != 0) {
        throw gui::exception("gui::sdl::material", "Could not get renderer information.");
    }

    if (m_info.max_texture_width != 0) {
        if (m_dimensions.x > static_cast<std::size_t>(m_info.max_texture_width) ||
            m_dimensions.y > static_cast<std::size_t>(m_info.max_texture_height)) {
            return false;
        }
    }

    bool b_canvas_updated = false;

    if (m_dimensions.x > m_canvas_dimensions_.x || m_dimensions.y > m_canvas_dimensions_.y) {
        // SDL is not efficient at resizing render texture, so use an exponential growth pattern
        // to avoid re-allocating a new render texture on every resize operation.
        vector2ui m_canvas_dimensions = m_canvas_dimensions_;
        if (m_dimensions.x > m_canvas_dimensions_.x)
            m_canvas_dimensions.x = m_dimensions.x + m_dimensions.x / 2;
        if (m_dimensions.y > m_canvas_dimensions_.y)
            m_canvas_dimensions.y = m_dimensions.y + m_dimensions.y / 2;

        SDL_Texture* p_texture = SDL_CreateTexture(
            p_renderer_, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, m_canvas_dimensions.x,
            m_canvas_dimensions.y);

        if (p_texture == nullptr) {
            throw gui::exception(
                "gui::sdl::material", "Could not create render target "
                                      "with dimensions " +
                                          utils::to_string(m_canvas_dimensions.x) + " x " +
                                          utils::to_string(m_canvas_dimensions.y) + ".");
        }

        SDL_DestroyTexture(p_texture_);
        p_texture_          = p_texture;
        m_canvas_dimensions_ = m_canvas_dimensions;
        b_canvas_updated     = true;
    }

    m_dimensions_ = m_dimensions;
    m_rect_       = bounds2f(0, m_dimensions_.x, 0, m_dimensions_.y);

    return b_canvas_updated;
}

const ub32color* material::lock_pointer(std::size_t* p_pitch) const {
    void* p_pixel_data = nullptr;
    int   i_pitch     = 0;
    if (SDL_LockTexture(p_texture_, nullptr, &p_pixel_data, &i_pitch) != 0) {
        throw gui::exception("gui::sdl::material", "Could not lock texture for copying pixels.");
    }

    if (p_pitch)
        *p_pitch = static_cast<std::size_t>(i_pitch) / sizeof(ub32color);

    return reinterpret_cast<ub32color*>(p_pixel_data);
}

ub32color* material::lock_pointer(std::size_t* p_pitch) {
    if (!b_is_owner_) {
        throw gui::exception(
            "gui::sdl::material", "A material in an atlas cannot update its data.");
    }

    return const_cast<ub32color*>(const_cast<const material*>(this)->lock_pointer(p_pitch));
}

void material::unlock_pointer() const {
    SDL_UnlockTexture(p_texture_);
}

SDL_Texture* material::get_render_texture() {
    if (!b_render_target_)
        return nullptr;

    return p_texture_;
}

SDL_Texture* material::get_texture() const {
    return p_texture_;
}

SDL_Renderer* material::get_renderer() {
    return p_renderer_;
}

} // namespace lxgui::gui::sdl
