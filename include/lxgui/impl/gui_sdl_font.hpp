#ifndef LXGUI_GUI_SDL_FONT_HPP
#define LXGUI_GUI_SDL_FONT_HPP

#include "lxgui/impl/gui_sdl_material.hpp"

#include <lxgui/utils.hpp>
#include <lxgui/gui_font.hpp>

#include <vector>

struct SDL_Renderer;

namespace lxgui {
namespace gui {
namespace sdl
{
    struct character_info
    {
        uint   uiCodePoint;
        quad2f mUVs;
    };

    /// A texture containing characters
    /** This is the SDL implementation of the gui::font.
        It uses SDL_ttf to render glyphs and get character data.
    */
    class font final : public gui::font
    {
    public :

        /// Constructor.
        /** \param pRenderer The SDL render to create the font for
        *   \param sFontFile The name of the font file to read
        *   \param uiSize    The requested size of the characters (in points)
        *   \param uiOutline The thickness of the outline (in points)
        *   \param bPreMultipliedAlphaSupported Set to 'true' if the renderer supports pre-multipled alpha
        */
        font(SDL_Renderer* pRenderer, const std::string& sFontFile, uint uiSize, uint uiOutline,
            bool bPreMultipliedAlphaSupported);

        /// Get the size of the font in pixels.
        /** \return The size of the font in pixels
        */
        uint get_size() const override;

        /// Returns the uv coordinates of a character on the texture.
        /** \param uiChar The unicode character
        *   \return The uv coordinates of this character on the texture
        *   \note The uv coordinates are normalised, i.e. they range from
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
        std::weak_ptr<gui::material> get_texture() const override;

        /// Update the material to use for rendering.
        /** \param pMat The material to use for rendering
        */
        void update_texture(std::shared_ptr<gui::material> pMat) override;

    private :

        float fYOffset_ = 0.0f;
        uint uiSize_ = 0u;
        uint uiOutline_ = 0u;

        std::shared_ptr<sdl::material> pTexture_;
        std::vector<character_info>    lCharacterList_;
    };
}
}
}

#endif
