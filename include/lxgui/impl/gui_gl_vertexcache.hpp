#ifndef LXGUI_GUI_GL_VERTEXCACHE_HPP
#define LXGUI_GUI_GL_VERTEXCACHE_HPP

#include "lxgui/gui_vertexcache.hpp"
#include "lxgui/utils.hpp"

#include <limits>
#include <memory>

namespace lxgui::gui::gl {

/**
 * \brief An object representing cached vertex data on the GPU
 * \details A vertex cache stores vertices and indices that can be used to draw
 * any shape on the screen. If the type is TRIANGLES, each group of 3
 * vertices forms a triangle, while if the type is QUADS, each group of 4
 * vertices forms a quad.
 *
 * Using this class enables more efficient rendering of large groups
 * of sprites or quads that share the same material (texture). This
 * is especially true if the data to render does not change often.
 *
 * A vertex cache can be rendered with gui::renderer::render_vertex_cache().
 *
 * \note This is an abstract class that must be inherited
 * from and created by the corresponding gui::renderer.
 */
class vertex_cache final : public gui::vertex_cache {
public:
    /**
     * \brief Constructor.
     * \param t The type of data this cache will hold
     * \details A default constructed vertex cache holds no data. Use update()
     * to store vertices to be rendered. The size hint can enable the cache to be
     * pre-allocated, which will avoid a reallocation when update() is called.
     */
    explicit vertex_cache(type t);

    /// Destructor.
    ~vertex_cache() override;

    /**
     * \brief Update the data stored in the cache, reusing existing indices.
     * \param vertex_data The vertices to cache
     * \param num_vertex The number of vertices to cache
     */
    void update_data(const vertex* vertex_data, std::size_t num_vertex);

    /**
     * \brief Update the indices stored in the cache, reusing existing data.
     * \param vertex_indices The indices to use for drawing triangles
     * \param num_indices The number of indices to cache
     */
    void update_indices(const std::uint32_t* vertex_indices, std::size_t num_indices);

    /**
     * \brief Update the indices stored in the cache, but only if the current index cache is smaller.
     * \param vertex_indices The indices to use for drawing triangles
     * \param num_indices The number of indices to cache
     * \note This function assumes that the index buffer is always initialised with
     * valid indices, and that only the *number* of indices changes. Indices that
     * were set in previous calls of update_indices() are assumed to not change
     * value.
     */
    void update_indices_if_grow(const std::uint32_t* vertex_indices, std::size_t num_indices);

    /**
     * \brief Update the data stored in the cache to form new triangles.
     * \param vertex_data The vertices to cache
     * \param num_vertex The number of vertices to cache
     * \note If the type if TRIANGLES, num_vertex must be a multiple of 3.
     * If the type if QUADS, num_vertex must be a multiple of 4.
     */
    void update(const vertex* vertex_data, std::size_t num_vertex) override;

    /**
     * \brief Renders the cache.
     * \note This does not bind the material, just binds the cache and renders it
     * with whatever shader / texture is currently bound.
     */
    void render() const;

private:
    std::size_t   current_size_vertex_     = 0u;
    std::size_t   current_size_index_      = 0u;
    std::size_t   current_capacity_vertex_ = 0u;
    std::size_t   current_capacity_index_  = 0u;
    std::uint32_t vertex_array_            = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t vertex_buffer_           = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t index_buffer_            = std::numeric_limits<std::uint32_t>::max();
};

} // namespace lxgui::gui::gl

#endif
