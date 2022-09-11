#include "lxgui/gui_font_string.hpp"
#include "lxgui/gui_layout_node.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

namespace lxgui::gui {

void font_string::parse_layout(const layout_node& node) {
    layered_region::parse_layout(node);

    if (const layout_node* color_node = node.try_get_child("Color"))
        set_text_color(parse_color_node_(*color_node));

    parse_shadow_node_(node);
}

void font_string::parse_attributes_(const layout_node& node) {
    layered_region::parse_attributes_(node);

    set_font(
        node.get_attribute_value_or<std::string>("font", ""),
        node.get_attribute_value_or<float>("fontHeight", 0.0f));

    if (const auto attr = node.try_get_attribute_value<std::string>("text"))
        set_text(utils::utf8_to_unicode(get_manager().get_localizer().localize(attr.value())));
    if (const auto attr = node.try_get_attribute_value<bool>("nonSpaceWrap"))
        set_non_space_wrap_enabled(attr.value());
    if (const auto attr = node.try_get_attribute_value<float>("spacing"))
        set_spacing(attr.value());
    if (const auto attr = node.try_get_attribute_value<float>("lineSpacing"))
        set_line_spacing(attr.value());

    if (const auto attr = node.try_get_attribute_value<std::string>("outline")) {
        std::string outline = utils::to_lower(attr.value());
        if (outline == "normal" || outline == "thick") {
            // TODO: fix this in https://github.com/cschreib/lxgui/issues/121
            set_outlined(true);
        } else if (outline == "none")
            set_outlined(false);
        else {
            gui::out << gui::warning << node.get_location() << ": "
                     << "Unknown outline type for " << name_ << ": \"" << outline << "\"."
                     << std::endl;
        }
    }

    if (const auto attr = node.try_get_attribute_value<alignment_x>("alignX"))
        set_alignment_x(attr.value());
    if (const auto attr = node.try_get_attribute_value<alignment_y>("alignY"))
        set_alignment_y(attr.value());
    if (const auto attr =
            node.try_get_attribute_value<vertex_cache_strategy>("vertexCacheStrategy"))
        set_vertex_cache_strategy(attr.value());
}

void font_string::parse_shadow_node_(const layout_node& node) {
    if (const layout_node* shadow_node = node.try_get_child("Shadow")) {
        enable_shadow();

        if (const layout_node* color_node = shadow_node->try_get_child("Color"))
            set_shadow_color(parse_color_node_(*color_node));

        if (const layout_node* offset_node = shadow_node->try_get_child("Offset")) {
            set_shadow_offset(parse_offset_node_or_(*offset_node, 0.0f));
        }
    }
}

} // namespace lxgui::gui
