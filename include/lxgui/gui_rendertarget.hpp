#ifndef LXGUI_GUI_RENDERTARGET_HPP
#define LXGUI_GUI_RENDERTARGET_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_bounds2.hpp"
#include "lxgui/gui_color.hpp"

namespace lxgui {
namespace gui
{
    /// A place to render things (the screen, a texture, ...)
    /** \note This is an abstract class that must be inherited
    *         from and created by the corresponding gui::renderer.
    */
    class render_target
    {
    public :

        /// Constructor.
        render_target() = default;

        /// Destructor.
        virtual ~render_target() = default;

        /// Non-copiable
        render_target(const render_target&) = delete;

        /// Non-movable
        render_target(render_target&&) = delete;

        /// Non-copiable
        render_target& operator=(const render_target&) = delete;

        /// Non-movable
        render_target& operator=(render_target&&) = delete;

        /// Begins rendering on this target.
        virtual void begin() = 0;

        /// Ends rendering on this target.
        virtual void end() = 0;

        /// Clears the content of this render_target.
        /** \param mColor The color to use as background
        */
        virtual void clear(const color& mColor) = 0;

        /// Returns this render target's pixel rect.
        /** \return This render target's pixel rect
        */
        virtual bounds2f get_rect() const = 0;

        /// Sets this render target's dimensions.
        /** \param uiWidth This render target's width
        *   \param uiHeight This render target's height
        *   \return 'true' if the function had to re-create a
        *           new render target
        */
        virtual bool set_dimensions(uint uiWidth, uint uiHeight) = 0;

        /// Returns this render target's canvas width.
        /** \return This render target's canvas width
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported :
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        virtual uint get_canvas_width() const = 0;

        /// Returns this render target's canvas height.
        /** \return This render target's canvas height
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported :
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        virtual uint get_canvas_height() const = 0;
    };
}
}

#endif
