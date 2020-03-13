#ifndef GUI_GL_RENDERTARGET_HPP
#define GUI_GL_RENDERTARGET_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_refptr.hpp>
#include <lxgui/gui_rendertarget.hpp>
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/impl/gui_gl_matrix4.hpp"

namespace gui {
namespace gl
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

        /// Destructor.
        ~render_target();

        /// Begins rendering on this target.
        void begin();

        /// Ends rendering on this target.
        void end();

        /// Clears the content of this render_target.
        /** \param mColor The color to use as background
        */
        void clear(const color& mColor);

        /// Returns this render target's width.
        /** \return This render target's width
        */
        uint get_width() const;

        /// Returns this render target's height.
        /** \return This render target's height
        */
        uint get_height() const;

        /// Sets this render target's dimensions.
        /** \param uiWidth This render target's width
        *   \param uiHeight This render target's height
        *   \return 'true' if the function had to re-create a
        *           new render target
        */
        bool set_dimensions(uint uiWidth, uint uiHeight);

        /// Returns this render target's real width.
        /** \return This render target's real width
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported :
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        uint get_real_width() const;

        /// Returns this render target's real height.
        /** \return This render target's real height
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported :
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        uint get_real_height() const;

        /// Returns the associated texture for rendering.
        /** \return The underlying pixel buffer, that you can use to render its content
        */
        utils::wptr<gl::material> get_material();

        /// Checks if the machine is capable of using render targets.
        /** \note If not, this function throws a gui::exception.
        */
        static void check_availability();

    private :

        void update_view_matrix_() const;

        uint                        uiFBOHandle_;
        utils::refptr<gl::material> pTexture_;

        mutable bool    bUpdateViewMatrix_;
        mutable matrix4 mViewMatrix_;
    };
}
}

#endif
