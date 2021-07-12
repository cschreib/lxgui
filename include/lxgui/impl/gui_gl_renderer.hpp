#ifndef LXGUI_GUI_GL_RENDERER_HPP
#define LXGUI_GUI_GL_RENDERER_HPP

#include "lxgui/impl/gui_gl_rendertarget.hpp"
#if defined(LXGUI_OPENGL3)
    #include "lxgui/impl/gui_gl_vertexcache.hpp"
#endif

#include <lxgui/gui_renderer.hpp>
#include <lxgui/gui_matrix4.hpp>
#include <lxgui/utils.hpp>

#include <memory>
#include <array>

namespace lxgui {
namespace gui {
namespace gl
{
    /// Open implementation of rendering
    class renderer final : public gui::renderer
    {
    public :

        /// Constructor.
        /** \param uiWindowWidth  The initial width of the window (in pixels)
        *   \param uiWindowHeight The initial height of the window (in pixels)
        *   \param bInitGLEW      Set to 'true' to initialise GLEW
        *   \note Use notify_window_resized() if the size of the window changes later.
        */
        explicit renderer(uint uiWindowWidth, uint uiWindowHeight, bool bInitGLEW = true);

        /// Returns a human-readable name for this renderer.
        /** \return A human-readable name for this renderer
        */
        std::string get_name() const override;

        /// Begins rendering on a particular render target.
        /** \param pTarget The render target (main screen if nullptr)
        */
        void begin_(std::shared_ptr<gui::render_target> pTarget) const override;

        /// Ends rendering.
        void end_() const override;

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
        void set_view_(const matrix4f& mViewMatrix) const override;

        /// Returns the current view matrix to use when rendering (viewport).
        /** \return The current view matrix to use when rendering
        *   \note See set_view() for more information. The returned matrix may be different
        *         from the matrix given to set_view(), if the rendering backend does not
        *         support certain transformations.
        */
        matrix4f get_view() const override;

        /// Renders a set of quads.
        /** \param pMaterial The material to use for rendering, or null if none
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. When multiple quads share the same material, it is
        *         always more efficient to call this method than calling render_quad
        *         repeatedly, as it allows to reduce the number of draw calls.
        */
        void render_quads_(const gui::material* pMaterial,
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
        *         not all implementations support vertex caches. See is_vertex_cache_supported().
        */
        void render_cache_(const gui::material* pMaterial, const gui::vertex_cache& mCache,
            const matrix4f& mModelTransform) const override;

        /// Returns the maximum texture width/height (in pixels).
        /** \return The maximum texture width/height (in pixels)
        */
        uint get_texture_max_size() const override;

        /// Checks if the renderer supports texture atlases natively.
        /** \return 'true' if enabled, 'false' otherwise
        *   \note If 'false', texture atlases will be implemented using a generic
        *         solution with render targets.
        */
        bool is_texture_atlas_natively_supported() const override;

        /// Checks if the renderer supports setting colors for each vertex of a textured quad.
        /** \return 'true' if supported, 'false' otherwise
        */
        bool is_texture_vertex_color_supported() const override;

        /// Creates a new material from arbitrary pixel data.
        /** \param uiWidth    The width of the material
        *   \param uiHeight   The height of the material
        *   \param pPixelData The color data for all the pixels in the material
        *   \param mFilter    The filtering to apply to the texture
        *   \return The new material
        */
        std::shared_ptr<gui::material> create_material(uint uiWidth, uint uiHeight,
            const ub32color* pPixelData,
            material::filter mFilter = material::filter::NONE) const override;

        /// Creates a new material from a portion of a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \param mLocation     The portion of the render target to use as material
        *   \return The new material
        */
        std::shared_ptr<gui::material> create_material(
            std::shared_ptr<gui::render_target> pRenderTarget,
            const quad2f& mLocation) const override;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        *   \param mFilter  The filtering to apply to the target texture when displayed
        */
        std::shared_ptr<gui::render_target> create_render_target(
            uint uiWidth, uint uiHeight,
            material::filter mFilter = material::filter::NONE) const override;

        /// Checks if the renderer supports vertex caches.
        /** \return 'true' if supported, 'false' otherwise
        */
        bool is_vertex_cache_supported() const override;

        /// Creates a new empty vertex cache.
        /** \param mType The type of data this cache will hold
        *   \note Not all implementations support vertex caches. See is_vertex_cache_supported().
        */
        std::shared_ptr<gui::vertex_cache> create_vertex_cache(
            gui::vertex_cache::type mType) const override;

        /// Notifies the renderer that the render window has been resized.
        /** \param uiNewWidth  The new window width
        *   \param uiNewHeight The new window height
        */
        void notify_window_resized(uint uiNewWidth, uint uiNewHeight) override;

    #if !defined(LXGUI_OPENGL3)
        /// Checks if a given OpenGL extension is supported by the machine.
        /** \return 'true' if that is the case, 'false' else.
        */
        static bool is_gl_extension_supported(const std::string& sExtension);
    #endif

    protected :

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \param mFilter   The filtering to apply to the texture
        *   \return The new material
        *   \note Only PNG textures are supported by this implementation (parsed by libpng).
        */
        std::shared_ptr<gui::material> create_material_(const std::string& sFileName,
            material::filter mFilter) const override;

        /// Creates a new atlas with a given texture filter mode.
        /** \param mFilter The filtering to apply to the texture
        *   \return The new atlas
        */
        std::shared_ptr<gui::atlas> create_atlas_(material::filter mFilter) const override;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note This implementation uses FreeType to load vector fonts and rasterize them.
        *         Bitmap fonts are not yet supported.
        */
        std::shared_ptr<gui::font> create_font_(const std::string& sFontFile,
            uint uiSize) const override;

    private :

        void update_view_matrix_() const;
    #if defined(LXGUI_OPENGL3)
        void compile_programs_();
        void setup_buffers_();
    #endif

        std::shared_ptr<gui::material> create_material_png(const std::string& sFileName,
            material::filter mFilter) const;

        uint uiWindowWidth_ = 0u;
        uint uiWindowHeight_ = 0u;

        mutable std::shared_ptr<gui::gl::render_target> pCurrentTarget_;
        mutable matrix4f mCurrentViewMatrix_ = matrix4f::IDENTITY;

    #if defined(LXGUI_OPENGL3)
        struct shader_cache
        {
            shader_cache() = default;
            shader_cache(const shader_cache&) = delete;
            shader_cache(shader_cache&&) = delete;
            ~shader_cache();

            uint uiProgram_ = 0;
            int iSamplerLocation_ = 0;
            int iProjLocation_ = 0;
            int iModelLocation_ = 0;
            int iTypeLocation_ = 0;
        };

        static thread_local std::weak_ptr<shader_cache> pStaticShaderCache_;
        std::shared_ptr<shader_cache> pShaderCache_;

        static constexpr uint CACHE_CYCLE_SIZE = 1024u;
        mutable std::array<std::shared_ptr<gl::vertex_cache>,CACHE_CYCLE_SIZE> pQuadCache_;
        mutable std::array<std::shared_ptr<gl::vertex_cache>,CACHE_CYCLE_SIZE> pArrayCache_;
        mutable uint uiQuadCycleCache_ = 0u;
        mutable uint uiArrayCycleCache_ = 0u;

        mutable uint uiPreviousTexture_ = (uint)-1;
    #endif
    };
}
}
}

#endif
