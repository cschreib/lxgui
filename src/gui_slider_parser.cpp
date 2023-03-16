#include "lxgui/gui_layout_node.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

void slider::parse_attributes_(const layout_node& node) {
    frame::parse_attributes_(node);

    if (const auto attr = node.try_get_attribute_value<float>("valueStep"))
        set_value_step(attr.value());
    if (const auto attr = node.try_get_attribute_value<float>("minValue"))
        set_min_value(attr.value());
    if (const auto attr = node.try_get_attribute_value<float>("maxValue"))
        set_max_value(attr.value());
    if (const auto attr = node.try_get_attribute_value<float>("defaultValue"))
        set_value(attr.value());
    if (const auto attr = node.try_get_attribute_value<layer>("drawLayer"))
        set_thumb_draw_layer(attr.value());
    if (const auto attr = node.try_get_attribute_value<orientation>("orientation"))
        set_orientation(attr.value());
}

void slider::parse_all_nodes_before_children_(const layout_node& node) {
    frame::parse_all_nodes_before_children_(node);

    if (const layout_node* thumb_node = node.try_get_child("ThumbTexture")) {
        layout_node defaulted = *thumb_node;
        defaulted.get_or_set_attribute_value("name", "$parentThumbTexture");

        auto thumb_texture = parse_region_(defaulted, "ARTWORK", "Texture");
        if (thumb_texture) {
            thumb_texture->set_manually_inherited(true);
            set_thumb_texture(utils::static_pointer_cast<texture>(thumb_texture));
        }

        warn_for_not_accessed_node(defaulted);
        thumb_node->bypass_access_check();
    }
}

} // namespace lxgui::gui
