#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

void slider::parse_attributes_(const layout_node& m_node) {
    frame::parse_attributes_(m_node);

    if (const layout_attribute* p_attr = m_node.try_get_attribute("valueStep"))
        set_value_step(p_attr->get_value<float>());
    if (const layout_attribute* p_attr = m_node.try_get_attribute("minValue"))
        set_min_value(p_attr->get_value<float>());
    if (const layout_attribute* p_attr = m_node.try_get_attribute("maxValue"))
        set_max_value(p_attr->get_value<float>());
    if (const layout_attribute* p_attr = m_node.try_get_attribute("defaultValue"))
        set_value(p_attr->get_value<float>());
    if (const layout_attribute* p_attr = m_node.try_get_attribute("drawLayer"))
        set_thumb_draw_layer(p_attr->get_value<std::string>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("orientation")) {
        std::string orientation = p_attr->get_value<std::string>();
        if (orientation == "HORIZONTAL")
            set_orientation(orientation::horizontal);
        else if (orientation == "VERTICAL")
            set_orientation(orientation::vertical);
        else {
            gui::out << gui::warning << m_node.get_location()
                     << " : "
                        "Unknown Slider orientation : \"" +
                            orientation +
                            "\". Expecting either :\n"
                            "\"HORIZONTAL\" or \"VERTICAL\". Attribute ignored."
                     << std::endl;
        }
    }
}

void slider::parse_all_nodes_before_children_(const layout_node& m_node) {
    frame::parse_all_nodes_before_children_(m_node);

    if (const layout_node* p_thumb_node = m_node.try_get_child("ThumbTexture")) {
        layout_node m_defaulted = *p_thumb_node;
        m_defaulted.get_or_set_attribute_value("name", "$parentThumbTexture");

        auto p_thumb_texture = parse_region_(m_defaulted, "ARTWORK", "Texture");
        if (p_thumb_texture) {
            p_thumb_texture->set_special();
            set_thumb_texture(utils::static_pointer_cast<texture>(p_thumb_texture));
        }

        warn_for_not_accessed_node(m_defaulted);
        p_thumb_node->bypass_access_check();
    }
}

} // namespace lxgui::gui
