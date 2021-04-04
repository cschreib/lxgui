#include "lxgui/gui_vertexcache.hpp"
#include "lxgui/gui_exception.hpp"
#include "lxgui/utils_string.hpp"

#include <array>
#include <vector>

namespace lxgui {
namespace gui
{

void vertex_cache::update_quads(const vertex* lVertexData, uint uiNumVertex)
{
    static constexpr std::array<uint, 6> lQuadIDs = {{0, 1, 2, 2, 3, 0}};
    static thread_local std::vector<uint> lRepeatedIds;

    if (uiNumVertex % 4 != 0)
    {
        throw gui::exception("gui::vertex_cache",
            "Number of vertices in quad array must be a multiple of 4 "
            "(got "+utils::to_string(uiNumVertex)+").");
    }

    // Update the vertex data
    update_data(lVertexData, uiNumVertex);

    // Update the repeated quads IDs array if it needs to grow
    uint uiNumIndices = (uiNumVertex/4u)*6u;
    if (uiNumIndices > lRepeatedIds.size())
    {
        uint uiOldSize = lRepeatedIds.size();
        lRepeatedIds.resize(uiNumIndices);
        for (uint i = uiOldSize; i < uiNumIndices; ++i)
        {
            lRepeatedIds[i] = (i/6)*4 + lQuadIDs[i%6];
        }
    }

    // Update the index cache
    update_indices_if_grow(lRepeatedIds.data(), uiNumIndices);
}

}
}
