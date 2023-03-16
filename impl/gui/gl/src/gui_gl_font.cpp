#include "lxgui/impl/gui_gl_font.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/utils_file_system.hpp"
#include "lxgui/utils_string.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_STROKER_H

// Convert fixed point to floating point
template<std::size_t Point, typename T>
float ft_float(T value) {
    return static_cast<float>(value) / static_cast<float>(1 << Point);
}

// Convert integer or floating point to fixed point
template<std::size_t Point, typename T>
FT_Fixed ft_fixed(T value) {
    return static_cast<FT_Fixed>(
        std::round(static_cast<float>(value) * static_cast<float>(1 << Point)));
}

// Convert fixed point to integer pixels
template<std::size_t Point, typename T>
T ft_floor(T value) {
    return (value & -(1 << Point)) / (1 << Point);
}

template<std::size_t Point, typename T>
T ft_ceil(T value) {
    return ft_floor<Point>(value + (1 << Point) - 1);
}

template<std::size_t Point, typename T>
T ft_round(T value) {
    return std::round(ft_float<Point>(value));
}

namespace lxgui::gui::gl {

namespace {

// Global state for the Freetype library (one per thread)
thread_local std::size_t ft_count  = 0u;
thread_local FT_Library  shared_ft = nullptr;

FT_Library get_freetype() {
    if (ft_count == 0u) {
        if (FT_Init_FreeType(&shared_ft))
            throw lxgui::gui::exception("gui::gl::font", "Error initializing FreeType !");
    }

    ++ft_count;
    return shared_ft;
}

void release_freetype() {
    if (ft_count != 0u) {
        --ft_count;
        if (ft_count == 0u)
            FT_Done_FreeType(shared_ft);
    }
}

} // namespace

font::font(
    const std::string&                   font_file,
    std::size_t                          size,
    std::size_t                          outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             default_code_point) :
    size_(size), default_code_point_(default_code_point) {
    // NOTE: Code inspired from Ogre::Font, from the OGRE3D graphics engine
    // http://www.ogre3d.org
    // ... and SFML
    // https://www.sfml-dev.org
    //
    // Some tweaking has been done to improve the text quality:
    //  - Disable hinting (FT_LOAD_NO_HINTING)
    //  - Character width is calculated as: max(x_bearing + width, advance),
    //    since advance sometimes doesn't cover the whole glyph
    //    (typical example is the 'w' character, in Consolas:9).

    if (!utils::file_exists(font_file))
        throw gui::exception("gui::gl::font", "Cannot find file \"" + font_file + "\".");

    FT_Library ft      = get_freetype();
    FT_Stroker stroker = nullptr;
    FT_Glyph   glyph   = nullptr;

    try {
        // Add some space between letters to prevent artifacts
        const std::size_t spacing = 1;

        if (FT_New_Face(ft, font_file.c_str(), 0, &face_) != 0) {
            throw gui::exception(
                "gui::gl::font", "Error loading font: \"" + font_file + "\": cannot load face.");
        }

        if (outline > 0) {
            if (FT_Stroker_New(ft, &stroker) != 0) {
                throw gui::exception(
                    "gui::gl::font",
                    "Error loading font: \"" + font_file + "\": cannot create stroker.");
            }
        }

        if (FT_Select_Charmap(face_, FT_ENCODING_UNICODE) != 0) {
            throw gui::exception(
                "gui::gl::font",
                "Error loading font: \"" + font_file + "\": cannot select Unicode character map.");
        }

        if (FT_Set_Pixel_Sizes(face_, 0, size) != 0) {
            throw gui::exception(
                "gui::gl::font",
                "Error loading font: \"" + font_file + "\": cannot set font size.");
        }

        FT_Int32 load_flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_HINTING;
        if (outline != 0)
            load_flags |= FT_LOAD_NO_BITMAP;

        // Calculate maximum width, height and bearing
        std::size_t max_height = 0, max_width = 0;
        std::size_t num_char = 0;
        for (const code_point_range& range : code_points) {
            for (char32_t code_point = range.first; code_point <= range.last; ++code_point) {
                if (FT_Load_Char(face_, code_point, load_flags) != 0)
                    continue;

                if (FT_Get_Glyph(face_->glyph, &glyph) != 0)
                    continue;

                if (glyph->format == FT_GLYPH_FORMAT_OUTLINE && outline > 0) {
                    FT_Stroker_Set(
                        stroker, ft_fixed<6>(outline), FT_STROKER_LINECAP_ROUND,
                        FT_STROKER_LINEJOIN_ROUND, 0);
                    FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
                }

                FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
                const FT_Bitmap& bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph)->bitmap;

                if (bitmap.rows > max_height)
                    max_height = bitmap.rows;

                if (bitmap.width > max_width)
                    max_width = bitmap.width;

                ++num_char;

                FT_Done_Glyph(glyph);
                glyph = nullptr;
            }
        }

        max_height = max_height + 2 * outline;
        max_width  = max_width + 2 * outline;

        // Calculate the size of the texture
        std::size_t tex_size = (max_width + spacing) * (max_height + spacing) * num_char;
        std::size_t tex_side = static_cast<std::size_t>(std::sqrt(static_cast<float>(tex_size)));

        // Add a bit of overhead since we won't be able to tile this area perfectly
        tex_side += std::max(max_width, max_height);
        tex_size = tex_side * tex_side;

        // Round up to nearest power of two
        {
            std::size_t i = 1;
            while (tex_side > i)
                i *= 2;
            tex_side = i;
        }

        // Set up area as square
        std::size_t final_width  = tex_side;
        std::size_t final_height = tex_side;

        // Reduce height if we don't actually need a square
        if (final_width * final_height / 2 >= tex_size)
            final_height = final_height / 2;

        std::vector<color32> data(final_width * final_height);
        std::fill(data.begin(), data.end(), color32{0, 0, 0, 0});

        std::size_t x = 0, y = 0;

        if (FT_HAS_KERNING(face_))
            kerning_ = true;

        float y_offset = 0.0f;
        if (FT_IS_SCALABLE(face_)) {
            FT_Fixed scale = face_->size->metrics.y_scale;
            y_offset       = ft_ceil<6>(FT_MulFix(face_->ascender, scale)) +
                       ft_ceil<6>(FT_MulFix(face_->descender, scale));
        } else {
            y_offset = ft_ceil<6>(face_->size->metrics.ascender) +
                       ft_ceil<6>(face_->size->metrics.descender);
        }

        for (const code_point_range& range : code_points) {
            range_info info;
            info.range = range;
            info.data.resize(range.last - range.first + 1);

            for (char32_t code_point = range.first; code_point <= range.last; ++code_point) {
                character_info& ci = info.data[code_point - range.first];
                ci.code_point      = code_point;

                if (FT_Load_Char(face_, code_point, load_flags) != 0) {
                    gui::out << gui::warning << "gui::gl::font: Cannot load character "
                             << code_point << " in font \"" << font_file << "\"." << std::endl;
                    continue;
                }

                if (FT_Get_Glyph(face_->glyph, &glyph) != 0) {
                    gui::out << gui::warning << "gui::gl::font: Cannot get glyph for character "
                             << code_point << " in font \"" << font_file << "\"." << std::endl;
                    continue;
                }

                if (glyph->format == FT_GLYPH_FORMAT_OUTLINE && outline > 0) {
                    FT_Stroker_Set(
                        stroker, ft_fixed<6>(outline), FT_STROKER_LINECAP_ROUND,
                        FT_STROKER_LINEJOIN_ROUND, 0);
                    FT_Glyph_Stroke(&glyph, stroker, true);
                }

                // Warning: after this line, do not use glyph! Use bitmap_glyph.root
                FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
                FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(glyph);

                const FT_Bitmap& bitmap = bitmap_glyph->bitmap;

                // If at end of row, jump to next line
                if (x + bitmap.width > final_width - 1) {
                    y += max_height + spacing;
                    x = 0;
                }

                // Some characters do not have a bitmap, like white spaces.
                // This is legal, and we should just have blank geometry for them.
                const color32::chanel* buffer = bitmap.buffer;
                if (buffer) {
                    for (std::size_t j = 0; j < bitmap.rows; ++j) {
                        std::size_t row_offset = (y + j) * final_width + x;
                        for (std::size_t i = 0; i < bitmap.width; ++i, ++buffer)
                            data[i + row_offset] = color32{255, 255, 255, *buffer};
                    }
                }

                ci.uvs.left   = x / float(final_width);
                ci.uvs.top    = y / float(final_height);
                ci.uvs.right  = (x + bitmap.width) / float(final_width);
                ci.uvs.bottom = (y + bitmap.rows) / float(final_height);

                ci.rect.left   = bitmap_glyph->left;
                ci.rect.right  = ci.rect.left + bitmap.width;
                ci.rect.top    = y_offset - bitmap_glyph->top;
                ci.rect.bottom = ci.rect.top + bitmap.rows;

                ci.advance = ft_round<16>(bitmap_glyph->root.advance.x);

                // Advance a column
                x += bitmap.width + spacing;

                FT_Done_Glyph(glyph);
                glyph = nullptr;
            }

            range_list_.push_back(std::move(info));
        }

        FT_Stroker_Done(stroker);

        gl::material::premultiply_alpha(data);

        texture_ = std::make_shared<gl::material>(vector2ui(final_width, final_height));
        texture_->update_texture(data.data());
    } catch (...) {
        if (glyph)
            FT_Done_Glyph(glyph);
        if (stroker)
            FT_Stroker_Done(stroker);
        if (face_)
            FT_Done_Face(face_);
        release_freetype();
        throw;
    }
}

font::~font() {
    if (face_)
        FT_Done_Face(face_);
    release_freetype();
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

float font::get_character_kerning(char32_t c1, char32_t c2) const {
    if (kerning_) {
        FT_Vector kerning;
        FT_UInt   prev = FT_Get_Char_Index(face_, c1);
        FT_UInt   next = FT_Get_Char_Index(face_, c2);
        if (FT_Get_Kerning(face_, prev, next, FT_KERNING_UNFITTED, &kerning) != 0)
            return ft_round<6>(kerning.x);
        else
            return 0.0f;
    } else
        return 0.0f;
}

std::weak_ptr<gui::material> font::get_texture() const {
    return texture_;
}

void font::update_texture(std::shared_ptr<gui::material> mat) {
    texture_ = std::static_pointer_cast<gl::material>(mat);
}

} // namespace lxgui::gui::gl
