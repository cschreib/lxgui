#include "lxgui/impl/gui_gl_font.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/utils_filesystem.hpp"
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
thread_local std::size_t ui_ft_count = 0u;
thread_local FT_Library  m_shared_ft = nullptr;

FT_Library get_freetype() {
    if (ui_ft_count == 0u) {
        if (FT_Init_FreeType(&m_shared_ft))
            throw lxgui::gui::exception("gui::gl::font", "Error initializing FreeType !");
    }

    ++ui_ft_count;
    return m_shared_ft;
}

void release_freetype() {
    if (ui_ft_count != 0u) {
        --ui_ft_count;
        if (ui_ft_count == 0u)
            FT_Done_FreeType(m_shared_ft);
    }
}

} // namespace

font::font(
    const std::string&                   font_file,
    std::size_t                          ui_size,
    std::size_t                          ui_outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             ui_default_code_point) :
    ui_size_(ui_size), ui_default_code_point_(ui_default_code_point) {
    // NOTE : Code inspired from Ogre::Font, from the OGRE3D graphics engine
    // http://www.ogre3d.org
    // ... and SFML
    // https://www.sfml-dev.org
    //
    // Some tweaking has been done to improve the text quality :
    //  - Disable hinting (FT_LOAD_NO_HINTING)
    //  - Character width is calculated as : max(x_bearing + width, advance),
    //    since advance sometimes doesn't cover the whole glyph
    //    (typical example is the 'w' character, in Consolas:9).

    if (!utils::file_exists(font_file))
        throw gui::exception("gui::gl::font", "Cannot find file \"" + font_file + "\".");

    FT_Library m_ft      = get_freetype();
    FT_Stroker m_stroker = nullptr;
    FT_Glyph   m_glyph   = nullptr;

    try {
        // Add some space between letters to prevent artifacts
        const std::size_t ui_spacing = 1;

        if (FT_New_Face(m_ft, font_file.c_str(), 0, &m_face_) != 0) {
            throw gui::exception(
                "gui::gl::font", "Error loading font : \"" + font_file + "\" : cannot load face.");
        }

        if (ui_outline > 0) {
            if (FT_Stroker_New(m_ft, &m_stroker) != 0) {
                throw gui::exception(
                    "gui::gl::font",
                    "Error loading font : \"" + font_file + "\" : cannot create stroker.");
            }
        }

        if (FT_Select_Charmap(m_face_, FT_ENCODING_UNICODE) != 0) {
            throw gui::exception(
                "gui::gl::font", "Error loading font : \"" + font_file +
                                     "\" : cannot select Unicode character map.");
        }

        if (FT_Set_Pixel_Sizes(m_face_, 0, ui_size) != 0) {
            throw gui::exception(
                "gui::gl::font",
                "Error loading font : \"" + font_file + "\" : cannot set font size.");
        }

        FT_Int32 load_flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_HINTING;
        if (ui_outline != 0)
            load_flags |= FT_LOAD_NO_BITMAP;

        // Calculate maximum width, height and bearing
        std::size_t ui_max_height = 0, ui_max_width = 0;
        std::size_t ui_num_char = 0;
        for (const code_point_range& m_range : code_points) {
            for (char32_t ui_code_point = m_range.ui_first; ui_code_point <= m_range.ui_last;
                 ++ui_code_point) {
                if (FT_Load_Char(m_face_, ui_code_point, load_flags) != 0)
                    continue;

                if (FT_Get_Glyph(m_face_->glyph, &m_glyph) != 0)
                    continue;

                if (m_glyph->format == FT_GLYPH_FORMAT_OUTLINE && ui_outline > 0) {
                    FT_Stroker_Set(
                        m_stroker, ft_fixed<6>(ui_outline), FT_STROKER_LINECAP_ROUND,
                        FT_STROKER_LINEJOIN_ROUND, 0);
                    FT_Glyph_StrokeBorder(&m_glyph, m_stroker, false, true);
                }

                FT_Glyph_To_Bitmap(&m_glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
                const FT_Bitmap& m_bitmap = reinterpret_cast<FT_BitmapGlyph>(m_glyph)->bitmap;

                if (m_bitmap.rows > ui_max_height)
                    ui_max_height = m_bitmap.rows;

                if (m_bitmap.width > ui_max_width)
                    ui_max_width = m_bitmap.width;

                ++ui_num_char;

                FT_Done_Glyph(m_glyph);
                m_glyph = nullptr;
            }
        }

        ui_max_height = ui_max_height + 2 * ui_outline;
        ui_max_width  = ui_max_width + 2 * ui_outline;

        // Calculate the size of the texture
        std::size_t ui_tex_size =
            (ui_max_width + ui_spacing) * (ui_max_height + ui_spacing) * ui_num_char;
        std::size_t ui_tex_side =
            static_cast<std::size_t>(std::sqrt(static_cast<float>(ui_tex_size)));

        // Add a bit of overhead since we won't be able to tile this area perfectly
        ui_tex_side += std::max(ui_max_width, ui_max_height);
        ui_tex_size = ui_tex_side * ui_tex_side;

        // Round up to nearest power of two
        {
            std::size_t i = 1;
            while (ui_tex_side > i)
                i *= 2;
            ui_tex_side = i;
        }

        // Set up area as square
        std::size_t ui_final_width  = ui_tex_side;
        std::size_t ui_final_height = ui_tex_side;

        // Reduce height if we don't actually need a square
        if (ui_final_width * ui_final_height / 2 >= ui_tex_size)
            ui_final_height = ui_final_height / 2;

        std::vector<ub32color> data(ui_final_width * ui_final_height);
        std::fill(data.begin(), data.end(), ub32color(0, 0, 0, 0));

        std::size_t x = 0, y = 0;

        if (FT_HAS_KERNING(m_face_))
            b_kerning_ = true;

        float f_y_offset = 0.0f;
        if (FT_IS_SCALABLE(m_face_)) {
            FT_Fixed m_scale = m_face_->size->metrics.y_scale;
            f_y_offset       = ft_ceil<6>(FT_MulFix(m_face_->ascender, m_scale)) +
                         ft_ceil<6>(FT_MulFix(m_face_->descender, m_scale));
        } else {
            f_y_offset = ft_ceil<6>(m_face_->size->metrics.ascender) +
                         ft_ceil<6>(m_face_->size->metrics.descender);
        }

        for (const code_point_range& m_range : code_points) {
            range_info m_info;
            m_info.m_range = m_range;
            m_info.data.resize(m_range.ui_last - m_range.ui_first + 1);

            for (char32_t ui_code_point = m_range.ui_first; ui_code_point <= m_range.ui_last;
                 ++ui_code_point) {
                character_info& m_ci = m_info.data[ui_code_point - m_range.ui_first];
                m_ci.ui_code_point   = ui_code_point;

                if (FT_Load_Char(m_face_, ui_code_point, load_flags) != 0) {
                    gui::out << gui::warning << "gui::gl::font : Cannot load character "
                             << ui_code_point << " in font \"" << font_file << "\"." << std::endl;
                    continue;
                }

                if (FT_Get_Glyph(m_face_->glyph, &m_glyph) != 0) {
                    gui::out << gui::warning << "gui::gl::font : Cannot get glyph for character "
                             << ui_code_point << " in font \"" << font_file << "\"." << std::endl;
                    continue;
                }

                if (m_glyph->format == FT_GLYPH_FORMAT_OUTLINE && ui_outline > 0) {
                    FT_Stroker_Set(
                        m_stroker, ft_fixed<6>(ui_outline), FT_STROKER_LINECAP_ROUND,
                        FT_STROKER_LINEJOIN_ROUND, 0);
                    FT_Glyph_Stroke(&m_glyph, m_stroker, true);
                }

                // Warning: after this line, do not use mGlyph! Use mBitmapGlyph.root
                FT_Glyph_To_Bitmap(&m_glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
                FT_BitmapGlyph m_bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(m_glyph);

                const FT_Bitmap& m_bitmap = m_bitmap_glyph->bitmap;

                // If at end of row, jump to next line
                if (x + m_bitmap.width > ui_final_width - 1) {
                    y += ui_max_height + ui_spacing;
                    x = 0;
                }

                // Some characters do not have a bitmap, like white spaces.
                // This is legal, and we should just have blank geometry for them.
                const ub32color::chanel* buffer = m_bitmap.buffer;
                if (buffer) {
                    for (std::size_t j = 0; j < m_bitmap.rows; ++j) {
                        std::size_t ui_row_offset = (y + j) * ui_final_width + x;
                        for (std::size_t i = 0; i < m_bitmap.width; ++i, ++buffer)
                            data[i + ui_row_offset] = ub32color(255, 255, 255, *buffer);
                    }
                }

                m_ci.m_uvs.left   = x / float(ui_final_width);
                m_ci.m_uvs.top    = y / float(ui_final_height);
                m_ci.m_uvs.right  = (x + m_bitmap.width) / float(ui_final_width);
                m_ci.m_uvs.bottom = (y + m_bitmap.rows) / float(ui_final_height);

                m_ci.m_rect.left   = m_bitmap_glyph->left;
                m_ci.m_rect.right  = m_ci.m_rect.left + m_bitmap.width;
                m_ci.m_rect.top    = f_y_offset - m_bitmap_glyph->top;
                m_ci.m_rect.bottom = m_ci.m_rect.top + m_bitmap.rows;

                m_ci.f_advance = ft_round<16>(m_bitmap_glyph->root.advance.x);

                // Advance a column
                x += m_bitmap.width + ui_spacing;

                FT_Done_Glyph(m_glyph);
                m_glyph = nullptr;
            }

            range_list_.push_back(std::move(m_info));
        }

        FT_Stroker_Done(m_stroker);

        gl::material::premultiply_alpha(data);

        p_texture_ = std::make_shared<gl::material>(vector2ui(ui_final_width, ui_final_height));
        p_texture_->update_texture(data.data());
    } catch (...) {
        if (m_glyph)
            FT_Done_Glyph(m_glyph);
        if (m_stroker)
            FT_Stroker_Done(m_stroker);
        if (m_face_)
            FT_Done_Face(m_face_);
        release_freetype();
        throw;
    }
}

font::~font() {
    if (m_face_)
        FT_Done_Face(m_face_);
    release_freetype();
}

std::size_t font::get_size() const {
    return ui_size_;
}

const font::character_info* font::get_character_(char32_t c) const {
    for (const auto& m_info : range_list_) {
        if (c < m_info.m_range.ui_first || c > m_info.m_range.ui_last)
            continue;

        return &m_info.data[c - m_info.m_range.ui_first];
    }

    if (c != ui_default_code_point_)
        return get_character_(ui_default_code_point_);
    else
        return nullptr;
}

bounds2f font::get_character_uvs(char32_t c) const {
    const character_info* p_char = get_character_(c);
    if (!p_char)
        return bounds2f{};

    vector2f m_top_left     = p_texture_->get_canvas_uv(p_char->m_uvs.top_left(), true);
    vector2f m_bottom_right = p_texture_->get_canvas_uv(p_char->m_uvs.bottom_right(), true);
    return bounds2f(m_top_left.x, m_bottom_right.x, m_top_left.y, m_bottom_right.y);
}

bounds2f font::get_character_bounds(char32_t c) const {
    const character_info* p_char = get_character_(c);
    if (!p_char)
        return bounds2f{};

    return p_char->m_rect;
}

float font::get_character_width(char32_t c) const {
    const character_info* p_char = get_character_(c);
    if (!p_char)
        return 0.0f;

    return p_char->f_advance;
}

float font::get_character_height(char32_t c) const {
    const character_info* p_char = get_character_(c);
    if (!p_char)
        return 0.0f;

    return p_char->m_rect.height();
}

float font::get_character_kerning(char32_t c1, char32_t c2) const {
    if (b_kerning_) {
        FT_Vector m_kerning;
        FT_UInt   ui_prev = FT_Get_Char_Index(m_face_, c1);
        FT_UInt   ui_next = FT_Get_Char_Index(m_face_, c2);
        if (FT_Get_Kerning(m_face_, ui_prev, ui_next, FT_KERNING_UNFITTED, &m_kerning) != 0)
            return ft_round<6>(m_kerning.x);
        else
            return 0.0f;
    } else
        return 0.0f;
}

std::weak_ptr<gui::material> font::get_texture() const {
    return p_texture_;
}

void font::update_texture(std::shared_ptr<gui::material> p_mat) {
    p_texture_ = std::static_pointer_cast<gl::material>(p_mat);
}

} // namespace lxgui::gui::gl
