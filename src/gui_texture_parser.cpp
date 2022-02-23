#include "lxgui/gui_layout_node.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

void texture::parse_layout(const layout_node& node) {
    layered_region::parse_layout(node);

    parse_tex_coords_node_(node);

    if (const layout_node* color_node = node.try_get_child("Color"))
        set_solid_color(parse_color_node_(*color_node));

    parse_gradient_node_(node);
}

void texture::parse_attributes_(const layout_node& node) {
    layered_region::parse_attributes_(node);

    if (const layout_attribute* attr = node.try_get_attribute("filter"))
        set_filter_mode(attr->get_value<std::string>());

    if (const layout_attribute* attr = node.try_get_attribute("file"))
        set_texture(attr->get_value<std::string>());
}

void texture::parse_tex_coords_node_(const layout_node& node) {
    if (const layout_node* tex_coords_node = node.try_get_child("TexCoords")) {
        set_tex_rect(
            {tex_coords_node->get_attribute_value_or<float>("left", 0.0f),
             tex_coords_node->get_attribute_value_or<float>("top", 0.0f),
             tex_coords_node->get_attribute_value_or<float>("right", 1.0f),
             tex_coords_node->get_attribute_value_or<float>("bottom", 1.0f)});
    }
}

void texture::parse_gradient_node_(const layout_node& node) {
    if (const layout_node* gradient_node = node.try_get_child("Gradient")) {
        std::string orientation =
            gradient_node->get_attribute_value_or<std::string>("orientation", "HORIZONTAL");

        gradient::orientation orient;
        if (orientation == "HORIZONTAL")
            orient = gradient::orientation::horizontal;
        else if (orientation == "VERTICAL")
            orient = gradient::orientation::vertical;
        else {
            gui::out << gui::warning << gradient_node->get_location()
                     << ": Unknown gradient orientation for " << name_ << ": \"" << orientation
                     << "\". No gradient will be shown for this texture." << std::endl;
            return;
        }

        const layout_node* min_color_node = gradient_node->try_get_child("MinColor");
        if (!min_color_node) {
            gui::out << gui::warning << node.get_location()
                     << ": Gradient requires MinColor child node." << std::endl;
            return;
        }

        const layout_node* max_color_node = gradient_node->try_get_child("MaxColor");
        if (!max_color_node) {
            gui::out << gui::warning << node.get_location()
                     << ": Gradient requires MaxColor child node." << std::endl;
            return;
        }

        set_gradient(gradient{
            orient, parse_color_node_(*min_color_node), parse_color_node_(*max_color_node)});
    }
}

} // namespace lxgui::gui
