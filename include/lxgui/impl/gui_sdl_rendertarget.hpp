#ifndef LXGUI_GUI_SDL_RENDERTARGET_HPP
#define LXGUI_GUI_SDL_RENDERTARGET_HPP

#include <lxgui/gui_rendertarget.hpp>
#include "lxgui/impl/gui_sdl_material.hpp"

#include <lxgui/utils.hpp>

#include <memory>

struct SDL_Renderer;
struct SDL_Texture;

namespace lxgui {
namespace gui {
namespace sdl
{
    /// A place to render things (the screen, a texture, ...)
    class render_target : public gui::render_target
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
        std::weak_ptr<sdl::material> get_material();

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

        void update_view_matrix_() const;

        std::shared_ptr<sdl::material> pTexture_;
    };
}
}
}

#endif
