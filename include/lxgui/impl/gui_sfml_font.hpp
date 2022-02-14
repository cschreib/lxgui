#ifndef LXGUI_GUI_SFML_FONT_HPP
#define LXGUI_GUI_SFML_FONT_HPP

#include "lxgui/gui_font.hpp"
#include "lxgui/impl/gui_sfml_material.hpp"
#include "lxgui/utils.hpp"

#include <SFML/Graphics/Font.hpp>
#include <vector>

namespace lxgui::gui::sfml {

/// A texture containing characters
/** This is the SFML implementation of the gui::font.
    It uses sf::Font to render glyphs and get character data.
*/
class font final : public gui::font {
public:
    /// Constructor.
    /** \param sFontFile   The name of the font file to read
     *   \param uiSize      The requested size of the characters (in points)
     *   \param uiOutline   The thickness of the outline (in points)
     *   \param code_points The list of Unicode characters to load
     *   \param uiDefaultCodePoint The character to display as fallback
     */
    font(
        const std::string&                   s_font_file,
        std::size_t                          ui_size,
        std::size_t                          ui_outline,
        const std::vector<code_point_range>& code_points,
        char32_t                             ui_default_code_point);

    /// Get the size of the font in pixels.
    /** \return The size of the font in pixels
     */
    std::size_t get_size() const override;

    /// Returns the uv coordinates of a character on the texture.
    /** \param uiChar The unicode character
     *   \return The uv coordinates of this character on the texture
     *   \note The uv coordinates are normalised, i.e. they range from
     *         0 to 1. They are arranged as {u1, v1, u2, v2}.
     */
    bounds2f get_character_uvs(char32_t ui_char) const override;

    /// Returns the rect coordinates of a character as it should be drawn relative to the baseline.
    /** \param uiChar The unicode character
     *   \return The rect coordinates of this character (in pixels, relative to the baseline)
     */
    bounds2f get_character_bounds(char32_t ui_char) const override;

    /// Returns the width of a character in pixels.
    /** \param uiChar The unicode character
     *   \return The width of the character in pixels.
     */
    float get_character_width(char32_t ui_char) const override;

    /// Returns the height of a character in pixels.
    /** \param uiChar The unicode character
     *   \return The height of the character in pixels.
     */
    float get_character_height(char32_t ui_char) const override;

    /// Return the kerning amount between two characters.
    /** \param uiChar1 The first unicode character
     *   \param uiChar2 The second unicode character
     *   \return The kerning amount between the two characters
     *   \note Kerning is a font rendering adjustment that makes some
     *         letters closer, for example in 'VA', there is room for
     *         the two to be closer than with 'VW'. This has no effect
     *         for fixed width fonts (like Courrier, etc).
     */
    float get_character_kerning(char32_t ui_char1, char32_t ui_char2) const override;

    /// Returns the underlying material to use for rendering.
    /** \return The underlying material to use for rendering
     */
    std::weak_ptr<gui::material> get_texture() const override;

    /// Update the material to use for rendering.
    /** \param pMat The material to use for rendering
     */
    void update_texture(std::shared_ptr<gui::material> p_mat) override;

private:
    char32_t get_character_(char32_t ui_char) const;

    sf::Font    m_font_;
    std::size_t ui_size_               = 0u;
    std::size_t ui_outline_            = 0u;
    char32_t    ui_default_code_point_ = 0u;

    std::shared_ptr<sfml::material> p_texture_;
    std::vector<code_point_range>   code_points_;
};

} // namespace lxgui::gui::sfml

#endif
