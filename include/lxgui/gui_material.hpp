#ifndef LXGUI_GUI_MATERIAL_HPP
#define LXGUI_GUI_MATERIAL_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_quad2.hpp"
#include "lxgui/gui_vector2.hpp"

namespace lxgui {
namespace gui
{
    /// A class that holds rendering data
    /** This is an abstract class that must be implemented
    *   and created by the corresponding gui::renderer.
    */
    class material
    {
    public :

        enum class wrap
        {
            REPEAT,
            CLAMP
        };

        enum class filter
        {
            NONE,
            LINEAR
        };

        /// Constructor.
        material() = default;

        /// Destructor.
        virtual ~material() = default;

        /// Returns the pixel rect in pixels of the canvas containing this texture (if any).
        /** \return The pixel rect in pixels of the canvas containing this texture (if any)
        */
        virtual quad2f get_rect() const = 0;

        /// Returns the physical width in pixels of the canvas containing this texture (if any).
        /** \return The physical width in pixels of the canvas containing this (if any)
        *   \note When a texture is loaded, most of the time it will fill the entire "canvas",
        *         namely, the 2D pixel array containing the texture data. However, some old
        *         hardware don't support textures that have non power-of-two dimensions.
        *         If the user creates a material for such a texture, the gui::renderer will
        *         create a bigger canvas that has power-of-two dimensions, and store the
        *         texture in it. Likewise, if a texture is placed in a wider texture atlas,
        *         the canvas will contain more than one texture.
        */
        virtual float get_canvas_width() const = 0;

        /// Returns the physical height in pixels of the canvas containing this texture (if any).
        /** \return The physical height in pixels of the canvas containing this texture (if any)
        *   \note When a texture is loaded, most of the time it will fill the entire "canvas",
        *         namely, the 2D pixel array containing the texture data. However, some old
        *         hardware don't support textures that have non power-of-two dimensions.
        *         If the user creates a material for such a texture, the gui::renderer will
        *         create a bigger canvas that has power-of-two dimensions, and store the
        *         texture in it. Likewise, if a texture is placed in a wider texture atlas,
        *         the canvas will contain more than one texture.
        */
        virtual float get_canvas_height() const = 0;

        /// Returns normalised UV coordinates on the canvas, given local UV coordinates.
        /** \param mTextureUV      The original UV coordinates, local to this texture
        *   \param bFromNormalized Set to 'true' if input coordinates are normalised to [0,1]
        *                          and 'false' if input coordinates are in pixels
        */
        vector2f get_canvas_uv(const vector2f& mTextureUV, bool bFromNormalized) const;

        /// Returns local UV coordinates on the texture, given canvas UV coordinates.
        /** \param mCanvasUV     The canvas UV coordinates
        *   \param bAsNormalized Set to 'true' if output coordinates should be normalised to [0,1]
        *                        and 'false' if output coordinates should be in pixels
        */
        vector2f get_local_uv(const vector2f& mCanvasUV, bool bAsNormalized) const;
    };
}
}

#endif
