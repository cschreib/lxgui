#include "lxgui/impl/gui_sdl_font.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_sdl_material.hpp"
#include "lxgui/utils_string.hpp"

#include <SDL.h>
#include <SDL_ttf.h>

namespace lxgui::gui::sdl {

font::font(
    SDL_Renderer*                        renderer,
    const std::string&                   font_file,
    std::size_t                          size,
    std::size_t                          outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             default_code_point,
    bool                                 pre_multiplied_alpha_supported) :
    size_(size), default_code_point_(default_code_point) {
    if (!TTF_WasInit() && TTF_Init() != 0) {
        throw gui::exception(
            "gui::sdl::font", "Could not initialise SDL_ttf: " + std::string(TTF_GetError()));
    }

    TTF_Font* fnt = TTF_OpenFont(font_file.c_str(), size_);
    if (!fnt) {
        throw gui::exception(
            "gui::sdl::font", "Could not load font file '" + font_file + "' at size " +
                                  utils::to_string(size) + ": " + std::string(TTF_GetError()) +
                                  ".");
    }

    if (outline > 0)
        TTF_SetFontOutline(fnt, outline);

    // Add some space between letters to prevent artifacts
    std::size_t spacing = 1;

    int max_height = 0, max_width = 0;

    // Calculate maximum width and height
    std::size_t num_char = 0;
    for (const code_point_range& range : code_points) {
        for (char32_t code_point = range.first; code_point <= range.last; ++code_point) {
            if (code_point > std::numeric_limits<Uint16>::max())
                break;

            const Uint16 alt_char = static_cast<Uint16>(code_point);

            int min_x = 0, max_x = 0, min_y = 0, max_y = 0, advance = 0;
            if (TTF_GlyphMetrics(fnt, alt_char, &min_x, &max_x, &min_y, &max_y, &advance) != 0)
                continue;

            int char_height = max_y - min_y;
            if (char_height > max_height)
                max_height = char_height;

            int char_width = max_x - min_x;
            if (char_width > max_width)
                max_width = char_width;

            ++num_char;
        }
    }

    max_height = max_height + 2 * outline;
    max_width  = max_width + 2 * outline;

    // Calculate the size of the texture
    std::size_t tex_size = (max_width + spacing) * (max_height + spacing) * num_char;
    std::size_t tex_side = static_cast<std::size_t>(std::sqrt((float)tex_size));
    tex_side += std::max(max_width, max_height);

    // Round up to nearest power of two
    {
        std::size_t i = 1;
        while (tex_side > i)
            i *= 2;
        tex_side = i;
    }

    std::size_t final_width, final_height;
    if (tex_side * tex_side / 2 >= tex_size)
        final_height = tex_side / 2;
    else
        final_height = tex_side;

    final_width = tex_side;

    texture_ = std::make_shared<sdl::material>(renderer, vector2ui(final_width, final_height));

    vector2ui canvas_dimensions       = texture_->get_canvas_dimensions();
    vector2f  canvas_dimensions_float = vector2f(canvas_dimensions);

    std::size_t pitch          = 0;
    ub32color*  texture_pixels = texture_->lock_pointer(&pitch);
    std::fill(texture_pixels, texture_pixels + pitch * canvas_dimensions.y, ub32color(0, 0, 0, 0));

    std::size_t x = 0, y = 0;
    std::size_t line_max_height = max_height;

    const SDL_Color color = {255, 255, 255, 255};

    const float y_offset = TTF_FontDescent(fnt);

    for (const code_point_range& range : code_points) {
        range_info info;
        info.range = range;
        info.data.resize(range.last - range.first + 1);

        for (char32_t code_point = range.first; code_point <= range.last; ++code_point) {
            character_info& ci = info.data[code_point - range.first];
            ci.code_point      = code_point;

            if (code_point > std::numeric_limits<Uint16>::max()) {
                gui::out << gui::warning << "gui::sdl::font : Cannot load character " << code_point
                         << " because SDL_ttf only accepts 16bit code points." << std::endl;
                break;
            }

            const Uint16 alt_char = static_cast<Uint16>(code_point);

            int min_x = 0, max_x = 0, min_y = 0, max_y = 0, advance = 0;
            if (TTF_GlyphMetrics(fnt, alt_char, &min_x, &max_x, &min_y, &max_y, &advance) != 0) {
                gui::out << gui::warning << "gui::sdl::font : Cannot load character " << code_point
                         << " in font \"" << font_file << "\"." << std::endl;
                continue;
            }

            SDL_Surface* glyph_surface = TTF_RenderGlyph_Blended(fnt, alt_char, color);
            if (!glyph_surface) {
                gui::out << gui::warning << "gui::sdl::font : Cannot draw character " << code_point
                         << " in font \"" << font_file << "\"." << std::endl;
                continue;
            }

            if (glyph_surface->format->format != SDL_PIXELFORMAT_ARGB8888) {
                throw gui::exception(
                    "gui::sdl::font", "SDL_ttf output format is not ARGB8888 (got " +
                                          utils::to_string(glyph_surface->format->format) + ")");
            }

            const std::size_t glyph_width  = glyph_surface->w;
            const std::size_t glyph_height = glyph_surface->h;

            line_max_height = std::max(line_max_height, glyph_height);

            // If at end of row, jump to next line
            if (x + glyph_width > static_cast<std::size_t>(canvas_dimensions.x) - 1) {
                y += line_max_height + spacing;
                x = 0;
            }

            // SDL_ttf outputs glyphs in BGRA (little-endian) and we use RGBA;
            // this is fine because we always render glyphs in white, and don't care about
            // the color information.
            ub32color*  glyph_pixels = reinterpret_cast<ub32color*>(glyph_surface->pixels);
            std::size_t glyph_pitch  = glyph_surface->pitch / sizeof(ub32color);
            for (std::size_t j = 0; j < glyph_height; ++j)
                for (std::size_t i = 0; i < glyph_width; ++i)
                    texture_pixels[x + i + (y + j) * pitch] = glyph_pixels[i + j * glyph_pitch];

            SDL_FreeSurface(glyph_surface);

            ci.uvs.left   = x / canvas_dimensions_float.x;
            ci.uvs.top    = y / canvas_dimensions_float.y;
            ci.uvs.right  = (x + glyph_width) / canvas_dimensions_float.x;
            ci.uvs.bottom = (y + glyph_height) / canvas_dimensions_float.y;

            // NB: do not use iMinX etc here; SDL_ttf has already applied them to the rendered glyph
            ci.rect.left   = -static_cast<float>(outline);
            ci.rect.right  = ci.rect.left + glyph_width;
            ci.rect.top    = y_offset - static_cast<float>(outline);
            ci.rect.bottom = ci.rect.top + glyph_height;

            ci.advance = advance;

            // Advance a column
            x += glyph_width + spacing;
        }

        range_list_.push_back(std::move(info));
    }

    TTF_CloseFont(fnt);

    // Pre-multiply alpha
    if (pre_multiplied_alpha_supported) {
        const std::size_t area = canvas_dimensions.x * canvas_dimensions.y;
        for (std::size_t i = 0; i < area; ++i) {
            float a = texture_pixels[i].a / 255.0f;
            texture_pixels[i].r *= a;
            texture_pixels[i].g *= a;
            texture_pixels[i].b *= a;
        }
    }

    texture_->unlock_pointer();
}

std::size_t font::get_size() const {
    return size_;
}

const font::character_info* font::get_character_(char32_t c) const {
    for (const auto& info : range_list_) {
        if (c < info.range.first || c > info.range.last)
            continue;

        return &info.data[c - info.range.first];
    }

    if (c != default_code_point_)
        return get_character_(default_code_point_);
    else
        return nullptr;
}

bounds2f font::get_character_uvs(char32_t c) const {
    const character_info* info = get_character_(c);
    if (!info)
        return bounds2f{};

    vector2f top_left     = texture_->get_canvas_uv(info->uvs.top_left(), true);
    vector2f bottom_right = texture_->get_canvas_uv(info->uvs.bottom_right(), true);
    return bounds2f(top_left.x, bottom_right.x, top_left.y, bottom_right.y);
}

bounds2f font::get_character_bounds(char32_t c) const {
    const character_info* info = get_character_(c);
    if (!info)
        return bounds2f{};

    return info->rect;
}

float font::get_character_width(char32_t c) const {
    const character_info* info = get_character_(c);
    if (!info)
        return 0.0f;

    return info->advance;
}

float font::get_character_height(char32_t c) const {
    const character_info* info = get_character_(c);
    if (!info)
        return 0.0f;

    return info->rect.height();
}

float font::get_character_kerning(char32_t, char32_t) const {
    // Note: SDL_ttf does not expose kerning
    return 0.0f;
}

std::weak_ptr<gui::material> font::get_texture() const {
    return texture_;
}

void font::update_texture(std::shared_ptr<gui::material> mat) {
    texture_ = std::static_pointer_cast<sdl::material>(mat);
}

} // namespace lxgui::gui::sdl
