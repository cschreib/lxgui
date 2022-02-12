#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

namespace lxgui::gui {

void font_string::parse_layout(const layout_node& m_node) {
    layered_region::parse_layout(m_node);

    if (const layout_node* p_color_node = m_node.try_get_child("Color"))
        set_text_color(parse_color_node_(*p_color_node));

    parse_shadow_node_(m_node);
}

void font_string::parse_attributes_(const layout_node& m_node) {
    layered_region::parse_attributes_(m_node);

    set_font(
        m_node.get_attribute_value_or<std::string>("font", ""),
        m_node.get_attribute_value_or<float>("fontHeight", 0.0f));

    if (const layout_attribute* p_attr = m_node.try_get_attribute("text")) {
        set_text(utils::utf8_to_unicode(
            get_manager().get_localizer().localize(p_attr->get_value<std::string>())));
    }

    if (const layout_attribute* p_attr = m_node.try_get_attribute("nonspacewrap"))
        set_non_space_wrap(p_attr->get_value<bool>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("spacing"))
        set_spacing(p_attr->get_value<float>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("lineSpacing"))
        set_line_spacing(p_attr->get_value<float>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("outline")) {
        const std::string& s_outline = p_attr->get_value<std::string>();
        if (s_outline == "NORMAL" || s_outline == "THICK")
            set_outlined(true);
        else if (s_outline == "NONE")
            set_outlined(false);
        else {
            gui::out << gui::warning << m_node.get_location() << " : "
                     << "Unknown outline type for " << s_name_ << " : \"" << s_outline << "\"."
                     << std::endl;
        }
    }

    if (const layout_attribute* p_attr = m_node.try_get_attribute("alignX")) {
        const std::string& s_align_x = p_attr->get_value<std::string>();
        if (s_align_x == "LEFT")
            set_alignment_x(alignment_x::left);
        else if (s_align_x == "CENTER")
            set_alignment_x(alignment_x::center);
        else if (s_align_x == "RIGHT")
            set_alignment_x(alignment_x::right);
        else {
            gui::out << gui::warning << m_node.get_location() << " : "
                     << "Unknown horizontal alignment behavior for " << s_name_ << " : \"" << s_align_x
                     << "\"." << std::endl;
        }
    }

    if (const layout_attribute* p_attr = m_node.try_get_attribute("alignY")) {
        const std::string& s_align_y = p_attr->get_value<std::string>();
        if (s_align_y == "TOP")
            set_alignment_y(alignment_y::top);
        else if (s_align_y == "MIDDLE")
            set_alignment_y(alignment_y::middle);
        else if (s_align_y == "BOTTOM")
            set_alignment_y(alignment_y::bottom);
        else {
            gui::out << gui::warning << m_node.get_location() << " : "
                     << "Unknown vertical alignment behavior for " << s_name_ << " : \"" << s_align_y
                     << "\"." << std::endl;
        }
    }
}

void font_string::parse_shadow_node_(const layout_node& m_node) {
    if (const layout_node* p_shadow_node = m_node.try_get_child("Shadow")) {
        set_shadow(true);

        if (const layout_node* p_color_node = p_shadow_node->try_get_child("Color"))
            set_shadow_color(parse_color_node_(*p_color_node));

        if (const layout_node* p_offset_node = p_shadow_node->try_get_child("Offset")) {
            set_shadow_offset(vector2f(
                p_offset_node->get_attribute_value_or<float>("x", 0.0),
                p_offset_node->get_attribute_value_or<float>("y", 0.0)));
        }
    }
}

} // namespace lxgui::gui
