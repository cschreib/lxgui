#ifndef LXGUI_GUI_SDL_RENDERER_HPP
#define LXGUI_GUI_SDL_RENDERER_HPP

#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/impl/gui_sdl_rendertarget.hpp"
#include "lxgui/utils.hpp"

#include <memory>

struct SDL_Renderer;

namespace lxgui::gui::sdl {

class material;

/// SDL implementation of rendering
class renderer final : public gui::renderer {
public:
    /// Constructor.
    /** \param rdr A pre-initialised SDL renderer
     *   \param initialise_sdl_image Set to 'true' if SDL Image has not been initialised yet
     */
    explicit renderer(SDL_Renderer* rdr, bool initialise_sdl_image);

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
    /** \param dimensions The dimensions of the material
     *   \param pPixelData  The color data for all the pixels in the material
     *   \param filt     The filtering to apply to the texture
     *   \return The new material
     */
    std::shared_ptr<gui::material> create_material(
        const vector2ui& dimensions,
        const ub32color* p_pixel_data,
        material::filter filt = material::filter::none) override;

    /// Creates a new material from a portion of a render target.
    /** \param pRenderTarget The render target from which to read the pixels
     *   \param location     The portion of the render target to use as material
     *   \return The new material
     */
    std::shared_ptr<gui::material> create_material(
        std::shared_ptr<gui::render_target> p_render_target, const bounds2f& location) override;

    /// Creates a new render target.
    /** \param dimensions The dimensions of the render target
     *   \param filt     The filtering to apply to the target texture when displayed
     */
    std::shared_ptr<gui::render_target> create_render_target(
        const vector2ui& dimensions, material::filter filt = material::filter::none) override;

    /// Checks if the renderer supports vertex caches.
    /** \return 'true' if supported, 'false' otherwise
     */
    bool is_vertex_cache_supported() const override;

    /// Creates a new empty vertex cache.
    /** \param type The type of data this cache will hold
     *   \note Not all implementations support vertex caches. See is_vertex_cache_supported().
     */
    std::shared_ptr<gui::vertex_cache> create_vertex_cache(gui::vertex_cache::type type) override;

    /// Notifies the renderer that the render window has been resized.
    /** \param new_dimensions The new window dimensions
     */
    void notify_window_resized(const vector2ui& new_dimensions) override;

    /// Returns the SDL renderer implementation.
    /** \return the SDL renderer implementation
     */
    SDL_Renderer* get_sdl_renderer() const;

protected:
    /// Creates a new material from a texture file.
    /** \param file_name The name of the file
     *   \param filt   The filtering to apply to the texture
     *   \return The new material
     *   \note Only PNG textures are supported by this implementation (parsed by libpng).
     */
    std::shared_ptr<gui::material>
    create_material_(const std::string& file_name, material::filter filt) override;

    /// Creates a new atlas with a given texture filter mode.
    /** \param filt The filtering to apply to the texture
     *   \return The new atlas
     */
    std::shared_ptr<gui::atlas> create_atlas_(material::filter filt) override;

    /// Creates a new font.
    /** \param font_file   The file from which to read the font
     *   \param uiSize      The requested size of the characters (in points)
     *   \param uiOutline   The thickness of the outline (in points)
     *   \param code_points The list of Unicode characters to load
     *   \param uiDefaultCodePoint The character to display as fallback
     *   \note This implementation uses FreeType to load vector fonts and rasterize them.
     *         Bitmap fonts are not yet supported.
     */
    std::shared_ptr<gui::font> create_font_(
        const std::string&                   font_file,
        std::size_t                          ui_size,
        std::size_t                          ui_outline,
        const std::vector<code_point_range>& code_points,
        char32_t                             ui_default_code_point) override;

    /// Begins rendering on a particular render target.
    /** \param pTarget The render target (main screen if nullptr)
     */
    void begin_(std::shared_ptr<gui::render_target> p_target) override;

    /// Ends rendering.
    void end_() override;

    /// Sets the view matrix to use when rendering (viewport).
    /** \param view_matrix The view matrix
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
    void set_view_(const matrix4f& view_matrix) override;

    /// Renders a quad from a material and array of vertices.
    /** \param pMat         The material to use to to render the quad, or null if none
     *   \param vertex_list The list of 4 vertices making up the quad
     *   \note This function is meant to be called between begin() and
     *         end() only.
     */
    void render_quad_(const sdl::material* p_mat, const std::array<vertex, 4>& vertex_list);

    /// Renders a set of quads.
    /** \param pMaterial The material to use for rendering, or null if none
     *   \param quad_list The list of the quads you want to render
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
    void render_quads_(
        const gui::material*                      p_material,
        const std::vector<std::array<vertex, 4>>& quad_list) override;

    /// Renders a vertex cache.
    /** \param pMaterial       The material to use for rendering, or null if none
     *   \param cache          The vertex cache
     *   \param model_transform The transformation matrix to apply to vertices
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
        const gui::vertex_cache& cache,
        const matrix4f&          model_transform) override;

private:
    SDL_Renderer* p_renderer_                     = nullptr;
    bool          pre_multiplied_alpha_supported_ = false;
    std::size_t   ui_texture_max_size_            = 0u;

    vector2ui window_dimensions_;
    matrix4f  view_matrix_;
    matrix4f  raw_view_matrix_;
    matrix4f  target_view_matrix_;

    std::shared_ptr<gui::sdl::render_target> p_current_target_;
};

} // namespace lxgui::gui::sdl

#endif
