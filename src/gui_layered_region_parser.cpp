#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_layout_node.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

namespace lxgui::gui {

void layered_region::parse_layout(const layout_node& node) {
    parse_attributes_(node);

    parse_size_node_(node);
    parse_anchor_node_(node);
}

void layered_region::parse_attributes_(const layout_node& node) {
    if (const layout_attribute* attr = node.try_get_attribute("hidden"))
        set_shown(attr->get_value<bool>());

    if (node.get_attribute_value_or<bool>("setAllAnchors", false))
        set_all_anchors("$parent");
}

} // namespace lxgui::gui
