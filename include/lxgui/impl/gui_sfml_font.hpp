#ifndef GUI_SFML_FONT_HPP
#define GUI_SFML_FONT_HPP

#include "lxgui/impl/gui_sfml_material.hpp"

#include <lxgui/utils.hpp>
#include <lxgui/gui_font.hpp>

#include <SFML/Graphics/Font.hpp>

#include <vector>

namespace lxgui {
namespace gui {
namespace sfml
{
    /// A texture containing characters
    /** This is the SFML implementation of the gui::font.
        It uses sf::Font to render glyphs and get character data.
    */
    class font : public gui::font
    {
    public :

        /// Constructor.
        /** \param sFontFile The name of the font file to read
        *   \param uiSize    The requested size in pixels of the font
        */
        font(const std::string& sFontFile, uint uiSize);

        /// Returns the uv coordinates of a character on the texture.
        /** \param uiChar The unicode character
        *   \return The uv coordinates of this character on the texture
        *   \note The uv coordinates are normalisez, i.e. they range from
        *         0 to 1. They are arranged as {u1, v1, u2, v2}.
        */
        quad2f get_character_uvs(char32_t uiChar) const override;

        /// Returns the rect coordinates of a character as it should be drawn relative to the baseline.
        /** \param uiChar The unicode character
        *   \return The rect coordinates of this character (in pixels, relative to the baseline)
        */
        quad2f get_character_bounds(char32_t uiChar) const override;

        /// Returns the width of a character in pixels.
        /** \param uiChar The unicode character
        *   \return The width of the character in pixels.
        */
        float get_character_width(char32_t uiChar) const override;

        /// Returns the height of a character in pixels.
        /** \param uiChar The unicode character
        *   \return The height of the character in pixels.
        */
        float get_character_height(char32_t uiChar) const override;

        /// Return the kerning amount between two characters.
        /** \param uiChar1 The first unicode character
        *   \param uiChar2 The second unicode character
        *   \return The kerning amount between the two characters
        *   \note Kerning is a font rendering adjustment that makes some
        *         letters closer, for example in 'VA', there is room for
        *         the two to be closer than with 'VW'. This has no effect
        *         for fixed width fonts (like Courrier, etc).
        */
        float get_character_kerning(char32_t uiChar1, char32_t uiChar2) const override;

        /// Returns the underlying material to use for rendering.
        /** \return The underlying material to use for rendering
        */
        utils::wptr<gui::material> get_texture() const override;

    private :

        sf::Font                      mFont_;
        uint                          uiSize_;
        uint                          uiSizeSFML_;
        utils::refptr<sfml::material> pTexture_;
    };
}
}
}

#endif
