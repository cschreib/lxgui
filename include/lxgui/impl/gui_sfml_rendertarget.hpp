#ifndef LXGUI_GUI_SFML_RENDERTARGET_HPP
#define LXGUI_GUI_SFML_RENDERTARGET_HPP

#include <lxgui/gui_rendertarget.hpp>
#include "lxgui/impl/gui_sfml_material.hpp"

#include <lxgui/utils.hpp>

#include <memory>

namespace sf
{
    class RenderTexture;
}

namespace lxgui {
namespace gui {
namespace sfml
{
    /// A place to render things (the screen, a texture, ...)
    class render_target final : public gui::render_target
    {
    public :

        /// Constructor.
        /** \param uiWidth  The width of the render_target
        *   \param uiHeight The height of the render_target
        *   \param mFilter  The filtering to apply to the target texture when displayed
        */
        render_target(uint uiWidth, uint uiHeight,
            material::filter mFilter = material::filter::NONE);

        /// Begins rendering on this target.
        void begin() override;

        /// Ends rendering on this target.
        void end() override;

        /// Clears the content of this render_target.
        /** \param mColor The color to use as background
        */
        void clear(const color& mColor) override;

        /// Returns this render target's pixel rect.
        /** \return This render target's pixel rect
        */
        bounds2f get_rect() const override;

        /// Sets this render target's dimensions.
        /** \param uiWidth This render target's width
        *   \param uiHeight This render target's height
        *   \return 'true' if the function had to re-create a
        *           new render target
        */
        bool set_dimensions(uint uiWidth, uint uiHeight) override;

        /// Returns this render target's canvas width.
        /** \return This render target's canvas width
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported :
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        uint get_canvas_width() const override;

        /// Returns this render target's canvas height.
        /** \return This render target's canvas height
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported :
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        uint get_canvas_height() const override;

        /// Returns the associated texture for rendering.
        /** \return The underlying pixel buffer, that you can use to render its content
        */
        std::weak_ptr<sfml::material> get_material();

        /// Returns the underlying SFML render texture object.
        /** return The underlying SFML render texture object
        */
        sf::RenderTexture* get_render_texture();

    private :

        void update_view_matrix_() const;

        std::shared_ptr<sfml::material> pTexture_;
        sf::RenderTexture*              pRenderTexture_ = nullptr;
    };
}
}
}

#endif
