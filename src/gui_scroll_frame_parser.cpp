#include "lxgui/gui_layout_node.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_scroll_frame.hpp"

namespace lxgui::gui {

void scroll_frame::parse_all_nodes_before_children_(const layout_node& node) {
    frame::parse_all_nodes_before_children_(node);
    parse_scroll_child_node_(node);
}

void scroll_frame::parse_scroll_child_node_(const layout_node& node) {
    if (const layout_node* scroll_child_node = node.try_get_child("ScrollChild")) {
        if (scroll_child_node->get_children_count() == 0) {
            gui::out << gui::warning << scroll_child_node->get_location()
                     << ": ScrollChild node needs a child node." << std::endl;
            return;
        }

        if (scroll_child_node->get_children_count() > 1) {
            gui::out << gui::warning << scroll_child_node->get_location()
                     << ": ScrollChild node needs only one child node; other nodes will be ignored."
                     << std::endl;
            return;
        }

        const layout_node& child_node   = scroll_child_node->get_child(0);
        auto               scroll_child = parse_child_(child_node, "");
        if (!scroll_child)
            return;

        const layout_node* anchors = child_node.try_get_child("Anchors");
        if (anchors) {
            gui::out << gui::warning << anchors->get_location() << ": "
                     << "Scroll child's anchors are ignored." << std::endl;
        }

        if (!child_node.has_child("Size")) {
            gui::out << gui::warning << child_node.get_location()
                     << ": Scroll child needs its size to be defined in a Size block." << std::endl;
        }

        this->set_scroll_child(remove_child(scroll_child));
    }
}

} // namespace lxgui::gui
