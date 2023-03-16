#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_layout_node.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/utils_std.hpp"

namespace lxgui::gui {

color region::parse_color_node_(const layout_node& node) {
    if (const auto attr = node.try_get_attribute_value<std::string>("c")) {
        const std::string& s = attr.value();
        if (!s.empty() && s[0] == '#')
            return color(s);
    }

    return color(
        node.get_attribute_value_or<float>("r", 0.0f),
        node.get_attribute_value_or<float>("g", 0.0f),
        node.get_attribute_value_or<float>("b", 0.0f),
        node.get_attribute_value_or<float>("a", 1.0f));
}

vector2<std::optional<float>> region::parse_offset_node_(const layout_node& node) {
    return {node.try_get_attribute_value<float>("x"), node.try_get_attribute_value<float>("y")};
}

vector2<float> region::parse_offset_node_or_(const layout_node& node, float fallback) {
    return {
        node.get_attribute_value_or<float>("x", fallback),
        node.get_attribute_value_or<float>("y", fallback)};
}

std::pair<anchor_type, vector2<std::optional<float>>>
region::parse_dimension_node_(const layout_node& node) {
    const layout_node* abs_dim_node = node.try_get_child("AbsDimension");
    const layout_node* rel_dim_node = node.try_get_child("RelDimension");

    if (abs_dim_node && rel_dim_node) {
        gui::out << gui::warning << node.get_location() << ": " << node.get_name()
                 << " node can only contain one of AbsDimension or RelDimension, but not both. "
                    "RelDimension ignored."
                 << std::endl;
    }

    if (!abs_dim_node && !rel_dim_node) {
        gui::out << gui::warning << node.get_location() << ": " << node.get_name()
                 << " node must contain one of AbsDimension or RelDimension." << std::endl;
        return {};
    }

    anchor_type        type        = anchor_type::abs;
    const layout_node* chosen_node = nullptr;
    if (abs_dim_node) {
        type        = anchor_type::abs;
        chosen_node = abs_dim_node;
    } else {
        type        = anchor_type::rel;
        chosen_node = rel_dim_node;
    }

    return std::make_pair(type, parse_offset_node_(*chosen_node));
}

void region::parse_size_node_(const layout_node& node) {
    if (const layout_node* size_block = node.try_get_child("Size")) {
        auto dimensions = parse_dimension_node_(*size_block);
        bool has_x      = dimensions.second.x.has_value();
        bool has_y      = dimensions.second.y.has_value();
        if (dimensions.first == anchor_type::abs) {
            if (has_x && has_y) {
                set_dimensions(vector2f(dimensions.second.x.value(), dimensions.second.y.value()));
            } else if (has_x)
                set_width(dimensions.second.x.value());
            else if (has_y)
                set_height(dimensions.second.y.value());
        } else {
            if (has_x && has_y) {
                set_relative_dimensions(
                    vector2f(dimensions.second.x.value(), dimensions.second.y.value()));
            } else if (has_x)
                set_relative_width(dimensions.second.x.value());
            else if (has_y)
                set_relative_height(dimensions.second.y.value());
        }
    }
}

void region::parse_anchor_node_(const layout_node& node) {
    if (const layout_node* anchors_node = node.try_get_child("Anchors")) {
        std::vector<point> found_points;
        for (const auto& anchor_node : anchors_node->get_children()) {
            if (anchor_node.get_name() != "Anchor" && anchor_node.get_name() != "") {
                gui::out << gui::warning << anchor_node.get_location() << ": "
                         << "unexpected node '" << anchor_node.get_name() << "'; ignored."
                         << std::endl;
                continue;
            }

            point       pt = anchor_node.get_attribute_value_or<point>("point", point::top_left);
            std::string parent = anchor_node.get_attribute_value_or<std::string>(
                "relativeTo", parent_ || is_virtual() ? "$parent" : "");
            point relative_point = anchor_node.get_attribute_value_or<point>("relativePoint", pt);

            if (utils::find(found_points, pt) != found_points.end()) {
                gui::out << gui::warning << anchor_node.get_location() << ": "
                         << "anchor point \"" << utils::to_string(pt)
                         << "\" has already been defined for \"" << name_ << "\". anchor skipped."
                         << std::endl;
                continue;
            }

            anchor_data a(pt, parent, relative_point);

            if (const layout_node* offset_node = anchor_node.try_get_child("Offset")) {
                auto dimensions = parse_dimension_node_(*offset_node);
                a.type          = dimensions.first;
                a.offset        = vector2f(
                    dimensions.second.x.value_or(0.0f), dimensions.second.y.value_or(0.0f));
            }

            set_anchor(a);
        }
    }
}

void region::parse_layout(const layout_node& node) {
    parse_attributes_(node);
    parse_size_node_(node);
    parse_anchor_node_(node);
}

void region::parse_attributes_(const layout_node& node) {
    if (node.get_attribute_value_or<bool>("setAllAnchors", false))
        set_all_anchors("$parent");
}

} // namespace lxgui::gui
