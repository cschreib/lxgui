#ifndef LXGUI_GUI_VERTEXCACHE_HPP
#define LXGUI_GUI_VERTEXCACHE_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_vertex.hpp"

#include <vector>
#include <memory>

namespace lxgui {
namespace gui
{
    /// An object representing cached vertex data on the GPU
    /** A vertex cache stores vertices and indices that can be used to draw
    *   any shape on the screen. If the type is TRIANGLES, each group of 3
    *   vertices forms a triangle, while if the type is QUADS, each group of 4
    *   vertices forms a quad.
    *
    *   Using this class enables more efficient rendering of large groups
    *   of sprites or quads that share the same material (texture). This
    *   is especially true if the data to render does not change often.
    *
    *   A vertex cache can be rendered with gui::renderer::render_vertex_cache().
    *
    *   \note This is an abstract class that must be inherited
    *         from and created by the corresponding gui::renderer.
    */
    class vertex_cache
    {
    public :

        /// The type of vertex data contained in a vertex_cache.
        enum class type
        {
            TRIANGLES, /// 3 vertices per element
            QUADS      /// 4 vertices per element
        };

        /// Constructor.
        /** \param mType The type of data this cache will hold
        *   \details A default constructed vertex cache holds no data. Use update()
        *            to store vertices to be rendered.
        */
        explicit vertex_cache(type mType);

        /// Destructor.
        virtual ~vertex_cache() = default;

        /// Update the data stored in the cache to form new triangles.
        /** \param lVertexData The vertices to cache
        *   \param uiNumVertex The number of vertices to cache
        *   \param uiPosition  The position in the cache where vertices should be copied
        *   \note If the type if TRIANGLES, uiNumVertex must be a multiple of 3.
        *         If the type if QUADS, uiNumVertex must be a multiple of 4.
        *   \note The cache will automatically grow if there is not enough space to store
        *         all the requested vertices. If you need the buffer to shrink in size,
        *         call clear() before update().
        */
        virtual void update(const vertex* lVertexData, uint uiNumVertex, uint uiPosition = 0u) = 0;

        /// Returns the number of vertices currently stored in the cache.
        /** \return The number of vertices currently stored in the cache
        */
        virtual uint get_num_vertex() const = 0;

        /// Reset the size of this cache to zero.
        /** \return The number of vertices currently stored in the cache
        */
        virtual void clear() = 0;

    protected:

        type mType_ = type::TRIANGLES;
    };
}
}

#endif
