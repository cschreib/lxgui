#ifndef LXGUI_GUI_GL_RENDERTARGET_HPP
#define LXGUI_GUI_GL_RENDERTARGET_HPP

#include "lxgui/impl/gui_gl_material.hpp"

#include <lxgui/gui_rendertarget.hpp>
#include <lxgui/gui_matrix4.hpp>
#include <lxgui/utils.hpp>

#include <memory>

namespace lxgui {
namespace gui {
namespace gl
{
    /// A place to render things (the screen, a texture, ...)
    class render_target final : public gui::render_target
    {
    public :

        /// Constructor.
        /** \param mDimensions The dimensions of the render_target
        *   \param mFilter     The filtering to apply to the target texture when displayed
        */
        render_target(const vector2ui& mDimensions,
            material::filter mFilter = material::filter::NONE);

        /// Destructor.
        ~render_target() override;

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
        /** \param mDimensions The new dimensions (in pixels)
        *   \return 'true' if the function had to re-create a new render target
        */
        bool set_dimensions(const vector2ui& mDimensions) override;

        /// Returns this render target's canvas dimension.
        /** \return This render target's canvas dimension
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported:
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        vector2ui get_canvas_dimensions() const override;

        /// Returns the associated texture for rendering.
        /** \return The underlying pixel buffer, that you can use to render its content
        */
        std::weak_ptr<gl::material> get_material();

        /// Returns the view matrix of this render target.
        /** \return The view matrix of this render target
        */
        const matrix4f& get_view_matrix() const;

        /// Checks if the machine is capable of using render targets.
        /** \note If not, this function throws a gui::exception.
        */
        static void check_availability();

    private :

        std::uint32_t                 uiFBOHandle_ = 0;
        std::shared_ptr<gl::material> pTexture_;

        matrix4f mViewMatrix_;
    };
}
}
}

#endif
