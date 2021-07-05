#ifndef LXGUI_GUI_SDL_RENDERER_HPP
#define LXGUI_GUI_SDL_RENDERER_HPP

#include "lxgui/impl/gui_sdl_rendertarget.hpp"

#include <lxgui/gui_renderer.hpp>
#include <lxgui/gui_matrix4.hpp>
#include <lxgui/utils.hpp>

#include <memory>

struct SDL_Renderer;

namespace lxgui {
namespace gui {
namespace sdl
{
    class material;

    /// SDL implementation of rendering
    class renderer final : public gui::renderer
    {
    public :

        /// Constructor.
        /** \param pRenderer A pre-initialised SDL renderer
        *   \param bInitialiseSDLImage Set to 'true' if SDL Image has not been initialised yet
        */
        explicit renderer(SDL_Renderer* pRenderer, bool bInitialiseSDLImage);

        /// Begins rendering on a particular render target.
        /** \param pTarget The render target (main screen if nullptr)
        */
        void begin(std::shared_ptr<gui::render_target> pTarget = nullptr) const override;

        /// Ends rendering.
        void end() const override;

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
        void set_view(const matrix4f& mViewMatrix) const override;

        /// Returns the current view matrix to use when rendering (viewport).
        /** \return The current view matrix to use when rendering
        *   \note See set_view() for more information. The returned matrix may be different
        *         from the matrix given to set_view(), if the rendering backend does not
        *         support certain transformations.
        */
        matrix4f get_view() const override;

        /// Renders a quad from a material and array of vertices.
        /** \param pMat        The material to use to to render the quad, or null if none
        *   \param lVertexList The lsit of 4 vertices making up the quad
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        void render_quad(const sdl::material* pMat,
            const std::array<vertex,4>& lVertexList) const;

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        void render_quad(const quad& mQuad) const override;

        /// Renders a set of quads.
        /** \param pMaterial The material to use for rendering, or null if none
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. When multiple quads share the same material, it is
        *         always more efficient to call this method than calling render_quad
        *         repeatedly, as it allows to reduce the number of draw calls.
        */
        void render_quads(const gui::material* pMaterial,
            const std::vector<std::array<vertex,4>>& lQuadList) const override;

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
        void render_cache(const gui::material* pMaterial, const gui::vertex_cache& mCache,
            const matrix4f& mModelTransform) const override;

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
        std::shared_ptr<gui::material> create_atlas_material(const std::string& sAtlasCategory,
            const std::string& sFileName, material::filter mFilter = material::filter::NONE) const override;

        /// Creates a new material from a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \return The new material
        */
        std::shared_ptr<gui::material> create_material(
            std::shared_ptr<gui::render_target> pRenderTarget) const override;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        */
        std::shared_ptr<gui::render_target> create_render_target(
            uint uiWidth, uint uiHeight) const override;

        /// Checks if the renderer supports vertex caches.
        /** \return 'true' if supported, 'false' otherwise
        */
        bool has_vertex_cache() const override;

        /// Creates a new empty vertex cache.
        /** \param mType The type of data this cache will hold
        *   \note Not all implementations support vertex caches. See has_vertex_cache().
        */
        std::shared_ptr<gui::vertex_cache> create_vertex_cache(
            gui::vertex_cache::type mType) const override;

        /// Notifies the renderer that the render window has been resized.
        /** \param uiNewWidth  The new window width
        *   \param uiNewHeight The new window height
        */
        void notify_window_resized(uint uiNewWidth, uint uiNewHeight) override;

    protected :

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \param mFilter   The filtering to apply to the texture
        *   \return The new material
        *   \note Only PNG textures are supported by this implementation (parsed by libpng).
        */
        std::shared_ptr<gui::material> create_material_(const std::string& sFileName,
            material::filter mFilter) const override;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note This implementation uses FreeType to load vector fonts and rasterize them.
        *         Bitmap fonts are not yet supported.
        */
        std::shared_ptr<gui::font> create_font_(const std::string& sFontFile,
            uint uiSize) const override;

    private :

        SDL_Renderer* pRenderer_ = nullptr;
        bool bPreMultipliedAlphaSupported_ = false;

        uint uiWindowWidth_ = 0u;
        uint uiWindowHeight_ = 0u;
        mutable matrix4f mViewMatrix_;
        mutable matrix4f mRawViewMatrix_;
        mutable matrix4f mTargetViewMatrix_;

        mutable std::shared_ptr<gui::sdl::render_target> pCurrentTarget_;
    };
}
}
}

#endif
