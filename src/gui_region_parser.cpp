#include "lxgui/gui_region.hpp"
#include "lxgui/gui_layoutnode.hpp"

namespace lxgui {
namespace gui
{
void region::parse_layout(const layout_node& mNode)
{
    parse_attributes_(mNode);
    parse_size_node_(mNode);
    parse_anchor_node_(mNode);
}

void region::parse_attributes_(const layout_node& mNode)
{
    if (mNode.get_attribute_value_or<bool>("setAllPoints", false))
        set_all_points("$parent");
}
}
}
