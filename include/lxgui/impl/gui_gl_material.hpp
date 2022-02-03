#ifndef LXGUI_GUI_GL_MATERIAL_HPP
#define LXGUI_GUI_GL_MATERIAL_HPP

#include "lxgui/utils.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_bounds2.hpp"

#include <vector>
#include <memory>

namespace lxgui {
namespace gui {
namespace gl
{
    /// A class that holds rendering data
    /** This implementation can contain either a plain color
    *   or a real OpenGL texture. It is also used by the
    *   gui::gl::render_target class to store the output data.
    */
    class material final : public gui::material
    {
    public :

        /// Constructor for textures.
        /** \param mDimensions The requested texture dimensions
        *   \param mWrap       How to adjust texture coordinates that are outside the [0,1] range
        *   \param mFilter     Use texture filtering or not (see set_filter())
        */
        material(const vector2ui& mDimensions, wrap mWrap = wrap::REPEAT,
            filter mFilter = filter::NONE);

        /// Constructor for atlas textures.
        /** \param uiTextureHandle   The handle to the texture object of the atlas
        *   \param mCanvasDimensions The dimensions of the texture atlas
        *   \param mRect             The position of this texture inside the atlas
        *   \param mFilter           Use texture filtering or not (see set_filter())
        */
        material(std::uint32_t uiTextureHandle, const vector2ui& mCanvasDimensions,
            const bounds2f mRect, filter mFilter = filter::NONE);

        material(const material& tex) = delete;
        material(material&& tex) = delete;
        material& operator = (const material& tex) = delete;
        material& operator = (material&& tex) = delete;

        /// Destructor.
        ~material() override;

        /// Returns the pixel rect in pixels of the canvas containing this texture (if any).
        /** \return The pixel rect in pixels of the canvas containing this texture (if any)
        */
        bounds2f get_rect() const override;

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
        vector2ui get_canvas_dimensions() const override;

        /// Checks if another material is based on the same texture as the current material.
        /** \return 'true' if both materials use the same texture, 'false' otherwise
        */
        bool uses_same_texture(const gui::material& mOther) const override;

        /// Resizes this texture.
        /** \param mDimensions The new texture dimensions
        *   \return 'true' if the function had to re-create a new texture object
        *   \note All the previous data that was stored in this texture will be lost.
        */
        bool set_dimensions(const vector2ui& mDimensions);

        /// Premultiplies the texture by alpha component.
        /** \param lData The pixel data to pre-multiply
        *   \note Premultiplied alpha is a rendering technique that allows perfect
        *         alpha blending when using render targets.
        */
        static void premultiply_alpha(std::vector<ub32color>& lData);

        /// Sets the wrap mode of this texture.
        /** \param mWrap How to adjust texture coordinates that are outside the [0,1] range
        */
        void set_wrap(wrap mWrap);

        /// Sets the filter mode of this texture.
        /** \param mFilter Use texture filtering or not
        *   \note When texture filtering is disabled, enlarged textures get pixelated.
        *         Else, the GPU uses an averaging algorithm to blur the pixels.
        */
        void set_filter(filter mFilter);

        /// Returns the filter mode of this texture.
        /** \return The filter mode of this texture
        */
        filter get_filter() const;

        /// Sets this material as the active one.
        void bind() const;

        /// Updates the texture that is in GPU memory.
        /** \param pData The new pixel data
        */
        void update_texture(const ub32color* pData);

        /// Returns the OpenGL texture handle.
        /** \note For internal use.
        */
        std::uint32_t get_handle_() const;

        /// Checks if the machine is capable of using some features.
        /** \note The function checks for non power of two capability.
        *         If the graphics card doesn't support it, the material
        *         class will automatically create power of two textures.
        */
        static void check_availability();

        /// Returns the maximum size available for a texture, in pixels.
        /** \return The maximum size available for a texture, in pixels
        */
        static std::size_t get_max_size();

    private:

        vector2ui     mCanvasDimensions_;
        wrap          mWrap_ = wrap::REPEAT;
        filter        mFilter_ = filter::NONE;
        std::uint32_t uiTextureHandle_ = 0u;
        bounds2f      mRect_;
        bool          bIsOwner_ = false;

        static bool ONLY_POWER_OF_TWO;
        static std::size_t MAXIMUM_SIZE;
    };
}
}
}

#endif
