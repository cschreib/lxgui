#include "lxgui/gui_frame.hpp"

#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_event_emitter.hpp"
#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_frame_renderer.hpp"
#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/utils_range.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <functional>
#include <lxgui/extern_sol2_as_args.hpp>
#include <lxgui/extern_sol2_state.hpp>
#include <lxgui/extern_sol2_variadic_args.hpp>
#include <sstream>

namespace lxgui::gui {

frame::frame(utils::control_block& block, manager& mgr, const frame_core_attributes& attr) :
    base(block, mgr, attr), event_receiver_(mgr.get_event_emitter()), frame_renderer_(attr.rdr) {
    type_.push_back(class_name);

    initialize_(*this, attr);

    if (parent_) {
        level_ = parent_->get_level() + 1;
    }

    effective_frame_renderer_ = compute_top_level_frame_renderer_();
    effective_strata_         = compute_effective_frame_strata_();

    if (!is_virtual_) {
        // Tell the renderer to render this region
        get_effective_frame_renderer()->notify_rendered_frame(observer_from(this), true);
    }
}

frame::~frame() {
    // Disable callbacks
    signal_list_.clear();

    // Children must be destroyed first
    child_list_.clear();
    region_list_.clear();
    title_region_ = nullptr;

    if (!is_virtual_) {
        // Tell the renderer to no longer render this region
        get_effective_frame_renderer()->notify_rendered_frame(observer_from(this), false);
    }

    get_manager().get_root().notify_hovered_frame_dirty();

    set_focus(false);
}

void frame::render() const {
    base::render();

    if (!is_visible() || !is_ready_)
        return;

    if (backdrop_) {
        backdrop_->render();
    }

    // Render child regions
    for (const auto& layer : layer_list_) {
        if (layer.is_disabled) {
            continue;
        }

        for (const auto& reg : layer.region_list) {
            reg->render();
        }
    }
}

std::string frame::serialize(const std::string& tab) const {
    std::ostringstream str;

    str << base::serialize(tab);
    if (auto frame_renderer = utils::dynamic_pointer_cast<frame>(effective_frame_renderer_))
        str << tab << "  # Man. render: " << frame_renderer->get_name() << "\n";
    str << tab << "  # Strata     : "
        << (strata_.has_value() ? utils::to_string(strata_.value()) : "parent") << "\n";
    str << tab << "  # Level      : " << level_ << "\n";
    str << tab << "  # TopLevel   : " << is_top_level_;
    if (!is_top_level_ && get_top_level_parent())
        str << " (" << get_top_level_parent()->get_name() << ")\n";
    else
        str << "\n";
    if (!is_mouse_click_enabled_ && !is_mouse_move_enabled_ && !!is_mouse_wheel_enabled_)
        str << tab << "  # Inputs     : none\n";
    else {
        str << tab << "  # Inputs      :\n";
        str << tab << "  |-###\n";
        if (is_mouse_click_enabled_)
            str << tab << "  |   # mouse click\n";
        if (is_mouse_move_enabled_)
            str << tab << "  |   # mouse move\n";
        if (is_mouse_wheel_enabled_)
            str << tab << "  |   # mouse wheel\n";
        str << tab << "  |-###\n";
    }
    str << tab << "  # Movable    : " << is_movable_ << "\n";
    str << tab << "  # Resizable  : " << is_resizable_ << "\n";
    str << tab << "  # Clamped    : " << is_clamped_to_screen_ << "\n";
    str << tab << "  # HRect inset :\n";
    str << tab << "  |-###\n";
    str << tab << "  |   # left  : " << abs_hit_rect_inset_list_.left << "\n";
    str << tab << "  |   # right : " << abs_hit_rect_inset_list_.right << "\n";
    str << tab << "  |   # top   : " << abs_hit_rect_inset_list_.top << "\n";
    str << tab << "  |   # bottom: " << abs_hit_rect_inset_list_.bottom << "\n";
    str << tab << "  |-###\n";
    str << tab << "  # Min width  : " << min_width_ << "\n";
    str << tab << "  # Max width  : " << max_width_ << "\n";
    str << tab << "  # Min height : " << min_height_ << "\n";
    str << tab << "  # Max height : " << max_height_ << "\n";
    str << tab << "  # Scale      : " << scale_ << "\n";
    if (title_region_) {
        str << tab << "  # Title reg.  :\n";
        str << tab << "  |-###\n";
        str << title_region_->serialize(tab + "  | ");
        str << tab << "  |-###\n";
    }
    if (backdrop_) {
        const bounds2f& insets = backdrop_->get_background_insets();

        str << tab << "  # Backdrop    :\n";
        str << tab << "  |-###\n";
        str << tab << "  |   # Background: " << backdrop_->get_background_file() << "\n";
        str << tab << "  |   # Tilling   : " << backdrop_->is_background_tilling() << "\n";
        if (backdrop_->is_background_tilling())
            str << tab << "  |   # Tile size : " << backdrop_->get_tile_size() << "\n";
        str << tab << "  |   # BG Insets  :\n";
        str << tab << "  |   |-###\n";
        str << tab << "  |   |   # left  : " << insets.left << "\n";
        str << tab << "  |   |   # right : " << insets.right << "\n";
        str << tab << "  |   |   # top   : " << insets.top << "\n";
        str << tab << "  |   |   # bottom: " << insets.bottom << "\n";
        str << tab << "  |   |-###\n";
        str << tab << "  |   # Edge      : " << backdrop_->get_edge_file() << "\n";
        str << tab << "  |   # Edge size : " << backdrop_->get_edge_size() << "\n";
        str << tab << "  |-###\n";
    }

    if (!region_list_.empty()) {
        if (child_list_.size() == 1)
            str << tab << "  # Region: \n";
        else
            str << tab << "  # Regions    : " << region_list_.size() << "\n";
        str << tab << "  |-###\n";

        for (auto& obj : get_regions()) {
            str << obj.serialize(tab + "  | ");
            str << tab << "  |-###\n";
        }
    }

    if (!child_list_.empty()) {
        if (child_list_.size() == 1)
            str << tab << "  # Child: \n";
        else
            str << tab << "  # Children   : " << child_list_.size() << "\n";
        str << tab << "  |-###\n";

        for (const auto& child : get_children()) {
            str << child.serialize(tab + "  | ");
            str << tab << "  |-###\n";
        }
    }

    return str.str();
}

bool frame::can_use_script(const std::string& script_name) const {
    return script_name == "OnChar" || script_name == "OnDragStart" || script_name == "OnDragStop" ||
           script_name == "OnDragMove" || script_name == "OnEnter" || script_name == "OnEvent" ||
           script_name == "OnFocusGained" || script_name == "OnFocusLost" ||
           script_name == "OnHide" || script_name == "OnKeyDown" || script_name == "OnKeyUp" ||
           script_name == "OnKeyRepeat" || script_name == "OnLeave" || script_name == "OnLoad" ||
           script_name == "OnMouseDown" || script_name == "OnMouseUp" ||
           script_name == "OnMouseMove" || script_name == "OnDoubleClick" ||
           script_name == "OnMouseWheel" || script_name == "OnReceiveDrag" ||
           script_name == "OnShow" || script_name == "OnSizeChanged" || script_name == "OnUpdate";
}

void frame::copy_from(const region& obj) {
    base::copy_from(obj);

    const frame* frame_obj = down_cast<frame>(&obj);
    if (!frame_obj)
        return;

    for (const auto& item : frame_obj->signal_list_) {
        for (const auto& function : frame_obj->get_script(item.first)) {
            this->add_script(item.first, function);
        }
    }

    this->set_frame_strata(frame_obj->get_frame_strata());
    // NB: level is not inherited on purpose; this is difficult to make sense of
    this->set_top_level(frame_obj->is_top_level());

    this->enable_mouse_click(frame_obj->is_mouse_click_enabled());
    this->enable_mouse_move(frame_obj->is_mouse_move_enabled());
    this->enable_mouse_wheel(frame_obj->is_mouse_wheel_enabled());
    this->enable_keyboard(frame_obj->is_keyboard_enabled());

    this->set_movable(frame_obj->is_movable());
    this->set_clamped_to_screen(frame_obj->is_clamped_to_screen());
    this->set_resizable(frame_obj->is_resizable());

    this->set_abs_hit_rect_insets(frame_obj->get_abs_hit_rect_insets());
    this->set_rel_hit_rect_insets(frame_obj->get_rel_hit_rect_insets());

    this->set_max_dimensions(frame_obj->get_max_dimensions());
    this->set_min_dimensions(frame_obj->get_min_dimensions());

    this->set_scale(frame_obj->get_scale());

    for (const auto& art : frame_obj->region_list_) {
        if (!art || art->is_manually_inherited())
            continue;

        region_core_attributes attr;
        attr.object_type = art->get_object_type();
        attr.name        = art->get_raw_name();
        attr.inheritance = {art};

        auto new_art = create_layered_region(art->get_draw_layer(), std::move(attr));
        if (!new_art)
            continue;

        new_art->notify_loaded();
    }

    if (frame_obj->backdrop_) {
        backdrop_ = std::unique_ptr<backdrop>(new backdrop(*this));
        backdrop_->copy_from(*frame_obj->backdrop_);
    }

    if (frame_obj->title_region_) {
        this->create_title_region();
        if (title_region_)
            title_region_->copy_from(*frame_obj->title_region_);
    }

    for (const auto& child : frame_obj->child_list_) {
        if (!child || child->is_manually_inherited())
            continue;

        frame_core_attributes attr;
        attr.object_type = child->get_object_type();
        attr.name        = child->get_raw_name();
        attr.inheritance = {child};

        auto new_child = create_child(std::move(attr));
        if (!new_child)
            continue;

        new_child->notify_loaded();
    }
}

void frame::create_title_region() {
    if (title_region_) {
        gui::out << gui::warning << "gui::" << type_.back() << ": \"" << name_
                 << "\" already has a title region." << std::endl;
        return;
    }

    region_core_attributes attr;
    attr.object_type = "Region";
    attr.is_virtual  = is_virtual();
    attr.name        = "$parentTitleRegion";
    attr.parent      = observer_from(this);

    auto title_region = get_manager().get_factory().create_region(get_registry(), attr);

    if (!title_region)
        return;

    title_region->set_manually_inherited(true);

    if (!title_region->is_virtual()) {
        // Add shortcut to region as entry in Lua table
        auto& lua                      = get_lua_();
        lua[get_name()]["TitleRegion"] = lua[title_region->get_name()];
    }

    title_region_ = std::move(title_region);
    title_region_->notify_loaded();
}

utils::observer_ptr<const frame> frame::get_child(const std::string& name) const {
    for (const auto& child : child_list_) {
        if (!child)
            continue;

        if (child->get_name() == name)
            return child;

        const std::string& raw_name = child->get_raw_name();
        if (utils::starts_with(raw_name, "$parent") && raw_name.substr(7) == name)
            return child;
    }

    return nullptr;
}

frame::region_list_view frame::get_regions() {
    return region_list_view(region_list_);
}

frame::const_region_list_view frame::get_regions() const {
    return const_region_list_view(region_list_);
}

utils::observer_ptr<const layered_region> frame::get_region(const std::string& name) const {
    for (const auto& reg : region_list_) {
        if (!reg)
            continue;

        if (reg->get_name() == name)
            return reg;

        const std::string& raw_name = reg->get_raw_name();
        if (utils::starts_with(raw_name, "$parent") && raw_name.substr(7) == name)
            return reg;
    }

    return nullptr;
}

void frame::set_dimensions(const vector2f& dimensions) {
    base::set_dimensions(vector2f(
        std::min(std::max(dimensions.x, min_width_), max_width_),
        std::min(std::max(dimensions.y, min_height_), max_height_)));
}

void frame::set_width(float abs_width) {
    base::set_width(std::min(std::max(abs_width, min_width_), max_width_));
}

void frame::set_height(float abs_height) {
    base::set_height(std::min(std::max(abs_height, min_height_), max_height_));
}

void frame::check_position_() {
    if (borders_.right - borders_.left < min_width_) {
        borders_.right = borders_.left + min_width_;
    } else if (borders_.right - borders_.left > max_width_) {
        borders_.right = borders_.left + max_width_;
    }

    if (borders_.bottom - borders_.top < min_height_) {
        borders_.bottom = borders_.top + min_height_;
    } else if (borders_.bottom - borders_.top > max_height_) {
        borders_.bottom = borders_.top + max_height_;
    }

    if (is_clamped_to_screen_) {
        vector2f screen_dimensions = get_effective_frame_renderer()->get_target_dimensions();

        if (borders_.right > screen_dimensions.x) {
            float width = borders_.right - borders_.left;
            if (width > screen_dimensions.x) {
                borders_.left  = 0;
                borders_.right = screen_dimensions.x;
            } else {
                borders_.right = screen_dimensions.x;
                borders_.left  = screen_dimensions.x - width;
            }
        }

        if (borders_.left < 0) {
            float width = borders_.right - borders_.left;
            if (borders_.right - borders_.left > screen_dimensions.x) {
                borders_.left  = 0;
                borders_.right = screen_dimensions.x;
            } else {
                borders_.left  = 0;
                borders_.right = width;
            }
        }

        if (borders_.bottom > screen_dimensions.y) {
            float height = borders_.bottom - borders_.top;
            if (height > screen_dimensions.y) {
                borders_.top    = 0;
                borders_.bottom = screen_dimensions.y;
            } else {
                borders_.bottom = screen_dimensions.y;
                borders_.top    = screen_dimensions.y - height;
            }
        }

        if (borders_.top < 0) {
            float height = borders_.bottom - borders_.top;
            if (height > screen_dimensions.y) {
                borders_.top    = 0;
                borders_.bottom = screen_dimensions.y;
            } else {
                borders_.top    = 0;
                borders_.bottom = height;
            }
        }
    }
}

void frame::disable_draw_layer(layer layer_id) {
    layer_container& layer = layer_list_[static_cast<std::size_t>(layer_id)];
    if (!layer.is_disabled) {
        layer.is_disabled = true;
        notify_renderer_need_redraw();
    }
}

void frame::enable_draw_layer(layer layer_id) {
    layer_container& layer = layer_list_[static_cast<std::size_t>(layer_id)];
    if (!layer.is_disabled) {
        layer.is_disabled = false;
        notify_renderer_need_redraw();
    }
}

void frame::enable_mouse(bool is_mouse_enabled) {
    enable_mouse_click(is_mouse_enabled);
    enable_mouse_move(is_mouse_enabled);
}

void frame::enable_mouse_click(bool is_mouse_enabled) {
    is_mouse_click_enabled_ = is_mouse_enabled;
}

void frame::enable_mouse_move(bool is_mouse_enabled) {
    is_mouse_move_enabled_ = is_mouse_enabled;
}

void frame::enable_mouse_wheel(bool is_mouse_wheel_enabled) {
    is_mouse_wheel_enabled_ = is_mouse_wheel_enabled;
}

void frame::enable_keyboard(bool is_keyboard_enabled) {
    is_keyboard_enabled_ = is_keyboard_enabled;
}

void frame::enable_key_capture(const std::string& key_name) {
    reg_key_list_.insert(key_name);
}

void frame::enable_key_capture(input::key key_id) {
    reg_key_list_.insert(std::string{input::get_key_codename(key_id)});
}

void frame::disable_key_capture(const std::string& key_name) {
    reg_key_list_.erase(key_name);
}

void frame::disable_key_capture(input::key key_id) {
    reg_key_list_.erase(std::string{input::get_key_codename(key_id)});
}

void frame::disable_key_capture() {
    reg_key_list_.clear();
}

void frame::notify_loaded() {
    base::notify_loaded();

    if (!is_virtual_) {
        alive_checker checker(*this);
        fire_script("OnLoad");
        if (!checker.is_alive())
            return;
    }
}

void frame::notify_layers_need_update() {
    build_layer_list_flag_ = true;
}

void frame::set_parent_(utils::observer_ptr<frame> parent) {
    auto* raw_new_parent  = parent.get();
    auto* raw_prev_parent = get_parent().get();

    if (!is_virtual()) {
        if (raw_prev_parent) {
            // Remove shortcut to child
            std::string raw_name = get_raw_name();
            if (utils::starts_with(raw_name, "$parent")) {
                raw_name.erase(0, std::string("$parent").size());
                sol::state& lua                            = get_lua_();
                lua[raw_prev_parent->get_name()][raw_name] = sol::lua_nil;
            }
        }
    }

    base::set_parent_(parent);

    if (!is_virtual()) {
        // Notify visibility
        if (raw_new_parent) {
            if (raw_new_parent->is_visible() && is_shown())
                notify_visible();
            else
                notify_invisible();
        } else {
            if (is_shown())
                notify_visible();
            else
                notify_invisible();
        }

        if (raw_new_parent) {
            // Add shortcut to child as entry in Lua table
            std::string raw_name = get_raw_name();
            if (utils::starts_with(raw_name, "$parent")) {
                raw_name.erase(0, std::string("$parent").size());
                auto& lua                                 = get_lua_();
                lua[raw_new_parent->get_name()][raw_name] = lua[get_name()];
            }
        }

        const auto new_top_level_renderer = compute_top_level_frame_renderer_();
        if (new_top_level_renderer != effective_frame_renderer_) {
            notify_frame_renderer_changed_(new_top_level_renderer);
        }

        const auto new_strata = compute_effective_frame_strata_();
        if (new_strata != effective_strata_) {
            notify_frame_strata_changed_(new_strata);
        }
    }
}

void frame::notify_frame_strata_changed_(frame_strata new_strata_id) {
    const auto old_strata = effective_strata_;

    effective_strata_ = new_strata_id;

    get_effective_frame_renderer()->notify_frame_strata_changed(
        observer_from(this), old_strata, effective_strata_);

    for (const auto& child : child_list_) {
        if (!child)
            continue;

        if (!child->get_frame_strata().has_value())
            child->notify_frame_strata_changed_(new_strata_id);
    }
}

bool frame::has_script(const std::string& script_name) const {
    const auto iter = signal_list_.find(script_name);
    if (iter == signal_list_.end())
        return false;

    return !iter->second.empty();
}

utils::observer_ptr<layered_region> frame::add_region(utils::owner_ptr<layered_region> reg) {
    if (!reg)
        return nullptr;

    reg->set_parent_(observer_from(this));

    utils::observer_ptr<layered_region> added_region = reg;
    region_list_.push_back(std::move(reg));

    notify_layers_need_update();
    notify_renderer_need_redraw();

    if (!is_virtual_) {
        // Add shortcut to region as entry in Lua table
        std::string raw_name = added_region->get_raw_name();
        if (utils::starts_with(raw_name, "$parent")) {
            raw_name.erase(0, std::string("$parent").size());
            auto& lua                 = get_lua_();
            lua[get_name()][raw_name] = lua[added_region->get_name()];
        }
    }

    return added_region;
}

utils::owner_ptr<layered_region>
frame::remove_region(const utils::observer_ptr<layered_region>& reg) {
    if (!reg)
        return nullptr;

    layered_region* raw_pointer = reg.get();

    auto iter = utils::find_if(region_list_, [&](auto& obj) { return obj.get() == raw_pointer; });

    if (iter == region_list_.end()) {
        gui::out << gui::warning << "gui::" << type_.back() << ": "
                 << "Trying to remove \"" << reg->get_name() << "\" from \"" << name_
                 << "\"'s children, but it was not one of this frame's children." << std::endl;
        return nullptr;
    }

    // NB: the iterator is not removed yet; it will be removed later in update().
    auto removed_region = std::move(*iter);

    notify_layers_need_update();
    notify_renderer_need_redraw();
    removed_region->set_parent_(nullptr);

    if (!is_virtual_) {
        // Remove shortcut to region
        std::string raw_name = removed_region->get_raw_name();
        if (utils::starts_with(raw_name, "$parent")) {
            raw_name.erase(0, std::string("$parent").size());
            sol::state& lua           = get_lua_();
            lua[get_name()][raw_name] = sol::lua_nil;
        }
    }

    return removed_region;
}

utils::observer_ptr<layered_region>
frame::create_layered_region(layer layer_id, region_core_attributes attr) {
    attr.is_virtual = is_virtual();
    attr.parent     = observer_from(this);

    auto reg = get_manager().get_factory().create_layered_region(get_registry(), attr);

    if (!reg)
        return nullptr;

    reg->set_draw_layer(layer_id);

    return add_region(std::move(reg));
}

utils::observer_ptr<frame> frame::create_child(frame_core_attributes attr) {
    attr.is_virtual = is_virtual();
    attr.parent     = observer_from(this);

    auto new_frame = get_manager().get_factory().create_frame(get_registry(), attr);

    if (!new_frame)
        return nullptr;

    return add_child(std::move(new_frame));
}

utils::observer_ptr<frame> frame::add_child(utils::owner_ptr<frame> child) {
    if (!child)
        return nullptr;

    child->set_parent_(observer_from(this));

    utils::observer_ptr<frame> added_child = child;
    child_list_.push_back(std::move(child));

    return added_child;
}

utils::owner_ptr<frame> frame::remove_child(const utils::observer_ptr<frame>& child) {
    if (!child)
        return nullptr;

    frame* raw_pointer = child.get();
    auto   iter = utils::find_if(child_list_, [&](auto& obj) { return obj.get() == raw_pointer; });

    if (iter == child_list_.end()) {
        gui::out << gui::warning << "gui::" << type_.back() << ": "
                 << "Trying to remove \"" << child->get_name() << "\" from \"" << name_
                 << "\"'s children, but it was not one of this frame's children." << std::endl;
        return nullptr;
    }

    // NB: the iterator is not removed yet; it will be removed later in update().
    auto removed_child = std::move(*iter);

    removed_child->set_parent_(nullptr);

    return removed_child;
}

frame::child_list_view frame::get_children() {
    return child_list_view(child_list_);
}

frame::const_child_list_view frame::get_children() const {
    return const_child_list_view(child_list_);
}

float frame::get_effective_scale() const {
    if (parent_)
        return scale_ * parent_->get_effective_scale();
    else
        return scale_;
}

int frame::get_level() const {
    return level_;
}

std::optional<frame_strata> frame::get_frame_strata() const {
    return strata_;
}

frame_strata frame::get_effective_frame_strata() const {
    return effective_strata_;
}

frame_strata frame::compute_effective_frame_strata_() const {
    if (strata_.has_value())
        return strata_.value();

    if (parent_ && parent_->get_effective_frame_renderer() == get_effective_frame_renderer())
        return parent_->get_effective_frame_strata();

    // Default if no defined strata and no parent set.
    return frame_strata::medium;
}

utils::observer_ptr<const frame> frame::get_top_level_parent() const {
    auto obj = observer_from(this);
    do {
        if (obj->is_top_level())
            return obj;

        obj = obj->get_parent();
    } while (obj);

    return nullptr;
}

const backdrop* frame::get_backdrop() const {
    return backdrop_.get();
}

backdrop* frame::get_backdrop() {
    return backdrop_.get();
}

backdrop& frame::get_or_create_backdrop() {
    if (!backdrop_)
        backdrop_ = std::unique_ptr<backdrop>(new backdrop(*this));

    return *backdrop_;
}

const std::string& frame::get_frame_type() const {
    return type_.back();
}

const bounds2f& frame::get_abs_hit_rect_insets() const {
    return abs_hit_rect_inset_list_;
}

const bounds2f& frame::get_rel_hit_rect_insets() const {
    return rel_hit_rect_inset_list_;
}

vector2f frame::get_max_dimensions() const {
    return vector2f(max_width_, max_height_);
}

vector2f frame::get_min_dimensions() const {
    return vector2f(min_width_, min_height_);
}

std::size_t frame::get_num_children() const {
    return std::count_if(
        child_list_.begin(), child_list_.end(), [](const auto& child) { return child != nullptr; });
}

std::size_t frame::get_rough_num_children() const {
    return child_list_.size();
}

std::size_t frame::get_num_regions() const {
    return std::count_if(
        region_list_.begin(), region_list_.end(), [](const auto& reg) { return reg != nullptr; });
}

std::size_t frame::get_rough_num_regions() const {
    return region_list_.size();
}

float frame::get_scale() const {
    return scale_;
}

bool frame::is_clamped_to_screen() const {
    return is_clamped_to_screen_;
}

bool frame::is_in_region(const vector2f& position) const {
    if (title_region_ && title_region_->is_in_region(position))
        return true;

    bool is_in_x_range = borders_.left + abs_hit_rect_inset_list_.left <= position.x &&
                         position.x <= borders_.right - abs_hit_rect_inset_list_.right - 1.0f;
    bool is_in_y_range = borders_.top + abs_hit_rect_inset_list_.top <= position.y &&
                         position.y <= borders_.bottom - abs_hit_rect_inset_list_.bottom - 1.0f;

    return is_in_x_range && is_in_y_range;
}

utils::observer_ptr<const frame>
frame::find_topmost_frame(const std::function<bool(const frame&)>& predicate) const {
    if (predicate(*this))
        return observer_from(this);

    return nullptr;
}

bool frame::is_mouse_click_enabled() const {
    return is_mouse_click_enabled_;
}

bool frame::is_mouse_move_enabled() const {
    return is_mouse_move_enabled_;
}

bool frame::is_mouse_wheel_enabled() const {
    return is_mouse_wheel_enabled_;
}

bool frame::is_drag_enabled(const std::string& button_name) const {
    return reg_drag_list_.find(button_name) != reg_drag_list_.end();
}

bool frame::is_keyboard_enabled() const {
    return is_keyboard_enabled_;
}

bool frame::is_key_capture_enabled(const std::string& key_name) const {
    return reg_key_list_.find(key_name) != reg_key_list_.end();
}

bool frame::is_movable() const {
    return is_movable_;
}

bool frame::is_resizable() const {
    return is_resizable_;
}

bool frame::is_top_level() const {
    return is_top_level_;
}

bool frame::is_user_placed() const {
    return is_user_placed_;
}

std::string frame::get_adjusted_script_name(const std::string& script_name) {
    std::string adjusted_name = script_name;
    for (auto iter = adjusted_name.begin(); iter != adjusted_name.end(); ++iter) {
        if ('A' <= *iter && *iter <= 'Z') {
            *iter = std::tolower(*iter);
            if (iter != adjusted_name.begin())
                iter = adjusted_name.insert(iter, '_');
        }
    }

    return adjusted_name;
}

std::string
hijack_sol_error_line(std::string original_message, const std::string& file, std::size_t line_nbr) {
    auto pos1 = original_message.find("[string \"" + file);
    if (pos1 == std::string::npos)
        return original_message;

    auto pos2 = original_message.find_first_of('"', pos1 + 9);
    if (pos2 == std::string::npos)
        return original_message;

    original_message.erase(pos1, pos2 - pos1 + 2);
    original_message.insert(pos1, file);

    auto pos3 = original_message.find_first_of(':', pos1 + file.size());
    if (pos3 == std::string::npos)
        return original_message;

    auto pos4 = original_message.find_first_of(":>", pos3 + 1);
    if (pos4 == std::string::npos)
        return original_message;

    auto offset =
        utils::from_string<std::size_t>(original_message.substr(pos3 + 1, pos4 - pos3 - 1));
    if (!offset.has_value())
        return original_message;

    original_message.erase(pos3 + 1, pos4 - pos3 - 1);
    original_message.insert(pos3 + 1, utils::to_string(line_nbr + offset.value() - 1));
    pos4 = original_message.find_first_of(':', pos3 + 1);

    auto pos5 = original_message.find("[string \"" + file, pos4);
    if (pos5 == std::string::npos)
        return original_message;

    std::string message = original_message.substr(pos4 + 1);
    original_message.erase(pos4 + 1);
    original_message += hijack_sol_error_line(message, file, line_nbr);

    return original_message;
}

std::string hijack_sol_error_message(
    std::string_view original_message, const std::string& file, std::size_t line_nbr) {
    std::string new_error;
    for (auto line : utils::cut(original_message, "\n")) {
        if (!new_error.empty())
            new_error += '\n';

        new_error += hijack_sol_error_line(std::string{line}, file, line_nbr);
    }

    return new_error;
}

utils::connection frame::define_script_(
    const std::string& script_name,
    const std::string& content,
    bool               append,
    const script_info& info) {
    // Create the Lua function from the provided string
    sol::state& lua = get_lua_();

    std::string str = "return function(self";

    constexpr std::size_t max_args = 9;
    for (std::size_t i = 0; i < max_args; ++i)
        str += ", arg" + utils::to_string(i + 1);

    str += ") " + content + " end";

    auto result = lua.do_string(str, info.file_name);

    if (!result.valid()) {
        std::string err = hijack_sol_error_message(
            result.get<sol::error>().what(), info.file_name, info.line_nbr);

        gui::out << gui::error << err << std::endl;

        get_manager().get_event_emitter().fire_event("LUA_ERROR", {err});
        return {};
    }

    sol::protected_function handler = result;

    // Forward it as any other Lua function
    return define_script_(script_name, std::move(handler), append, info);
}

utils::connection frame::define_script_(
    const std::string&      script_name,
    sol::protected_function handler,
    bool                    append,
    const script_info&      info) {

    auto wrapped_handler = [handler = std::move(handler),
                            info](frame& self, const event_data& args) {
        sol::state& lua     = self.get_manager().get_lua();
        lua_State*  lua_raw = lua.lua_state();

        std::vector<sol::object> lua_args;

        // Set arguments
        for (std::size_t i = 0; i < args.get_num_param(); ++i) {
            const utils::variant& arg = args.get(i);
            if (std::holds_alternative<utils::empty>(arg))
                lua_args.emplace_back(sol::lua_nil);
            else
                lua_args.emplace_back(lua_raw, sol::in_place, arg);
        }

        // Get a reference to self
        sol::object self_lua = lua[self.get_name()];
        if (self_lua == sol::lua_nil)
            throw gui::exception("Lua glue object is nil");

        // Call the function
        auto result = handler(self_lua, sol::as_args(lua_args));
        // WARNING: after this point, the frame (self_lua) may be deleted.
        // Do not use any member variable or member function directly.

        // Handle errors
        if (!result.valid()) {
            throw gui::exception(hijack_sol_error_message(
                result.get<sol::error>().what(), info.file_name, info.line_nbr));
        }
    };

    return define_script_(script_name, std::move(wrapped_handler), append, info);
}

utils::connection frame::define_script_(
    const std::string& script_name,
    script_function    handler,
    bool               append,
    const script_info& /*info*/) {
    if (!is_virtual()) {
        // Register the function so it can be called directly from Lua
        std::string adjusted_name = get_adjusted_script_name(script_name);

        get_lua_()[get_name()][adjusted_name].set_function(
            [=](frame& self, sol::variadic_args v_args) {
                event_data data;
                for (auto&& arg : v_args) {
                    lxgui::utils::variant variant;
                    if (!arg.is<sol::lua_nil_t>())
                        variant = arg;

                    data.add(std::move(variant));
                }

                self.fire_script(script_name, data);
            });
    }

    auto& handler_list = signal_list_[script_name];
    if (!append) {
        // Just disable existing scripts, it may not be safe to modify the handler list
        // if this script is being defined during a handler execution.
        // They will be deleted later, when we know it is safe.
        handler_list.disconnect_all();
    }

    // TODO: add file/line info if the handler comes from C++
    // https://github.com/cschreib/lxgui/issues/96
    return handler_list.connect(std::move(handler));
}

script_list_view frame::get_script(const std::string& script_name) const {
    auto iter_h = signal_list_.find(script_name);
    if (iter_h == signal_list_.end())
        throw gui::exception(type_.back(), "no script registered for " + script_name);

    return iter_h->second.slots();
}

void frame::remove_script(const std::string& script_name) {
    auto iter_h = signal_list_.find(script_name);
    if (iter_h == signal_list_.end())
        return;

    // Just disable existing scripts, it may not be safe to modify the handler list
    // if this script is being defined during a handler execution.
    // They will be deleted later, when we know it is safe.
    iter_h->second.disconnect_all();

    if (!is_virtual()) {
        std::string adjusted_name             = get_adjusted_script_name(script_name);
        get_lua_()[get_name()][adjusted_name] = sol::lua_nil;
    }
}

void frame::on_event_(std::string_view event_name, const event_data& event) {
    event_data data;
    data.add(std::string(event_name));
    for (std::size_t i = 0; i < event.get_num_param(); ++i)
        data.add(event.get(i));

    fire_script("OnEvent", data);
}

void frame::fire_script(const std::string& script_name, const event_data& data) {
    if (!is_loaded())
        return;

    auto iter_h = signal_list_.find(script_name);
    if (iter_h == signal_list_.end())
        return;

    // Make a copy of useful pointers: in case the frame is deleted, we will need this
    auto&       event_emitter  = get_manager().get_event_emitter();
    auto&       addon_registry = *get_manager().get_addon_registry();
    const auto* old_addon      = addon_registry.get_current_addon();
    addon_registry.set_current_addon(get_addon());

    try {
        // Call the handlers
        iter_h->second(*this, data);
    } catch (const std::exception& e) {
        std::string err = e.what();
        gui::out << gui::error << err << std::endl;
        event_emitter.fire_event("LUA_ERROR", {err});
    }

    addon_registry.set_current_addon(old_addon);
}

void frame::register_event(const std::string& event_name) {
    if (is_virtual_)
        return;

    event_receiver_.register_event(
        event_name, [=](const event_data& event) { return on_event_(event_name, event); });
}

void frame::unregister_event(const std::string& event_name) {
    if (is_virtual_)
        return;

    event_receiver_.unregister_event(event_name);
}

void frame::enable_drag(const std::string& button_name) {
    reg_drag_list_.insert(button_name);
}

void frame::enable_drag(input::mouse_button button_id) {
    reg_drag_list_.insert(std::string{input::get_mouse_button_codename(button_id)});
}

void frame::disable_drag(const std::string& button_name) {
    reg_drag_list_.erase(button_name);
}

void frame::disable_drag(input::mouse_button button_id) {
    reg_drag_list_.erase(std::string{input::get_mouse_button_codename(button_id)});
}

void frame::disable_drag() {
    reg_drag_list_.clear();
}

void frame::set_clamped_to_screen(bool is_clamped_to_screen) {
    is_clamped_to_screen_ = is_clamped_to_screen;
}

void frame::set_frame_strata(std::optional<frame_strata> strata_id) {
    strata_ = strata_id;

    const auto new_effective_strata = compute_effective_frame_strata_();

    if (!is_virtual() && new_effective_strata != effective_strata_) {
        notify_frame_strata_changed_(new_effective_strata);
    }
}

void frame::set_backdrop(std::unique_ptr<backdrop> bdrop) {
    backdrop_ = std::move(bdrop);
    notify_renderer_need_redraw();
}

void frame::set_abs_hit_rect_insets(const bounds2f& insets) {
    abs_hit_rect_inset_list_ = insets;
}

void frame::set_rel_hit_rect_insets(const bounds2f& insets) {
    rel_hit_rect_inset_list_ = insets;
}

void frame::set_level(int level_id) {
    if (level_id == level_)
        return;

    std::swap(level_id, level_);

    if (!is_virtual_) {
        get_effective_frame_renderer()->notify_frame_level_changed(
            observer_from(this), level_id, level_);
    }
}

void frame::set_max_dimensions(const vector2f& max) {
    set_max_width(max.x);
    set_max_height(max.y);
}

void frame::set_min_dimensions(const vector2f& min) {
    set_min_width(min.x);
    set_min_height(min.y);
}

void frame::set_max_height(float max_height) {
    if (max_height < 0.0f)
        max_height = std::numeric_limits<float>::infinity();

    if (max_height >= min_height_)
        max_height_ = max_height;

    if (max_height_ != max_height && !is_virtual_)
        notify_borders_need_update();
}

void frame::set_max_width(float max_width) {
    if (max_width < 0.0f)
        max_width = std::numeric_limits<float>::infinity();

    if (max_width >= min_width_)
        max_width_ = max_width;

    if (max_width_ != max_width && !is_virtual_)
        notify_borders_need_update();
}

void frame::set_min_height(float min_height) {
    if (min_height <= max_height_)
        min_height_ = min_height;

    if (min_height_ != min_height && !is_virtual_)
        notify_borders_need_update();
}

void frame::set_min_width(float min_width) {
    if (min_width <= max_width_)
        min_width_ = min_width;

    if (min_width_ != min_width && !is_virtual_)
        notify_borders_need_update();
}

void frame::set_movable(bool is_movable) {
    is_movable_ = is_movable;
}

utils::owner_ptr<region> frame::release_from_parent() {
    utils::observer_ptr<frame> self = observer_from(this);
    if (parent_)
        return parent_->remove_child(self);
    else
        return get_manager().get_root().remove_root_frame(self);
}

void frame::set_resizable(bool is_resizable) {
    is_resizable_ = is_resizable;
}

void frame::set_scale(float scale) {
    scale_ = scale;
    if (scale_ != scale)
        notify_renderer_need_redraw();
}

void frame::set_top_level(bool is_top_level) {
    is_top_level_ = is_top_level;
}

void frame::raise() {
    if (!is_top_level_)
        return;

    int  old_level = level_;
    auto rdr       = get_effective_frame_renderer();
    int  top_level = rdr->get_highest_level(get_effective_frame_strata());

    if (old_level == top_level) {
        // This frame is already on top, nothing to do.
        return;
    }

    level_   = top_level + 1;
    int diff = level_ - old_level;

    if (!is_virtual()) {
        rdr->notify_frame_level_changed(observer_from(this), old_level, level_);
    }

    for (auto& child : get_children())
        child.add_level_(diff);
}

void frame::enable_auto_focus(bool enable) {
    is_auto_focus_ = enable;
}

bool frame::is_auto_focus_enabled() const {
    return is_auto_focus_;
}

void frame::set_focus(bool focus) {
    auto& root = get_manager().get_root();
    if (focus)
        root.request_focus(observer_from(this));
    else
        root.release_focus(*this);
}

bool frame::has_focus() const {
    return is_focused_;
}

void frame::notify_focus(bool focus) {
    if (is_focused_ == focus)
        return;

    is_focused_ = focus;

    if (is_focused_)
        fire_script("OnFocusGained");
    else
        fire_script("OnFocusLost");
}

void frame::add_level_(int amount) {
    int old_level = level_;
    level_ += amount;

    if (!is_virtual()) {
        get_effective_frame_renderer()->notify_frame_level_changed(
            observer_from(this), old_level, level_);
    }

    for (auto& child : get_children())
        child.add_level_(amount);
}

void frame::set_user_placed(bool is_user_placed) {
    is_user_placed_ = is_user_placed;
}

void frame::start_moving() {
    if (!is_movable_)
        return;

    set_user_placed(true);
    get_manager().get_root().start_moving(observer_from(this));
}

void frame::stop_moving() {
    if (get_manager().get_root().is_moving(*this))
        get_manager().get_root().stop_moving();
}

void frame::start_sizing(const point& p) {
    if (!is_resizable_)
        return;

    set_user_placed(true);
    get_manager().get_root().start_sizing(observer_from(this), p);
}

void frame::stop_sizing() {
    if (get_manager().get_root().is_sizing(*this))
        get_manager().get_root().stop_sizing();
}

void frame::notify_frame_renderer_changed_(
    const utils::observer_ptr<frame_renderer>& new_renderer) {

    effective_frame_renderer_->notify_rendered_frame(observer_from(this), false);
    effective_frame_renderer_ = new_renderer;
    effective_frame_renderer_->notify_rendered_frame(observer_from(this), true);

    for (const auto& child : child_list_) {
        if (auto* raw_ptr = child.get(); raw_ptr && !raw_ptr->get_frame_renderer())
            child->notify_frame_renderer_changed_(new_renderer);
    }
}

void frame::set_frame_renderer(utils::observer_ptr<frame_renderer> rdr) {
    if (rdr == frame_renderer_)
        return;

    frame_renderer_                   = std::move(rdr);
    const auto new_effective_renderer = compute_top_level_frame_renderer_();

    if (effective_frame_renderer_ != new_effective_renderer) {
        notify_frame_renderer_changed_(new_effective_renderer);
    }
}

utils::observer_ptr<const frame_renderer> frame::get_effective_frame_renderer() const {
    return effective_frame_renderer_;
}

utils::observer_ptr<const frame_renderer> frame::compute_top_level_frame_renderer_() const {
    if (frame_renderer_)
        return frame_renderer_;
    else if (parent_)
        return parent_->get_effective_frame_renderer();
    else
        return get_manager().get_root().observer_from_this();
}

void frame::notify_visible() {
    alive_checker checker(*this);

    if (is_auto_focus_) {
        set_focus(true);
        if (!checker.is_alive())
            return;
    }

    base::notify_visible();

    for (auto& obj : get_regions()) {
        if (obj.is_shown()) {
            obj.notify_visible();
            if (!checker.is_alive())
                return;
        }
    }

    for (auto& child : get_children()) {
        if (child.is_shown()) {
            child.notify_visible();
            if (!checker.is_alive())
                return;
        }
    }

    fire_script("OnShow");
    if (!checker.is_alive())
        return;

    get_manager().get_root().notify_hovered_frame_dirty();
}

void frame::notify_invisible() {
    alive_checker checker(*this);

    set_focus(false);
    if (!checker.is_alive())
        return;

    base::notify_invisible();

    for (auto& child : get_children()) {
        if (child.is_shown()) {
            child.notify_invisible();
            if (!checker.is_alive())
                return;
        }
    }

    fire_script("OnHide");
    if (!checker.is_alive())
        return;

    get_manager().get_root().notify_hovered_frame_dirty();
}

void frame::notify_renderer_need_redraw() {
    if (is_virtual_)
        return;

    get_effective_frame_renderer()->notify_strata_needs_redraw(get_effective_frame_strata());
}

void frame::notify_scaling_factor_updated() {
    base::notify_scaling_factor_updated();

    if (title_region_)
        title_region_->notify_scaling_factor_updated();

    for (auto& child : get_children())
        child.notify_scaling_factor_updated();

    for (auto& obj : get_regions())
        obj.notify_scaling_factor_updated();
}

void frame::notify_mouse_in_frame(bool mouse_in_frame, const vector2f& /*position*/) {
    if (mouse_in_frame == is_mouse_in_frame_)
        return;

    is_mouse_in_frame_ = mouse_in_frame;

    alive_checker checker(*this);
    if (is_mouse_in_frame_) {
        fire_script("OnEnter");
        if (!checker.is_alive())
            return;
    } else {
        fire_script("OnLeave");
        if (!checker.is_alive())
            return;
    }
}

void frame::update_borders_() {
    const bool old_ready       = is_ready_;
    const auto old_border_list = borders_;

    base::update_borders_();

    check_position_();

    if (borders_ != old_border_list || is_ready_ != old_ready) {
        if (borders_.width() != old_border_list.width() ||
            borders_.height() != old_border_list.height()) {
            alive_checker checker(*this);
            fire_script("OnSizeChanged");
            if (!checker.is_alive())
                return;
        }

        get_manager().get_root().notify_hovered_frame_dirty();

        if (backdrop_)
            backdrop_->notify_borders_updated();
    }
}

void frame::update(float delta) {
    alive_checker checker(*this);

    base::update(delta);

    if (build_layer_list_flag_) {
        // Clear layers' content
        for (auto& layer : layer_list_)
            layer.region_list.clear();

        // Fill layers with regions (with font_string rendered last within the same layer)
        for (const auto& reg : region_list_) {
            if (reg && !reg->is_object_type("font_string")) {
                layer_list_[static_cast<std::size_t>(reg->get_draw_layer())].region_list.push_back(
                    reg);
            }
        }

        for (const auto& reg : region_list_) {
            if (reg && reg->is_object_type("font_string")) {
                layer_list_[static_cast<std::size_t>(reg->get_draw_layer())].region_list.push_back(
                    reg);
            }
        }

        build_layer_list_flag_ = false;
    }

    if (is_visible()) {
        fire_script("OnUpdate", {delta});
        if (!checker.is_alive())
            return;
    }

    if (title_region_)
        title_region_->update(delta);

    // Update regions
    for (auto& obj : get_regions()) {
        obj.update(delta);
    }

    // Remove deleted regions
    {
        auto iter_remove = std::remove_if(
            region_list_.begin(), region_list_.end(), [](auto& obj) { return obj == nullptr; });

        region_list_.erase(iter_remove, region_list_.end());
    }

    // Update children
    for (auto& child : get_children()) {
        child.update(delta);
        if (!checker.is_alive())
            return;
    }

    // Remove deleted children
    {
        auto iter_remove = std::remove_if(
            child_list_.begin(), child_list_.end(), [](auto& obj) { return obj == nullptr; });

        child_list_.erase(iter_remove, child_list_.end());
    }

    // Remove empty handlers
    {
        auto iter_list = signal_list_.begin();
        while (iter_list != signal_list_.end()) {
            if (iter_list->second.empty()) {
                iter_list = signal_list_.erase(iter_list);
            } else {
                ++iter_list;
            }
        }
    }
}

} // namespace lxgui::gui
