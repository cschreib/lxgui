#include "lxgui/gui_layout_node.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_status_bar.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

void status_bar::parse_attributes_(const layout_node& node) {
    frame::parse_attributes_(node);

    if (const auto attr = node.try_get_attribute_value<float>("minValue"))
        set_min_value(attr.value());
    if (const auto attr = node.try_get_attribute_value<float>("maxValue"))
        set_max_value(attr.value());
    if (const auto attr = node.try_get_attribute_value<float>("defaultValue"))
        set_value(attr.value());
    if (const auto attr = node.try_get_attribute_value<layer>("drawLayer"))
        set_bar_draw_layer(attr.value());
    if (const auto attr = node.try_get_attribute_value<orientation>("orientation"))
        set_orientation(attr.value());
    if (const auto attr = node.try_get_attribute_value<bool>("reversed"))
        set_reversed(attr.value());
}

void status_bar::parse_all_nodes_before_children_(const layout_node& node) {
    frame::parse_all_nodes_before_children_(node);

    const layout_node* texture_node = node.try_get_child("BarTexture");
    const layout_node* color_node   = node.try_get_child("BarColor");
    if (color_node && texture_node) {
        gui::out << gui::warning << node.get_location()
                 << ": StatusBar can only contain one of BarTexture or BarColor, but not both. "
                    "BarColor ignored."
                 << std::endl;
    }

    if (texture_node) {
        layout_node defaulted = *texture_node;
        defaulted.get_or_set_attribute_value("name", "$parentBarTexture");

        auto bar_texture = parse_region_(defaulted, "ARTWORK", "Texture");
        if (bar_texture) {
            bar_texture->set_manually_inherited(true);
            set_bar_texture(utils::static_pointer_cast<texture>(bar_texture));
        }

        warn_for_not_accessed_node(defaulted);
        texture_node->bypass_access_check();
    } else if (color_node) {
        set_bar_color(parse_color_node_(*color_node));
    }
}

} // namespace lxgui::gui
