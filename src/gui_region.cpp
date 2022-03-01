#include "lxgui/gui_region.hpp"

#include "lxgui/gui_addon.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_virtual_registry.hpp"
#include "lxgui/gui_virtual_root.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <lxgui/extern_sol2_state.hpp>
#include <sstream>

namespace lxgui::gui {

region::region(utils::control_block& block, manager& mgr) :
    utils::enable_observer_from_this<region>(block), manager_(mgr) {
    type_.push_back(class_name);
}

region::~region() {
    if (!is_virtual_) {
        // Tell this region's anchor parents that it is no longer anchored to them
        for (auto& a : anchor_list_) {
            if (a) {
                if (auto* anchor_parent = a->get_parent().get())
                    anchor_parent->remove_anchored_object(*this);
            }

            a.reset();
        }

        // Replace anchors pointing to this region by absolute anchors
        // (need to copy the anchored object list, because the objects will attempt to
        // modify it when un-anchored, which would invalidate our iteration)
        std::vector<utils::observer_ptr<region>> temp_anchored_object_list =
            std::move(anchored_object_list_);
        for (const auto& obj : temp_anchored_object_list) {
            if (!obj)
                continue;

            std::vector<point> anchored_point_list;
            for (const auto& a : obj->get_point_list()) {
                if (a && a->get_parent().get() == this)
                    anchored_point_list.push_back(a->object_point);
            }

            for (const auto& p : anchored_point_list) {
                const anchor& a          = obj->get_point(p);
                anchor_data   new_anchor = anchor_data(p, "", point::top_left);
                new_anchor.offset        = a.offset;

                switch (a.parent_point) {
                case point::top_left: new_anchor.offset += border_list_.top_left(); break;
                case point::top: new_anchor.offset.y += border_list_.top; break;
                case point::top_right: new_anchor.offset += border_list_.top_right(); break;
                case point::right: new_anchor.offset.x += border_list_.right; break;
                case point::bottom_right: new_anchor.offset += border_list_.bottom_right(); break;
                case point::bottom: new_anchor.offset.y += border_list_.bottom; break;
                case point::bottom_left: new_anchor.offset += border_list_.bottom_left(); break;
                case point::left: new_anchor.offset.x += border_list_.left; break;
                case point::center: new_anchor.offset += border_list_.center(); break;
                }

                obj->set_point(new_anchor);
            }

            obj->update_anchors_();
        }

        remove_glue();
    }

    // Unregister this object from the GUI manager
    if (!is_virtual() || parent_ == nullptr)
        get_registry().remove_region(*this);
}

std::string region::serialize(const std::string& tab) const {
    std::ostringstream str;

    str << tab << "  # Name       : " << name_
        << " (" + std::string(is_ready_ ? "ready" : "not ready") +
               std::string(is_special_ ? ", special)\n" : ")\n");
    str << tab << "  # Raw name   : " << raw_name_ << "\n";
    str << tab << "  # Lua name   : " << lua_name_ << "\n";
    str << tab << "  # Type       : " << type_.back() << "\n";
    if (parent_)
        str << tab << "  # Parent     : " << parent_->get_name() << "\n";
    else
        str << tab << "  # Parent     : none\n";
    str << tab << "  # Num anchors: " << get_num_point() << "\n";
    if (!anchor_list_.empty()) {
        str << tab << "  |-###\n";
        for (const auto& a : anchor_list_) {
            if (a) {
                str << a->serialize(tab);
                str << tab << "  |-###\n";
            }
        }
    }
    str << tab << "  # Borders :\n";
    str << tab << "  |-###\n";
    str << tab << "  |   # left  : " << border_list_.left << "\n";
    str << tab << "  |   # top   : " << border_list_.top << "\n";
    str << tab << "  |   # right : " << border_list_.right << "\n";
    str << tab << "  |   # bottom: " << border_list_.bottom << "\n";
    str << tab << "  |-###\n";
    str << tab << "  # Alpha      : " << alpha_ << "\n";
    str << tab << "  # Shown      : " << is_shown_ << "\n";
    str << tab << "  # Abs width  : " << dimensions_.x << "\n";
    str << tab << "  # Abs height : " << dimensions_.y << "\n";

    return str.str();
}

void region::copy_from(const region& obj) {
    this->set_alpha(obj.get_alpha());
    this->set_shown(obj.is_shown());
    this->set_dimensions(obj.get_dimensions());

    for (const std::optional<anchor>& a : obj.get_point_list()) {
        if (a) {
            this->set_point(a->get_data());
        }
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
    return alpha_;
}

float region::get_effective_alpha() const {
    if (parent_) {
        return parent_->get_effective_alpha() * get_alpha();
    } else {
        return get_alpha();
    }
}

void region::set_alpha(float alpha) {
    if (alpha_ != alpha) {
        alpha_ = alpha;
        notify_renderer_need_redraw();
    }
}

void region::show() {
    if (is_shown_)
        return;

    is_shown_ = true;

    if (!is_visible_ && (!parent_ || parent_->is_visible()))
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

void region::set_dimensions(const vector2f& dimensions) {
    if (dimensions_ == dimensions)
        return;

    dimensions_ = dimensions;

    if (!is_virtual_) {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void region::set_width(float abs_width) {
    if (dimensions_.x == abs_width)
        return;

    dimensions_.x = abs_width;

    if (!is_virtual_) {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void region::set_height(float abs_height) {
    if (dimensions_.y == abs_height)
        return;

    dimensions_.y = abs_height;

    if (!is_virtual_) {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void region::set_relative_dimensions(const vector2f& dimensions) {
    if (parent_)
        set_dimensions(dimensions * parent_->get_apparent_dimensions());
    else
        set_dimensions(dimensions * get_top_level_renderer()->get_target_dimensions());
}

void region::set_relative_width(float rel_width) {
    if (parent_)
        set_width(rel_width * parent_->get_apparent_dimensions().x);
    else
        set_width(rel_width * get_top_level_renderer()->get_target_dimensions().x);
}

void region::set_relative_height(float rel_height) {
    if (parent_)
        set_height(rel_height * parent_->get_apparent_dimensions().y);
    else
        set_height(rel_height * get_top_level_renderer()->get_target_dimensions().y);
}

const vector2f& region::get_dimensions() const {
    return dimensions_;
}

vector2f region::get_apparent_dimensions() const {
    return vector2f(border_list_.width(), border_list_.height());
}

bool region::is_apparent_width_defined() const {
    return dimensions_.x > 0.0f || (defined_border_list_.left && defined_border_list_.right);
}

bool region::is_apparent_height_defined() const {
    return dimensions_.y > 0.0f || (defined_border_list_.top && defined_border_list_.bottom);
}

bool region::is_in_region(const vector2f& position) const {
    return (
        (border_list_.left <= position.x && position.x <= border_list_.right - 1) &&
        (border_list_.top <= position.y && position.y <= border_list_.bottom - 1));
}

void region::set_name_(const std::string& name) {
    if (name_.empty()) {
        name_ = lua_name_ = raw_name_ = name;
        if (utils::starts_with(name_, "$parent")) {
            if (parent_)
                utils::replace(lua_name_, "$parent", parent_->get_lua_name());
            else {
                gui::out << gui::warning << "gui::" << type_.back() << ": \"" << name_
                         << "\" has no parent" << std::endl;
                utils::replace(lua_name_, "$parent", "");
            }
        }

        if (!is_virtual_)
            name_ = lua_name_;
    } else {
        gui::out << gui::warning << "gui::" << type_.back() << ": "
                 << "set_name() can only be called once." << std::endl;
    }
}

void region::set_parent_(utils::observer_ptr<frame> parent) {
    if (parent == observer_from_this()) {
        gui::out << gui::error << "gui::" << type_.back() << ": Cannot call set_parent(this)."
                 << std::endl;
        return;
    }

    if (parent_ != parent) {
        parent_ = std::move(parent);

        if (!is_virtual_)
            notify_borders_need_update();
    }
}

void region::set_name_and_parent_(const std::string& name, utils::observer_ptr<frame> parent) {
    if (parent == observer_from_this()) {
        gui::out << gui::error << "gui::" << type_.back() << ": Cannot call set_parent(this)."
                 << std::endl;
        return;
    }

    if (parent_ == parent && name == name_)
        return;

    parent_ = std::move(parent);
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
    for (auto& a : anchor_list_) {
        if (a) {
            a.reset();
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
        gui::out << gui::error << "gui::" << type_.back() << ": Cannot call set_all_points(this)."
                 << std::endl;
        return;
    }

    clear_all_points();

    anchor_list_[static_cast<int>(point::top_left)].emplace(
        *this, anchor_data(point::top_left, obj_name));

    anchor_list_[static_cast<int>(point::bottom_right)].emplace(
        *this, anchor_data(point::bottom_right, obj_name));

    defined_border_list_ = bounds2<bool>(true, true, true, true);

    if (!is_virtual_) {
        update_anchors_();
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void region::set_all_points(const utils::observer_ptr<region>& obj) {
    if (obj == observer_from_this()) {
        gui::out << gui::error << "gui::" << type_.back() << ": Cannot call set_all_points(this)."
                 << std::endl;
        return;
    }

    set_all_points(obj ? obj->get_name() : "");
}

void region::set_point(const anchor_data& a) {
    anchor_list_[static_cast<int>(a.object_point)].emplace(*this, a);

    switch (a.object_point) {
    case point::top_left:
        defined_border_list_.top  = true;
        defined_border_list_.left = true;
        break;
    case point::top: defined_border_list_.top = true; break;
    case point::top_right:
        defined_border_list_.top   = true;
        defined_border_list_.right = true;
        break;
    case point::right: defined_border_list_.right = true; break;
    case point::bottom_right:
        defined_border_list_.bottom = true;
        defined_border_list_.right  = true;
        break;
    case point::bottom: defined_border_list_.bottom = true; break;
    case point::bottom_left:
        defined_border_list_.bottom = true;
        defined_border_list_.left   = true;
        break;
    case point::left: defined_border_list_.left = true; break;
    default: break;
    }

    if (!is_virtual_) {
        update_anchors_();
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

bool region::depends_on(const region& obj) const {
    for (const auto& a : anchor_list_) {
        if (!a)
            continue;

        const region* parent = a->get_parent().get();
        if (parent == &obj)
            return true;

        if (parent)
            return parent->depends_on(obj);
    }

    return false;
}

std::size_t region::get_num_point() const {
    std::size_t num_anchors = 0u;
    for (const auto& a : anchor_list_) {
        if (a)
            ++num_anchors;
    }

    return num_anchors;
}

anchor& region::modify_point(point p) {
    auto& a = anchor_list_[static_cast<int>(p)];
    if (!a) {
        throw gui::exception(
            "region", "Cannot modify a point that does not exist. Use set_point() first.");
    }

    return *a;
}

const anchor& region::get_point(point p) const {
    const auto& a = anchor_list_[static_cast<int>(p)];
    if (!a) {
        throw gui::exception(
            "region", "Cannot get a point that does not exist. Use set_point() first.");
    }

    return *a;
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

void region::add_anchored_object(region& obj) {
    anchored_object_list_.push_back(observer_from(&obj));
}

void region::remove_anchored_object(region& obj) {
    auto iter =
        utils::find_if(anchored_object_list_, [&](const auto& ptr) { return ptr.get() == &obj; });

    if (iter != anchored_object_list_.end())
        anchored_object_list_.erase(iter);
}

float region::round_to_pixel(float value, utils::rounding_method method) const {
    float scaling_factor = get_manager().get_interface_scaling_factor();
    return utils::round(value, 1.0f / scaling_factor, method);
}

vector2f region::round_to_pixel(const vector2f& position, utils::rounding_method method) const {
    float scaling_factor = get_manager().get_interface_scaling_factor();
    return vector2f(
        utils::round(position.x, 1.0f / scaling_factor, method),
        utils::round(position.y, 1.0f / scaling_factor, method));
}

bool region::make_borders_(float& min, float& max, float center, float size) const {
    if (std::isinf(min) && std::isinf(max)) {
        if (!std::isinf(size) && size > 0.0f && !std::isinf(center)) {
            min = center - size / 2.0f;
            max = center + size / 2.0f;
        } else
            return false;
    } else if (std::isinf(max)) {
        if (!std::isinf(size) && size > 0.0f)
            max = min + size;
        else if (!std::isinf(center))
            max = min + 2.0f * (center - min);
        else
            return false;
    } else if (std::isinf(min)) {
        if (!std::isinf(size) && size > 0.0f)
            min = max - size;
        else if (!std::isinf(center))
            min = max - 2.0f * (max - center);
        else
            return false;
    }

    return true;
}

void region::read_anchors_(
    float& left, float& right, float& top, float& bottom, float& x_center, float& y_center) const {
    left   = +std::numeric_limits<float>::infinity();
    right  = -std::numeric_limits<float>::infinity();
    top    = +std::numeric_limits<float>::infinity();
    bottom = -std::numeric_limits<float>::infinity();

    for (const auto& opt_anchor : anchor_list_) {
        if (!opt_anchor)
            continue;

        const anchor&  a = opt_anchor.value();
        const vector2f p = a.get_point(*this);

        switch (a.object_point) {
        case point::top_left:
            top  = std::min<float>(top, p.y);
            left = std::min<float>(left, p.x);
            break;
        case point::top:
            top      = std::min<float>(top, p.y);
            x_center = p.x;
            break;
        case point::top_right:
            top   = std::min<float>(top, p.y);
            right = std::max<float>(right, p.x);
            break;
        case point::right:
            right    = std::max<float>(right, p.x);
            y_center = p.y;
            break;
        case point::bottom_right:
            bottom = std::max<float>(bottom, p.y);
            right  = std::max<float>(right, p.x);
            break;
        case point::bottom:
            bottom   = std::max<float>(bottom, p.y);
            x_center = p.x;
            break;
        case point::bottom_left:
            bottom = std::max<float>(bottom, p.y);
            left   = std::min<float>(left, p.x);
            break;
        case point::left:
            left     = std::min<float>(left, p.x);
            y_center = p.y;
            break;
        case point::center:
            x_center = p.x;
            y_center = p.y;
            break;
        }
    }
}

void region::update_borders_() {
// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

    DEBUG_LOG("  Update anchors for " + lua_name_);

    const bool old_is_ready    = is_ready_;
    const auto old_border_list = border_list_;

    is_ready_ = true;

    if (!anchor_list_.empty()) {
        float left = 0.0f, right = 0.0f, top = 0.0f, bottom = 0.0f;
        float x_center = 0.0f, y_center = 0.0f;

        float rounded_width =
            round_to_pixel(dimensions_.x, utils::rounding_method::nearest_not_zero);
        float rounded_height =
            round_to_pixel(dimensions_.y, utils::rounding_method::nearest_not_zero);

        DEBUG_LOG("  Read anchors");
        read_anchors_(left, right, top, bottom, x_center, y_center);
        DEBUG_LOG("    left=" + utils::to_string(left));
        DEBUG_LOG("    right=" + utils::to_string(right));
        DEBUG_LOG("    top=" + utils::to_string(top));
        DEBUG_LOG("    bottom=" + utils::to_string(bottom));
        DEBUG_LOG("    x_center=" + utils::to_string(x_center));
        DEBUG_LOG("    y_center=" + utils::to_string(y_center));

        DEBUG_LOG("  Make borders");
        if (!make_borders_(top, bottom, y_center, rounded_height))
            is_ready_ = false;
        if (!make_borders_(left, right, x_center, rounded_width))
            is_ready_ = false;

        if (is_ready_) {
            if (right < left)
                right = left + 1;
            if (bottom < top)
                bottom = top + 1;

            border_list_ = bounds2f(left, right, top, bottom);
        } else
            border_list_ = bounds2f::zero;
    } else {
        border_list_ = bounds2f(0.0, 0.0, dimensions_.x, dimensions_.y);
        is_ready_    = false;
    }

    DEBUG_LOG("  Final borders");
    border_list_.left   = round_to_pixel(border_list_.left);
    border_list_.right  = round_to_pixel(border_list_.right);
    border_list_.top    = round_to_pixel(border_list_.top);
    border_list_.bottom = round_to_pixel(border_list_.bottom);

    DEBUG_LOG("    left=" + utils::to_string(border_list_.left));
    DEBUG_LOG("    right=" + utils::to_string(border_list_.right));
    DEBUG_LOG("    top=" + utils::to_string(border_list_.top));
    DEBUG_LOG("    bottom=" + utils::to_string(border_list_.bottom));

    if (border_list_ != old_border_list || is_ready_ != old_is_ready) {
        DEBUG_LOG("  Fire redraw");
        notify_renderer_need_redraw();
    }

    DEBUG_LOG("  @");
#undef DEBUG_LOG
}

void region::update_anchors_() {
    std::vector<utils::observer_ptr<region>> anchor_parent_list;
    for (auto& a : anchor_list_) {
        if (!a)
            continue;

        utils::observer_ptr<region> obj = a->get_parent();
        if (obj) {
            if (obj->depends_on(*this)) {
                gui::out << gui::error << "gui::" << type_.back() << ": Cyclic anchor dependency ! "
                         << "\"" << name_ << "\" and \"" << obj->get_name()
                         << "\" depend on eachothers (directly or indirectly). \""
                         << utils::to_string(a->object_point) << "\" anchor removed." << std::endl;

                a.reset();
                continue;
            }

            if (utils::find(anchor_parent_list, obj) == anchor_parent_list.end())
                anchor_parent_list.push_back(obj);
        }
    }

    for (const auto& parent : previous_anchor_parent_list_) {
        if (utils::find(anchor_parent_list, parent) == anchor_parent_list.end())
            parent->remove_anchored_object(*this);
    }

    for (const auto& parent : anchor_parent_list) {
        if (utils::find(previous_anchor_parent_list_, parent) == previous_anchor_parent_list_.end())
            parent->add_anchored_object(*this);
    }

    previous_anchor_parent_list_ = std::move(anchor_parent_list);
}

void region::notify_borders_need_update() {
    if (is_virtual())
        return;

    update_borders_();

    for (const auto& object : anchored_object_list_)
        object->notify_borders_need_update();
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
    if (!parent_)
        return get_manager().get_root().observer_from_this();
    return parent_->get_top_level_renderer();
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

    const addon* addon = get_addon();
    if (new_file[0] == '|' && addon) {
        new_file[0] = '/';
        new_file    = addon->directory + new_file;
    }

    return new_file;
}

void region::set_addon(const addon* a) {
    if (addon_) {
        gui::out << gui::warning << "gui::" << type_.back()
                 << ": set_addon() can only be called once." << std::endl;
        return;
    }

    addon_ = a;
}

const addon* region::get_addon() const {
    if (!addon_ && parent_)
        return parent_->get_addon();
    else
        return addon_;
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