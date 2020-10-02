#ifndef LXGUI_GUI_RENDERER_IMPL_HPP
#define LXGUI_GUI_RENDERER_IMPL_HPP

#include "lxgui/gui_material.hpp"

#include <vector>
#include <memory>

namespace lxgui {
namespace gui
{
    class manager;
    class material;
    class font;
    class render_target;
    class color;
    struct quad;
    struct vertex;

    /// Abstract type for implementation specific management
    class renderer_impl
    {
    public :

        /// Constructor.
        renderer_impl() = default;

        /// Destructor.
        virtual ~renderer_impl() = default;

        /// Gives a pointer to the base class.
        /** \note This function is automatically called by gui::manager
        *         on creation.
        */
        void set_parent(manager* pParent);

        /// Begins rendering on a particular render target.
        /** \param pTarget The render target (main screen if nullptr)
        */
        virtual void begin(std::shared_ptr<render_target> pTarget = nullptr) const = 0;

        /// Ends rendering.
        virtual void end() const = 0;

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        virtual void render_quad(const quad& mQuad) const = 0;

        /// Renders a set of quads.
        /** \param mQuad     The base quad to use for rendering (material, blending, ...)
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. It is always more efficient to call this method
        *         than calling render_quad repeatedly, as it allows to batch
        *         count reduction.
        */
        virtual void render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const = 0;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \return The new material
        *   \note Supported texture formats are defined by implementation.
        *         The gui library is completely unaware of this.
        */
        virtual std::shared_ptr<material> create_material(const std::string& sFileName,
            material::filter mFilter = material::filter::NONE) const = 0;

        /// Creates a new material from a plain color.
        /** \param mColor The color to use
        *   \return The new material
        */
        virtual std::shared_ptr<material> create_material(const color& mColor) const = 0;

        /// Creates a new material from a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \return The new material
        */
        virtual std::shared_ptr<material> create_material(std::shared_ptr<render_target> pRenderTarget) const = 0;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        */
        virtual std::shared_ptr<render_target> create_render_target(uint uiWidth, uint uiHeight) const = 0;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note Even though the gui has been designed to use vector fonts files
        *         (such as .ttf or .otf font formats), nothing prevents the implementation
        *         from using any other font type, including bitmap fonts.
        */
        virtual std::shared_ptr<font> create_font(const std::string& sFontFile, uint uiSize) const = 0;

        /// Notifies the renderer that the render window has been resized.
        /** \param uiNewWidth  The new window width
        *   \param uiNewHeight The new window height
        */
        virtual void notify_window_resized(uint uiNewWidth, uint uiNewHeight);

    protected :

        manager* pParent_ = nullptr;
    };
}
}

#endif
