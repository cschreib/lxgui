#include "lxgui/impl/gui_sfml_vertexcache.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/utils_string.hpp"

#include <SFML/Graphics/Vertex.hpp>
#include <array>

namespace lxgui::gui::sfml {

vertex_cache::vertex_cache(type m_type) :
    gui::vertex_cache(m_type), m_buffer_(sf::PrimitiveType::Triangles) {}

void to_sfml(const vertex& v, sf::Vertex& sv) {
    sv.position.x  = v.pos.x;
    sv.position.y  = v.pos.y;
    sv.color.r     = static_cast<sf::Uint8>(v.col.a * v.col.r * 255.0); // Premultipled alpha
    sv.color.g     = static_cast<sf::Uint8>(v.col.a * v.col.g * 255.0); // Premultipled alpha
    sv.color.b     = static_cast<sf::Uint8>(v.col.a * v.col.b * 255.0); // Premultipled alpha
    sv.color.a     = static_cast<sf::Uint8>(v.col.a * 255.0);
    sv.texCoords.x = v.uvs.x;
    sv.texCoords.y = v.uvs.y;
}

void vertex_cache::update(const vertex* l_vertex_data, std::size_t ui_num_vertex) {
    if (m_type_ == type::quads) {
        static constexpr std::array<std::size_t, 6> l_quad_i_ds = {{0, 1, 2, 2, 3, 0}};

        std::size_t ui_num_quads          = ui_num_vertex / 4u;
        std::size_t ui_num_vertex_expanded = ui_num_quads * 6u;
        if (ui_num_vertex_expanded > m_buffer_.getVertexCount())
            m_buffer_.create(ui_num_vertex_expanded);

        std::vector<sf::Vertex> l_vertices(ui_num_vertex_expanded);
        for (std::size_t i = 0; i < ui_num_vertex_expanded; ++i) {
            auto&       sv = l_vertices[i];
            const auto& v  = l_vertex_data[(i / 6u) * 4u + l_quad_i_ds[i % 6u]];
            to_sfml(v, sv);
        }

        m_buffer_.update(l_vertices.data(), ui_num_vertex_expanded, 0);
        ui_num_vertex_ = ui_num_vertex_expanded;
    } else {
        if (ui_num_vertex > m_buffer_.getVertexCount())
            m_buffer_.create(ui_num_vertex);

        std::vector<sf::Vertex> l_vertices(ui_num_vertex);
        for (std::size_t i = 0; i < ui_num_vertex; ++i) {
            auto&       sv = l_vertices[i];
            const auto& v  = l_vertex_data[i];
            to_sfml(v, sv);
        }

        m_buffer_.update(l_vertices.data(), ui_num_vertex, 0);
        ui_num_vertex_ = ui_num_vertex;
    }
}

std::size_t vertex_cache::get_num_vertex() const {
    return ui_num_vertex_;
}

const sf::VertexBuffer& vertex_cache::get_impl() const {
    return m_buffer_;
}

} // namespace lxgui::gui::sfml
