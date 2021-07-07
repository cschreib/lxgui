#include "lxgui/impl/gui_sfml_atlas.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"
#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui {
namespace sfml
{

atlas_page::atlas_page(material::filter mFilter) {}

std::shared_ptr<gui::material> atlas_page::add_material_(const gui::material& mMat,
    const quad2f& mLocation) const
{
    // TODO
    return nullptr;
}

atlas::atlas(material::filter mFilter) : gui::atlas(mFilter) {}

std::unique_ptr<gui::atlas_page> atlas::create_page_() const
{
    return std::make_unique<sfml::atlas_page>(mFilter_);
}

}
}
}
