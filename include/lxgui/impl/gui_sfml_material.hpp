#ifndef LXGUI_GUI_SFML_MATERIAL_HPP
#define LXGUI_GUI_SFML_MATERIAL_HPP

#include <lxgui/utils.hpp>
#include <lxgui/gui_material.hpp>
#include <lxgui/gui_color.hpp>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>

#include <vector>
#include <memory>

namespace lxgui {
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
        explicit material(const sf::Image& mData, wrap mWrap = wrap::REPEAT,
            filter mFilter = filter::NONE);

        /// Constructor for textures.
        /** \param sFileName     The file from which the texture data is loaded
        *   \param mWrap         How to adjust texture coordinates that are outside the [0,1] range
        *   \param mFilter       Use texture filtering or not (see set_filter())
        */
        explicit material(const std::string& sFileName, wrap mWrap = wrap::REPEAT,
            filter mFilter = filter::NONE);

        /// Constructor for plain colors.
        /** \param mColor The plain color to use
        */
        explicit material(const color& mColor);

        material(const material& tex) = delete;
        material(material&& tex) = delete;
        material& operator = (const material& tex) = delete;
        material& operator = (material&& tex) = delete;

        /// Returns the type of this texture (texture or color).
        /** \return The type of this texture (texture or color)
        */
        type get_type() const override;

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

        /// Returns the underlying SFML render texture object.
        /** return The underlying SFML render texture object
        */
        sf::RenderTexture* get_render_texture();

        /// Returns the underlying SFML texture object.
        /** return The underlying SFML texture object
        */
        const sf::Texture* get_texture() const;

    private:

        struct texture_data
        {
            uint   uiWidth_ = 0u, uiHeight_ = 0u;
            uint   uiRealWidth_ = 0u, uiRealHeight_ = 0u;
            wrap   mWrap_ = wrap::REPEAT;
            filter mFilter_ = filter::NONE;

            bool              bRenderTarget_ = false;
            sf::RenderTexture mRenderTexture_;
            sf::Texture       mTexture_;
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
}

#endif
