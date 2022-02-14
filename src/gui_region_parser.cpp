#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/utils_std.hpp"

namespace lxgui::gui {

color region::parse_color_node_(const layout_node& m_node) {
    if (const layout_attribute* p_attr = m_node.try_get_attribute("c")) {
        std::string s_color = p_attr->get_value<std::string>();
        if (!s_color.empty() && s_color[0] == '#')
            return color(s_color);
    }

    return color(
        m_node.get_attribute_value_or<float>("r", 0.0f),
        m_node.get_attribute_value_or<float>("g", 0.0f),
        m_node.get_attribute_value_or<float>("b", 0.0f),
        m_node.get_attribute_value_or<float>("a", 1.0f));
}

std::pair<anchor_type, vector2<std::optional<float>>>
region::parse_dimension_(const layout_node& m_node) {
    const layout_node* p_abs_dim_node = m_node.try_get_child("AbsDimension");
    const layout_node* p_rel_dim_node = m_node.try_get_child("RelDimension");

    if (p_abs_dim_node && p_rel_dim_node) {
        gui::out << gui::warning << m_node.get_location() << " : " << m_node.get_name()
                 << " node can only contain one of AbsDimension or RelDimension, "
                    "but not both. RelDimension ignored."
                 << std::endl;
    }

    if (!p_abs_dim_node && !p_rel_dim_node) {
        gui::out << gui::warning << m_node.get_location() << " : " << m_node.get_name()
                 << " node must contain one of AbsDimension or RelDimension." << std::endl;
        return {};
    }

    anchor_type        m_type = anchor_type::abs;
    const layout_node* p_node = nullptr;
    if (p_abs_dim_node) {
        m_type = anchor_type::abs;
        p_node = p_abs_dim_node;
    } else {
        m_type = anchor_type::rel;
        p_node = p_rel_dim_node;
    }

    vector2<std::optional<float>> m_vec;
    if (const layout_attribute* p_attr = p_node->try_get_attribute("x"))
        m_vec.x = p_attr->get_value<float>();
    else
        m_vec.x = std::nullopt;

    if (const layout_attribute* p_attr = p_node->try_get_attribute("y"))
        m_vec.y = p_attr->get_value<float>();
    else
        m_vec.y = std::nullopt;

    return std::make_pair(m_type, m_vec);
}

void region::parse_size_node_(const layout_node& m_node) {
    if (const layout_node* p_size_block = m_node.try_get_child("Size")) {
        auto m_dimensions = parse_dimension_(*p_size_block);
        bool b_has_x      = m_dimensions.second.x.has_value();
        bool b_has_y      = m_dimensions.second.y.has_value();
        if (m_dimensions.first == anchor_type::abs) {
            if (b_has_x && b_has_y) {
                set_dimensions(
                    vector2f(m_dimensions.second.x.value(), m_dimensions.second.y.value()));
            } else if (b_has_x)
                set_width(m_dimensions.second.x.value());
            else if (b_has_y)
                set_height(m_dimensions.second.y.value());
        } else {
            if (b_has_x && b_has_y) {
                set_relative_dimensions(
                    vector2f(m_dimensions.second.x.value(), m_dimensions.second.y.value()));
            } else if (b_has_x)
                set_relative_width(m_dimensions.second.x.value());
            else if (b_has_y)
                set_relative_height(m_dimensions.second.y.value());
        }
    }
}

void region::parse_anchor_node_(const layout_node& m_node) {
    if (const layout_node* p_anchors_node = m_node.try_get_child("Anchors")) {
        std::vector<std::string> found_points;
        for (const auto& m_anchor_node : p_anchors_node->get_children()) {
            if (m_anchor_node.get_name() != "Anchor" && m_anchor_node.get_name() != "") {
                gui::out << gui::warning << m_anchor_node.get_location() << " : "
                         << "unexpected node '" << m_anchor_node.get_name() << "'; ignored."
                         << std::endl;
                continue;
            }

            std::string s_point =
                m_anchor_node.get_attribute_value_or<std::string>("point", "TOP_LEFT");
            std::string s_parent = m_anchor_node.get_attribute_value_or<std::string>(
                "relativeTo", p_parent_ || is_virtual() ? "$parent" : "");
            std::string s_relative_point =
                m_anchor_node.get_attribute_value_or<std::string>("relativePoint", s_point);

            if (utils::find(found_points, s_point) != found_points.end()) {
                gui::out << gui::warning << m_anchor_node.get_location() << " : "
                         << "anchor point \"" << s_point
                         << "\" has already been defined "
                            "for \""
                         << s_name_ << "\". anchor skipped." << std::endl;
                continue;
            }

            anchor_data m_anchor(
                anchor::get_anchor_point(s_point), s_parent,
                anchor::get_anchor_point(s_relative_point));

            if (const layout_node* p_offset_node = m_anchor_node.try_get_child("Offset")) {
                auto m_dimensions = parse_dimension_(*p_offset_node);
                m_anchor.m_type   = m_dimensions.first;
                m_anchor.m_offset = vector2f(
                    m_dimensions.second.x.value_or(0.0f), m_dimensions.second.y.value_or(0.0f));
            }

            set_point(m_anchor);
        }
    }
}

void region::parse_layout(const layout_node& m_node) {
    parse_attributes_(m_node);
    parse_size_node_(m_node);
    parse_anchor_node_(m_node);
}

void region::parse_attributes_(const layout_node& m_node) {
    if (m_node.get_attribute_value_or<bool>("setAllPoints", false))
        set_all_points("$parent");
}

} // namespace lxgui::gui
