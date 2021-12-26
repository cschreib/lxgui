#include "lxgui/impl/gui_sfml_vertexcache.hpp"

#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

#include <SFML/Graphics/Vertex.hpp>

#include <array>

namespace lxgui {
namespace gui {
namespace sfml
{

vertex_cache::vertex_cache(type mType) :
    gui::vertex_cache(mType), mBuffer_(sf::PrimitiveType::Triangles)
{
}

void to_sfml(const vertex& v, sf::Vertex& sv)
{
    sv.position.x = v.pos.x;
    sv.position.y = v.pos.y;
    sv.color.r = static_cast<sf::Uint8>(v.col.a*v.col.r*255.0); // Premultipled alpha
    sv.color.g = static_cast<sf::Uint8>(v.col.a*v.col.g*255.0); // Premultipled alpha
    sv.color.b = static_cast<sf::Uint8>(v.col.a*v.col.b*255.0); // Premultipled alpha
    sv.color.a = static_cast<sf::Uint8>(v.col.a*255.0);
    sv.texCoords.x = v.uvs.x;
    sv.texCoords.y = v.uvs.y;
}

void vertex_cache::update(const vertex* lVertexData, std::size_t uiNumVertex)
{
    if (mType_ == type::QUADS)
    {
        static constexpr std::array<std::size_t, 6> lQuadIDs = {{0, 1, 2, 2, 3, 0}};

        std::size_t uiNumQuads = uiNumVertex/4u;
        std::size_t uiNumVertexExpanded = uiNumQuads*6u;
        if (uiNumVertexExpanded > mBuffer_.getVertexCount())
            mBuffer_.create(uiNumVertexExpanded);

        std::vector<sf::Vertex> lVertices(uiNumVertexExpanded);
        for (std::size_t i = 0; i < uiNumVertexExpanded; ++i)
        {
            auto& sv = lVertices[i];
            const auto& v = lVertexData[(i/6u)*4u + lQuadIDs[i%6u]];
            to_sfml(v, sv);
        }

        mBuffer_.update(lVertices.data(), uiNumVertexExpanded, 0);
        uiNumVertex_ = uiNumVertexExpanded;
    }
    else
    {
        if (uiNumVertex > mBuffer_.getVertexCount())
            mBuffer_.create(uiNumVertex);

        std::vector<sf::Vertex> lVertices(uiNumVertex);
        for (std::size_t i = 0; i < uiNumVertex; ++i)
        {
            auto& sv = lVertices[i];
            const auto& v = lVertexData[i];
            to_sfml(v, sv);
        }

        mBuffer_.update(lVertices.data(), uiNumVertex, 0);
        uiNumVertex_ = uiNumVertex;
    }
}

std::size_t vertex_cache::get_num_vertex() const
{
    return uiNumVertex_;
}

const sf::VertexBuffer& vertex_cache::get_impl() const
{
    return mBuffer_;
}

}
}
}
