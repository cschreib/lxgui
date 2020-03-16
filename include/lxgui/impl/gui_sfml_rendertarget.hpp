#ifndef GUI_SFML_RENDERTARGET_HPP
#define GUI_SFML_RENDERTARGET_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_refptr.hpp>
#include <lxgui/gui_rendertarget.hpp>
#include "lxgui/impl/gui_sfml_material.hpp"

namespace sf
{
    class RenderTexture;
}

namespace gui {
namespace sfml
{
    /// A place to render things (the screen, a texture, ...)
    class render_target : public gui::render_target
    {
    public :

        /// Constructor.
        /** \param uiWidth  The width of the render_target
        *   \param uiHeight The height of the render_target
        */
        render_target(uint uiWidth, uint uiHeight);

        /// Begins rendering on this target.
        void begin() override;

        /// Ends rendering on this target.
        void end() override;

        /// Clears the content of this render_target.
        /** \param mColor The color to use as background
        */
        void clear(const color& mColor) override;

        /// Returns this render target's width.
        /** \return This render target's width
        */
        uint get_width() const override;

        /// Returns this render target's height.
        /** \return This render target's height
        */
        uint get_height() const override;

        /// Sets this render target's dimensions.
        /** \param uiWidth This render target's width
        *   \param uiHeight This render target's height
        *   \return 'true' if the function had to re-create a
        *           new render target
        */
        bool set_dimensions(uint uiWidth, uint uiHeight) override;

        /// Returns this render target's real width.
        /** \return This render target's real width
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported :
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        uint get_real_width() const override;

        /// Returns this render target's real height.
        /** \return This render target's real height
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported :
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        uint get_real_height() const override;

        /// Returns the associated texture for rendering.
        /** \return The underlying pixel buffer, that you can use to render its content
        */
        utils::wptr<sfml::material> get_material();

        /// Checks if the machine is capable of using render targets.
        /** \note If not, this function throws a gui::exception.
        */
        static void check_availability();

        /// Returns the underlying SFML render texture object.
        /** return The underlying SFML render texture object
        */
        sf::RenderTexture* get_render_texture();

    private :

        void update_view_matrix_() const;

        utils::refptr<sfml::material> pTexture_;
        sf::RenderTexture*            pRenderTexture_;
    };
}
}

#endif
