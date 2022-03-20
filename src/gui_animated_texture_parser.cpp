#include "lxgui/gui_animated_texture.hpp"
#include "lxgui/gui_layout_node.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

namespace lxgui::gui {

void animated_texture::parse_layout(const layout_node& node) {
    layered_region::parse_layout(node);

    // TODO
}

void animated_texture::parse_attributes_(const layout_node& node) {
    layered_region::parse_attributes_(node);

    if (const layout_attribute* attr = node.try_get_attribute("file"))
        set_texture(attr->get_value<std::string>());

    // TODO
}

} // namespace lxgui::gui
