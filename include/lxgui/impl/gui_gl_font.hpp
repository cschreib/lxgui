#ifndef LXGUI_GUI_GL_FONT_HPP
#define LXGUI_GUI_GL_FONT_HPP

#include "lxgui/gui_font.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/utils.hpp"

#include <ft2build.h>
#include <vector>
#include FT_FREETYPE_H

namespace lxgui::gui::gl {

/**
 * \brief A texture containing characters
 * This is the OpenGL implementation of the gui::font.
 * It uses the freetype library to read data from .ttf and
 * .otf files and to render the characters on the font texture.
 */
class font final : public gui::font {
public:
    /**
     * \brief Constructor.
     * \param font_file The name of the font file to read
     * \param size The requested size of the characters (in points)
     * \param outline The thickness of the outline (in points)
     * \param code_points The list of Unicode characters to load
     * \param default_code_point The character to display as fallback
     */
    font(
        const std::string&                   font_file,
        std::size_t                          size,
        std::size_t                          outline,
        const std::vector<code_point_range>& code_points,
        char32_t                             default_code_point);

    /// Destructor.
    ~font() override;

    /**
     * \brief Get the size of the font in pixels.
     * \return The size of the font in pixels
     */
    std::size_t get_size() const override;

    /**
     * \brief Returns the uv coordinates of a character on the texture.
     * \param c The unicode character
     * \return The uv coordinates of this character on the texture
     * \note The uv coordinates are normalized, i.e. they range from
     *       0 to 1. They are arranged as {u1, v1, u2, v2}.
     */
    bounds2f get_character_uvs(char32_t c) const override;

    /**
     * \brief Returns the rect coordinates of a character as it should be drawn relative to the baseline.
     * \param c The unicode character
     * \return The rect coordinates of this character (in pixels, relative to the baseline)
     */
    bounds2f get_character_bounds(char32_t c) const override;

    /**
     * \brief Returns the width of a character in pixels.
     * \param c The unicode character
     * \return The width of the character in pixels.
     */
    float get_character_width(char32_t c) const override;

    /**
     * \brief Returns the height of a character in pixels.
     * \param c The unicode character
     * \return The height of the character in pixels.
     */
    float get_character_height(char32_t c) const override;

    /**
     * \brief Return the kerning amount between two characters.
     * \param c1 The first unicode character
     * \param c2 The second unicode character
     * \return The kerning amount between the two characters
     * \note Kerning is a font rendering adjustment that makes some
     *       letters closer, for example in 'VA', there is room for
     *       the two to be closer than with 'VW'. This has no effect
     *       for fixed width fonts (like Courrier, etc).
     */
    float get_character_kerning(char32_t c1, char32_t c2) const override;

    /**
     * \brief Returns the underlying material to use for rendering.
     * \return The underlying material to use for rendering
     */
    std::weak_ptr<gui::material> get_texture() const override;

    /**
     * \brief Update the material to use for rendering.
     * \param mat The material to use for rendering
     */
    void update_texture(std::shared_ptr<gui::material> mat) override;

private:
    struct character_info {
        char32_t code_point = 0;
        bounds2f uvs;
        bounds2f rect;
        float    advance = 0.0f;
    };

    struct range_info {
        code_point_range            range;
        std::vector<character_info> data;
    };

    const character_info* get_character_(char32_t c) const;

    FT_Face     face_               = nullptr;
    std::size_t size_               = 0u;
    bool        kerning_            = false;
    char32_t    default_code_point_ = 0u;

    std::shared_ptr<gl::material> texture_;
    std::vector<range_info>       range_list_;
};

} // namespace lxgui::gui::gl

#endif
