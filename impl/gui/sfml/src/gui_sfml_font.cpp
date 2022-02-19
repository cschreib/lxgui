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
    if (!font_.loadFromFile(font_file)) {
        throw gui::exception("gui::sfml::font", "Could not load font file '" + font_file + "'.");
    }

    // Need to request in advance the glyphs that we will use
    // in order for SFLM to draw them on its internal texture
    for (const code_point_range& range : code_points_) {
        for (char32_t ui_code_point = range.ui_first; ui_code_point <= range.ui_last;
             ++ui_code_point) {
            font_.getGlyph(ui_code_point, ui_size_, false, ui_outline);
        }
    }

    sf::Image data = font_.getTexture(ui_size_).copyToImage();
    sfml::material::premultiply_alpha(data);
    p_texture_ = std::make_shared<sfml::material>(data);
}

std::size_t font::get_size() const {
    return ui_size_;
}

char32_t font::get_character_(char32_t c) const {
    for (const auto& range : code_points_) {
        if (c < range.ui_first || c > range.ui_last)
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

    const sf::IntRect& sf_rect  = font_.getGlyph(c, ui_size_, false, ui_outline_).textureRect;
    const bounds2f&    tex_rect = p_texture_->get_rect();

    bounds2f rect;
    rect.left   = sf_rect.left / tex_rect.width();
    rect.right  = (sf_rect.left + sf_rect.width) / tex_rect.width();
    rect.top    = sf_rect.top / tex_rect.height();
    rect.bottom = (sf_rect.top + sf_rect.height) / tex_rect.height();

    vector2f top_left     = p_texture_->get_canvas_uv(rect.top_left(), true);
    vector2f bottom_right = p_texture_->get_canvas_uv(rect.bottom_right(), true);
    return bounds2f(top_left.x, bottom_right.x, top_left.y, bottom_right.y);
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
    const sf::FloatRect& mSFRect  = mFont_.getGlyph(c, uiSize_, false, uiOutline_).bounds;

    bounds2f mRect;
    mRect.left   = mSFRect.left;
    mRect.right  = mSFRect.left + mSFRect.width;
    mRect.top    = mSFRect.top + fYOffset;
    mRect.bottom = mSFRect.top + fYOffset + mSFRect.height;
#else
    // TODO: this should use the font ascender + descender for fYOffset
    // https://github.com/cschreib/lxgui/issues/97
    const float          y_offset = ui_size_;
    const float          offset   = static_cast<float>(ui_outline_);
    const sf::FloatRect& sf_rect  = font_.getGlyph(c, ui_size_, false, ui_outline_).bounds;

    bounds2f rect;
    rect.left   = sf_rect.left - offset;
    rect.right  = sf_rect.left - offset + sf_rect.width;
    rect.top    = sf_rect.top - offset + y_offset;
    rect.bottom = sf_rect.top - offset + y_offset + sf_rect.height;
#endif

    return rect;
}

float font::get_character_width(char32_t c) const {
    c = get_character_(c);
    if (c == 0)
        return 0.0f;

    return font_.getGlyph(c, ui_size_, false, ui_outline_).advance;
}

float font::get_character_height(char32_t c) const {
    c = get_character_(c);
    if (c == 0)
        return 0.0f;

    return font_.getGlyph(c, ui_size_, false, ui_outline_).bounds.height;
}

float font::get_character_kerning(char32_t c1, char32_t c2) const {
    c1 = get_character_(c1);
    c2 = get_character_(c2);
    if (c1 == 0 || c2 == 0)
        return 0.0f;

    return font_.getKerning(c1, c2, ui_size_);
}

std::weak_ptr<gui::material> font::get_texture() const {
    return p_texture_;
}

void font::update_texture(std::shared_ptr<gui::material> p_mat) {
    p_texture_ = std::static_pointer_cast<sfml::material>(p_mat);
}

} // namespace lxgui::gui::sfml
