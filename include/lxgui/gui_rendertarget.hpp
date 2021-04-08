#ifndef LXGUI_GUI_RENDERTARGET_HPP
#define LXGUI_GUI_RENDERTARGET_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
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

        /// Begins rendering on this target.
        virtual void begin() = 0;

        /// Ends rendering on this target.
        virtual void end() = 0;

        /// Clears the content of this render_target.
        /** \param mColor The color to use as background
        */
        virtual void clear(const color& mColor) = 0;

        /// Returns this render target's width.
        /** \return This render target's width
        */
        virtual uint get_width() const = 0;

        /// Returns this render target's height.
        /** \return This render target's height
        */
        virtual uint get_height() const = 0;

        /// Sets this render target's dimensions.
        /** \param uiWidth This render target's width
        *   \param uiHeight This render target's height
        *   \return 'true' if the function had to re-create a
        *           new render target
        */
        virtual bool set_dimensions(uint uiWidth, uint uiHeight) = 0;

        /// Returns this render target's real width.
        /** \return This render target's real width
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported :
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        virtual uint get_real_width() const = 0;

        /// Returns this render target's real height.
        /** \return This render target's real height
        *   \note This is the physical size of the render target.
        *         On some systems, abitrary dimensions are not supported :
        *         they can be promoted to the nearest power of two from
        *         for example.
        */
        virtual uint get_real_height() const = 0;
    };
}
}

#endif
