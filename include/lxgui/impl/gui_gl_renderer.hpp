#ifndef LXGUI_GUI_GL_RENDERER_HPP
#define LXGUI_GUI_GL_RENDERER_HPP

#include "lxgui/impl/gui_gl_render_target.hpp"
#if defined(LXGUI_OPENGL3)
#    include "lxgui/impl/gui_gl_vertex_cache.hpp"
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
    /**
     * \brief Constructor.
     * \param window_dimensions The initial window dimensions (in pixels)
     * \param init_glew Set to 'true' to initialise GLEW
     * \note Use notify_window_resized() if the size of the window changes later.
     */
    explicit renderer(const vector2ui& window_dimensions, bool init_glew = true);

    /**
     * \brief Returns a human-readable name for this renderer.
     * \return A human-readable name for this renderer
     */
    std::string get_name() const override;

    /**
     * \brief Returns the current view matrix to use when rendering (viewport).
     * \return The current view matrix to use when rendering
     * \note See set_view() for more information. The returned matrix may be different
     * from the matrix given to set_view(), if the rendering backend does not
     * support certain transformations.
     */
    matrix4f get_view() const override;

    /**
     * \brief Returns the maximum texture width/height (in pixels).
     * \return The maximum texture width/height (in pixels)
     */
    std::size_t get_texture_max_size() const override;

    /**
     * \brief Checks if the renderer supports texture atlases natively.
     * \return 'true' if enabled, 'false' otherwise
     * \note If 'false', texture atlases will be implemented using a generic
     * solution with render targets.
     */
    bool is_texture_atlas_supported() const override;

    /**
     * \brief Checks if the renderer supports setting colors for each vertex of a textured quad.
     * \return 'true' if supported, 'false' otherwise
     */
    bool is_texture_vertex_color_supported() const override;

    /**
     * \brief Creates a new material from arbitrary pixel data.
     * \param dimensions The dimensions of the material
     * \param pixel_data The color data for all the pixels in the material
     * \param filt The filtering to apply to the texture
     * \return The new material
     */
    std::shared_ptr<gui::material> create_material(
        const vector2ui& dimensions,
        const color32*   pixel_data,
        material::filter filt = material::filter::none) override;

    /**
     * \brief Creates a new material from a portion of a render target.
     * \param target The render target from which to read the pixels
     * \param location The portion of the render target to use as material
     * \return The new material
     */
    std::shared_ptr<gui::material>
    create_material(std::shared_ptr<gui::render_target> target, const bounds2f& location) override;

    /**
     * \brief Creates a new render target.
     * \param dimensions The dimensions of the render target
     * \param filt The filtering to apply to the target texture when displayed
     */
    std::shared_ptr<gui::render_target> create_render_target(
        const vector2ui& dimensions, material::filter filt = material::filter::none) override;

    /**
     * \brief Checks if the renderer supports vertex caches.
     * \return 'true' if supported, 'false' otherwise
     */
    bool is_vertex_cache_supported() const override;

    /**
     * \brief Creates a new empty vertex cache.
     * \param type The type of data this cache will hold
     * \note Not all implementations support vertex caches. See is_vertex_cache_supported().
     */
    std::shared_ptr<gui::vertex_cache> create_vertex_cache(gui::vertex_cache::type type) override;

    /**
     * \brief Notifies the renderer that the render window has been resized.
     * \param new_dimensions The new window dimensions
     */
    void notify_window_resized(const vector2ui& new_dimensions) override;

#if !defined(LXGUI_OPENGL3)
    /**
     * \brief Checks if a given OpenGL extension is supported by the machine.
     * \return 'true' if that is the case, 'false' else.
     */
    static bool is_gl_extension_supported(const std::string& extension);
#endif

protected:
    /**
     * \brief Creates a new material from a texture file.
     * \param file_name The name of the file
     * \param filt The filtering to apply to the texture
     * \return The new material
     * \note Only PNG textures are supported by this implementation (parsed by libpng).
     */
    std::shared_ptr<gui::material>
    create_material_(const std::string& file_name, material::filter filt) override;

    /**
     * \brief Creates a new atlas with a given texture filter mode.
     * \param filt The filtering to apply to the texture
     * \return The new atlas
     */
    std::shared_ptr<gui::atlas> create_atlas_(material::filter filt) override;

    /**
     * \brief Creates a new font.
     * \param font_file The file from which to read the font
     * \param size The requested size of the characters (in points)
     * \param outline The thickness of the outline (in points)
     * \param code_points The list of Unicode characters to load
     * \param default_code_point The character to display as fallback
     * \note This implementation uses FreeType to load vector fonts and rasterize them.
     * Bitmap fonts are not yet supported.
     */
    std::shared_ptr<gui::font> create_font_(
        const std::string&                   font_file,
        std::size_t                          size,
        std::size_t                          outline,
        const std::vector<code_point_range>& code_points,
        char32_t                             default_code_point) override;

    /**
     * \brief Begins rendering on a particular render target.
     * \param target The render target (main screen if nullptr)
     */
    void begin_(std::shared_ptr<gui::render_target> target) override;

    /// Ends rendering.
    void end_() override;

    /**
     * \brief Sets the view matrix to use when rendering (viewport).
     * \param view_matrix The view matrix
     * \note This function is called by default in begin(), which resets the
     * view to span the entire render target (or the entire screen). Therefore
     * it is only necessary to use this function when a custom view is required.
     * The view matrix converts custom "world" coordinates into screen-space
     * coordinates, where the X and Y coordinates represent the horizontal and
     * vertical dimensions on the screen, respectively, and range from -1 to +1.
     * In screen-space coordinates, the top-left corner of the screen has
     * coordinates (-1,-1), and the bottom-left corner of the screen is (+1,+1).
     * \warning Although the view is specified here as a matrix for maximum flexibility,
     * some backends do not actually support arbitrary view matrices. For such
     * backends, the view matrix will be simplified to a simpler 2D translate +
     * rotate + scale transform, or even just translate + scale.
     */
    void set_view_(const matrix4f& view_matrix) override;

    /**
     * \brief Renders a set of quads.
     * \param mat The material to use for rendering, or null if none
     * \param quad_list The list of the quads you want to render
     * \note This function is meant to be called between begin() and
     * end() only. When multiple quads share the same material, it is
     * always more efficient to call this method than calling render_quad
     * repeatedly, as it allows to reduce the number of draw calls.
     */
    void render_quads_(
        const gui::material* mat, const std::vector<std::array<vertex, 4>>& quad_list) override;

    /**
     * \brief Renders a vertex cache.
     * \param mat The material to use for rendering, or null if none
     * \param cache The vertex cache
     * \param model_transform The transformation matrix to apply to vertices
     * \note This function is meant to be called between begin() and
     * end() only. When multiple quads share the same material, it is
     * always more efficient to call this method than calling render_quad
     * repeatedly, as it allows to reduce the number of draw calls. This method
     * is also more efficient than render_quads(), as the vertex data is
     * already cached to the GPU and does not need sending again. However,
     * not all implementations support vertex caches. See is_vertex_cache_supported().
     * Note finally that rendering a vertex cache always triggers a draw
     * call, no matter what, even when quad batching is enabled. For this reason,
     * if quad batching is enabled, only use vertex caches for large vertex arrays
     * and not for just a handful of quads. Benchmark when in doubt.
     */
    void render_cache_(
        const gui::material*     mat,
        const gui::vertex_cache& cache,
        const matrix4f&          model_transform) override;

private:
    void update_view_matrix_() const;
#if defined(LXGUI_OPENGL3)
    void compile_programs_();
    void setup_buffers_();
#endif

    std::shared_ptr<gui::material>
    create_material_png_(const std::string& file_name, material::filter filt) const;

    vector2ui window_dimensions_;

    std::shared_ptr<gui::gl::render_target> current_target_;
    matrix4f                                current_view_matrix_ = matrix4f::identity;

#if defined(LXGUI_OPENGL3)
    struct shader_cache {
        shader_cache()                    = default;
        shader_cache(const shader_cache&) = delete;
        shader_cache(shader_cache&&)      = delete;
        ~shader_cache();

        std::uint32_t program          = 0;
        int           sampler_location = 0;
        int           proj_location    = 0;
        int           model_location   = 0;
        int           type_location    = 0;
    };

    static thread_local std::weak_ptr<shader_cache> static_shader_cache;
    std::shared_ptr<shader_cache>                   shader_cache_;

    static constexpr std::size_t                                    cache_cycle_size = 1024u;
    std::array<std::shared_ptr<gl::vertex_cache>, cache_cycle_size> quad_cache_;
    std::array<std::shared_ptr<gl::vertex_cache>, cache_cycle_size> array_cache_;
    std::uint32_t                                                   quad_cycle_cache_  = 0u;
    std::uint32_t                                                   array_cycle_cache_ = 0u;

    std::uint32_t previous_texture_ = std::numeric_limits<std::uint32_t>::max();
#endif
};

} // namespace lxgui::gui::gl

#endif
