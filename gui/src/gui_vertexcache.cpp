#include "lxgui/gui_vertexcache.hpp"
#include "lxgui/gui_material.hpp"

namespace lxgui {
namespace gui
{

vertex_cache::vertex_cache(std::shared_ptr<material> pMaterial) : pMaterial_(pMaterial)
{
}

std::shared_ptr<material> vertex_cache::get_material() const
{
    return pMaterial_;
}

}
}
