#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

void slider::parse_attributes_(const layout_node& node) {
    frame::parse_attributes_(node);

    if (const layout_attribute* attr = node.try_get_attribute("valueStep"))
        set_value_step(attr->get_value<float>());
    if (const layout_attribute* attr = node.try_get_attribute("minValue"))
        set_min_value(attr->get_value<float>());
    if (const layout_attribute* attr = node.try_get_attribute("maxValue"))
        set_max_value(attr->get_value<float>());
    if (const layout_attribute* attr = node.try_get_attribute("defaultValue"))
        set_value(attr->get_value<float>());
    if (const layout_attribute* attr = node.try_get_attribute("drawLayer"))
        set_thumb_draw_layer(attr->get_value<std::string>());

    if (const layout_attribute* attr = node.try_get_attribute("orientation")) {
        std::string orient = attr->get_value<std::string>();
        if (orient == "HORIZONTAL")
            set_orientation(orientation::horizontal);
        else if (orient == "VERTICAL")
            set_orientation(orientation::vertical);
        else {
            gui::out << gui::warning << node.get_location() << " : Unknown Slider orientation : \""
                     << orient
                     << "\". Expecting either :\n\"HORIZONTAL\" or \"VERTICAL\". Attribute ignored."
                     << std::endl;
        }
    }
}

void slider::parse_all_nodes_before_children_(const layout_node& node) {
    frame::parse_all_nodes_before_children_(node);

    if (const layout_node* thumb_node = node.try_get_child("ThumbTexture")) {
        layout_node defaulted = *thumb_node;
        defaulted.get_or_set_attribute_value("name", "$parentThumbTexture");

        auto thumb_texture = parse_region_(defaulted, "ARTWORK", "Texture");
        if (thumb_texture) {
            thumb_texture->set_special();
            set_thumb_texture(utils::static_pointer_cast<texture>(thumb_texture));
        }

        warn_for_not_accessed_node(defaulted);
        thumb_node->bypass_access_check();
    }
}

} // namespace lxgui::gui
