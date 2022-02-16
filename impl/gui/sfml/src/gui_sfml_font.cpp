#include "lxgui/impl/gui_sfml_font.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_sfml_material.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui::sfml {

font::font(
    const std::string&                   font_file,
    std::size_t                          ui_size,
    std::size_t                          ui_outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             ui_default_code_point) :
    ui_size_(ui_size),
    ui_outline_(ui_outline),
    ui_default_code_point_(ui_default_code_point),
    code_points_(code_points) {
    if (!m_font_.loadFromFile(font_file)) {
        throw gui::exception("gui::sfml::font", "Could not load font file '" + font_file + "'.");
    }

    // Need to request in advance the glyphs that we will use
    // in order for SFLM to draw them on its internal texture
    for (const code_point_range& m_range : code_points_) {
        for (char32_t ui_code_point = m_range.ui_first; ui_code_point <= m_range.ui_last;
             ++ui_code_point) {
            m_font_.getGlyph(ui_code_point, ui_size_, false, ui_outline);
        }
    }

    sf::Image m_data = m_font_.getTexture(ui_size_).copyToImage();
    sfml::material::premultiply_alpha(m_data);
    p_texture_ = std::make_shared<sfml::material>(m_data);
}

std::size_t font::get_size() const {
    return ui_size_;
}

char32_t font::get_character_(char32_t c) const {
    for (const auto& m_range : code_points_) {
        if (c < m_range.ui_first || c > m_range.ui_last)
            continue;

        return c;
    }

    if (c != ui_default_code_point_)
        return get_character_(ui_default_code_point_);
    else
        return 0;
}

bounds2f font::get_character_uvs(char32_t c) const {
    c = get_character_(c);
    if (c == 0)
        return bounds2f{};

    const sf::IntRect& m_sf_rect  = m_font_.getGlyph(c, ui_size_, false, ui_outline_).textureRect;
    const bounds2f&    m_tex_rect = p_texture_->get_rect();

    bounds2f m_rect;
    m_rect.left   = m_sf_rect.left / m_tex_rect.width();
    m_rect.right  = (m_sf_rect.left + m_sf_rect.width) / m_tex_rect.width();
    m_rect.top    = m_sf_rect.top / m_tex_rect.height();
    m_rect.bottom = (m_sf_rect.top + m_sf_rect.height) / m_tex_rect.height();

    vector2f m_top_left     = p_texture_->get_canvas_uv(m_rect.top_left(), true);
    vector2f m_bottom_right = p_texture_->get_canvas_uv(m_rect.bottom_right(), true);
    return bounds2f(m_top_left.x, m_bottom_right.x, m_top_left.y, m_bottom_right.y);
}

bounds2f font::get_character_bounds(char32_t c) const {
    c = get_character_(c);
    if (c == 0)
        return bounds2f{};

#if defined(SFML_HAS_OUTLINE_GLYPH_FIX)
    // This code requires https://github.com/SFML/SFML/pull/1827

    // TODO: this should use the font ascender + descender for fYOffset
    // https://github.com/cschreib/lxgui/issues/97
    const float          fYOffset = uiSize_;
    const sf::FloatRect& mSFRect  = mFont_.getGlyph(uiChar, uiSize_, false, uiOutline_).bounds;

    bounds2f mRect;
    mRect.left   = mSFRect.left;
    mRect.right  = mSFRect.left + mSFRect.width;
    mRect.top    = mSFRect.top + fYOffset;
    mRect.bottom = mSFRect.top + fYOffset + mSFRect.height;
#else
    // TODO: this should use the font ascender + descender for fYOffset
    // https://github.com/cschreib/lxgui/issues/97
    const float          f_y_offset = ui_size_;
    const float          f_offset   = static_cast<float>(ui_outline_);
    const sf::FloatRect& m_sf_rect  = m_font_.getGlyph(c, ui_size_, false, ui_outline_).bounds;

    bounds2f m_rect;
    m_rect.left   = m_sf_rect.left - f_offset;
    m_rect.right  = m_sf_rect.left - f_offset + m_sf_rect.width;
    m_rect.top    = m_sf_rect.top - f_offset + f_y_offset;
    m_rect.bottom = m_sf_rect.top - f_offset + f_y_offset + m_sf_rect.height;
#endif

    return m_rect;
}

float font::get_character_width(char32_t c) const {
    c = get_character_(c);
    if (c == 0)
        return 0.0f;

    return m_font_.getGlyph(c, ui_size_, false, ui_outline_).advance;
}

float font::get_character_height(char32_t c) const {
    c = get_character_(c);
    if (c == 0)
        return 0.0f;

    return m_font_.getGlyph(c, ui_size_, false, ui_outline_).bounds.height;
}

float font::get_character_kerning(char32_t c1, char32_t c2) const {
    c1 = get_character_(c1);
    c2 = get_character_(c2);
    if (c1 == 0 || c2 == 0)
        return 0.0f;

    return m_font_.getKerning(c1, c2, ui_size_);
}

std::weak_ptr<gui::material> font::get_texture() const {
    return p_texture_;
}

void font::update_texture(std::shared_ptr<gui::material> p_mat) {
    p_texture_ = std::static_pointer_cast<sfml::material>(p_mat);
}

} // namespace lxgui::gui::sfml
