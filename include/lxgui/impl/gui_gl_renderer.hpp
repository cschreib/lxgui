#ifndef LXGUI_GUI_GL_RENDERER_HPP
#define LXGUI_GUI_GL_RENDERER_HPP

#include "lxgui/impl/gui_gl_rendertarget.hpp"

#include <lxgui/gui_renderer_impl.hpp>
#include <lxgui/gui_matrix4.hpp>
#include <lxgui/utils.hpp>

#include <map>
#include <memory>
#include <array>

namespace lxgui {
namespace gui {
namespace gl
{
    /// Open implementation of rendering
    class renderer : public gui::renderer_impl
    {
    public :

        /// Constructor.
        explicit renderer(bool bInitGLEW = true);

    #if defined(LXGUI_OPENGL3)
        ~renderer() noexcept override;
    #endif

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

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        void render_quad(const quad& mQuad) const override;

        /// Renders a set of quads.
        /** \param mMaterial The material to use for rendering (texture, color, blending, ...)
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. It is always more efficient to call this method
        *         than calling render_quad repeatedly, as it allows to batch
        *         count reduction.
        */
        void render_quads(const gui::material& mMaterial, const std::vector<std::array<vertex,4>>& lQuadList) const override;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \param mFilter   The filtering to apply to the texture
        *   \return The new material
        *   \note Only PNG textures are supported by this implementation (parsed by libpng).
        */
        std::shared_ptr<gui::material> create_material(const std::string& sFileName,
            material::filter mFilter = material::filter::NONE) const override;

        /// Creates a new material from a plain color.
        /** \param mColor The color to use
        *   \return The new material
        */
        std::shared_ptr<gui::material> create_material(const color& mColor) const override;

        /// Creates a new material from a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \return The new material
        */
        std::shared_ptr<gui::material> create_material(std::shared_ptr<gui::render_target> pRenderTarget) const override;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        */
        std::shared_ptr<gui::render_target> create_render_target(uint uiWidth, uint uiHeight) const override;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note This implementation uses FreeType to load vector fonts and rasterize them.
        *         Bitmap fonts are not yet supported.
        */
        std::shared_ptr<gui::font> create_font(const std::string& sFontFile, uint uiSize) const override;

        /// Checks if the renderer supports vertex caches.
        /** \return 'true' if supported, 'false' otherwise
        */
        bool has_vertex_cache() const override;

        /// Creates a new empty vertex cache.
        /** \param pMaterial The material to use to render the vertices
        *   \param uiSizeHint An estimate of how much data will be stored in this cache
        *   \note Not all implementations support vertex caches. See has_vertex_cache().
        *         The size hint can enable the cache to be pre-allocated, which will avoid a
        *         reallocation when data is pushed to the cache.
        */
        std::shared_ptr<gui::vertex_cache> create_vertex_cache(std::shared_ptr<gui::material> pMaterial,
            uint uiSizeHint) const override;

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

    private :

        void update_view_matrix_() const;
    #if defined(LXGUI_OPENGL3)
        void compile_programs_();
        void setup_buffers_();
    #endif

        std::shared_ptr<gui::material> create_material_png(const std::string& sFileName,
            material::filter mFilter) const;

        mutable std::map<std::string, std::weak_ptr<gui::material>> lTextureList_;
        mutable std::map<std::string, std::weak_ptr<gui::font>>     lFontList_;

        mutable bool     bUpdateViewMatrix_ = true;
        mutable matrix4f mViewMatrix_;

        mutable std::shared_ptr<gui::gl::render_target> pCurrentTarget_;

    #if defined(LXGUI_OPENGL3)
        struct shader_cache
        {
            shader_cache() = default;
            shader_cache(const shader_cache&) = delete;
            shader_cache(shader_cache&&) = delete;
            ~shader_cache();

            uint uiTextureProgram_ = 0;
            uint uiColorProgram_ = 0;
            int iTextureSamplerLocation_ = 0;
            int iTextureProjLocation_ = 0;
            int iColorProjLocation_ = 0;
            int iColorColLocation_ = 0;
        };

        static thread_local std::weak_ptr<shader_cache> pStaticShaderCache_;
        std::shared_ptr<shader_cache> pShaderCache_;

        mutable std::array<uint,4> uiVertexArray_;
        mutable std::array<uint,4> uiVertexBuffers_;
        mutable uint uiPreviousProgram_ = (uint)-1;
        mutable uint uiPreviousTexture_ = (uint)-1;
        mutable std::vector<uint> lRepeatedIds_;
    #endif
    };
}
}
}

#endif
