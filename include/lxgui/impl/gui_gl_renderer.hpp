#ifndef LXGUI_GUI_GL_RENDERER_HPP
#define LXGUI_GUI_GL_RENDERER_HPP

#include "lxgui/impl/gui_gl_rendertarget.hpp"
#if defined(LXGUI_OPENGL3)
#    include "lxgui/impl/gui_gl_vertexcache.hpp"
#endif

#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/utils.hpp"

#include <array>
#include <limits>
#include <memory>

namespace lxgui::gui::gl {

/// Open implementation of rendering
class renderer final : public gui::renderer {
public:
    /// Constructor.
    /** \param mWindowDimensions The initial window dimensions (in pixels)
     *   \param bInitGLEW         Set to 'true' to initialise GLEW
     *   \note Use notify_window_resized() if the size of the window changes later.
     */
    explicit renderer(const vector2ui& m_window_dimensions, bool b_init_glew = true);

    /// Returns a human-readable name for this renderer.
    /** \return A human-readable name for this renderer
     */
    std::string get_name() const override;

    /// Returns the current view matrix to use when rendering (viewport).
    /** \return The current view matrix to use when rendering
     *   \note See set_view() for more information. The returned matrix may be different
     *         from the matrix given to set_view(), if the rendering backend does not
     *         support certain transformations.
     */
    matrix4f get_view() const override;

    /// Returns the maximum texture width/height (in pixels).
    /** \return The maximum texture width/height (in pixels)
     */
    std::size_t get_texture_max_size() const override;

    /// Checks if the renderer supports texture atlases natively.
    /** \return 'true' if enabled, 'false' otherwise
     *   \note If 'false', texture atlases will be implemented using a generic
     *         solution with render targets.
     */
    bool is_texture_atlas_supported() const override;

    /// Checks if the renderer supports setting colors for each vertex of a textured quad.
    /** \return 'true' if supported, 'false' otherwise
     */
    bool is_texture_vertex_color_supported() const override;

    /// Creates a new material from arbitrary pixel data.
    /** \param mDimensions The dimensions of the material
     *   \param pPixelData  The color data for all the pixels in the material
     *   \param mFilter     The filtering to apply to the texture
     *   \return The new material
     */
    std::shared_ptr<gui::material> create_material(
        const vector2ui& m_dimensions,
        const ub32color* p_pixel_data,
        material::filter m_filter = material::filter::none) override;

    /// Creates a new material from a portion of a render target.
    /** \param pRenderTarget The render target from which to read the pixels
     *   \param mLocation     The portion of the render target to use as material
     *   \return The new material
     */
    std::shared_ptr<gui::material> create_material(
        std::shared_ptr<gui::render_target> p_render_target, const bounds2f& m_location) override;

    /// Creates a new render target.
    /** \param mDimensions The dimensions of the render target
     *   \param mFilter     The filtering to apply to the target texture when displayed
     */
    std::shared_ptr<gui::render_target> create_render_target(
        const vector2ui& m_dimensions, material::filter m_filter = material::filter::none) override;

    /// Checks if the renderer supports vertex caches.
    /** \return 'true' if supported, 'false' otherwise
     */
    bool is_vertex_cache_supported() const override;

    /// Creates a new empty vertex cache.
    /** \param mType The type of data this cache will hold
     *   \note Not all implementations support vertex caches. See is_vertex_cache_supported().
     */
    std::shared_ptr<gui::vertex_cache> create_vertex_cache(gui::vertex_cache::type m_type) override;

    /// Notifies the renderer that the render window has been resized.
    /** \param mNewDimensions The new window dimensions
     */
    void notify_window_resized(const vector2ui& m_new_dimensions) override;

#if !defined(LXGUI_OPENGL3)
    /// Checks if a given OpenGL extension is supported by the machine.
    /** \return 'true' if that is the case, 'false' else.
     */
    static bool is_gl_extension_supported(const std::string& sExtension);
#endif

protected:
    /// Creates a new material from a texture file.
    /** \param sFileName The name of the file
     *   \param mFilter   The filtering to apply to the texture
     *   \return The new material
     *   \note Only PNG textures are supported by this implementation (parsed by libpng).
     */
    std::shared_ptr<gui::material>
    create_material_(const std::string& s_file_name, material::filter m_filter) override;

    /// Creates a new atlas with a given texture filter mode.
    /** \param mFilter The filtering to apply to the texture
     *   \return The new atlas
     */
    std::shared_ptr<gui::atlas> create_atlas_(material::filter m_filter) override;

    /// Creates a new font.
    /** \param sFontFile   The file from which to read the font
     *   \param uiSize      The requested size of the characters (in points)
     *   \param uiOutline   The thickness of the outline (in points)
     *   \param lCodePoints The list of Unicode characters to load
     *   \param uiDefaultCodePoint The character to display as fallback
     *   \note This implementation uses FreeType to load vector fonts and rasterize them.
     *         Bitmap fonts are not yet supported.
     */
    std::shared_ptr<gui::font> create_font_(
        const std::string&                   s_font_file,
        std::size_t                          ui_size,
        std::size_t                          ui_outline,
        const std::vector<code_point_range>& l_code_points,
        char32_t                             ui_default_code_point) override;

    /// Begins rendering on a particular render target.
    /** \param pTarget The render target (main screen if nullptr)
     */
    void begin_(std::shared_ptr<gui::render_target> p_target) override;

    /// Ends rendering.
    void end_() override;

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
    void set_view_(const matrix4f& m_view_matrix) override;

    /// Renders a set of quads.
    /** \param pMaterial The material to use for rendering, or null if none
     *   \param lQuadList The list of the quads you want to render
     *   \note This function is meant to be called between begin() and
     *         end() only. When multiple quads share the same material, it is
     *         always more efficient to call this method than calling render_quad
     *         repeatedly, as it allows to reduce the number of draw calls.
     */
    void render_quads_(
        const gui::material*                      p_material,
        const std::vector<std::array<vertex, 4>>& l_quad_list) override;

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
     *         Note finally that rendering a vertex cache always triggers a draw
     *         call, no matter what, even when quad batching is enabled. For this reason,
     *         if quad batching is enabled, only use vertex caches for large vertex arrays
     *         and not for just a handful of quads. Benchmark when in doubt.
     */
    void render_cache_(
        const gui::material*     p_material,
        const gui::vertex_cache& m_cache,
        const matrix4f&          m_model_transform) override;

private:
    void update_view_matrix_() const;
#if defined(LXGUI_OPENGL3)
    void compile_programs_();
    void setup_buffers_();
#endif

    std::shared_ptr<gui::material>
    create_material_png_(const std::string& s_file_name, material::filter m_filter) const;

    vector2ui m_window_dimensions_;

    std::shared_ptr<gui::gl::render_target> p_current_target_;
    matrix4f                                m_current_view_matrix_ = matrix4f::identity;

#if defined(LXGUI_OPENGL3)
    struct shader_cache {
        shader_cache()                    = default;
        shader_cache(const shader_cache&) = delete;
        shader_cache(shader_cache&&)      = delete;
        ~shader_cache();

        std::uint32_t ui_program       = 0;
        int           sampler_location = 0;
        int           proj_location    = 0;
        int           model_location   = 0;
        int           type_location    = 0;
    };

    static thread_local std::weak_ptr<shader_cache> p_static_shader_cache;
    std::shared_ptr<shader_cache>                   p_shader_cache_;

    static constexpr std::size_t                                    cache_cycle_size = 1024u;
    std::array<std::shared_ptr<gl::vertex_cache>, cache_cycle_size> p_quad_cache_;
    std::array<std::shared_ptr<gl::vertex_cache>, cache_cycle_size> p_array_cache_;
    std::uint32_t                                                   ui_quad_cycle_cache_  = 0u;
    std::uint32_t                                                   ui_array_cycle_cache_ = 0u;

    std::uint32_t ui_previous_texture_ = std::numeric_limits<std::uint32_t>::max();
#endif
};

} // namespace lxgui::gui::gl

#endif
