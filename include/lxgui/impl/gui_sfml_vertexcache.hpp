#ifndef LXGUI_GUI_SFML_VERTEXCACHE_HPP
#define LXGUI_GUI_SFML_VERTEXCACHE_HPP

#include <lxgui/gui_vertexcache.hpp>
#include <lxgui/utils.hpp>

#include <SFML/Graphics/VertexBuffer.hpp>

#include <memory>

namespace lxgui {
namespace gui {
namespace sfml
{
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
    class vertex_cache : public gui::vertex_cache
    {
    public :

        /// Constructor.
        /** \param mType The type of data this cache will hold
        *   \details A default constructed vertex cache holds no data. Use update()
        *            to store vertices to be rendered. The size hint can enable the cache to be
        *            pre-allocated, which will avoid a reallocation when update() is called.
        */
        explicit vertex_cache(type mType);

        /// Update the data stored in the cache to form new triangles.
        /** \param lVertexData The vertices to cache
        *   \param uiNumVertex The number of vertices to cache
        *   \note If the type if TRIANGLES, uiNumVertex must be a multiple of 3.
        *         If the type if QUADS, uiNumVertex must be a multiple of 4.
        */
        void update(const vertex* lVertexData, uint uiNumVertex) override;

        /// Returns the number of vertices currently stored in the cache.
        /** \return The number of vertices currently stored in the cache
        */
        uint get_num_vertex() const;

        /// Returns the SFML vertex buffer object.
        /** \return The SFML vertex buffer object
        */
        const sf::VertexBuffer& get_impl() const;

    private :

        uint uiNumVertex_ = 0u;
        sf::VertexBuffer mBuffer_;
    };
}
}
}

#endif
