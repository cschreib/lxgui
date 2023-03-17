#include "lxgui/impl/gui_sfml_vertex_cache.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/utils_string.hpp"

#include <SFML/Graphics/Vertex.hpp>
#include <array>

namespace lxgui::gui::sfml {

vertex_cache::vertex_cache(type t) : type_(t), buffer_(sf::PrimitiveType::Triangles) {
    // Needed to initialise the buffer; will resize automatically later.
    if (!buffer_.create(0u))
        throw gui::exception("sfml::vertex_cache", "Could not initialise buffer");
}

void to_sfml(const vertex& v, sf::Vertex& sv) {
    sv.position.x  = v.pos.x;
    sv.position.y  = v.pos.y;
    sv.color.r     = static_cast<std::uint8_t>(v.col.a * v.col.r * 255.0); // Premultipled alpha
    sv.color.g     = static_cast<std::uint8_t>(v.col.a * v.col.g * 255.0); // Premultipled alpha
    sv.color.b     = static_cast<std::uint8_t>(v.col.a * v.col.b * 255.0); // Premultipled alpha
    sv.color.a     = static_cast<std::uint8_t>(v.col.a * 255.0);
    sv.texCoords.x = v.uvs.x;
    sv.texCoords.y = v.uvs.y;
}

void vertex_cache::update(const vertex* vertex_data, std::size_t num_vertex) {
    if (type_ == type::quads) {
        static constexpr std::array<std::size_t, 6> quad_ids = {{0, 1, 2, 2, 3, 0}};

        if (num_vertex % 4u != 0u) {
            throw gui::exception(
                "gui::gl::vertex_cache",
                "Number of vertices in quad array must be a multiple of 4 (got " +
                    utils::to_string(num_vertex) + ").");
        }

        const std::size_t       num_quads           = num_vertex / 4u;
        const std::size_t       num_vertex_expanded = num_quads * 6u;
        std::vector<sf::Vertex> sf_vertices(num_vertex_expanded);
        for (std::size_t i = 0; i < num_vertex_expanded; ++i) {
            auto&       sv = sf_vertices[i];
            const auto& v  = vertex_data[(i / 6u) * 4u + quad_ids[i % 6u]];
            to_sfml(v, sv);
        }

        if (!buffer_.update(sf_vertices.data(), num_vertex_expanded, 0)) {
            throw gui::exception("sfml::vertex_cache", "Could not update cache");
        }

        num_vertex_ = num_vertex_expanded;
    } else {
        if (num_vertex % 3u != 0u) {
            throw gui::exception(
                "gui::gl::vertex_cache",
                "Number of vertices in triangle array must be a multiple of 3 (got " +
                    utils::to_string(num_vertex) + ").");
        }

        std::vector<sf::Vertex> sf_vertices(num_vertex);
        for (std::size_t i = 0; i < num_vertex; ++i) {
            auto&       sv = sf_vertices[i];
            const auto& v  = vertex_data[i];
            to_sfml(v, sv);
        }

        if (!buffer_.update(sf_vertices.data(), num_vertex, 0)) {
            throw gui::exception("sfml::vertex_cache", "Could not update cache");
        }

        num_vertex_ = num_vertex;
    }
}

std::size_t vertex_cache::get_vertex_count() const {
    // NB: Don't use buffer_.getVertexCount(). This is the capacity, not the size.
    return num_vertex_;
}

const sf::VertexBuffer& vertex_cache::get_impl() const {
    return buffer_;
}

} // namespace lxgui::gui::sfml
