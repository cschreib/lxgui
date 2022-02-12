#include "lxgui/gui_anchor.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/utils_string.hpp"

#include <sstream>

namespace lxgui::gui {

anchor::anchor(region& m_object, const anchor_data& m_anchor) : anchor_data(m_anchor) {
    if (!m_object.is_virtual()) {
        if (s_parent == "$default")
            s_parent = m_object.get_parent() ? "$parent" : "";

        update_parent_(m_object);
    }
}

void anchor::update_parent_(region& m_object) {
    p_parent_ = nullptr;

    if (s_parent.empty())
        return;

    utils::observer_ptr<frame> p_obj_parent = m_object.get_parent();

    std::string s_parent_full_name = s_parent;
    if (p_obj_parent) {
        utils::replace(s_parent_full_name, "$parent", p_obj_parent->get_lua_name());
    } else if (s_parent_full_name.find("$parent") != s_parent_full_name.npos) {
        gui::out << gui::error << "gui::" << m_object.get_object_type() << " : "
                 << "region \"" << m_object.get_name() << "\" tries to anchor to \""
                 << s_parent_full_name << "\", but '$parent' does not exist." << std::endl;
        return;
    }

    utils::observer_ptr<region> p_new_parent =
        m_object.get_registry().get_region_by_name(s_parent_full_name);

    if (!p_new_parent) {
        gui::out << gui::error << "gui::" << m_object.get_object_type() << " : "
                 << "region \"" << m_object.get_name() << "\" tries to anchor to \""
                 << s_parent_full_name << "\" but this region does not (yet?) exist." << std::endl;
        return;
    }

    p_parent_ = p_new_parent;
}

vector2f anchor::get_point(const region& m_object) const {
    vector2f m_parent_pos;
    vector2f m_parent_size;
    if (const region* p_raw_parent = p_parent_.get()) {
        m_parent_pos  = p_raw_parent->get_borders().top_left();
        m_parent_size = p_raw_parent->get_apparent_dimensions();
    } else {
        m_parent_size = m_object.get_top_level_renderer()->get_target_dimensions();
    }

    vector2f m_offset_abs;
    if (m_type == anchor_type::abs)
        m_offset_abs = m_offset;
    else
        m_offset_abs = m_offset * m_parent_size;

    m_offset_abs = m_object.round_to_pixel(m_offset_abs, utils::rounding_method::nearest_not_zero);

    vector2f m_parent_offset;
    switch (m_parent_point) {
    case anchor_point::top_left:
        m_parent_offset.x = 0.0f;
        m_parent_offset.y = 0.0f;
        break;
    case anchor_point::left:
        m_parent_offset.x = 0.0f;
        m_parent_offset.y = m_parent_size.y / 2.0f;
        break;
    case anchor_point::bottom_left:
        m_parent_offset.x = 0.0f;
        m_parent_offset.y = m_parent_size.y;
        break;
    case anchor_point::top:
        m_parent_offset.x = m_parent_size.x / 2.0f;
        m_parent_offset.y = 0.0f;
        break;
    case anchor_point::center: m_parent_offset = m_parent_size / 2.0f; break;
    case anchor_point::bottom:
        m_parent_offset.x = m_parent_size.x / 2.0f;
        m_parent_offset.y = m_parent_size.y;
        break;
    case anchor_point::top_right:
        m_parent_offset.x = m_parent_size.x;
        m_parent_offset.y = 0.0f;
        break;
    case anchor_point::right:
        m_parent_offset.x = m_parent_size.x;
        m_parent_offset.y = m_parent_size.y / 2.0f;
        break;
    case anchor_point::bottom_right:
        m_parent_offset.x = m_parent_size.x;
        m_parent_offset.y = m_parent_size.y;
        break;
    }

    return m_offset_abs + m_parent_offset + m_parent_pos;
}

std::string anchor::serialize(const std::string& s_tab) const {
    std::stringstream s_str;

    s_str << s_tab << "  |   # Point      : " << get_string_point(m_point) << "\n";
    if (p_parent_)
        s_str << s_tab << "  |   # Parent     : " << p_parent_->get_name();
    else
        s_str << s_tab << "  |   # Parent     : none";
    if (!s_parent.empty())
        s_str << " (raw name : " << s_parent << ")\n";
    else
        s_str << "\n";
    s_str << s_tab << "  |   # Rel. point : " << get_string_point(m_parent_point) << "\n";
    if (m_type == anchor_type::abs) {
        s_str << s_tab << "  |   # Offset X   : " << m_offset.x << "\n";
        s_str << s_tab << "  |   # Offset Y   : " << m_offset.y << "\n";
    } else {
        s_str << s_tab << "  |   # Offset X   : " << m_offset.x << " (rel)\n";
        s_str << s_tab << "  |   # Offset Y   : " << m_offset.y << " (rel)\n";
    }

    return s_str.str();
}

std::string anchor::get_string_point(anchor_point m_p) {
    switch (m_p) {
    case anchor_point::top_left: return "TOP_LEFT";
    case anchor_point::top: return "TOP";
    case anchor_point::top_right: return "TOP_RIGHT";
    case anchor_point::right: return "RIGHT";
    case anchor_point::bottom_right: return "BOTTOM_RIGHT";
    case anchor_point::bottom: return "BOTTOM";
    case anchor_point::bottom_left: return "BOTTOM_LEFT";
    case anchor_point::left: return "LEFT";
    case anchor_point::center: return "CENTER";
    }
    return "";
}

anchor_point anchor::get_anchor_point(const std::string& s_point) {
    if (s_point == "TOP_LEFT")
        return anchor_point::top_left;
    else if (s_point == "TOP")
        return anchor_point::top;
    else if (s_point == "TOP_RIGHT")
        return anchor_point::top_right;
    else if (s_point == "RIGHT")
        return anchor_point::right;
    else if (s_point == "BOTTOM_RIGHT")
        return anchor_point::bottom_right;
    else if (s_point == "BOTTOM")
        return anchor_point::bottom;
    else if (s_point == "BOTTOM_LEFT")
        return anchor_point::bottom_left;
    else if (s_point == "LEFT")
        return anchor_point::left;
    else if (s_point == "CENTER")
        return anchor_point::center;
    return anchor_point::top_left;
}

} // namespace lxgui::gui
