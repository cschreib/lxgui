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

    if (const layout_attribute* attr = node.try_get_attribute("text")) {
        set_text(utils::utf8_to_unicode(
            get_manager().get_localizer().localize(attr->get_value<std::string>())));
    }

    if (const layout_attribute* attr = node.try_get_attribute("nonSpaceWrap"))
        set_non_space_wrap_enabled(attr->get_value<bool>());

    if (const layout_attribute* attr = node.try_get_attribute("spacing"))
        set_spacing(attr->get_value<float>());

    if (const layout_attribute* attr = node.try_get_attribute("lineSpacing"))
        set_line_spacing(attr->get_value<float>());

    if (const layout_attribute* attr = node.try_get_attribute("outline")) {
        const std::string& outline = attr->get_value<std::string>();
        if (outline == "NORMAL" || outline == "THICK")
            set_outlined(true);
        else if (outline == "NONE")
            set_outlined(false);
        else {
            gui::out << gui::warning << node.get_location() << ": "
                     << "Unknown outline type for " << name_ << ": \"" << outline << "\"."
                     << std::endl;
        }
    }

    if (const layout_attribute* attr = node.try_get_attribute("alignX")) {
        const std::string& align_x = attr->get_value<std::string>();
        if (align_x == "LEFT")
            set_alignment_x(alignment_x::left);
        else if (align_x == "CENTER")
            set_alignment_x(alignment_x::center);
        else if (align_x == "RIGHT")
            set_alignment_x(alignment_x::right);
        else {
            gui::out << gui::warning << node.get_location() << ": "
                     << "Unknown horizontal alignment behavior for " << name_ << ": \"" << align_x
                     << "\"." << std::endl;
        }
    }

    if (const layout_attribute* attr = node.try_get_attribute("alignY")) {
        const std::string& align_y = attr->get_value<std::string>();
        if (align_y == "TOP")
            set_alignment_y(alignment_y::top);
        else if (align_y == "MIDDLE")
            set_alignment_y(alignment_y::middle);
        else if (align_y == "BOTTOM")
            set_alignment_y(alignment_y::bottom);
        else {
            gui::out << gui::warning << node.get_location() << ": "
                     << "Unknown vertical alignment behavior for " << name_ << ": \"" << align_y
                     << "\"." << std::endl;
        }
    }
}

void font_string::parse_shadow_node_(const layout_node& node) {
    if (const layout_node* shadow_node = node.try_get_child("Shadow")) {
        enable_shadow();

        if (const layout_node* color_node = shadow_node->try_get_child("Color"))
            set_shadow_color(parse_color_node_(*color_node));

        if (const layout_node* offset_node = shadow_node->try_get_child("Offset")) {
            set_shadow_offset(vector2f(
                offset_node->get_attribute_value_or<float>("x", 0.0),
                offset_node->get_attribute_value_or<float>("y", 0.0)));
        }
    }
}

} // namespace lxgui::gui
