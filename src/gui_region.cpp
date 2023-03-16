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

region::region(utils::control_block& block, manager& mgr, const region_core_attributes& attr) :
    utils::enable_observer_from_this<region>(block), manager_(mgr) {

    if (attr.is_virtual)
        set_virtual_();

    if (attr.parent)
        set_parent_(attr.parent);

    set_name_(attr.name);

    initialize_(*this, attr);
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
            for (const auto& a : obj->get_anchors()) {
                if (a && a->get_parent().get() == this)
                    anchored_point_list.push_back(a->object_point);
            }

            for (const auto& p : anchored_point_list) {
                const anchor& a          = obj->get_anchor(p);
                anchor_data   new_anchor = anchor_data(p, "", point::top_left);
                new_anchor.offset        = a.offset;

                switch (a.parent_point) {
                case point::top_left: new_anchor.offset += borders_.top_left(); break;
                case point::top: new_anchor.offset.y += borders_.top; break;
                case point::top_right: new_anchor.offset += borders_.top_right(); break;
                case point::right: new_anchor.offset.x += borders_.right; break;
                case point::bottom_right: new_anchor.offset += borders_.bottom_right(); break;
                case point::bottom: new_anchor.offset.y += borders_.bottom; break;
                case point::bottom_left: new_anchor.offset += borders_.bottom_left(); break;
                case point::left: new_anchor.offset.x += borders_.left; break;
                case point::center: new_anchor.offset += borders_.center(); break;
                }

                obj->set_anchor(new_anchor);
            }
        }

        remove_glue();
    }

    // Unregister this object from the GUI manager
    if (!is_virtual() || parent_ == nullptr)
        get_registry().remove_region(*this);
}

std::string region::serialize(const std::string& tab) const {
    std::ostringstream str;

    str << tab << "  # Name       : " << name_ << " ("
        << std::string(is_valid_ ? "valid" : "not valid")
        << std::string(is_manually_inherited_ ? ", manually inherited" : "") << ")\n";
    str << tab << "  # Raw name   : " << raw_name_ << "\n";
    str << tab << "  # Type       : " << get_region_type() << "\n";
    if (parent_)
        str << tab << "  # Parent     : " << parent_->get_name() << "\n";
    else
        str << tab << "  # Parent     : none\n";
    str << tab << "  # Num anchors: " << get_anchor_count() << "\n";
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
    str << tab << "  |   # left  : " << borders_.left << "\n";
    str << tab << "  |   # top   : " << borders_.top << "\n";
    str << tab << "  |   # right : " << borders_.right << "\n";
    str << tab << "  |   # bottom: " << borders_.bottom << "\n";
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

    for (const std::optional<anchor>& a : obj.get_anchors()) {
        if (a) {
            this->set_anchor(a->get_data());
        }
    }
}

const std::string& region::get_name() const {
    return name_;
}

const std::string& region::get_raw_name() const {
    return raw_name_;
}

const std::string& region::get_region_type() const {
    return get_type_list_().back();
}

bool region::is_region_type(const std::string& type_name) const {
    const auto& type_list = get_type_list_();
    return utils::find(type_list, type_name) != type_list.end();
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

bool region::is_valid() const {
    return is_valid_;
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
        set_dimensions(dimensions * get_effective_frame_renderer()->get_target_dimensions());
}

void region::set_relative_width(float rel_width) {
    if (parent_)
        set_width(rel_width * parent_->get_apparent_dimensions().x);
    else
        set_width(rel_width * get_effective_frame_renderer()->get_target_dimensions().x);
}

void region::set_relative_height(float rel_height) {
    if (parent_)
        set_height(rel_height * parent_->get_apparent_dimensions().y);
    else
        set_height(rel_height * get_effective_frame_renderer()->get_target_dimensions().y);
}

const vector2f& region::get_dimensions() const {
    return dimensions_;
}

vector2f region::get_apparent_dimensions() const {
    return vector2f(borders_.width(), borders_.height());
}

bool region::is_apparent_width_defined() const {
    return dimensions_.x > 0.0f || (defined_borders_.left && defined_borders_.right);
}

bool region::is_apparent_height_defined() const {
    return dimensions_.y > 0.0f || (defined_borders_.top && defined_borders_.bottom);
}

bool region::is_in_region(const vector2f& position) const {
    return (
        (borders_.left <= position.x && position.x <= borders_.right - 1) &&
        (borders_.top <= position.y && position.y <= borders_.bottom - 1));
}

void region::set_name_(const std::string& name) {
    if (name_.empty()) {
        name_ = raw_name_ = name;
        if (utils::starts_with(name_, "$parent")) {
            if (parent_)
                utils::replace(name_, "$parent", parent_->get_name());
            else {
                gui::out << gui::warning << "gui::" << get_region_type() << ": \"" << name_
                         << "\" has no parent" << std::endl;
                utils::replace(name_, "$parent", "");
            }
        }
    } else {
        gui::out << gui::warning << "gui::" << get_region_type() << ": "
                 << "set_name() can only be called once." << std::endl;
    }
}

void region::set_parent_(utils::observer_ptr<frame> parent) {
    if (parent == observer_from_this()) {
        gui::out << gui::error << "gui::" << get_region_type() << ": Cannot call set_parent(this)."
                 << std::endl;
        return;
    }

    if (parent_ == parent)
        return;

    parent_ = std::move(parent);

    if (!is_virtual()) {
        notify_borders_need_update();
    }
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
    return borders_.center();
}

float region::get_left() const {
    return borders_.left;
}

float region::get_right() const {
    return borders_.right;
}

float region::get_top() const {
    return borders_.top;
}

float region::get_bottom() const {
    return borders_.bottom;
}

const bounds2f& region::get_borders() const {
    return borders_;
}

void region::clear_all_anchors() {
    bool had_anchors = false;
    for (auto& a : anchor_list_) {
        if (a) {
            if (!is_virtual_) {
                if (auto parent = a->get_parent()) {
                    parent->remove_anchored_object(*this);
                }
            }

            a.reset();
            had_anchors = true;
        }
    }

    defined_borders_ = bounds2<bool>(false, false, false, false);

    if (had_anchors || !is_virtual_) {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

bool check_cyclic_anchors(const region& self, anchor& a) {
    if (utils::observer_ptr<region> other = a.get_parent()) {
        if (other->depends_on(self)) {
            gui::out << gui::error << "gui::" << self.get_region_type()
                     << ": Cyclic anchor dependency ! "
                     << "\"" << self.get_name() << "\" and \"" << other->get_name()
                     << "\" depend on eachothers (directly or indirectly). \""
                     << utils::to_string(a.object_point) << "\" anchor removed." << std::endl;

            return false;
        }
    }

    return true;
}

void region::set_all_anchors(const std::string& obj_name) {
    if (obj_name == name_) {
        gui::out << gui::error << "gui::" << get_region_type()
                 << ": Cannot call set_all_anchors(this)." << std::endl;
        return;
    }

    clear_all_anchors();

    auto& top_left     = anchor_list_[static_cast<int>(point::top_left)];
    auto& bottom_right = anchor_list_[static_cast<int>(point::bottom_right)];

    top_left.emplace(*this, anchor_data(point::top_left, obj_name));

    if (!check_cyclic_anchors(*this, top_left.value())) {
        top_left = std::nullopt;
        return;
    }

    bottom_right.emplace(*this, anchor_data(point::bottom_right, obj_name));

    defined_borders_ = bounds2<bool>(true, true, true, true);

    if (!is_virtual_) {
        if (auto* parent = top_left->get_parent().get()) {
            parent->add_anchored_object(*this);
        }

        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void region::set_all_anchors(const utils::observer_ptr<region>& obj) {
    if (obj == observer_from_this()) {
        gui::out << gui::error << "gui::" << get_region_type()
                 << ": Cannot call set_all_anchors(this)." << std::endl;
        return;
    }

    set_all_anchors(obj ? obj->get_name() : "");
}

void set_defined_borders(bounds2<bool>& defined_borders, point p, bool defined) {
    switch (p) {
    case point::top_left:
        defined_borders.top  = defined;
        defined_borders.left = defined;
        break;
    case point::top: defined_borders.top = defined; break;
    case point::top_right:
        defined_borders.top   = defined;
        defined_borders.right = defined;
        break;
    case point::right: defined_borders.right = defined; break;
    case point::bottom_right:
        defined_borders.bottom = defined;
        defined_borders.right  = defined;
        break;
    case point::bottom: defined_borders.bottom = defined; break;
    case point::bottom_left:
        defined_borders.bottom = defined;
        defined_borders.left   = defined;
        break;
    case point::left: defined_borders.left = defined; break;
    default: break;
    }
}

void region::set_anchor(const anchor_data& a) {
    auto& modified_anchor = anchor_list_[static_cast<int>(a.object_point)];
    auto  previous_parent = modified_anchor.has_value() ? modified_anchor->get_parent() : nullptr;

    modified_anchor.emplace(*this, a);

    set_defined_borders(defined_borders_, a.object_point, true);

    auto new_parent = modified_anchor->get_parent();
    if (new_parent != previous_parent) {
        if (previous_parent) {
            previous_parent->remove_anchored_object(*this);
        }

        if (new_parent) {
            if (!check_cyclic_anchors(*this, modified_anchor.value())) {
                modified_anchor.reset();
                set_defined_borders(defined_borders_, a.object_point, false);
            } else {
                new_parent->add_anchored_object(*this);
            }
        }
    }

    if (!is_virtual_) {
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

std::size_t region::get_anchor_count() const {
    std::size_t num_anchors = 0u;
    for (const auto& a : anchor_list_) {
        if (a)
            ++num_anchors;
    }

    return num_anchors;
}

anchor& region::modify_anchor(point p) {
    auto& a = anchor_list_[static_cast<int>(p)];
    if (!a) {
        throw gui::exception(
            "region", "Cannot modify a point that does not exist. Use set_anchor() first.");
    }

    return *a;
}

const anchor& region::get_anchor(point p) const {
    const auto& a = anchor_list_[static_cast<int>(p)];
    if (!a) {
        throw gui::exception(
            "region", "Cannot get an anchor that does not exist. Use set_anchor() first.");
    }

    return *a;
}

const std::array<std::optional<anchor>, 9>& region::get_anchors() const {
    return anchor_list_;
}

bool region::is_virtual() const {
    return is_virtual_;
}

void region::set_virtual_() {
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
        } else {
            return false;
        }
    } else if (std::isinf(max)) {
        if (!std::isinf(size) && size > 0.0f) {
            max = min + size;
        } else if (!std::isinf(center)) {
            max = min + 2.0f * (center - min);
        } else {
            return false;
        }
    } else if (std::isinf(min)) {
        if (!std::isinf(size) && size > 0.0f) {
            min = max - size;
        } else if (!std::isinf(center)) {
            min = max - 2.0f * (max - center);
        } else {
            return false;
        }
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

    const bool old_is_ready    = is_valid_;
    const auto old_border_list = borders_;

    is_valid_ = true;

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
        if (!make_borders_(top, bottom, y_center, rounded_height)) {
            is_valid_ = false;
        }

        if (!make_borders_(left, right, x_center, rounded_width)) {
            is_valid_ = false;
        }

        if (is_valid_) {
            if (right < left) {
                right = left + 1;
            }
            if (bottom < top) {
                bottom = top + 1;
            }

            borders_ = bounds2f(left, right, top, bottom);
        } else {
            borders_ = bounds2f::zero;
        }
    } else {
        borders_  = bounds2f(0.0, 0.0, dimensions_.x, dimensions_.y);
        is_valid_ = false;
    }

    DEBUG_LOG("  Final borders");
    borders_.left   = round_to_pixel(borders_.left);
    borders_.right  = round_to_pixel(borders_.right);
    borders_.top    = round_to_pixel(borders_.top);
    borders_.bottom = round_to_pixel(borders_.bottom);

    DEBUG_LOG("    left=" + utils::to_string(borders_.left));
    DEBUG_LOG("    right=" + utils::to_string(borders_.right));
    DEBUG_LOG("    top=" + utils::to_string(borders_.top));
    DEBUG_LOG("    bottom=" + utils::to_string(borders_.bottom));

    if (borders_ != old_border_list || is_valid_ != old_is_ready) {
        DEBUG_LOG("  Fire redraw");
        notify_renderer_need_redraw();
    }

    DEBUG_LOG("  @");
#undef DEBUG_LOG
}

void region::notify_borders_need_update() {
    if (is_virtual())
        return;

    const bool old_valid       = is_valid_;
    const auto old_border_list = borders_;

    update_borders_();

    if (borders_ != old_border_list || is_valid_ != old_valid) {
        for (const auto& object : anchored_object_list_)
            object->notify_borders_need_update();
    }
}

void region::notify_scaling_factor_updated() {
    notify_borders_need_update();
}

void region::update(float) {}

void region::render() const {}

sol::state& region::get_lua_() {
    return get_manager().get_lua();
}

const sol::state& region::get_lua_() const {
    return get_manager().get_lua();
}

void region::remove_glue() {
    get_lua_().globals()[get_name()]              = sol::lua_nil;
    get_lua_().globals()["_METADATA"][get_name()] = sol::lua_nil;
}

void region::set_manually_inherited(bool manually_inherited) {
    is_manually_inherited_ = manually_inherited;
}

bool region::is_manually_inherited() const {
    return is_manually_inherited_;
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

utils::observer_ptr<const frame_renderer> region::get_effective_frame_renderer() const {
    if (!parent_)
        return get_manager().get_root().observer_from_this();
    return parent_->get_effective_frame_renderer();
}

void region::notify_visible() {
    is_visible_ = true;
    notify_renderer_need_redraw();
}

void region::notify_invisible() {
    is_visible_ = false;
    notify_renderer_need_redraw();
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
        gui::out << gui::warning << "gui::" << get_region_type()
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

const std::vector<std::string>& region::get_type_list_() const {
    return get_type_list_impl_<region>();
}

} // namespace lxgui::gui
