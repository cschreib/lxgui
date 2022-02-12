#include "lxgui/gui_editbox.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_parser_common.hpp"

namespace lxgui::gui {
void edit_box::parse_attributes_(const layout_node& m_node) {
    base::parse_attributes_(m_node);

    if (const layout_attribute* p_attr = m_node.try_get_attribute("letters"))
        set_max_letters(p_attr->get_value<std::size_t>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("blinkSpeed"))
        set_blink_speed(p_attr->get_value<float>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("numeric"))
        set_numeric_only(p_attr->get_value<bool>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("positive"))
        set_positive_only(p_attr->get_value<bool>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("integer"))
        set_integer_only(p_attr->get_value<bool>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("password"))
        enable_password_mode(p_attr->get_value<bool>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("multiLine"))
        set_multi_line(p_attr->get_value<bool>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("historyLines"))
        set_max_history_lines(p_attr->get_value<std::size_t>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("ignoreArrows"))
        set_arrows_ignored(p_attr->get_value<bool>());
}

void edit_box::parse_all_nodes_before_children_(const layout_node& m_node) {
    base::parse_all_nodes_before_children_(m_node);

    parse_text_insets_node_(m_node);
    parse_font_string_node_(m_node);

    if (const layout_node* p_highlight_node = m_node.try_get_child("HighlightColor"))
        set_highlight_color(parse_color_node_(*p_highlight_node));
}

void edit_box::parse_font_string_node_(const layout_node& m_node) {
    if (const layout_node* p_font_string_node = m_node.try_get_child("FontString")) {
        layout_node m_defaulted = *p_font_string_node;
        m_defaulted.get_or_set_attribute_value("name", "$parentFontString");

        auto p_font_string = parse_region_(m_defaulted, "ARTWORK", "FontString");
        if (p_font_string) {
            p_font_string->set_special();
            set_font_string(utils::static_pointer_cast<font_string>(p_font_string));
        }

        if (const layout_node* p_error_node = m_defaulted.try_get_child("Anchors")) {
            gui::out << gui::warning << p_error_node->get_location() << " : "
                     << "edit_box : font_string's anchors will be ignored." << std::endl;
        }

        if (const layout_node* p_error_node = m_defaulted.try_get_child("Size")) {
            gui::out << gui::warning << p_error_node->get_location() << " : "
                     << "edit_box : font_string's Size will be ignored." << std::endl;
        }

        warn_for_not_accessed_node(m_defaulted);
        p_font_string_node->bypass_access_check();
    }
}

void edit_box::parse_text_insets_node_(const layout_node& m_node) {
    if (const layout_node* p_text_insets_node = m_node.try_get_child("TextInsets")) {
        set_text_insets(bounds2f(
            p_text_insets_node->get_attribute_value_or<float>("left", 0.0f),
            p_text_insets_node->get_attribute_value_or<float>("right", 0.0f),
            p_text_insets_node->get_attribute_value_or<float>("top", 0.0f),
            p_text_insets_node->get_attribute_value_or<float>("bottom", 0.0f)));
    }
}
} // namespace lxgui::gui
