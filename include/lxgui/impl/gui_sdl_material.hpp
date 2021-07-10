#ifndef LXGUI_GUI_SDL_MATERIAL_HPP
#define LXGUI_GUI_SDL_MATERIAL_HPP

#include <lxgui/utils.hpp>
#include <lxgui/gui_material.hpp>
#include <lxgui/gui_color.hpp>

#include <vector>
#include <memory>
#include <variant>

struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;

namespace lxgui {
namespace gui {
namespace sdl
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
    *   or a real SDL_Texture. It is also used by the
    *   gui::sdl::render_target class to store the output data.
    */
    class material final : public gui::material
    {
    public :

        /// Constructor for textures.
        /** \param pRenderer     The SDL render to create the material for
        *   \param uiWidth       The requested texture width
        *   \param uiHeight      The requested texture height
        *   \param bRenderTarget Create the material for a render target or only for display
        *   \param mWrap         How to adjust texture coordinates that are outside the [0,1] range
        *   \param mFilter       Use texture filtering or not (see set_filter())
        */
        material(SDL_Renderer* pRenderer, uint uiWidth, uint uiHeight, bool bRenderTarget = false,
            wrap mWrap = wrap::REPEAT, filter mFilter = filter::NONE);

        /// Constructor for textures.
        /** \param pRenderer     The SDL render to create the material for
        *   \param sFileName     The file from which the texture data is loaded
        *   \param bPreMultipliedAlphaSupported Set to 'true' if the renderer supports pre-multipled alpha
        *   \param mWrap         How to adjust texture coordinates that are outside the [0,1] range
        *   \param mFilter       Use texture filtering or not (see set_filter())
        */
        material(SDL_Renderer* pRenderer, const std::string& sFileName,
            bool bPreMultipliedAlphaSupported, wrap mWrap = wrap::REPEAT,
            filter mFilter = filter::NONE);

        /// Constructor for atlas textures.
        /** \param pRenderer The SDL render to create the material for
        *   \param pTexture  The texture object of the atlas
        *   \param mRect     The position of this texture inside the atlas
        *   \param mFilter   Use texture filtering or not (see set_filter())
        */
        material(SDL_Renderer* pRenderer, SDL_Texture* pTexture, const quad2f& mRect,
            filter mFilter = filter::NONE);

        material(const material& tex) = delete;
        material(material&& tex) = delete;
        material& operator = (const material& tex) = delete;
        material& operator = (material&& tex) = delete;

        /// Destructor.
        ~material() noexcept override;

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

        /// Premultiplies an image by its alpha component.
        /** \note Premultiplied alpha is a rendering technique that allows perfect
        *         alpha blending when using render targets.
        */
        static void premultiply_alpha(SDL_Surface* pData);

        /// Returns the SDL blend mode corresponding to pre-multiplied alpha.
        /** \returns the SDL blend mode corresponding to pre-multiplied alpha
        */
        static int get_premultiplied_alpha_blend_mode();

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

        /// Return the wrap mode of this texture.
        /** \return The wrap mode of this texture
        */
        wrap get_wrap() const;

        /// Returns the underlying SDL texture object.
        /** return The underlying SDL texture object
        */
        SDL_Texture* get_texture() const;

        /// Returns the underlying SDL texture object (for render target).
        /** return The underlying SDL texture object (for render target)
        */
        SDL_Texture* get_render_texture();

        /// Returns the SDL renderer object that this material was created on.
        /** return The SDL renderer object that this material was created on
        */
        SDL_Renderer* get_renderer();

        /// Returns a pointer to the texture data, which can be modified.
        /** \param pPitch An output pointer to the size (in bytes) of a row (ignored if nullptr)
        *   \return A pointer to the texture data, which can be modified
        *   \note The pointer is owned by this class, you must not delete it.
        *         Make sure you call unlock_pointer() when you are done.
        */
        ub32color* lock_pointer(uint* pPitch = nullptr);

        /// \copydoc lock_pointer
        const ub32color* lock_pointer(uint* pPitch = nullptr) const;

        /// Stops modifying the texture data and update the texture in GPU memory.
        void unlock_pointer() const;

    private:

        SDL_Renderer* pRenderer_ = nullptr;

        uint   uiWidth_ = 0u, uiHeight_ = 0u;
        uint   uiRealWidth_ = 0u, uiRealHeight_ = 0u;
        quad2f mRect_;
        wrap   mWrap_ = wrap::REPEAT;
        filter mFilter_ = filter::NONE;
        bool   bRenderTarget_ = false;

        SDL_Texture* pTexture_ = nullptr;
        bool         bIsOwner_ = false;
    };
}
}
}

#endif
