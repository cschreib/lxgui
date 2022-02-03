#ifndef LXGUI_GUI_MATERIAL_HPP
#define LXGUI_GUI_MATERIAL_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_bounds2.hpp"
#include "lxgui/gui_vector2.hpp"

namespace lxgui {
namespace gui
{
    struct ub32color
    {
        using chanel = unsigned char;

        ub32color() = default;
        ub32color(chanel tr, chanel tg, chanel tb, chanel ta) : r(tr), g(tg), b(tb), a(ta) {}
        chanel r, g, b, a;
    };

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
        material(bool bIsAtlas);

        /// Destructor.
        virtual ~material() = default;

        /// Non-copiable
        material(const material&) = delete;

        /// Non-movable
        material(material&&) = delete;

        /// Non-copiable
        material& operator=(const material&) = delete;

        /// Non-movable
        material& operator=(material&&) = delete;

        /// Returns the pixel rect in pixels of the canvas containing this texture (if any).
        /** \return The pixel rect in pixels of the canvas containing this texture (if any)
        */
        virtual bounds2f get_rect() const = 0;

        /// Returns the physical dimensions (in pixels) of the canvas containing this texture (if any).
        /** \return The physical dimensions (in pixels) of the canvas containing this (if any)
        *   \note When a texture is loaded, most of the time it will fill the entire "canvas",
        *         namely, the 2D pixel array containing the texture data. However, some old
        *         hardware don't support textures that have non power-of-two dimensions.
        *         If the user creates a material for such a texture, the gui::renderer will
        *         create a bigger canvas that has power-of-two dimensions, and store the
        *         texture in it. Likewise, if a texture is placed in a wider texture atlas,
        *         the canvas will contain more than one texture.
        */
        virtual vector2ui get_canvas_dimensions() const = 0;

        /// Checks if another material is based on the same texture as the current material.
        /** \return 'true' if both materials use the same texture, 'false' otherwise
        */
        virtual bool uses_same_texture(const material& mOther) const = 0;

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

        /// Checks if the material is embedded in an atlas.
        /** \return 'true' if the material is inside an atlas, 'false' otherwise.
        */
        bool is_in_atlas() const;

    protected:

        bool bIsAtlas_ = false;
    };
}
}

#endif
