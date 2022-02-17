#include "lxgui/gui_region.hpp"

#include "lxgui/gui_addon.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_virtual_registry.hpp"
#include "lxgui/gui_virtual_root.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <sol/state.hpp>
#include <sstream>

namespace lxgui::gui {

region::region(utils::control_block& m_block, manager& m_manager) :
    utils::enable_observer_from_this<region>(m_block), m_manager_(m_manager) {
    type_.push_back(class_name);
}

region::~region() {
    if (!is_virtual_) {
        // Tell this region's anchor parents that it is no longer anchored to them
        for (auto& m_anchor : anchor_list_) {
            if (m_anchor) {
                if (auto* p_anchor_parent = m_anchor->get_parent().get())
                    p_anchor_parent->remove_anchored_object(*this);
            }

            m_anchor.reset();
        }

        // Replace anchors pointing to this region by absolute anchors
        // (need to copy the anchored object list, because the objects will attempt to
        // modify it when un-anchored, which would invalidate our iteration)
        std::vector<utils::observer_ptr<region>> temp_anchored_object_list =
            std::move(anchored_object_list_);
        for (const auto& p_obj : temp_anchored_object_list) {
            if (!p_obj)
                continue;

            std::vector<anchor_point> anchored_point_list;
            for (const auto& m_anchor : p_obj->get_point_list()) {
                if (m_anchor && m_anchor->get_parent().get() == this)
                    anchored_point_list.push_back(m_anchor->m_point);
            }

            for (const auto& m_point : anchored_point_list) {
                const anchor& m_anchor     = p_obj->get_point(m_point);
                anchor_data   m_new_anchor = anchor_data(m_point, "", anchor_point::top_left);
                m_new_anchor.m_offset      = m_anchor.m_offset;

                switch (m_anchor.m_parent_point) {
                case anchor_point::top_left:
                    m_new_anchor.m_offset += border_list_.top_left();
                    break;
                case anchor_point::top: m_new_anchor.m_offset.y += border_list_.top; break;
                case anchor_point::top_right:
                    m_new_anchor.m_offset += border_list_.top_right();
                    break;
                case anchor_point::right: m_new_anchor.m_offset.x += border_list_.right; break;
                case anchor_point::bottom_right:
                    m_new_anchor.m_offset += border_list_.bottom_right();
                    break;
                case anchor_point::bottom: m_new_anchor.m_offset.y += border_list_.bottom; break;
                case anchor_point::bottom_left:
                    m_new_anchor.m_offset += border_list_.bottom_left();
                    break;
                case anchor_point::left: m_new_anchor.m_offset.x += border_list_.left; break;
                case anchor_point::center: m_new_anchor.m_offset += border_list_.center(); break;
                }

                p_obj->set_point(m_new_anchor);
            }

            p_obj->update_anchors_();
        }

        remove_glue();
    }

    // Unregister this object from the GUI manager
    if (!is_virtual() || p_parent_ == nullptr)
        get_registry().remove_region(*this);
}

std::string region::serialize(const std::string& tab) const {
    std::ostringstream str;

    str << tab << "  # Name        : " << name_
        << " (" + std::string(is_ready_ ? "ready" : "not ready") +
               std::string(is_special_ ? ", special)\n" : ")\n");
    str << tab << "  # Raw name    : " << raw_name_ << "\n";
    str << tab << "  # Lua name    : " << lua_name_ << "\n";
    str << tab << "  # Type        : " << type_.back() << "\n";
    if (p_parent_)
        str << tab << "  # Parent      : " << p_parent_->get_name() << "\n";
    else
        str << tab << "  # Parent      : none\n";
    str << tab << "  # Num anchors : " << get_num_point() << "\n";
    if (!anchor_list_.empty()) {
        str << tab << "  |-###\n";
        for (const auto& m_anchor : anchor_list_) {
            if (m_anchor) {
                str << m_anchor->serialize(tab);
                str << tab << "  |-###\n";
            }
        }
    }
    str << tab << "  # Borders :\n";
    str << tab << "  |-###\n";
    str << tab << "  |   # left   : " << border_list_.left << "\n";
    str << tab << "  |   # top    : " << border_list_.top << "\n";
    str << tab << "  |   # right  : " << border_list_.right << "\n";
    str << tab << "  |   # bottom : " << border_list_.bottom << "\n";
    str << tab << "  |-###\n";
    str << tab << "  # Alpha       : " << f_alpha_ << "\n";
    str << tab << "  # Shown       : " << is_shown_ << "\n";
    str << tab << "  # Abs width   : " << m_dimensions_.x << "\n";
    str << tab << "  # Abs height  : " << m_dimensions_.y << "\n";

    return str.str();
}

void region::copy_from(const region& m_obj) {
    // Inherit properties
    this->set_alpha(m_obj.get_alpha());
    this->set_shown(m_obj.is_shown());
    this->set_dimensions(m_obj.get_dimensions());

    for (const auto& m_anchor : m_obj.get_point_list()) {
        if (!m_anchor)
            continue;

        this->set_point(m_anchor->get_data());
    }
}

const std::string& region::get_name() const {
    return name_;
}

const std::string& region::get_lua_name() const {
    return lua_name_;
}

const std::string& region::get_raw_name() const {
    return raw_name_;
}

const std::string& region::get_object_type() const {
    return type_.back();
}

const std::vector<std::string>& region::get_object_type_list() const {
    return type_;
}

bool region::is_object_type(const std::string& type_name) const {
    return utils::find(type_, type_name) != type_.end();
}

float region::get_alpha() const {
    return f_alpha_;
}

float region::get_effective_alpha() const {
    if (p_parent_) {
        return p_parent_->get_effective_alpha() * get_alpha();
    } else {
        return get_alpha();
    }
}

void region::set_alpha(float f_alpha) {
    if (f_alpha_ != f_alpha) {
        f_alpha_ = f_alpha;
        notify_renderer_need_redraw();
    }
}

void region::show() {
    if (is_shown_)
        return;

    is_shown_ = true;

    if (!is_visible_ && (!p_parent_ || p_parent_->is_visible()))
        notify_visible();
}

void region::hide() {
    if (!is_shown_)
        return;

    is_shown_ = false;

    if (is_visible_)
        notify_invisible();
}

void region::set_shown(bool is_shown) {
    if (is_shown)
        show();
    else
        hide();
}

bool region::is_shown() const {
    return is_shown_;
}

bool region::is_visible() const {
    return is_visible_;
}

void region::set_dimensions(const vector2f& m_dimensions) {
    if (m_dimensions_ == m_dimensions)
        return;

    m_dimensions_ = m_dimensions;

    if (!is_virtual_) {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void region::set_width(float f_abs_width) {
    if (m_dimensions_.x == f_abs_width)
        return;

    m_dimensions_.x = f_abs_width;

    if (!is_virtual_) {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void region::set_height(float f_abs_height) {
    if (m_dimensions_.y == f_abs_height)
        return;

    m_dimensions_.y = f_abs_height;

    if (!is_virtual_) {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void region::set_relative_dimensions(const vector2f& m_dimensions) {
    if (p_parent_)
        set_dimensions(m_dimensions * p_parent_->get_apparent_dimensions());
    else
        set_dimensions(m_dimensions * get_top_level_renderer()->get_target_dimensions());
}

void region::set_relative_width(float f_rel_width) {
    if (p_parent_)
        set_width(f_rel_width * p_parent_->get_apparent_dimensions().x);
    else
        set_width(f_rel_width * get_top_level_renderer()->get_target_dimensions().x);
}

void region::set_relative_height(float f_rel_height) {
    if (p_parent_)
        set_height(f_rel_height * p_parent_->get_apparent_dimensions().y);
    else
        set_height(f_rel_height * get_top_level_renderer()->get_target_dimensions().y);
}

const vector2f& region::get_dimensions() const {
    return m_dimensions_;
}

vector2f region::get_apparent_dimensions() const {
    return vector2f(border_list_.width(), border_list_.height());
}

bool region::is_apparent_width_defined() const {
    return m_dimensions_.x > 0.0f || (defined_border_list_.left && defined_border_list_.right);
}

bool region::is_apparent_height_defined() const {
    return m_dimensions_.y > 0.0f || (defined_border_list_.top && defined_border_list_.bottom);
}

bool region::is_in_region(const vector2f& m_position) const {
    return (
        (border_list_.left <= m_position.x && m_position.x <= border_list_.right - 1) &&
        (border_list_.top <= m_position.y && m_position.y <= border_list_.bottom - 1));
}

void region::set_name_(const std::string& name) {
    if (name_.empty()) {
        name_ = lua_name_ = raw_name_ = name;
        if (utils::starts_with(name_, "$parent")) {
            if (p_parent_)
                utils::replace(lua_name_, "$parent", p_parent_->get_lua_name());
            else {
                gui::out << gui::warning << "gui::" << type_.back() << " : \"" << name_
                         << "\" has no parent" << std::endl;
                utils::replace(lua_name_, "$parent", "");
            }
        }

        if (!is_virtual_)
            name_ = lua_name_;
    } else {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "set_name() can only be called once." << std::endl;
    }
}

void region::set_parent_(utils::observer_ptr<frame> p_parent) {
    if (p_parent == observer_from_this()) {
        gui::out << gui::error << "gui::" << type_.back() << " : Cannot call set_parent(this)."
                 << std::endl;
        return;
    }

    if (p_parent_ != p_parent) {
        p_parent_ = std::move(p_parent);

        if (!is_virtual_)
            notify_borders_need_update();
    }
}

void region::set_name_and_parent_(const std::string& name, utils::observer_ptr<frame> p_parent) {
    if (p_parent == observer_from_this()) {
        gui::out << gui::error << "gui::" << type_.back() << " : Cannot call set_parent(this)."
                 << std::endl;
        return;
    }

    if (p_parent_ == p_parent && name == name_)
        return;

    p_parent_ = std::move(p_parent);
    set_name_(name);

    if (!is_virtual_)
        notify_borders_need_update();
}

utils::owner_ptr<region> region::release_from_parent() {
    return nullptr;
}

void region::destroy() {
    // Gracefully disappear (triggers events, etc).
    hide();

    // Ignoring the return value destroys the object.
    release_from_parent();
}

vector2f region::get_center() const {
    return border_list_.center();
}

float region::get_left() const {
    return border_list_.left;
}

float region::get_right() const {
    return border_list_.right;
}

float region::get_top() const {
    return border_list_.top;
}

float region::get_bottom() const {
    return border_list_.bottom;
}

const bounds2f& region::get_borders() const {
    return border_list_;
}

void region::clear_all_points() {
    bool had_anchors = false;
    for (auto& m_anchor : anchor_list_) {
        if (m_anchor) {
            m_anchor.reset();
            had_anchors = true;
        }
    }

    if (had_anchors) {
        defined_border_list_ = bounds2<bool>(false, false, false, false);

        if (!is_virtual_) {
            update_anchors_();
            notify_borders_need_update();
            notify_renderer_need_redraw();
        }
    }
}

void region::set_all_points(const std::string& obj_name) {
    if (obj_name == name_) {
        gui::out << gui::error << "gui::" << type_.back() << " : Cannot call set_all_points(this)."
                 << std::endl;
        return;
    }

    clear_all_points();

    anchor_list_[static_cast<int>(anchor_point::top_left)].emplace(
        *this, anchor_data(anchor_point::top_left, obj_name));

    anchor_list_[static_cast<int>(anchor_point::bottom_right)].emplace(
        *this, anchor_data(anchor_point::bottom_right, obj_name));

    defined_border_list_ = bounds2<bool>(true, true, true, true);

    if (!is_virtual_) {
        update_anchors_();
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void region::set_all_points(const utils::observer_ptr<region>& p_obj) {
    if (p_obj == observer_from_this()) {
        gui::out << gui::error << "gui::" << type_.back() << " : Cannot call set_all_points(this)."
                 << std::endl;
        return;
    }

    set_all_points(p_obj ? p_obj->get_name() : "");
}

void region::set_point(const anchor_data& m_anchor) {
    anchor_list_[static_cast<int>(m_anchor.m_point)].emplace(*this, m_anchor);

    switch (m_anchor.m_point) {
    case anchor_point::top_left:
        defined_border_list_.top  = true;
        defined_border_list_.left = true;
        break;
    case anchor_point::top: defined_border_list_.top = true; break;
    case anchor_point::top_right:
        defined_border_list_.top   = true;
        defined_border_list_.right = true;
        break;
    case anchor_point::right: defined_border_list_.right = true; break;
    case anchor_point::bottom_right:
        defined_border_list_.bottom = true;
        defined_border_list_.right  = true;
        break;
    case anchor_point::bottom: defined_border_list_.bottom = true; break;
    case anchor_point::bottom_left:
        defined_border_list_.bottom = true;
        defined_border_list_.left   = true;
        break;
    case anchor_point::left: defined_border_list_.left = true; break;
    default: break;
    }

    if (!is_virtual_) {
        update_anchors_();
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

bool region::depends_on(const region& m_obj) const {
    for (const auto& m_anchor : anchor_list_) {
        if (!m_anchor)
            continue;

        const region* p_parent = m_anchor->get_parent().get();
        if (p_parent == &m_obj)
            return true;

        if (p_parent)
            return p_parent->depends_on(m_obj);
    }

    return false;
}

std::size_t region::get_num_point() const {
    std::size_t ui_num_anchors = 0u;
    for (const auto& m_anchor : anchor_list_) {
        if (m_anchor)
            ++ui_num_anchors;
    }

    return ui_num_anchors;
}

anchor& region::modify_point(anchor_point m_point) {
    auto& m_anchor = anchor_list_[static_cast<int>(m_point)];
    if (!m_anchor) {
        throw gui::exception(
            "region", "Cannot modify a point that does not exist. Use set_point() first.");
    }

    return *m_anchor;
}

const anchor& region::get_point(anchor_point m_point) const {
    const auto& m_anchor = anchor_list_[static_cast<int>(m_point)];
    if (!m_anchor) {
        throw gui::exception(
            "region", "Cannot get a point that does not exist. Use set_point() first.");
    }

    return *m_anchor;
}

const std::array<std::optional<anchor>, 9>& region::get_point_list() const {
    return anchor_list_;
}

bool region::is_virtual() const {
    return is_virtual_;
}

void region::set_virtual() {
    is_virtual_ = true;
}

void region::add_anchored_object(region& m_obj) {
    anchored_object_list_.push_back(observer_from(&m_obj));
}

void region::remove_anchored_object(region& m_obj) {
    auto m_iter = utils::find_if(
        anchored_object_list_, [&](const auto& p_ptr) { return p_ptr.get() == &m_obj; });

    if (m_iter != anchored_object_list_.end())
        anchored_object_list_.erase(m_iter);
}

float region::round_to_pixel(float f_value, utils::rounding_method m_method) const {
    float f_scaling_factor = get_manager().get_interface_scaling_factor();
    return utils::round(f_value, 1.0f / f_scaling_factor, m_method);
}

vector2f region::round_to_pixel(const vector2f& m_position, utils::rounding_method m_method) const {
    float f_scaling_factor = get_manager().get_interface_scaling_factor();
    return vector2f(
        utils::round(m_position.x, 1.0f / f_scaling_factor, m_method),
        utils::round(m_position.y, 1.0f / f_scaling_factor, m_method));
}

bool region::make_borders_(float& f_min, float& f_max, float f_center, float f_size) const {
    if (std::isinf(f_min) && std::isinf(f_max)) {
        if (!std::isinf(f_size) && f_size > 0.0f && !std::isinf(f_center)) {
            f_min = f_center - f_size / 2.0f;
            f_max = f_center + f_size / 2.0f;
        } else
            return false;
    } else if (std::isinf(f_max)) {
        if (!std::isinf(f_size) && f_size > 0.0f)
            f_max = f_min + f_size;
        else if (!std::isinf(f_center))
            f_max = f_min + 2.0f * (f_center - f_min);
        else
            return false;
    } else if (std::isinf(f_min)) {
        if (!std::isinf(f_size) && f_size > 0.0f)
            f_min = f_max - f_size;
        else if (!std::isinf(f_center))
            f_min = f_max - 2.0f * (f_max - f_center);
        else
            return false;
    }

    return true;
}

void region::read_anchors_(
    float& f_left,
    float& f_right,
    float& f_top,
    float& f_bottom,
    float& f_x_center,
    float& f_y_center) const {
    f_left   = +std::numeric_limits<float>::infinity();
    f_right  = -std::numeric_limits<float>::infinity();
    f_top    = +std::numeric_limits<float>::infinity();
    f_bottom = -std::numeric_limits<float>::infinity();

    for (const auto& m_opt_anchor : anchor_list_) {
        if (!m_opt_anchor)
            continue;

        const anchor&  m_anchor       = m_opt_anchor.value();
        const vector2f m_anchor_point = m_anchor.get_point(*this);

        switch (m_anchor.m_point) {
        case anchor_point::top_left:
            f_top  = std::min<float>(f_top, m_anchor_point.y);
            f_left = std::min<float>(f_left, m_anchor_point.x);
            break;
        case anchor_point::top:
            f_top      = std::min<float>(f_top, m_anchor_point.y);
            f_x_center = m_anchor_point.x;
            break;
        case anchor_point::top_right:
            f_top   = std::min<float>(f_top, m_anchor_point.y);
            f_right = std::max<float>(f_right, m_anchor_point.x);
            break;
        case anchor_point::right:
            f_right    = std::max<float>(f_right, m_anchor_point.x);
            f_y_center = m_anchor_point.y;
            break;
        case anchor_point::bottom_right:
            f_bottom = std::max<float>(f_bottom, m_anchor_point.y);
            f_right  = std::max<float>(f_right, m_anchor_point.x);
            break;
        case anchor_point::bottom:
            f_bottom   = std::max<float>(f_bottom, m_anchor_point.y);
            f_x_center = m_anchor_point.x;
            break;
        case anchor_point::bottom_left:
            f_bottom = std::max<float>(f_bottom, m_anchor_point.y);
            f_left   = std::min<float>(f_left, m_anchor_point.x);
            break;
        case anchor_point::left:
            f_left     = std::min<float>(f_left, m_anchor_point.x);
            f_y_center = m_anchor_point.y;
            break;
        case anchor_point::center:
            f_x_center = m_anchor_point.x;
            f_y_center = m_anchor_point.y;
            break;
        }
    }
}

void region::update_borders_() {
// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

    DEBUG_LOG("  Update anchors for " + sLuaName_);

    const bool old_is_ready    = is_ready_;
    const auto old_border_list = border_list_;

    is_ready_ = true;

    if (!anchor_list_.empty()) {
        float f_left = 0.0f, f_right = 0.0f, f_top = 0.0f, f_bottom = 0.0f;
        float f_x_center = 0.0f, f_y_center = 0.0f;

        float f_rounded_width =
            round_to_pixel(m_dimensions_.x, utils::rounding_method::nearest_not_zero);
        float f_rounded_height =
            round_to_pixel(m_dimensions_.y, utils::rounding_method::nearest_not_zero);

        DEBUG_LOG("  Read anchors");
        read_anchors_(f_left, f_right, f_top, f_bottom, f_x_center, f_y_center);
        DEBUG_LOG("    left=" + utils::to_string(fLeft));
        DEBUG_LOG("    right=" + utils::to_string(fRight));
        DEBUG_LOG("    top=" + utils::to_string(fTop));
        DEBUG_LOG("    bottom=" + utils::to_string(fBottom));
        DEBUG_LOG("    x_center=" + utils::to_string(fXCenter));
        DEBUG_LOG("    y_center=" + utils::to_string(fYCenter));

        DEBUG_LOG("  Make borders");
        if (!make_borders_(f_top, f_bottom, f_y_center, f_rounded_height))
            is_ready_ = false;
        if (!make_borders_(f_left, f_right, f_x_center, f_rounded_width))
            is_ready_ = false;

        if (is_ready_) {
            if (f_right < f_left)
                f_right = f_left + 1;
            if (f_bottom < f_top)
                f_bottom = f_top + 1;

            border_list_ = bounds2f(f_left, f_right, f_top, f_bottom);
        } else
            border_list_ = bounds2f::zero;
    } else {
        border_list_ = bounds2f(0.0, 0.0, m_dimensions_.x, m_dimensions_.y);
        is_ready_    = false;
    }

    DEBUG_LOG("  Final borders");
    border_list_.left   = round_to_pixel(border_list_.left);
    border_list_.right  = round_to_pixel(border_list_.right);
    border_list_.top    = round_to_pixel(border_list_.top);
    border_list_.bottom = round_to_pixel(border_list_.bottom);

    DEBUG_LOG("    left=" + utils::to_string(lBorderList_.left));
    DEBUG_LOG("    right=" + utils::to_string(lBorderList_.right));
    DEBUG_LOG("    top=" + utils::to_string(lBorderList_.top));
    DEBUG_LOG("    bottom=" + utils::to_string(lBorderList_.bottom));

    if (border_list_ != old_border_list || is_ready_ != old_is_ready) {
        DEBUG_LOG("  Fire redraw");
        notify_renderer_need_redraw();
    }

    DEBUG_LOG("  @");
#undef DEBUG_LOG
}

void region::update_anchors_() {
    std::vector<utils::observer_ptr<region>> anchor_parent_list;
    for (auto& m_anchor : anchor_list_) {
        if (!m_anchor)
            continue;

        utils::observer_ptr<region> p_obj = m_anchor->get_parent();
        if (p_obj) {
            if (p_obj->depends_on(*this)) {
                gui::out << gui::error << "gui::" << type_.back()
                         << " : Cyclic anchor dependency ! "
                         << "\"" << name_ << "\" and \"" << p_obj->get_name()
                         << "\" depend on "
                            "eachothers (directly or indirectly).\n\""
                         << anchor::get_anchor_point_name(m_anchor->m_point) << "\" anchor removed."
                         << std::endl;

                m_anchor.reset();
                continue;
            }

            if (utils::find(anchor_parent_list, p_obj) == anchor_parent_list.end())
                anchor_parent_list.push_back(p_obj);
        }
    }

    for (const auto& p_parent : previous_anchor_parent_list_) {
        if (utils::find(anchor_parent_list, p_parent) == anchor_parent_list.end())
            p_parent->remove_anchored_object(*this);
    }

    for (const auto& p_parent : anchor_parent_list) {
        if (utils::find(previous_anchor_parent_list_, p_parent) ==
            previous_anchor_parent_list_.end())
            p_parent->add_anchored_object(*this);
    }

    previous_anchor_parent_list_ = std::move(anchor_parent_list);
}

void region::notify_borders_need_update() {
    if (is_virtual())
        return;

    update_borders_();

    for (const auto& p_object : anchored_object_list_)
        p_object->notify_borders_need_update();
}

void region::notify_scaling_factor_updated() {
    notify_borders_need_update();
}

void region::update(float) {}

void region::render() const {}

sol::state& region::get_lua_() {
    return get_manager().get_lua();
}

void region::create_glue() {
    create_glue_(static_cast<region*>(this));
}

void region::remove_glue() {
    get_lua_().globals()[lua_name_] = sol::lua_nil;
}

void region::set_special() {
    is_special_ = true;
}

bool region::is_special() const {
    return is_special_;
}

void region::notify_renderer_need_redraw() {}

const std::vector<utils::observer_ptr<region>>& region::get_anchored_objects() const {
    return anchored_object_list_;
}

void region::notify_loaded() {
    is_loaded_ = true;
}

bool region::is_loaded() const {
    return is_loaded_;
}

utils::observer_ptr<const frame_renderer> region::get_top_level_renderer() const {
    if (!p_parent_)
        return get_manager().get_root().observer_from_this();
    return p_parent_->get_top_level_renderer();
}

void region::notify_visible() {
    is_visible_ = true;
}

void region::notify_invisible() {
    is_visible_ = false;
}

std::string region::parse_file_name(const std::string& file_name) const {
    if (file_name.empty())
        return file_name;

    std::string new_file = file_name;

    const addon* p_add_on = get_addon();
    if (new_file[0] == '|' && p_add_on) {
        new_file[0] = '/';
        new_file    = p_add_on->directory + new_file;
    }

    return new_file;
}

void region::set_addon(const addon* p_add_on) {
    if (p_add_on_) {
        gui::out << gui::warning << "gui::" << type_.back()
                 << " : set_addon() can only be called once." << std::endl;
        return;
    }

    p_add_on_ = p_add_on;
}

const addon* region::get_addon() const {
    if (!p_add_on_ && p_parent_)
        return p_parent_->get_addon();
    else
        return p_add_on_;
}

registry& region::get_registry() {
    return is_virtual() ? get_manager().get_virtual_root().get_registry()
                        : get_manager().get_root().get_registry();
}

const registry& region::get_registry() const {
    return is_virtual() ? get_manager().get_virtual_root().get_registry()
                        : get_manager().get_root().get_registry();
}

} // namespace lxgui::gui
