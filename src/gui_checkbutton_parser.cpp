#include "lxgui/gui_checkbutton.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

void check_button::parse_all_nodes_before_children_(const layout_node& m_node) {
    button::parse_all_nodes_before_children_(m_node);

    if (const layout_node* p_special_node = m_node.try_get_child("CheckedTexture")) {
        std::string layer = p_special_node->get_attribute_value_or<std::string>("layer", "ARTWORK");

        layout_node m_defaulted = *p_special_node;
        m_defaulted.get_or_set_attribute_value("name", "$parentCheckedTexture");
        m_defaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto p_texture = parse_region_(m_defaulted, layer, "Texture");
        if (p_texture) {
            p_texture->set_special();
            set_checked_texture(utils::static_pointer_cast<texture>(p_texture));
        }

        warn_for_not_accessed_node(m_defaulted);
        p_special_node->bypass_access_check();
    }

    if (const layout_node* p_special_node = m_node.try_get_child("DisabledCheckedTexture")) {
        std::string layer = p_special_node->get_attribute_value_or<std::string>("layer", "ARTWORK");

        layout_node m_defaulted = *p_special_node;
        m_defaulted.get_or_set_attribute_value("name", "$parentDisabledCheckedTexture");
        m_defaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto p_texture = parse_region_(m_defaulted, layer, "Texture");
        if (p_texture) {
            p_texture->set_special();
            set_disabled_checked_texture(utils::static_pointer_cast<texture>(p_texture));
        }

        warn_for_not_accessed_node(m_defaulted);
        p_special_node->bypass_access_check();
    }
}

} // namespace lxgui::gui
