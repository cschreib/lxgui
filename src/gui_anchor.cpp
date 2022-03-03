#include "lxgui/gui_anchor.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_frame_renderer.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/utils_string.hpp"

#include <sstream>

namespace lxgui::gui {

anchor::anchor(region& object, const anchor_data& data) : anchor_data(data) {
    if (!object.is_virtual()) {
        if (parent_name == "$default")
            parent_name = object.get_parent() ? "$parent" : "";

        update_parent_(object);
    }
}

void anchor::update_parent_(region& object) {
    parent_ = nullptr;

    if (parent_name.empty())
        return;

    utils::observer_ptr<frame> obj_parent = object.get_parent();

    std::string parent_full_name = parent_name;
    if (obj_parent) {
        utils::replace(parent_full_name, "$parent", obj_parent->get_lua_name());
    } else if (parent_full_name.find("$parent") != parent_full_name.npos) {
        gui::out << gui::error << "gui::" << object.get_object_type() << ": "
                 << "region \"" << object.get_name() << "\" tries to anchor to \""
                 << parent_full_name << "\", but '$parent' does not exist." << std::endl;
        return;
    }

    utils::observer_ptr<region> new_parent =
        object.get_registry().get_region_by_name(parent_full_name);

    if (!new_parent) {
        gui::out << gui::error << "gui::" << object.get_object_type() << ": "
                 << "region \"" << object.get_name() << "\" tries to anchor to \""
                 << parent_full_name << "\" but this region does not (yet?) exist." << std::endl;
        return;
    }

    parent_ = new_parent;
}

vector2f anchor::get_point(const region& object) const {
    vector2f parent_pos;
    vector2f parent_size;
    if (const region* raw_parent = parent_.get()) {
        parent_pos  = raw_parent->get_borders().top_left();
        parent_size = raw_parent->get_apparent_dimensions();
    } else {
        parent_size = object.get_top_level_frame_renderer()->get_target_dimensions();
    }

    vector2f offset_abs;
    if (type == anchor_type::abs)
        offset_abs = offset;
    else
        offset_abs = offset * parent_size;

    offset_abs = object.round_to_pixel(offset_abs, utils::rounding_method::nearest_not_zero);

    vector2f parent_offset;
    switch (parent_point) {
    case point::top_left:
        parent_offset.x = 0.0f;
        parent_offset.y = 0.0f;
        break;
    case point::left:
        parent_offset.x = 0.0f;
        parent_offset.y = parent_size.y / 2.0f;
        break;
    case point::bottom_left:
        parent_offset.x = 0.0f;
        parent_offset.y = parent_size.y;
        break;
    case point::top:
        parent_offset.x = parent_size.x / 2.0f;
        parent_offset.y = 0.0f;
        break;
    case point::center: parent_offset = parent_size / 2.0f; break;
    case point::bottom:
        parent_offset.x = parent_size.x / 2.0f;
        parent_offset.y = parent_size.y;
        break;
    case point::top_right:
        parent_offset.x = parent_size.x;
        parent_offset.y = 0.0f;
        break;
    case point::right:
        parent_offset.x = parent_size.x;
        parent_offset.y = parent_size.y / 2.0f;
        break;
    case point::bottom_right:
        parent_offset.x = parent_size.x;
        parent_offset.y = parent_size.y;
        break;
    }

    return offset_abs + parent_offset + parent_pos;
}

std::string anchor::serialize(const std::string& tab) const {
    std::stringstream str;

    str << tab << "  |   # Point     : " << utils::to_string(object_point) << "\n";
    if (parent_)
        str << tab << "  |   # Parent    : " << parent_->get_name();
    else
        str << tab << "  |   # Parent    : none";
    if (!parent_name.empty())
        str << " (raw name: " << parent_name << ")\n";
    else
        str << "\n";
    str << tab << "  |   # Rel. point: " << utils::to_string(parent_point) << "\n";
    if (type == anchor_type::abs) {
        str << tab << "  |   # Offset X  : " << offset.x << "\n";
        str << tab << "  |   # Offset Y  : " << offset.y << "\n";
    } else {
        str << tab << "  |   # Offset X  : " << offset.x << " (rel)\n";
        str << tab << "  |   # Offset Y  : " << offset.y << " (rel)\n";
    }

    return str.str();
}

} // namespace lxgui::gui
