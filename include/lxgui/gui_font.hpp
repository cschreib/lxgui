#ifndef GUI_FONT_HPP
#define GUI_FONT_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_refptr.hpp>
#include <lxgui/utils_wptr.hpp>
#include "lxgui/gui_vector2.hpp"
#include "lxgui/gui_quad2.hpp"

#include <array>

namespace gui
{
    class material;

    /// A texture containing characters
    /** This class is purely virtual. It needs to be implemented
    *   and created by the corresponding gui::manager_impl.
    */
    class font
    {
    public :

        /// Constructor.
        font();

        /// Destructor.
        virtual ~font();

        /// Returns the uv coordinates of a character on the texture.
        /** \param uiChar The unicode character
        *   \return The uv coordinates of this character on the texture
        *   \note The uv coordinates are normalisez, i.e. they range from
        *         0 to 1. They are arranged as {u1, v1, u2, v2}.
        */
        virtual quad2f get_character_uvs(char32_t uiChar) const = 0;

        /// Returns the width of a character in pixels.
        /** \param uiChar The unicode character
        *   \return The width of the character in pixels.
        */
        virtual float get_character_width(char32_t uiChar) const = 0;

        /// Return the kerning amount between two characters.
        /** \param uiChar1 The first unicode character
        *   \param uiChar2 The second unicode character
        *   \return The kerning amount between the two characters
        *   \note Kerning is a font rendering adjustment that makes some
        *         letters closer, for example in 'VA', there is room for
        *         the two to be closer than with 'VW'. This has no effect
        *         for fixed width fonts (like Courrier, etc).
        */
        virtual float get_character_kerning(char32_t uiChar1, char32_t uiChar2) const = 0;

        /// Returns the underlying material to use for rendering.
        /** \return The underlying material to use for rendering
        */
        virtual utils::wptr<material> get_texture() const = 0;
    };
}

#endif
