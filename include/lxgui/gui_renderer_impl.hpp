#ifndef LXGUI_GUI_RENDERER_IMPL_HPP
#define LXGUI_GUI_RENDERER_IMPL_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_material.hpp"

#include <vector>
#include <memory>
#include <string>

namespace lxgui {
namespace gui
{
    class manager;
    class material;
    class vertex_cache;
    class font;
    class render_target;
    class color;
    struct quad;
    struct vertex;
    struct matrix4f;

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

        /// Sets the view matrix to use when rendering (viewport).
        /** \param mViewMatrix The view matrix
        *   \note This function is called by default in begin(), which resets the
        *         view to span the entire render target (or the entire screen). Therefore
        *         it is only necessary to use this function when a custom view is required.
        *         The view matrix converts custom "world" coordinates into screen-space
        *         coordinates, where the X and Y coordinates represent the horizontal and
        *         vertical dimensions on the screen, respectively, and range from -1 to +1.
        *         In screen-space coordinates, the top-left corner of the screen has
        *         coordinates (-1,-1), and the bottom-left corner of the screen is (+1,+1).
        *   \warning Although the view is specified here as a matrix for maximum flexibility,
        *            some backends do not actually support arbitrary view matrices. For such
        *            backends, the view matrix will be simplified to a simpler 2D translate +
        *            rotate + scale transform, or even just translate + scale.
        */
        virtual void set_view(const matrix4f& mViewMatrix) const = 0;

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        virtual void render_quad(const quad& mQuad) const = 0;

        /// Renders a set of quads.
        /** \param mMaterial The material to use for rendering (texture, color, blending, ...)
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. When multiple quads share the same material, it is
        *         always more efficient to call this method than calling render_quad
        *         repeatedly, as it allows to reduce the number of draw calls.
        */
        virtual void render_quads(const material& mMaterial, const std::vector<std::array<vertex,4>>& lQuadList) const = 0;

        /// Renders a vertex cache.
        /** \param mMaterial       The material to use for rendering (texture, color, blending, ...)
        *   \param mCache          The vertex cache
        *   \param mModelTransform The transformation matrix to apply to vertices
        *   \note This function is meant to be called between begin() and
        *         end() only. When multiple quads share the same material, it is
        *         always more efficient to call this method than calling render_quad
        *         repeatedly, as it allows to reduce the number of draw calls. This method
        *         is also more efficient than render_quads(), as the vertex data is
        *         already cached to the GPU and does not need sending again. However,
        *         not all implementations support vertex caches. See has_vertex_cache().
        */
        virtual void render_cache(const material& mMaterial, const vertex_cache& mCache,
            const matrix4f& mModelTransform) const = 0;

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

        /// Checks if the renderer supports vertex caches.
        /** \return 'true' if supported, 'false' otherwise
        */
        virtual bool has_vertex_cache() const = 0;

        /// Creates a new empty vertex cache.
        /** \param uiSizeHint An estimate of how much data will be stored in this cache
        *   \note Not all implementations support vertex caches. See has_vertex_cache().
        *         The size hint can enable the cache to be pre-allocated, which will avoid a
        *         reallocation when data is pushed to the cache.
        */
        virtual std::shared_ptr<vertex_cache> create_vertex_cache(uint uiSizeHint) const = 0;

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
