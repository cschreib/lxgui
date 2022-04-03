#ifndef LXGUI_GUI_RENDERER_HPP
#define LXGUI_GUI_RENDERER_HPP

#include "lxgui/gui_code_point_range.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_vertex_cache.hpp"
#include "lxgui/lxgui.hpp"

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace lxgui::gui {

class font;
class atlas;
class render_target;
class color;
struct quad;
struct vertex;

/// Abstract type for implementation specific management
class renderer {
public:
    /// Constructor.
    renderer() = default;

    /// Non-copiable
    renderer(const renderer&) = delete;

    /// Non-movable
    renderer(renderer&&) = delete;

    /// Non-copiable
    renderer& operator=(const renderer&) = delete;

    /// Non-movable
    renderer& operator=(renderer&&) = delete;

    /// Destructor.
    virtual ~renderer() = default;

    /**
     * \brief Returns a human-readable name for this renderer.
     * \return A human-readable name for this renderer
     */
    virtual std::string get_name() const = 0;

    /**
     * \brief Checks if the renderer supports setting colors for each vertex of a textured quad.
     * \return 'true' if supported, 'false' otherwise
     */
    virtual bool is_texture_vertex_color_supported() const = 0;

    /**
     * \brief Checks if the renderer has quad render batching enabled.
     * \return 'true' if enabled, 'false' otherwise
     */
    bool is_quad_batching_enabled() const;

    /**
     * \brief Enables/disables quad batching.
     * \param enabled 'true' to enable quad batching, 'false' to disable it
     * \note Quad batching is enabled by default.
     * \note When quad batching is disabled, each call to render_quads() renders
     * immediately to the screen. This can lead to a large number of draw
     * calls. With quad batching enabled, the renderer accumulates quads
     * into a local cache, and only renders them when necessary
     * (i.e., when the texture changes, when another immediate rendering
     * call is requested, or the frame ends).
     */
    void set_quad_batching_enabled(bool enabled);

    /**
     * \brief Returns the maximum texture width/height (in pixels).
     * \return The maximum texture width/height (in pixels)
     */
    virtual std::size_t get_texture_max_size() const = 0;

    /**
     * \brief Checks if the renderer supports texture atlases natively.
     * \return 'true' if enabled, 'false' otherwise
     * \note If 'false', texture atlases will be implemented using a generic
     * solution with render targets.
     */
    virtual bool is_texture_atlas_supported() const = 0;

    /**
     * \brief Checks if the renderer has texture atlases enabled.
     * \return 'true' if enabled, 'false' otherwise
     */
    bool is_texture_atlas_enabled() const;

    /**
     * \brief Enables/disables texture atlases.
     * \param enabled 'true' to enable texture atlases, 'false' to disable them
     * \note Texture atlases are enabled by default. Changing this flag will only
     * impact newly created materials. Existing materials will not be affected.
     * \note In general, texture atlases only increase performance when vertex caches
     * are supported and used (see is_vertex_cache_supported()). The can actually decrease
     * performance when vertex caches are not supported, if texture tiling is
     * used a lot (e.g., in frame backdrop edges). It is therefore recommended to
     * disable texture atlases if vertex caches are not supported.
     */
    void set_texture_atlas_enabled(bool enabled);

    /**
     * \brief Returns the width/height of a texture atlas page (in pixels).
     * \return The width/height of a texture atlas page (in pixels)
     */
    std::size_t get_texture_atlas_page_size() const;

    /**
     * \brief Set the width/height of a texture atlas page (in pixels).
     * \param page_size The texture width/height in pixels
     * \note Changing this value will only impact newly created atlas pages.
     * Existing pages will not be affected.
     * \note Increase this value to allow more materials to fit on a single atlas
     * page, therefore improving performance. Decrease tihs value if the
     * memory usage from atlas textures is too large. Set it to zero
     * to fall back to the implementation-defined default value.
     */
    void set_texture_atlas_page_size(std::size_t page_size);

    /**
     * \brief Count the total number of texture atlas pages currently in use.
     * \return The total number of texture atlas pages currently in use
     */
    std::size_t get_texture_atlas_page_count() const;

    /**
     * \brief Checks if the renderer supports vertex caches.
     * \return 'true' if supported, 'false' otherwise
     */
    virtual bool is_vertex_cache_supported() const = 0;

    /**
     * \brief Checks if the renderer has enabled support for vertex caches.
     * \return 'true' if vertex caches are supported and enabled, 'false' otherwise
     */
    bool is_vertex_cache_enabled() const;

    /**
     * \brief Enables/disables vertex caches.
     * \param enabled 'true' to enable vertex caches, 'false' to disable them
     * \note Even if enabled with this function, vertex caches may not be supported
     * by the renderer, see is_vertex_cache_supported().
     */
    void set_vertex_cache_enabled(bool enabled);

    /// Automatically determines the best rendering settings for the current platform.
    void auto_detect_settings();

    /**
     * \brief Resets the number of batches to zero (for analytics only).
     * \note See get_batch_count() and get_vertex_count().
     * This should be called a the beginning of a frame.
     */
    void reset_counters();

    /**
     * \brief Returns the number of batches of vertices sent to the GPU since the last call to reset_counters.
     * \return The number of batches of vertices sent to the GPU since the last call to reset_counters
     * \note This will be zero unless is_quad_batching_enabled() is 'true'.
     */
    std::size_t get_batch_count() const;

    /**
     * \brief Returns the number of vertices sent to the GPU since the last call to reset_counters.
     * \return The number of vertices sent to the GPU since the last call to reset_counters
     */
    std::size_t get_vertex_count() const;

    /**
     * \brief Begins rendering on a particular render target.
     * \param target The render target (main screen if nullptr)
     */
    void begin(std::shared_ptr<render_target> target = nullptr);

    /// Ends rendering.
    void end();

    /**
     * \brief Flushes any pending quad batch render operation.
     * \note If is_quad_batching_enabled(), quad rendering is done in batches.
     * This means that the quads are not actually rendered until this
     * function is called. The renderer calls this function automatically in
     * various situations: when calling end(), when trying to render something
     * that is no included in the batching system (such as render_cache()), or
     * when changing the global state of the render (such as with set_view()).
     * If you have your own rendering operations that are not going through this
     * renderer (like raw OpenGL calls), make sure you call this function before
     * doing your own rendering.
     * \note This function is meant to be called between begin() and
     * end() only.
     */
    void flush_quad_batch();

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
    void set_view(const matrix4f& view_matrix);

    /**
     * \brief Returns the current view matrix to use when rendering (viewport).
     * \return The current view matrix to use when rendering
     * \note See set_view() for more information. The returned matrix may be different
     * from the matrix given to set_view(), if the rendering backend does not
     * support certain transformations.
     */
    virtual matrix4f get_view() const = 0;

    /**
     * \brief Renders a quad.
     * \param q The quad to render on the current render target
     * \note This function is meant to be called between begin() and
     * end() only.
     */
    void render_quad(const quad& q);

    /**
     * \brief Renders a set of quads.
     * \param mat The material to use for rendering, or null if none
     * \param quad_list The list of the quads you want to render
     * \note This function is meant to be called between begin() and
     * end() only. When multiple quads share the same material, it is
     * always more efficient to call this method than calling render_quad
     * repeatedly, as it allows to reduce the number of draw calls.
     */
    void render_quads(const material* mat, const std::vector<std::array<vertex, 4>>& quad_list);

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
    void render_cache(
        const material*     mat,
        const vertex_cache& cache,
        const matrix4f&     model_transform = matrix4f::identity);

    /**
     * \brief Creates a new material from a texture file.
     * \param file_name The name of the file
     * \param filt The filtering to apply to the texture
     * \return The new material
     * \note Supported texture formats are defined by implementation.
     * The gui library is completely unaware of this.
     */
    std::shared_ptr<material>
    create_material(const std::string& file_name, material::filter filt = material::filter::none);

    /**
     * \brief Creates a new material from a texture file.
     * \param atlas_category The category of atlas in which to create the texture
     * \param file_name The name of the file
     * \param filt The filtering to apply to the texture
     * \return The new material
     * \note Supported texture formats are defined by implementation.
     * The gui library is completely unaware of this.
     * \note The atlas category is a hint that the implementation can use to select
     * the texture atlas in which to place this new texture. If a group of
     * textures is known to be used to render objects that are often rendered
     * consecutively (for example, various tiles of a background), they should
     * be placed in the same category to maximize the chance of draw call batching.
     * Conversely, if two texture are known to rarely be used in the same context
     * (for example, a special effect particle texture and a UI button texture),
     * they should not be placed in the same category, as this could otherwise
     * fill up the atlas quickly, and reduce batching opportunities.
     * \note Because of how texture atlases work, it is not possible to use texture
     * coordinate wrapping for materials from an atlas. Trying to use coordinates
     * outside the [0,1] range will start reading texture data from another
     * material.
     */
    std::shared_ptr<material> create_atlas_material(
        const std::string& atlas_category,
        const std::string& file_name,
        material::filter   filt = material::filter::none);

    /**
     * \brief Creates a new material from a portion of a render target.
     * \param target The render target from which to read the pixels
     * \param location The portion of the render target to use as material
     * \return The new material
     */
    virtual std::shared_ptr<material>
    create_material(std::shared_ptr<render_target> target, const bounds2f& location) = 0;

    /**
     * \brief Creates a new material from an entire render target.
     * \param target The render target from which to read the pixels
     * \return The new material
     */
    std::shared_ptr<material> create_material(std::shared_ptr<render_target> target);

    /**
     * \brief Creates a new material from arbitrary pixel data.
     * \param dimensions The dimensions of the material
     * \param pixel_data The color data for all the pixels in the material
     * \param filt The filtering to apply to the texture
     * \return The new material
     */
    virtual std::shared_ptr<material> create_material(
        const vector2ui& dimensions,
        const color32*   pixel_data,
        material::filter filt = material::filter::none) = 0;

    /**
     * \brief Creates a new render target.
     * \param dimensions The dimensions of the render target
     * \param filt The filtering to apply to the target texture when displayed
     */
    virtual std::shared_ptr<render_target> create_render_target(
        const vector2ui& dimensions, material::filter filt = material::filter::none) = 0;

    /**
     * \brief Creates a new font.
     * \param font_file The file from which to read the font
     * \param size The requested size of the characters (in points)
     * \param outline The thickness of the outline (in points)
     * \param code_points The list of Unicode characters to load
     * \param default_code_point The character to display as fallback
     * \note Even though the gui has been designed to use vector fonts files
     * (such as .ttf or .otf font formats), nothing prevents the implementation
     * from using any other font type, including bitmap fonts.
     * \note If an outline thickness other than zero is requested, only the
     * outline itself will be rendered by the returned font. A non-outlined font
     * must be rendered above the outlined font to fill the actual characters.
     */
    std::shared_ptr<font> create_font(
        const std::string&                   font_file,
        std::size_t                          size,
        std::size_t                          outline,
        const std::vector<code_point_range>& code_points,
        char32_t                             default_code_point);

    /**
     * \brief Creates a new font.
     * \param atlas_category The category of atlas in which to create the font texture
     * \param font_file The file from which to read the font
     * \param size The requested size of the characters (in points)
     * \param outline The thickness of the outline (in points)
     * \param code_points The list of Unicode characters to load
     * \param default_code_point The character to display as fallback
     * \note Even though the gui has been designed to use vector fonts files
     * (such as .ttf or .otf font formats), nothing prevents the implementation
     * from using any other font type, including bitmap fonts.
     * \note See create_atlas_material() for more information on atlases.
     */
    std::shared_ptr<font> create_atlas_font(
        const std::string&                   atlas_category,
        const std::string&                   font_file,
        std::size_t                          size,
        std::size_t                          outline,
        const std::vector<code_point_range>& code_points,
        char32_t                             default_code_point);

    /**
     * \brief Creates a new empty vertex cache.
     * \param type The type of data this cache will hold
     * \note Not all implementations support vertex caches. See is_vertex_cache_supported().
     */
    virtual std::shared_ptr<vertex_cache> create_vertex_cache(gui::vertex_cache::type type) = 0;

    /**
     * \brief Notifies the renderer that the render window has been resized.
     * \param dimensions The new window dimensions
     */
    virtual void notify_window_resized(const vector2ui& dimensions);

protected:
    /**
     * \brief Begins rendering on a particular render target.
     * \param target The render target (main screen if nullptr)
     */
    virtual void begin_(std::shared_ptr<render_target> target) = 0;

    /// Ends rendering.
    virtual void end_() = 0;

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
    virtual void set_view_(const matrix4f& view_matrix) = 0;

    /**
     * \brief Renders a set of quads.
     * \param mat The material to use for rendering, or null if none
     * \param quad_list The list of the quads you want to render
     * \note This function is meant to be called between begin() and
     * end() only. When multiple quads share the same material, it is
     * always more efficient to call this method than calling render_quad
     * repeatedly, as it allows to reduce the number of draw calls.
     */
    virtual void
    render_quads_(const material* mat, const std::vector<std::array<vertex, 4>>& quad_list) = 0;

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
    virtual void render_cache_(
        const material* mat, const vertex_cache& cache, const matrix4f& model_transform) = 0;

    /**
     * \brief Creates a new material from a texture file.
     * \param file_name The name of the file
     * \param filt The filtering to apply to the texture
     * \return The new material
     * \note Supported texture formats are defined by implementation.
     * The gui library is completely unaware of this.
     */
    virtual std::shared_ptr<material>
    create_material_(const std::string& file_name, material::filter filt) = 0;

    /**
     * \brief Creates a new atlas with a given texture filter mode.
     * \param filt The filtering to apply to the texture
     * \return The new atlas
     */
    virtual std::shared_ptr<atlas> create_atlas_(material::filter filt) = 0;

    /**
     * \brief Creates a new font.
     * \param font_file The file from which to read the font
     * \param size The requested size of the characters (in points)
     * \param outline The thickness of the outline (in points)
     * \param code_points The list of Unicode characters to load
     * \param default_code_point The character to display as fallback
     * \note Even though the gui has been designed to use vector fonts files
     * (such as .ttf or .otf font formats), nothing prevents the implementation
     * from using any other font type, including bitmap fonts.
     */
    virtual std::shared_ptr<font> create_font_(
        const std::string&                   font_file,
        std::size_t                          size,
        std::size_t                          outline,
        const std::vector<code_point_range>& code_points,
        char32_t                             default_code_point) = 0;

    atlas& get_atlas_(const std::string& atlas_category, material::filter filt);

    std::unordered_map<std::string, std::weak_ptr<gui::material>> texture_list_;
    std::unordered_map<std::string, std::shared_ptr<gui::atlas>>  atlas_list_;
    std::unordered_map<std::string, std::weak_ptr<gui::font>>     font_list_;

private:
    bool uses_same_texture_(const material* mat1, const material* mat2) const;

    bool        texture_atlas_enabled_   = true;
    bool        vertex_cache_enabled_    = true;
    bool        quad_batching_enabled_   = true;
    std::size_t texture_atlas_page_size_ = 0u;

    struct quad_batcher {
        std::vector<std::array<vertex, 4>> data;
        std::shared_ptr<vertex_cache>      cache;
    };

    static constexpr std::size_t                        batching_cache_cycle_size = 16u;
    std::array<quad_batcher, batching_cache_cycle_size> quad_cache_;

    const gui::material* current_material_        = nullptr;
    std::size_t          current_quad_cache_      = 0u;
    std::size_t          batch_count_             = 0u;
    std::size_t          vertex_count_            = 0u;
    std::size_t          last_frame_batch_count_  = 0u;
    std::size_t          last_frame_vertex_count_ = 0u;
};

} // namespace lxgui::gui

#endif
