#include "lxgui/impl/gui_sdl_material.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"
#include "lxgui/utils_string.hpp"

#include <SDL.h>
#include <SDL_image.h>

namespace lxgui::gui::sdl {

int material::get_premultiplied_alpha_blend_mode() {
    static const SDL_BlendMode blend = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
        SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);

    return (int)blend;
}

material::material(
    SDL_Renderer*    renderer,
    const vector2ui& dimensions,
    bool             is_render_target,
    wrap             wrp,
    filter           filt) :
    gui::material(false), renderer_(renderer), is_owner_(true) {
    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer, &info) != 0) {
        throw gui::exception("gui::sdl::material", "Could not get renderer information.");
    }

    if (info.max_texture_width != 0) {
        if (dimensions.x > static_cast<std::size_t>(info.max_texture_width) ||
            dimensions.y > static_cast<std::size_t>(info.max_texture_height)) {
            throw gui::exception(
                "gui::sdl::material", "Texture dimensions not supported by hardware: (" +
                                          utils::to_string(dimensions.x) + " x " +
                                          utils::to_string(dimensions.y) + ").");
        }
    }

    // Set filtering
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filt == filter::none ? "0" : "1") == SDL_FALSE) {
        throw gui::exception("gui::sdl::material", "Could not set filtering hint");
    }

    texture_ = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ABGR8888,
        is_render_target ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STREAMING, dimensions.x,
        dimensions.y);

    if (texture_ == nullptr) {
        throw gui::exception(
            "gui::sdl::material", "Could not create " +
                                      std::string(is_render_target ? "render target" : "texture") +
                                      " with dimensions " + utils::to_string(dimensions.x) + " x " +
                                      utils::to_string(dimensions.y) + ".");
    }

    int    canvas_width = 0, canvas_height = 0, access = 0;
    Uint32 texture_format = 0;
    SDL_QueryTexture(texture_, &texture_format, &access, &canvas_width, &canvas_height);

    dimensions_        = dimensions;
    canvas_dimensions_ = vector2ui(canvas_width, canvas_height);
    wrap_              = wrp;
    filter_            = filt;
    is_render_target_  = is_render_target;

    rect_ = bounds2f(0, dimensions_.x, 0, dimensions_.y);
}

material::material(
    SDL_Renderer*      renderer,
    const std::string& file_name,
    bool               is_pre_multiplied_alpha_supported,
    wrap               wrp,
    filter             filt) :
    gui::material(false), renderer_(renderer), is_owner_(true) {
    // Load file
    SDL_Surface* surface = IMG_Load(file_name.c_str());
    if (surface == nullptr) {
        throw gui::exception("gui::sdl::material", "Could not load image file " + file_name + ".");
    }

    // Convert to RGBA 32bit
    SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(surface);
    if (converted_surface == NULL) {
        throw gui::exception(
            "gui::sdl::material", "Could convert image file " + file_name + " to RGBA format.");
    }

    // Pre-multiply alpha
    if (is_pre_multiplied_alpha_supported)
        premultiply_alpha(converted_surface);

    const std::size_t width  = converted_surface->w;
    const std::size_t height = converted_surface->h;

    // Set filtering
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filt == filter::none ? "0" : "1") == SDL_FALSE) {
        throw gui::exception("gui::sdl::material", "Could not set filtering hint");
    }

    // Create streamable texture
    texture_ = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, static_cast<int>(width),
        static_cast<int>(height));
    if (texture_ == nullptr) {
        SDL_FreeSurface(converted_surface);
        throw gui::exception(
            "gui::sdl::material", "Could not create texture with dimensions " +
                                      utils::to_string(width) + " x " + utils::to_string(height) +
                                      ".");
    }

    // Copy data into the texture
    std::size_t    pitch          = 0;
    color32*       texture_pixels = lock_pointer(&pitch);
    const color32* surface_pixels_start =
        reinterpret_cast<const color32*>(converted_surface->pixels);
    const color32* surface_pixels_end = surface_pixels_start + pitch * height;
    std::copy(surface_pixels_start, surface_pixels_end, texture_pixels);
    unlock_pointer();

    int    canvas_width = 0, canvas_height = 0, access = 0;
    Uint32 texture_format = 0;
    SDL_QueryTexture(texture_, &texture_format, &access, &canvas_width, &canvas_height);

    dimensions_        = vector2ui(width, height);
    canvas_dimensions_ = vector2ui(canvas_width, canvas_height);
    wrap_              = wrp;
    filter_            = filt;
    is_render_target_  = false;

    rect_ = bounds2f(0, dimensions_.x, 0, dimensions_.y);
}

material::material(SDL_Renderer* renderer, SDL_Texture* tex, const bounds2f& rect, filter filt) :
    gui::material(true),
    renderer_(renderer),
    rect_(rect),
    filter_(filt),
    texture_(tex),
    is_owner_(false) {
    int    canvas_width = 0, canvas_height = 0, access = 0;
    Uint32 texture_format = 0;
    SDL_QueryTexture(texture_, &texture_format, &access, &canvas_width, &canvas_height);

    dimensions_        = vector2ui(rect_.dimensions());
    canvas_dimensions_ = vector2ui(canvas_width, canvas_height);
}

material::~material() noexcept {
    if (texture_ && is_owner_)
        SDL_DestroyTexture(texture_);
}

void material::set_wrap(wrap wrp) {
    if (!is_owner_) {
        throw gui::exception(
            "gui::sdl::material", "A material in an atlas cannot change its wrapping mode.");
    }

    wrap_ = wrp;
}

material::wrap material::get_wrap() const {
    return wrap_;
}

void material::set_filter(filter filt) {
    if (!is_owner_) {
        throw gui::exception(
            "gui::sdl::material", "A material in an atlas cannot change its filtering.");
    }

    filter_ = filt;
}

material::filter material::get_filter() const {
    return filter_;
}

void material::premultiply_alpha(SDL_Surface* surface) {
    color32* pixel_data = reinterpret_cast<color32*>(surface->pixels);

    const std::size_t area = surface->w * surface->h;
    for (std::size_t i = 0; i < area; ++i) {
        float a         = static_cast<float>(pixel_data[i].a) / 255.0f;
        pixel_data[i].r = static_cast<unsigned char>(static_cast<float>(pixel_data[i].r) * a);
        pixel_data[i].g = static_cast<unsigned char>(static_cast<float>(pixel_data[i].g) * a);
        pixel_data[i].b = static_cast<unsigned char>(static_cast<float>(pixel_data[i].b) * a);
    }
}

bounds2f material::get_rect() const {
    return rect_;
}

vector2ui material::get_canvas_dimensions() const {
    return canvas_dimensions_;
}

bool material::uses_same_texture(const gui::material& other) const {
    return texture_ == static_cast<const sdl::material&>(other).texture_;
}

bool material::set_dimensions(const vector2ui& dimensions) {
    if (!is_owner_) {
        throw gui::exception("gui::sdl::material", "A material in an atlas cannot be resized.");
    }

    if (!is_render_target_)
        return false;

    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer_, &info) != 0) {
        throw gui::exception("gui::sdl::material", "Could not get renderer information.");
    }

    if (info.max_texture_width != 0) {
        if (dimensions.x > static_cast<std::size_t>(info.max_texture_width) ||
            dimensions.y > static_cast<std::size_t>(info.max_texture_height)) {
            return false;
        }
    }

    bool canvas_updated = false;

    if (dimensions.x > canvas_dimensions_.x || dimensions.y > canvas_dimensions_.y) {
        // SDL is not efficient at resizing render texture, so use an exponential growth pattern
        // to avoid re-allocating a new render texture on every resize operation.
        vector2ui canvas_dimensions = canvas_dimensions_;
        if (dimensions.x > canvas_dimensions_.x)
            canvas_dimensions.x = dimensions.x + dimensions.x / 2;
        if (dimensions.y > canvas_dimensions_.y)
            canvas_dimensions.y = dimensions.y + dimensions.y / 2;

        SDL_Texture* tex = SDL_CreateTexture(
            renderer_, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, canvas_dimensions.x,
            canvas_dimensions.y);

        if (tex == nullptr) {
            throw gui::exception(
                "gui::sdl::material", "Could not create render target with dimensions " +
                                          utils::to_string(canvas_dimensions.x) + " x " +
                                          utils::to_string(canvas_dimensions.y) + ".");
        }

        SDL_DestroyTexture(texture_);
        texture_           = tex;
        canvas_dimensions_ = canvas_dimensions;
        canvas_updated     = true;
    }

    dimensions_ = dimensions;
    rect_       = bounds2f(0, dimensions_.x, 0, dimensions_.y);

    return canvas_updated;
}

const color32* material::lock_pointer(std::size_t* pitch) const {
    void* pixel_data = nullptr;
    int   int_pitch  = 0;
    if (SDL_LockTexture(texture_, nullptr, &pixel_data, &int_pitch) != 0) {
        throw gui::exception("gui::sdl::material", "Could not lock texture for copying pixels.");
    }

    if (pitch)
        *pitch = static_cast<std::size_t>(int_pitch) / sizeof(color32);

    return reinterpret_cast<color32*>(pixel_data);
}

color32* material::lock_pointer(std::size_t* pitch) {
    if (!is_owner_) {
        throw gui::exception(
            "gui::sdl::material", "A material in an atlas cannot update its data.");
    }

    return const_cast<color32*>(const_cast<const material*>(this)->lock_pointer(pitch));
}

void material::unlock_pointer() const {
    SDL_UnlockTexture(texture_);
}

SDL_Texture* material::get_render_texture() {
    if (!is_render_target_)
        return nullptr;

    return texture_;
}

SDL_Texture* material::get_texture() const {
    return texture_;
}

SDL_Renderer* material::get_renderer() {
    return renderer_;
}

} // namespace lxgui::gui::sdl
