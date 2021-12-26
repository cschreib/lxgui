#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_layoutnode.hpp"

namespace lxgui {
namespace gui
{

void layered_region::parse_layout(const layout_node& mNode)
{
    parse_attributes_(mNode);

    parse_size_node_(mNode);
    parse_anchor_node_(mNode);
}

void layered_region::parse_attributes_(const layout_node& mNode)
{
    if (const layout_attribute* pAttr = mNode.try_get_attribute("hidden"))
        set_shown(pAttr->get_value<bool>());

    if (mNode.get_attribute_value_or<bool>("setAllPoints", false))
        set_all_points("$parent");
}

}
}
