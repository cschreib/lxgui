#ifndef LXGUI_GUI_GL_MATERIAL_HPP
#define LXGUI_GUI_GL_MATERIAL_HPP

#include <lxgui/utils.hpp>
#include <lxgui/gui_material.hpp>
#include <lxgui/gui_color.hpp>

#include <vector>
#include <memory>

namespace lxgui {
namespace gui {
namespace gl
{
    struct ub32color
    {
        using chanel = unsigned char;

        ub32color() = default;
        ub32color(chanel tr, chanel tg, chanel tb, chanel ta) : r(tr), g(tg), b(tb), a(ta) {}
        chanel r, g, b, a;
    };

    /// A class that holds rendering data
    /** This implementation can contain either a plain color
    *   or a real OpenGL texture. It is also used by the
    *   gui::gl::render_target class to store the output data.
    */
    class material final : public gui::material
    {
    public :

        /// Constructor for textures.
        /** \param uiWidth  The requested texture width
        *   \param uiHeight The requested texture height
        *   \param mWrap    How to adjust texture coordinates that are outside the [0,1] range
        *   \param mFilter  Use texture filtering or not (see set_filter())
        *   \param bGPUOnly If 'false', a copy of the texture is kept in CPU memory (RAM). Else
        *                   the texture only resides in GPU memory (used by render_target).
        */
        material(uint uiWidth, uint uiHeight, wrap mWrap = wrap::REPEAT,
            filter mFilter = filter::NONE, bool bGPUOnly = false);

        material(const material& tex) = delete;
        material(material&& tex) = delete;
        material& operator = (const material& tex) = delete;
        material& operator = (material&& tex) = delete;

        /// Destructor.
        ~material() override;

        /// Returns the pixel rect in pixels of the canvas containing this texture (if any).
        /** \return The pixel rect in pixels of the canvas containing this texture (if any)
        */
        quad2f get_rect() const override;

        /// Returns the physical width in pixels of the canvas containing this texture (if any).
        /** \return The physical width in pixels of the canvas containing this texture (if any)
        *   \note Some old hardware don't support textures that have non
        *         power of two dimensions. If the user creates such a material
        *         and its hardware doesn't support it, this class creates a
        *         bigger texture that has power of two dimensions (the
        *         "physical" dimensions).
        */
        float get_canvas_width() const override;

        /// Returns the physical height in pixels of the canvas containing this texture (if any).
        /** \return The physical height in pixels of the canvas containing this texture (if any)
        *   \note Some old hardware don't support textures that have non
        *         power of two dimensions. If the user creates such a material
        *         and its hardware doesn't support it, this class creates a
        *         bigger texture that has power of two dimensions (the
        *         "physical" dimensions).
        */
        float get_canvas_height() const override;

        /// Resizes this texture.
        /** \param uiWidth  The new texture width
        *   \param uiHeight The new texture height
        *   \return 'true' if the function had to re-create a new texture object
        *   \note All the previous data that was stored in this texture will be lost.
        */
        bool set_dimensions(uint uiWidth, uint uiHeight);

        /// Premultiplies the texture by alpha component.
        /** \note Premultiplied alpha is a rendering technique that allows perfect
        *         alpha blending when using render targets.
        */
        void premultiply_alpha();

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

        /// Sets this material as the active one.
        void bind() const;

        /// Returns the cached texture data (read only).
        /** \return The cached texture data (read only)
        */
        const std::vector<ub32color>& get_data() const;

        /// Returns the cached texture data (read and write).
        /** \return The cached texture data (read and write)
        *   \note If you modify the texture data, you need to call
        *         update_texture() when you're done, so that the
        *         texture that is in the GPU memory gets updated.
        */
        std::vector<ub32color>&       get_data();

        /// Sets the color of one pixel.
        /** \param x      The coordinate of the pixel in the texture
        *   \param y      The coordinate of the pixel in the texture
        *   \param mColor The new color of the pixel
        *   \note If you modify the texture data, you need to call
        *         update_texture() when you're done, so that the
        *         texture that is in the GPU memory gets updated.
        */
        void                          set_pixel(uint x, uint y, const ub32color& mColor);

        /// Returns the color of one pixel (read only).
        /** \param x      The coordinate of the pixel in the texture
        *   \param y      The coordinate of the pixel in the texture
        *   \return The color of the pixel
        */
        const ub32color&              get_pixel(uint x, uint y) const;

        /// Returns the color of one pixel.
        /** \param x The coordinate of the pixel in the texture
        *   \param y The coordinate of the pixel in the texture
        *   \return The color of the pixel
        *   \note If you modify the texture data, you need to call
        *         update_texture() when you're done, so that the
        *         texture that is in the GPU memory gets updated.
        */
        ub32color&                    get_pixel(uint x, uint y);

        /// Updates the texture that is in GPU memory.
        /** \note Whenever you modify pixels of the texture,
        *         remember to call this function when you're done,
        *         else your changes won't be applied to the GPU.
        */
        void update_texture();

        /// Removes the cached texture data (in CPU memory).
        /** \note For internal use.
        */
        void clear_cache_data_();

        /// Returns the OpenGL texture handle.
        /** \note For internal use.
        */
        uint get_handle_() const;

        /// Checks if the machine is capable of using some features.
        /** \note The function checks for non power of two capability.
        *         If the graphics card doesn't support it, the material
        *         class will automatically create power of two textures.
        */
        static void check_availability();

    private:

        uint   uiWidth_ = 0u, uiHeight_ = 0u;
        uint   uiRealWidth_ = 0u, uiRealHeight_ = 0u;
        wrap   mWrap_ = wrap::REPEAT;
        filter mFilter_ = filter::NONE;
        uint   uiTextureHandle_ = 0u;

        std::vector<ub32color> pData_;

        static bool ONLY_POWER_OF_TWO;
        static uint MAXIMUM_SIZE;
    };
}
}
}

#endif
