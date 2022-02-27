#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_layout_node.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_virtual_root.hpp"

namespace lxgui::gui {

void frame::parse_all_nodes_before_children_(const layout_node& node) {
    parse_attributes_(node);

    parse_size_node_(node);
    parse_resize_bounds_node_(node);
    parse_anchor_node_(node);
    parse_title_region_node_(node);
    parse_backdrop_node_(node);
    parse_hit_rect_insets_node_(node);

    parse_layers_node_(node);
}

void frame::parse_layout(const layout_node& node) {
    parse_all_nodes_before_children_(node);
    parse_frames_node_(node);
    parse_scripts_node_(node);
}

void frame::parse_attributes_(const layout_node& node) {
    if (const layout_attribute* attr = node.try_get_attribute("hidden"))
        set_shown(!attr->get_value<bool>());

    if (node.get_attribute_value_or<bool>("setAllPoints", false))
        set_all_points("$parent");

    if (const layout_attribute* attr = node.try_get_attribute("alpha"))
        set_alpha(attr->get_value<float>());
    if (const layout_attribute* attr = node.try_get_attribute("topLevel"))
        set_top_level(attr->get_value<bool>());
    if (const layout_attribute* attr = node.try_get_attribute("movable"))
        set_movable(attr->get_value<bool>());
    if (const layout_attribute* attr = node.try_get_attribute("resizable"))
        set_resizable(attr->get_value<bool>());

    set_frame_strata(node.get_attribute_value_or<std::string>("frameStrata", "PARENT"));

    if (const layout_attribute* attr = node.try_get_attribute("frameLevel")) {
        if (!is_virtual_) {
            std::string frame_level = attr->get_value<std::string>();
            if (frame_level != "PARENT")
                set_level(attr->get_value<int>());
        } else {
            gui::out << gui::warning << node.get_location() << ": "
                     << "\"frameLevel\" is not allowed for virtual regions. Ignored." << std::endl;
        }
    }
    if (const layout_attribute* attr = node.try_get_attribute("enableMouse"))
        enable_mouse(attr->get_value<bool>());
    if (const layout_attribute* attr = node.try_get_attribute("enableMouseWheel"))
        enable_mouse_wheel(attr->get_value<bool>());
    if (const layout_attribute* attr = node.try_get_attribute("clampedToScreen"))
        set_clamped_to_screen(attr->get_value<bool>());
    if (const layout_attribute* attr = node.try_get_attribute("autoFocus"))
        enable_auto_focus(attr->get_value<bool>());
}

void frame::parse_resize_bounds_node_(const layout_node& node) {
    if (const layout_node* resize_bounds_node = node.try_get_child("ResizeBounds")) {
        if (const layout_node* min_node = resize_bounds_node->try_get_child("Min")) {
            auto dimensions = parse_dimension_(*min_node);
            bool has_x      = dimensions.second.x.has_value();
            bool has_y      = dimensions.second.y.has_value();
            if (dimensions.first == anchor_type::abs) {
                if (has_x && has_y) {
                    set_min_dimensions(
                        vector2f(dimensions.second.x.value(), dimensions.second.y.value()));
                } else if (has_x)
                    set_min_width(dimensions.second.x.value());
                else if (has_y)
                    set_min_height(dimensions.second.y.value());
            } else {
                gui::out << gui::warning << min_node->get_location() << ": "
                         << "\"RelDimension\" for ResizeBounds:Min is not yet supported. Skipped."
                         << std::endl;
            }
        }

        if (const layout_node* max_node = resize_bounds_node->try_get_child("Max")) {
            auto dimensions = parse_dimension_(*max_node);
            bool has_x      = dimensions.second.x.has_value();
            bool has_y      = dimensions.second.y.has_value();
            if (dimensions.first == anchor_type::abs) {
                if (has_x && has_y) {
                    set_max_dimensions(
                        vector2f(dimensions.second.x.value(), dimensions.second.y.value()));
                } else if (has_x)
                    set_max_width(dimensions.second.x.value());
                else if (has_y)
                    set_max_height(dimensions.second.y.value());
            } else {
                gui::out << gui::warning << max_node->get_location() << ": "
                         << "\"RelDimension\" for ResizeBounds:Max is not yet supported. Skipped."
                         << std::endl;
            }
        }
    }
}

void frame::parse_title_region_node_(const layout_node& node) {
    if (const layout_node* title_region_node = node.try_get_child("TitleRegion")) {
        create_title_region();

        if (title_region_)
            title_region_->parse_layout(*title_region_node);
    }
}

void frame::parse_backdrop_node_(const layout_node& node) {
    if (const layout_node* backdrop_node = node.try_get_child("Backdrop")) {
        std::unique_ptr<backdrop> bdrop(new backdrop(*this));

        bdrop->set_background(
            parse_file_name(backdrop_node->get_attribute_value_or<std::string>("bgFile", "")));

        bdrop->set_edge(
            parse_file_name(backdrop_node->get_attribute_value_or<std::string>("edgeFile", "")));

        bdrop->set_background_tilling(backdrop_node->get_attribute_value_or<bool>("tile", false));

        if (const layout_node* bg_insets_node = backdrop_node->try_get_child("BackgroundInsets")) {
            const layout_node* abs_inset_node = bg_insets_node->try_get_child("AbsInset");
            const layout_node* rel_inset_node = bg_insets_node->try_get_child("RelInset");

            if (abs_inset_node && rel_inset_node) {
                gui::out << gui::warning << bg_insets_node->get_location()
                         << ": BackgroundInsets node can only contain one of AbsInset or "
                            "RelInset, but not both. RelInset ignored."
                         << std::endl;
            }

            if (!abs_inset_node && !rel_inset_node) {
                gui::out << gui::warning << bg_insets_node->get_location()
                         << ": BackgroundInsets node must contain one of AbsInset or RelInset."
                         << std::endl;
                return;
            }

            if (abs_inset_node) {
                bdrop->set_background_insets(bounds2f(
                    abs_inset_node->get_attribute_value_or<float>("left", 0.0f),
                    abs_inset_node->get_attribute_value_or<float>("right", 0.0f),
                    abs_inset_node->get_attribute_value_or<float>("top", 0.0f),
                    abs_inset_node->get_attribute_value_or<float>("bottom", 0.0f)));
            } else {
                gui::out << gui::warning << rel_inset_node->get_location() << ": "
                         << "RelInset for Backdrop:BackgroundInsets is not yet supported (" << name_
                         << ")." << std::endl;
            }
        }

        if (const layout_node* edge_insets_node = backdrop_node->try_get_child("EdgeInsets")) {
            const layout_node* abs_inset_node = edge_insets_node->try_get_child("AbsInset");
            const layout_node* rel_inset_node = edge_insets_node->try_get_child("RelInset");

            if (abs_inset_node && rel_inset_node) {
                gui::out << gui::warning << edge_insets_node->get_location()
                         << ": EdgeInsets node can only contain one of AbsInset or RelInset, but "
                            "not both. RelInset ignored."
                         << std::endl;
            }

            if (!abs_inset_node && !rel_inset_node) {
                gui::out << gui::warning << edge_insets_node->get_location()
                         << ": EdgeInsets node must contain one of AbsInset or RelInset."
                         << std::endl;
                return;
            }

            if (abs_inset_node) {
                bdrop->set_edge_insets(bounds2f(
                    abs_inset_node->get_attribute_value_or<float>("left", 0.0f),
                    abs_inset_node->get_attribute_value_or<float>("right", 0.0f),
                    abs_inset_node->get_attribute_value_or<float>("top", 0.0f),
                    abs_inset_node->get_attribute_value_or<float>("bottom", 0.0f)));
            } else {
                gui::out << gui::warning << rel_inset_node->get_location() << ": "
                         << "RelInset for Backdrop:EdgeInsets is not yet supported (" << name_
                         << ")." << std::endl;
            }
        }

        if (const layout_node* color_node = backdrop_node->try_get_child("BackgroundColor"))
            bdrop->set_background_color(parse_color_node_(*color_node));

        if (const layout_node* color_node = backdrop_node->try_get_child("EdgeColor"))
            bdrop->set_edge_color(parse_color_node_(*color_node));

        if (const layout_node* edge_size_node = backdrop_node->try_get_child("EdgeSize")) {
            const layout_node* abs_value_node = edge_size_node->try_get_child("AbsValue");
            const layout_node* rel_value_node = edge_size_node->try_get_child("RelValue");

            if (abs_value_node && rel_value_node) {
                gui::out << gui::warning << edge_size_node->get_location()
                         << ": EdgeSize node can only contain one of AbsValue or RelValue, but "
                            "not both. RelValue ignored."
                         << std::endl;
            }

            if (!abs_value_node && !rel_value_node) {
                gui::out << gui::warning << edge_size_node->get_location()
                         << ": EdgeSize node must contain one of AbsValue or RelValue."
                         << std::endl;
                return;
            }

            if (abs_value_node) {
                bdrop->set_edge_size(abs_value_node->get_attribute_value_or("x", 0.0f));
            } else {
                gui::out << gui::warning << rel_value_node->get_location() << ": "
                         << "RelValue for Backdrop:EdgeSize is not yet supported (" << name_ << ")."
                         << std::endl;
            }
        }

        if (const layout_node* tile_size_node = backdrop_node->try_get_child("TileSize")) {
            const layout_node* abs_value_node = tile_size_node->try_get_child("AbsValue");
            const layout_node* rel_value_node = tile_size_node->try_get_child("RelValue");

            if (abs_value_node && rel_value_node) {
                gui::out << gui::warning << tile_size_node->get_location()
                         << ": TileSize node can only contain one of AbsValue or RelValue, but "
                            "not both. RelValue ignored."
                         << std::endl;
            }

            if (!abs_value_node && !rel_value_node) {
                gui::out << gui::warning << tile_size_node->get_location()
                         << ": TileSize node must contain one of AbsValue or RelValue."
                         << std::endl;
                return;
            }

            if (abs_value_node) {
                bdrop->set_tile_size(abs_value_node->get_attribute_value_or("x", 0.0f));
            } else {
                gui::out << gui::warning << rel_value_node->get_location() << ": "
                         << "RelValue for Backdrop:TileSize is not yet supported (" << name_ << ")."
                         << std::endl;
            }
        }

        set_backdrop(std::move(bdrop));
    }
}

void frame::parse_hit_rect_insets_node_(const layout_node& node) {
    if (const layout_node* hit_rect_block = node.try_get_child("HitRectInsets")) {
        const layout_node* abs_inset_node = hit_rect_block->try_get_child("AbsInset");
        const layout_node* rel_inset_node = hit_rect_block->try_get_child("RelInset");

        if (abs_inset_node && rel_inset_node) {
            gui::out << gui::warning << hit_rect_block->get_location()
                     << ": HitRectInsets node can only contain one of AbsInset or RelInset, but "
                        "not both. RelInset ignored."
                     << std::endl;
        }

        if (!abs_inset_node && !rel_inset_node) {
            gui::out << gui::warning << hit_rect_block->get_location()
                     << ": HitRectInsets node must contain one of AbsInset or RelInset."
                     << std::endl;
            return;
        }

        if (abs_inset_node) {
            set_abs_hit_rect_insets(bounds2f(
                abs_inset_node->get_attribute_value_or<float>("left", 0.0f),
                abs_inset_node->get_attribute_value_or<float>("right", 0.0f),
                abs_inset_node->get_attribute_value_or<float>("top", 0.0f),
                abs_inset_node->get_attribute_value_or<float>("bottom", 0.0f)));
        } else {
            set_rel_hit_rect_insets(bounds2f(
                rel_inset_node->get_attribute_value_or<float>("left", 0.0f),
                rel_inset_node->get_attribute_value_or<float>("right", 0.0f),
                rel_inset_node->get_attribute_value_or<float>("top", 0.0f),
                rel_inset_node->get_attribute_value_or<float>("bottom", 0.0f)));
        }
    }
}

utils::observer_ptr<layered_region> frame::parse_region_(
    const layout_node& node, const std::string& layer_name, const std::string& type) {
    try {
        auto attr = parse_core_attributes(
            get_manager().get_root().get_registry(),
            get_manager().get_virtual_root().get_registry(), node, observer_from(this));

        if (!type.empty())
            attr.object_type = type;

        auto layer_id = utils::from_string<layer>(layer_name);
        if (!layer_id.has_value()) {
            gui::out << gui::warning << node.get_location() << ": "
                     << "Unknown layer type: \"" << layer_name << "\". Using \"ARTWORK\"."
                     << std::endl;
        }

        auto reg = create_layered_region(layer_id.value_or(layer::artwork), attr);
        if (!reg)
            return nullptr;

        try {
            reg->parse_layout(node);
            reg->notify_loaded();
            return reg;
        } catch (...) {
            reg->release_from_parent();
            throw;
        }
    } catch (const exception& e) {
        gui::out << gui::error << e.get_description() << std::endl;
    }

    return nullptr;
}

void frame::parse_layers_node_(const layout_node& node) {
    if (const layout_node* layers_node = node.try_get_child("Layers")) {
        for (const layout_node& layer_node : layers_node->get_children()) {
            if (layer_node.get_name() != "Layer" && layer_node.get_name() != "") {
                gui::out << gui::warning << layer_node.get_location() << ": "
                         << "unexpected node '" << layer_node.get_name() << "'; ignored."
                         << std::endl;
                continue;
            }

            std::string level = layer_node.get_attribute_value_or<std::string>("level", "ARTWORK");
            for (const layout_node& region_node : layer_node.get_children()) {
                parse_region_(region_node, level, "");
            }
        }
    }
}

utils::observer_ptr<frame> frame::parse_child_(const layout_node& node, const std::string& type) {
    try {
        auto attr = parse_core_attributes(
            get_manager().get_root().get_registry(),
            get_manager().get_virtual_root().get_registry(), node, observer_from(this));

        if (!type.empty())
            attr.object_type = type;

        auto obj = create_child(attr);
        if (!obj)
            return nullptr;

        try {
            obj->set_addon(get_addon());
            obj->parse_layout(node);
            obj->notify_loaded();
            return obj;
        } catch (...) {
            obj->release_from_parent();
            throw;
        }
    } catch (const exception& e) {
        gui::out << gui::error << e.get_description() << std::endl;
    }

    return nullptr;
}

void frame::parse_frames_node_(const layout_node& node) {
    if (const layout_node* frames_node = node.try_get_child("Frames")) {
        for (const layout_node& elem_node : frames_node->get_children()) {
            parse_child_(elem_node, "");
        }
    }
}

void frame::parse_scripts_node_(const layout_node& node) {
    if (const layout_node* scripts_node = node.try_get_child("Scripts")) {
        for (const layout_node& script_node : scripts_node->get_children()) {
            std::string name = std::string(script_node.get_name());

            const layout_attribute* actual_script_node = &script_node;
            if (const layout_attribute* run_node = script_node.try_get_attribute("run"))
                actual_script_node = run_node;

            std::string script = std::string(actual_script_node->get_value());
            script_info info{
                std::string(actual_script_node->get_filename()),
                static_cast<std::size_t>(actual_script_node->get_value_line_number())};

            if (script_node.get_attribute_value_or<bool>("override", false))
                set_script(name, std::move(script), std::move(info));
            else
                add_script(name, std::move(script), std::move(info));
        }
    }
}

} // namespace lxgui::gui
