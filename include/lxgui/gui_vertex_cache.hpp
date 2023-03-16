#ifndef LXGUI_GUI_VERTEX_CACHE_HPP
#define LXGUI_GUI_VERTEX_CACHE_HPP

#include "lxgui/gui_vertex.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <memory>
#include <vector>

namespace lxgui::gui {

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
class vertex_cache {
public:
    /// The type of vertex data contained in a vertex_cache.
    enum class type {
        triangles, /// 3 vertices per element
        quads /// 4 vertices per element
    };

    /**
     * \brief Constructor.
     * \param t The type of data this cache will hold
     * \details A default constructed vertex cache holds no data. Use update()
     * to store vertices to be rendered.
     */
    explicit vertex_cache(type t);

    /// Destructor.
    virtual ~vertex_cache() = default;

    /// Non-copiable
    vertex_cache(const vertex_cache&) = delete;

    /// Non-movable
    vertex_cache(vertex_cache&&) = delete;

    /// Non-copiable
    vertex_cache& operator=(const vertex_cache&) = delete;

    /// Non-movable
    vertex_cache& operator=(vertex_cache&&) = delete;

    /**
     * \brief Update the data stored in the cache to form new triangles.
     * \param vertex_data The vertices to cache
     * \param num_vertex The number of vertices to cache
     * \note If the type if TRIANGLES, num_vertex must be a multiple of 3.
     * If the type if QUADS, num_vertex must be a multiple of 4.
     */
    virtual void update(const vertex* vertex_data, std::size_t num_vertex) = 0;

    /**
     * \brief Returns the number of vertices stored in this cache.
     * \return The number of vertices stored in this cache
     */
    std::size_t get_vertex_count() const {
        return num_vertex_;
    }

protected:
    type        type_       = type::triangles;
    std::size_t num_vertex_ = 0;
};

} // namespace lxgui::gui

#endif
