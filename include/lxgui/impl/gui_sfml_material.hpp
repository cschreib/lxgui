#ifndef GUI_SFML_MATERIAL_HPP
#define GUI_SFML_MATERIAL_HPP

#include <lxgui/utils.hpp>
#include <lxgui/gui_material.hpp>
#include <lxgui/gui_color.hpp>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>

#include <vector>

namespace gui {
namespace sfml
{
    /// A class that holds rendering data
    /** This implementation can contain either a plain color
    *   or a real sf::Texture. It is also used by the
    *   gui::sfml::render_target class to store the output data.
    */
    class material : public gui::material
    {
    public :

        /// Constructor for textures.
        /** \param uiWidth       The requested texture width
        *   \param uiHeight      The requested texture height
        *   \param bRenderTarget Create the material for a render target or only for display
        *   \param mWrap         How to adjust texture coordinates that are outside the [0,1] range
        *   \param mFilter       Use texture filtering or not (see set_filter())
        */
        material(uint uiWidth, uint uiHeight, bool bRenderTarget, wrap mWrap = wrap::REPEAT,
            filter mFilter = filter::NONE);

        /// Constructor for textures.
        /** \param mData         The image data to use as texture
        *   \param mWrap         How to adjust texture coordinates that are outside the [0,1] range
        *   \param mFilter       Use texture filtering or not (see set_filter())
        */
        material(const sf::Image& mData, wrap mWrap = wrap::REPEAT,
            filter mFilter = filter::NONE);

        /// Constructor for textures.
        /** \param sFileName     The file from which the texture data is loaded
        *   \param mWrap         How to adjust texture coordinates that are outside the [0,1] range
        *   \param mFilter       Use texture filtering or not (see set_filter())
        */
        material(const std::string& sFileName, wrap mWrap = wrap::REPEAT,
            filter mFilter = filter::NONE);

        /// Constructor for plain colors.
        /** \param mColor The plain color to use
        */
        material(const color& mColor);

        /// Destructor.
        ~material() override = default;

        /// Returns the type of this texture (texture or color).
        /** \return The type of this texture (texture or color)
        */
        type get_type() const;

        /// Returns the width of the underlying texture (if any).
        /** \return The width of the underlying texture (if any)
        */
        float get_width() const override;

        /// Returns the height of the underlying texture (if any).
        /** \return The height of the underlying texture (if any)
        */
        float get_height() const override;

        /// Returns the physical width of the underlying texture (if any).
        /** \return The physical width of the underlying texture (if any)
        *   \note Some old hardware don't support textures that have non
        *         power of two dimensions. If the user creates such a material
        *         and its hardware doesn't support it, this class creates a
        *         bigger texture that has power of two dimensions (the
        *         "physical" dimensions).
        */
        float get_real_width() const override;

        /// Returns the physical height of the underlying texture (if any).
        /** \return The physical height of the underlying texture (if any)
        *   \note Some old hardware don't support textures that have non
        *         power of two dimensions. If the user creates such a material
        *         and its hardware doesn't support it, this class creates a
        *         bigger texture that has power of two dimensions (the
        *         "physical" dimensions).
        */
        float get_real_height() const override;

        /// Resizes this texture.
        /** \param uiWidth  The new texture width
        *   \param uiHeight The new texture height
        *   \return 'true' if the function had to re-create a new texture object
        *   \note All the previous data that was stored in this texture will be lost.
        */
        bool set_dimensions(uint uiWidth, uint uiHeight);

        /// Returns the plain color of this texture.
        /** \return The plain color of this texture
        */
        color get_color() const;

        /// Premultiplies an image by its alpha component.
        /** \note Premultiplied alpha is a rendering technique that allows perfect
        *         alpha blending when using render targets.
        */
        static void premultiply_alpha(sf::Image& mData);

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

        /// Returns the cached texture data (read only).
        /** \return The cached texture data (read only)
        */
        const sf::Image& get_data() const;

        /// Returns the cached texture data (read and write).
        /** \return The cached texture data (read and write)
        *   \note If you modify the texture data, you need to call
        *         update_texture() when you're done, so that the
        *         texture that is in the GPU memory gets updated.
        */
        sf::Image& get_data();

        /// Sets the color of one pixel.
        /** \param x      The coordinate of the pixel in the texture
        *   \param y      The coordinate of the pixel in the texture
        *   \param mColor The new color of the pixel
        *   \note If you modify the texture data, you need to call
        *         update_texture() when you're done, so that the
        *         texture that is in the GPU memory gets updated.
        */
        void       set_pixel(uint x, uint y, const sf::Color& mColor);

        /// Returns the color of one pixel (read only).
        /** \param x      The coordinate of the pixel in the texture
        *   \param y      The coordinate of the pixel in the texture
        *   \return The color of the pixel
        */
        sf::Color  get_pixel(uint x, uint y) const;

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

        /// Returns the underlying SFML render texture object.
        /** return The underlying SFML render texture object
        */
        sf::RenderTexture* get_render_texture();

        /// Returns the underlying SFML texture object.
        /** return The underlying SFML texture object
        */
        const sf::Texture* get_texture() const;

    private:

        material(const material& tex);
        material& operator = (const material& tex);

        struct texture_data
        {
            uint   uiWidth_, uiHeight_;
            uint   uiRealWidth_, uiRealHeight_;
            wrap   mWrap_;
            filter mFilter_;

            bool              bRenderTarget_;
            sf::RenderTexture mRenderTexture_;
            sf::Texture       mTexture_;
            sf::Image         mData_;
        };

        struct color_data
        {
            color mColor_;
        };

        type mType_;

        std::unique_ptr<texture_data> pTexData_;
        std::unique_ptr<color_data>   pColData_;

        static const uint MAXIMUM_SIZE;
    };
}
}

#endif
