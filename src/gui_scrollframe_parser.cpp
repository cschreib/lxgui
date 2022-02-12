#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_scrollframe.hpp"

namespace lxgui::gui {

void scroll_frame::parse_all_nodes_before_children_(const layout_node& m_node) {
    frame::parse_all_nodes_before_children_(m_node);
    parse_scroll_child_node_(m_node);
}

void scroll_frame::parse_scroll_child_node_(const layout_node& m_node) {
    if (const layout_node* p_scroll_child_node = m_node.try_get_child("ScrollChild")) {
        if (p_scroll_child_node->get_children_count() == 0) {
            gui::out << gui::warning << p_scroll_child_node->get_location()
                     << " : "
                        "ScrollChild node needs a child node."
                     << std::endl;
            return;
        }

        if (p_scroll_child_node->get_children_count() > 1) {
            gui::out << gui::warning << p_scroll_child_node->get_location()
                     << " : "
                        "ScrollChild node needs only one child node; other nodes will be ignored."
                     << std::endl;
            return;
        }

        const layout_node& m_child_node   = p_scroll_child_node->get_child(0);
        auto               p_scroll_child = parse_child_(m_child_node, "");
        if (!p_scroll_child)
            return;

        const layout_node* p_anchors = m_child_node.try_get_child("Anchors");
        if (p_anchors) {
            gui::out << gui::warning << p_anchors->get_location() << " : "
                     << "Scroll child's anchors are ignored." << std::endl;
        }

        if (!m_child_node.has_child("Size")) {
            gui::out << gui::warning << m_child_node.get_location()
                     << " : "
                        "Scroll child needs its size to be defined in a Size block."
                     << std::endl;
        }

        this->set_scroll_child(remove_child(p_scroll_child));
    }
}

} // namespace lxgui::gui
