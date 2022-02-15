#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

void texture::parse_layout(const layout_node& m_node) {
    layered_region::parse_layout(m_node);

    parse_tex_coords_node_(m_node);

    if (const layout_node* p_color_node = m_node.try_get_child("Color"))
        set_solid_color(parse_color_node_(*p_color_node));

    parse_gradient_node_(m_node);
}

void texture::parse_attributes_(const layout_node& m_node) {
    layered_region::parse_attributes_(m_node);

    if (const layout_attribute* p_attr = m_node.try_get_attribute("filter"))
        set_filter_mode(p_attr->get_value<std::string>());

    if (const layout_attribute* p_attr = m_node.try_get_attribute("file"))
        set_texture(p_attr->get_value<std::string>());
}

void texture::parse_tex_coords_node_(const layout_node& m_node) {
    if (const layout_node* p_tex_coords_node = m_node.try_get_child("TexCoords")) {
        set_tex_rect(
            {p_tex_coords_node->get_attribute_value_or<float>("left", 0.0f),
             p_tex_coords_node->get_attribute_value_or<float>("top", 0.0f),
             p_tex_coords_node->get_attribute_value_or<float>("right", 1.0f),
             p_tex_coords_node->get_attribute_value_or<float>("bottom", 1.0f)});
    }
}

void texture::parse_gradient_node_(const layout_node& m_node) {
    if (const layout_node* p_gradient_node = m_node.try_get_child("Gradient")) {
        std::string orientation =
            p_gradient_node->get_attribute_value_or<std::string>("orientation", "HORIZONTAL");

        gradient::orientation m_orient;
        if (orientation == "HORIZONTAL")
            m_orient = gradient::orientation::horizontal;
        else if (orientation == "VERTICAL")
            m_orient = gradient::orientation::vertical;
        else {
            gui::out << gui::warning << p_gradient_node->get_location()
                     << " : "
                        "Unknown gradient orientation for " +
                            name_ + " : \"" + orientation +
                            "\". "
                            "No gradient will be shown for this texture."
                     << std::endl;
            return;
        }

        const layout_node* p_min_color_node = p_gradient_node->try_get_child("MinColor");
        if (!p_min_color_node) {
            gui::out << gui::warning << m_node.get_location()
                     << " : "
                        "Gradient requires MinColor child node."
                     << std::endl;
            return;
        }

        const layout_node* p_max_color_node = p_gradient_node->try_get_child("MaxColor");
        if (!p_max_color_node) {
            gui::out << gui::warning << m_node.get_location()
                     << " : "
                        "Gradient requires MaxColor child node."
                     << std::endl;
            return;
        }

        set_gradient(gradient(
            m_orient, parse_color_node_(*p_min_color_node), parse_color_node_(*p_max_color_node)));
    }
}

} // namespace lxgui::gui
