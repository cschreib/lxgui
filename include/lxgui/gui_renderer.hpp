#ifndef LXGUI_GUI_RENDERER_IMPL_HPP
#define LXGUI_GUI_RENDERER_IMPL_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_vertexcache.hpp"

#include <vector>
#include <memory>
#include <string>

namespace lxgui {
namespace gui
{
    class font;
    class render_target;
    class color;
    struct quad;
    struct vertex;

    /// Abstract type for implementation specific management
    class renderer
    {
    public :

        /// Constructor.
        renderer() = default;

        /// Destructor.
        virtual ~renderer() = default;

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

        /// Returns the current view matrix to use when rendering (viewport).
        /** \return The current view matrix to use when rendering
        *   \note See set_view() for more information. The returned matrix may be different
        *         from the matrix given to set_view(), if the rendering backend does not
        *         support certain transformations.
        */
        virtual matrix4f get_view() const = 0;

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        virtual void render_quad(const quad& mQuad) const = 0;

        /// Renders a set of quads.
        /** \param pMaterial The material to use for rendering, or null if none
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. When multiple quads share the same material, it is
        *         always more efficient to call this method than calling render_quad
        *         repeatedly, as it allows to reduce the number of draw calls.
        */
        virtual void render_quads(const material* pMaterial,
            const std::vector<std::array<vertex,4>>& lQuadList) const = 0;

        /// Renders a vertex cache.
        /** \param pMaterial       The material to use for rendering, or null if none
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
        virtual void render_cache(const material* pMaterial, const vertex_cache& mCache,
            const matrix4f& mModelTransform = matrix4f::IDENTITY) const = 0;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \param mFilter   The filtering to apply to the texture
        *   \return The new material
        *   \note Supported texture formats are defined by implementation.
        *         The gui library is completely unaware of this.
        */
        virtual std::shared_ptr<material> create_material(const std::string& sFileName,
            material::filter mFilter = material::filter::NONE) const = 0;

        /// Creates a new material from a texture file.
        /** \param sAtlasCategory The category of atlas in which to create the texture
        *   \param sFileName      The name of the file
        *   \param mFilter        The filtering to apply to the texture
        *   \return The new material
        *   \note Supported texture formats are defined by implementation.
        *         The gui library is completely unaware of this.
        *   \note The atlas category is a hint that the implementation can use to select
        *         the texture atlas in which to place this new texture. If a group of
        *         textures is known to be used to render objects that are often rendered
        *         consecutively (for example, various tiles of a background), they should
        *         be placed in the same category to maximize the chance of draw call batching.
        *         Conversely, if two texture are known to rarely be used in the same context
        *         (for example, a special effect particle texture and a UI button texture),
        *         they should not be placed in the same category, as this could otherwise
        *         fill up the atlas quickly, and reduce batching opportunities.
        */
        virtual std::shared_ptr<material> create_atlas_material(const std::string& sAtlasCategory,
            const std::string& sFileName, material::filter mFilter = material::filter::NONE) const = 0;

        /// Creates a new material from a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \return The new material
        */
        virtual std::shared_ptr<material> create_material(
            std::shared_ptr<render_target> pRenderTarget) const = 0;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        */
        virtual std::shared_ptr<render_target> create_render_target(
            uint uiWidth, uint uiHeight) const = 0;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note Even though the gui has been designed to use vector fonts files
        *         (such as .ttf or .otf font formats), nothing prevents the implementation
        *         from using any other font type, including bitmap fonts.
        */
        virtual std::shared_ptr<font> create_font(const std::string& sFontFile,
            uint uiSize) const = 0;

        /// Checks if the renderer supports vertex caches.
        /** \return 'true' if supported, 'false' otherwise
        */
        virtual bool has_vertex_cache() const = 0;

        /// Creates a new empty vertex cache.
        /** \param mType The type of data this cache will hold
        *   \note Not all implementations support vertex caches. See has_vertex_cache().
        */
        virtual std::shared_ptr<vertex_cache> create_vertex_cache(
            gui::vertex_cache::type mType) const = 0;

        /// Notifies the renderer that the render window has been resized.
        /** \param uiNewWidth  The new window width
        *   \param uiNewHeight The new window height
        */
        virtual void notify_window_resized(uint uiNewWidth, uint uiNewHeight);
    };
}
}

#endif
