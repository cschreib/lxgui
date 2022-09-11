#include "lxgui/gui_animated_texture.hpp"
#include "lxgui/gui_layout_node.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

namespace lxgui::gui {

void animated_texture::parse_layout(const layout_node& node) {
    layered_region::parse_layout(node);
}

void animated_texture::parse_attributes_(const layout_node& node) {
    layered_region::parse_attributes_(node);

    if (const auto attr = node.try_get_attribute_value<std::string>("file"))
        set_texture(attr.value());

    if (const auto attr = node.try_get_attribute_value<float>("speed"))
        set_speed(attr.value());

    if (const auto attr = node.try_get_attribute_value<float>("state"))
        set_state(attr.value());

    if (const auto attr = node.try_get_attribute_value<bool>("paused"))
        set_paused(attr.value());
}

} // namespace lxgui::gui
