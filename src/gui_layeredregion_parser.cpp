#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

namespace lxgui::gui {

void layered_region::parse_layout(const layout_node& m_node) {
    parse_attributes_(m_node);

    parse_size_node_(m_node);
    parse_anchor_node_(m_node);
}

void layered_region::parse_attributes_(const layout_node& m_node) {
    if (const layout_attribute* p_attr = m_node.try_get_attribute("hidden"))
        set_shown(p_attr->get_value<bool>());

    if (m_node.get_attribute_value_or<bool>("setAllPoints", false))
        set_all_points("$parent");
}

} // namespace lxgui::gui
