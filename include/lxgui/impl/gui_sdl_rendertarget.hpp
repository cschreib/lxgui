#ifndef LXGUI_GUI_SDL_RENDERTARGET_HPP
#define LXGUI_GUI_SDL_RENDERTARGET_HPP

#include <lxgui/gui_rendertarget.hpp>
#include "lxgui/impl/gui_sdl_material.hpp"

#include <lxgui/utils.hpp>
#include <lxgui/gui_matrix4.hpp>

#include <memory>

struct SDL_Renderer;
struct SDL_Texture;

namespace lxgui {
namespace gui {
namespace sdl
{
    /// A place to render things (the screen, a texture, ...)
    class render_target final : public gui::render_target
    {
    public :

        /// Constructor.
        /** \param uiWidth  The width of the render_target
        *   \param uiHeight The height of the render_target
        */
        render_target(SDL_Renderer* pRenderer, uint uiWidth, uint uiHeight);

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
        quad2f get_rect() const override;

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
        std::weak_ptr<sdl::material> get_material();

        /// Returns the view matrix of this render target.
        /** \return The view matrix of this render target
        */
        const matrix4f& get_view_matrix() const;

        /// Checks if the machine is capable of using render targets.
        /** \param pRenderer The renderer to check for availability
        *   \note If not, this function throws a gui::exception.
        */
        static void check_availability(SDL_Renderer* pRenderer);

        /// Returns the underlying SDL render texture object.
        /** return The underlying SDL render texture object
        */
        SDL_Texture* get_render_texture();

    private :

        std::shared_ptr<sdl::material> pTexture_;
        mutable matrix4f mViewMatrix_;
    };
}
}
}

#endif
