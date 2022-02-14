#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_virtual_root.hpp"

namespace lxgui::gui {

void frame::parse_all_nodes_before_children_(const layout_node& m_node) {
    parse_attributes_(m_node);

    parse_size_node_(m_node);
    parse_resize_bounds_node_(m_node);
    parse_anchor_node_(m_node);
    parse_title_region_node_(m_node);
    parse_backdrop_node_(m_node);
    parse_hit_rect_insets_node_(m_node);

    parse_layers_node_(m_node);
}

void frame::parse_layout(const layout_node& m_node) {
    parse_all_nodes_before_children_(m_node);
    parse_frames_node_(m_node);
    parse_scripts_node_(m_node);
}

void frame::parse_attributes_(const layout_node& m_node) {
    if (const layout_attribute* p_attr = m_node.try_get_attribute("hidden"))
        set_shown(!p_attr->get_value<bool>());

    if (m_node.get_attribute_value_or<bool>("setAllPoints", false))
        set_all_points("$parent");

    if (const layout_attribute* p_attr = m_node.try_get_attribute("alpha"))
        set_alpha(p_attr->get_value<float>());
    if (const layout_attribute* p_attr = m_node.try_get_attribute("topLevel"))
        set_top_level(p_attr->get_value<bool>());
    if (const layout_attribute* p_attr = m_node.try_get_attribute("movable"))
        set_movable(p_attr->get_value<bool>());
    if (const layout_attribute* p_attr = m_node.try_get_attribute("resizable"))
        set_resizable(p_attr->get_value<bool>());

    set_frame_strata(m_node.get_attribute_value_or<std::string>("frameStrata", "PARENT"));

    if (const layout_attribute* p_attr = m_node.try_get_attribute("frameLevel")) {
        if (!b_virtual_) {
            std::string s_frame_level = p_attr->get_value<std::string>();
            int         level         = 0;
            if (s_frame_level != "PARENT" && utils::from_string(s_frame_level, level))
                set_level(level);
        } else {
            gui::out << gui::warning << m_node.get_location() << " : "
                     << "\"frameLevel\" is not allowed for virtual regions. Ignored." << std::endl;
        }
    }
    if (const layout_attribute* p_attr = m_node.try_get_attribute("enableMouse"))
        enable_mouse(p_attr->get_value<bool>());
    if (const layout_attribute* p_attr = m_node.try_get_attribute("enableMouseWheel"))
        enable_mouse_wheel(p_attr->get_value<bool>());
    if (const layout_attribute* p_attr = m_node.try_get_attribute("clampedToScreen"))
        set_clamped_to_screen(p_attr->get_value<bool>());
    if (const layout_attribute* p_attr = m_node.try_get_attribute("autoFocus"))
        enable_auto_focus(p_attr->get_value<bool>());
}

void frame::parse_resize_bounds_node_(const layout_node& m_node) {
    if (const layout_node* p_resize_bounds_node = m_node.try_get_child("ResizeBounds")) {
        if (const layout_node* p_min_node = p_resize_bounds_node->try_get_child("Min")) {
            auto m_dimensions = parse_dimension_(*p_min_node);
            bool b_has_x      = m_dimensions.second.x.has_value();
            bool b_has_y      = m_dimensions.second.y.has_value();
            if (m_dimensions.first == anchor_type::abs) {
                if (b_has_x && b_has_y) {
                    set_min_dimensions(
                        vector2f(m_dimensions.second.x.value(), m_dimensions.second.y.value()));
                } else if (b_has_x)
                    set_min_width(m_dimensions.second.x.value());
                else if (b_has_y)
                    set_min_height(m_dimensions.second.y.value());
            } else {
                gui::out << gui::warning << p_min_node->get_location() << " : "
                         << "\"RelDimension\" for ResizeBounds:Min is not yet supported. Skipped."
                         << std::endl;
            }
        }

        if (const layout_node* p_max_node = p_resize_bounds_node->try_get_child("Max")) {
            auto m_dimensions = parse_dimension_(*p_max_node);
            bool b_has_x      = m_dimensions.second.x.has_value();
            bool b_has_y      = m_dimensions.second.y.has_value();
            if (m_dimensions.first == anchor_type::abs) {
                if (b_has_x && b_has_y) {
                    set_max_dimensions(
                        vector2f(m_dimensions.second.x.value(), m_dimensions.second.y.value()));
                } else if (b_has_x)
                    set_max_width(m_dimensions.second.x.value());
                else if (b_has_y)
                    set_max_height(m_dimensions.second.y.value());
            } else {
                gui::out << gui::warning << p_max_node->get_location() << " : "
                         << "\"RelDimension\" for ResizeBounds:Max is not yet supported. Skipped."
                         << std::endl;
            }
        }
    }
}

void frame::parse_title_region_node_(const layout_node& m_node) {
    if (const layout_node* p_title_region_node = m_node.try_get_child("TitleRegion")) {
        create_title_region();

        if (p_title_region_)
            p_title_region_->parse_layout(*p_title_region_node);
    }
}

void frame::parse_backdrop_node_(const layout_node& m_node) {
    if (const layout_node* p_backdrop_node = m_node.try_get_child("Backdrop")) {
        std::unique_ptr<backdrop> p_backdrop(new backdrop(*this));

        p_backdrop->set_background(
            parse_file_name(p_backdrop_node->get_attribute_value_or<std::string>("bgFile", "")));

        p_backdrop->set_edge(
            parse_file_name(p_backdrop_node->get_attribute_value_or<std::string>("edgeFile", "")));

        p_backdrop->set_background_tilling(
            p_backdrop_node->get_attribute_value_or<bool>("tile", false));

        if (const layout_node* p_bg_insets_node =
                p_backdrop_node->try_get_child("BackgroundInsets")) {
            const layout_node* p_abs_inset_node = p_bg_insets_node->try_get_child("AbsInset");
            const layout_node* p_rel_inset_node = p_bg_insets_node->try_get_child("RelInset");

            if (p_abs_inset_node && p_rel_inset_node) {
                gui::out << gui::warning << p_bg_insets_node->get_location()
                         << " : "
                            "BackgroundInsets node can only contain one of AbsInset or RelInset, "
                            "but not both. "
                            "RelInset ignored."
                         << std::endl;
            }

            if (!p_abs_inset_node && !p_rel_inset_node) {
                gui::out << gui::warning << p_bg_insets_node->get_location()
                         << " : "
                            "BackgroundInsets node must contain one of AbsInset or RelInset."
                         << std::endl;
                return;
            }

            if (p_abs_inset_node) {
                p_backdrop->set_background_insets(bounds2f(
                    p_abs_inset_node->get_attribute_value_or<float>("left", 0.0f),
                    p_abs_inset_node->get_attribute_value_or<float>("right", 0.0f),
                    p_abs_inset_node->get_attribute_value_or<float>("top", 0.0f),
                    p_abs_inset_node->get_attribute_value_or<float>("bottom", 0.0f)));
            } else {
                gui::out << gui::warning << p_rel_inset_node->get_location() << " : "
                         << "RelInset for Backdrop:BackgroundInsets is not yet supported ("
                         << s_name_ << ")." << std::endl;
            }
        }

        if (const layout_node* p_edge_insets_node = p_backdrop_node->try_get_child("EdgeInsets")) {
            const layout_node* p_abs_inset_node = p_edge_insets_node->try_get_child("AbsInset");
            const layout_node* p_rel_inset_node = p_edge_insets_node->try_get_child("RelInset");

            if (p_abs_inset_node && p_rel_inset_node) {
                gui::out << gui::warning << p_edge_insets_node->get_location()
                         << " : "
                            "EdgeInsets node can only contain one of AbsInset or RelInset, but not "
                            "both. "
                            "RelInset ignored."
                         << std::endl;
            }

            if (!p_abs_inset_node && !p_rel_inset_node) {
                gui::out << gui::warning << p_edge_insets_node->get_location()
                         << " : "
                            "EdgeInsets node must contain one of AbsInset or RelInset."
                         << std::endl;
                return;
            }

            if (p_abs_inset_node) {
                p_backdrop->set_edge_insets(bounds2f(
                    p_abs_inset_node->get_attribute_value_or<float>("left", 0.0f),
                    p_abs_inset_node->get_attribute_value_or<float>("right", 0.0f),
                    p_abs_inset_node->get_attribute_value_or<float>("top", 0.0f),
                    p_abs_inset_node->get_attribute_value_or<float>("bottom", 0.0f)));
            } else {
                gui::out << gui::warning << p_rel_inset_node->get_location() << " : "
                         << "RelInset for Backdrop:EdgeInsets is not yet supported (" << s_name_
                         << ")." << std::endl;
            }
        }

        if (const layout_node* p_color_node = p_backdrop_node->try_get_child("BackgroundColor"))
            p_backdrop->set_background_color(parse_color_node_(*p_color_node));

        if (const layout_node* p_color_node = p_backdrop_node->try_get_child("EdgeColor"))
            p_backdrop->set_edge_color(parse_color_node_(*p_color_node));

        if (const layout_node* p_edge_size_node = p_backdrop_node->try_get_child("EdgeSize")) {
            const layout_node* p_abs_value_node = p_edge_size_node->try_get_child("AbsValue");
            const layout_node* p_rel_value_node = p_edge_size_node->try_get_child("RelValue");

            if (p_abs_value_node && p_rel_value_node) {
                gui::out
                    << gui::warning << p_edge_size_node->get_location()
                    << " : "
                       "EdgeSize node can only contain one of AbsValue or RelValue, but not both. "
                       "RelValue ignored."
                    << std::endl;
            }

            if (!p_abs_value_node && !p_rel_value_node) {
                gui::out << gui::warning << p_edge_size_node->get_location()
                         << " : "
                            "EdgeSize node must contain one of AbsValue or RelValue."
                         << std::endl;
                return;
            }

            if (p_abs_value_node) {
                p_backdrop->set_edge_size(p_abs_value_node->get_attribute_value_or("x", 0.0f));
            } else {
                gui::out << gui::warning << p_rel_value_node->get_location() << " : "
                         << "RelValue for Backdrop:EdgeSize is not yet supported (" << s_name_
                         << ")." << std::endl;
            }
        }

        if (const layout_node* p_tile_size_node = p_backdrop_node->try_get_child("TileSize")) {
            const layout_node* p_abs_value_node = p_tile_size_node->try_get_child("AbsValue");
            const layout_node* p_rel_value_node = p_tile_size_node->try_get_child("RelValue");

            if (p_abs_value_node && p_rel_value_node) {
                gui::out
                    << gui::warning << p_tile_size_node->get_location()
                    << " : "
                       "TileSize node can only contain one of AbsValue or RelValue, but not both. "
                       "RelValue ignored."
                    << std::endl;
            }

            if (!p_abs_value_node && !p_rel_value_node) {
                gui::out << gui::warning << p_tile_size_node->get_location()
                         << " : "
                            "TileSize node must contain one of AbsValue or RelValue."
                         << std::endl;
                return;
            }

            if (p_abs_value_node) {
                p_backdrop->set_tile_size(p_abs_value_node->get_attribute_value_or("x", 0.0f));
            } else {
                gui::out << gui::warning << p_rel_value_node->get_location() << " : "
                         << "RelValue for Backdrop:TileSize is not yet supported (" << s_name_
                         << ")." << std::endl;
            }
        }

        set_backdrop(std::move(p_backdrop));
    }
}

void frame::parse_hit_rect_insets_node_(const layout_node& m_node) {
    if (const layout_node* p_hit_rect_block = m_node.try_get_child("HitRectInsets")) {
        const layout_node* p_abs_inset_node = p_hit_rect_block->try_get_child("AbsInset");
        const layout_node* p_rel_inset_node = p_hit_rect_block->try_get_child("RelInset");

        if (p_abs_inset_node && p_rel_inset_node) {
            gui::out
                << gui::warning << p_hit_rect_block->get_location()
                << " : "
                   "HitRectInsets node can only contain one of AbsInset or RelInset, but not both. "
                   "RelInset ignored."
                << std::endl;
        }

        if (!p_abs_inset_node && !p_rel_inset_node) {
            gui::out << gui::warning << p_hit_rect_block->get_location()
                     << " : "
                        "HitRectInsets node must contain one of AbsInset or RelInset."
                     << std::endl;
            return;
        }

        if (p_abs_inset_node) {
            set_abs_hit_rect_insets(bounds2f(
                p_abs_inset_node->get_attribute_value_or<float>("left", 0.0f),
                p_abs_inset_node->get_attribute_value_or<float>("right", 0.0f),
                p_abs_inset_node->get_attribute_value_or<float>("top", 0.0f),
                p_abs_inset_node->get_attribute_value_or<float>("bottom", 0.0f)));
        } else {
            set_rel_hit_rect_insets(bounds2f(
                p_rel_inset_node->get_attribute_value_or<float>("left", 0.0f),
                p_rel_inset_node->get_attribute_value_or<float>("right", 0.0f),
                p_rel_inset_node->get_attribute_value_or<float>("top", 0.0f),
                p_rel_inset_node->get_attribute_value_or<float>("bottom", 0.0f)));
        }
    }
}

utils::observer_ptr<layered_region> frame::parse_region_(
    const layout_node& m_node, const std::string& s_layer, const std::string& s_type) {
    try {
        auto m_attr = parse_core_attributes(
            get_manager().get_root().get_registry(),
            get_manager().get_virtual_root().get_registry(), m_node, observer_from(this));

        if (!s_type.empty())
            m_attr.s_object_type = s_type;

        auto p_region = create_layered_region(parse_layer_type(s_layer), m_attr);
        if (!p_region)
            return nullptr;

        try {
            p_region->parse_layout(m_node);
            p_region->notify_loaded();
            return p_region;
        } catch (...) {
            p_region->release_from_parent();
            throw;
        }
    } catch (const exception& e) {
        gui::out << gui::error << e.get_description() << std::endl;
    }

    return nullptr;
}

void frame::parse_layers_node_(const layout_node& m_node) {
    if (const layout_node* p_layers_node = m_node.try_get_child("Layers")) {
        for (const layout_node& m_layer_node : p_layers_node->get_children()) {
            if (m_layer_node.get_name() != "Layer" && m_layer_node.get_name() != "") {
                gui::out << gui::warning << m_layer_node.get_location() << " : "
                         << "unexpected node '" << m_layer_node.get_name() << "'; ignored."
                         << std::endl;
                continue;
            }

            std::string s_level =
                m_layer_node.get_attribute_value_or<std::string>("level", "ARTWORK");
            for (const layout_node& m_region_node : m_layer_node.get_children()) {
                parse_region_(m_region_node, s_level, "");
            }
        }
    }
}

utils::observer_ptr<frame>
frame::parse_child_(const layout_node& m_node, const std::string& s_type) {
    try {
        auto m_attr = parse_core_attributes(
            get_manager().get_root().get_registry(),
            get_manager().get_virtual_root().get_registry(), m_node, observer_from(this));

        if (!s_type.empty())
            m_attr.s_object_type = s_type;

        auto p_frame = create_child(m_attr);
        if (!p_frame)
            return nullptr;

        try {
            p_frame->set_addon(get_addon());
            p_frame->parse_layout(m_node);
            p_frame->notify_loaded();
            return p_frame;
        } catch (...) {
            p_frame->release_from_parent();
            throw;
        }
    } catch (const exception& e) {
        gui::out << gui::error << e.get_description() << std::endl;
    }

    return nullptr;
}

void frame::parse_frames_node_(const layout_node& m_node) {
    if (const layout_node* p_frames_node = m_node.try_get_child("Frames")) {
        for (const layout_node& m_elem_node : p_frames_node->get_children()) {
            parse_child_(m_elem_node, "");
        }
    }
}

void frame::parse_scripts_node_(const layout_node& m_node) {
    if (const layout_node* p_scripts_node = m_node.try_get_child("Scripts")) {
        for (const layout_node& m_script_node : p_scripts_node->get_children()) {
            std::string s_name = std::string(m_script_node.get_name());

            const layout_attribute* p_node = &m_script_node;
            if (const layout_attribute* p_run = m_script_node.try_get_attribute("run"))
                p_node = p_run;

            std::string s_script = std::string(p_node->get_value());
            script_info m_info{
                std::string(p_node->get_filename()),
                static_cast<std::size_t>(p_node->get_value_line_number())};

            if (m_script_node.get_attribute_value_or<bool>("override", false))
                set_script(s_name, std::move(s_script), std::move(m_info));
            else
                add_script(s_name, std::move(s_script), std::move(m_info));
        }
    }
}

} // namespace lxgui::gui
