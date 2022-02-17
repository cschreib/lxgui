#include "lxgui/gui_frame.hpp"

#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/utils_range.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <functional>
#include <sol/as_args.hpp>
#include <sol/state.hpp>
#include <sol/variadic_args.hpp>
#include <sstream>

namespace lxgui::gui {

frame::frame(utils::control_block& m_block, manager& m_manager) :
    base(m_block, m_manager), m_event_receiver_(m_manager.get_event_emitter()) {
    type_.push_back(class_name);
}

frame::~frame() {
    // Disable callbacks
    signal_list_.clear();

    // Children must be destroyed first
    child_list_.clear();
    region_list_.clear();

    if (!is_virtual_) {
        // Tell the renderer to no longer render this region
        get_top_level_renderer()->notify_rendered_frame(observer_from(this), false);
        p_renderer_ = nullptr;
    }

    get_manager().get_root().notify_hovered_frame_dirty();

    set_focus(false);
}

void frame::render() const {
    if (!is_visible_ || !is_ready_)
        return;

    if (p_backdrop_)
        p_backdrop_->render();

    // Render child regions
    for (const auto& m_layer : layer_list_) {
        if (m_layer.is_disabled)
            continue;

        for (const auto& p_region : m_layer.region_list) {
            if (p_region->is_shown())
                p_region->render();
        }
    }
}

void frame::create_glue() {
    create_glue_(this);
}

std::string frame::serialize(const std::string& tab) const {
    std::ostringstream str;

    str << base::serialize(tab);
    if (auto p_frame_renderer = utils::dynamic_pointer_cast<frame>(p_renderer_))
        str << tab << "  # Man. render : " << p_frame_renderer->get_name() << "\n";
    str << tab << "  # Strata      : ";
    switch (m_strata_) {
    case frame_strata::parent: str << "PARENT\n"; break;
    case frame_strata::background: str << "BACKGROUND\n"; break;
    case frame_strata::low: str << "LOW\n"; break;
    case frame_strata::medium: str << "MEDIUM\n"; break;
    case frame_strata::high: str << "HIGH\n"; break;
    case frame_strata::dialog: str << "DIALOG\n"; break;
    case frame_strata::fullscreen: str << "FULLSCREEN\n"; break;
    case frame_strata::fullscreen_dialog: str << "FULLSCREEN_DIALOG\n"; break;
    case frame_strata::tooltip: str << "TOOLTIP\n"; break;
    }
    str << tab << "  # Level       : " << level_ << "\n";
    str << tab << "  # TopLevel    : " << is_top_level_;
    if (!is_top_level_ && get_top_level_parent())
        str << " (" << get_top_level_parent()->get_name() << ")\n";
    else
        str << "\n";
    if (!is_mouse_click_enabled_ && !is_mouse_move_enabled_ && !!is_mouse_wheel_enabled_)
        str << tab << "  # Inputs      : none\n";
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
    str << tab << "  # Movable     : " << is_movable_ << "\n";
    str << tab << "  # Resizable   : " << is_resizable_ << "\n";
    str << tab << "  # Clamped     : " << is_clamped_to_screen_ << "\n";
    str << tab << "  # HRect inset :\n";
    str << tab << "  |-###\n";
    str << tab << "  |   # left   : " << abs_hit_rect_inset_list_.left << "\n";
    str << tab << "  |   # right  : " << abs_hit_rect_inset_list_.right << "\n";
    str << tab << "  |   # top    : " << abs_hit_rect_inset_list_.top << "\n";
    str << tab << "  |   # bottom : " << abs_hit_rect_inset_list_.bottom << "\n";
    str << tab << "  |-###\n";
    str << tab << "  # Min width   : " << f_min_width_ << "\n";
    str << tab << "  # Max width   : " << f_max_width_ << "\n";
    str << tab << "  # Min height  : " << f_min_height_ << "\n";
    str << tab << "  # Max height  : " << f_max_height_ << "\n";
    str << tab << "  # Scale       : " << f_scale_ << "\n";
    if (p_title_region_) {
        str << tab << "  # Title reg.  :\n";
        str << tab << "  |-###\n";
        str << p_title_region_->serialize(tab + "  | ");
        str << tab << "  |-###\n";
    }
    if (p_backdrop_) {
        const bounds2f& insets = p_backdrop_->get_background_insets();

        str << tab << "  # Backdrop    :\n";
        str << tab << "  |-###\n";
        str << tab << "  |   # Background : " << p_backdrop_->get_background_file() << "\n";
        str << tab << "  |   # Tilling    : " << p_backdrop_->is_background_tilling() << "\n";
        if (p_backdrop_->is_background_tilling())
            str << tab << "  |   # Tile size  : " << p_backdrop_->get_tile_size() << "\n";
        str << tab << "  |   # BG Insets  :\n";
        str << tab << "  |   |-###\n";
        str << tab << "  |   |   # left   : " << insets.left << "\n";
        str << tab << "  |   |   # right  : " << insets.right << "\n";
        str << tab << "  |   |   # top    : " << insets.top << "\n";
        str << tab << "  |   |   # bottom : " << insets.bottom << "\n";
        str << tab << "  |   |-###\n";
        str << tab << "  |   # Edge       : " << p_backdrop_->get_edge_file() << "\n";
        str << tab << "  |   # Edge size  : " << p_backdrop_->get_edge_size() << "\n";
        str << tab << "  |-###\n";
    }

    if (!region_list_.empty()) {
        if (child_list_.size() == 1)
            str << tab << "  # Region : \n";
        else
            str << tab << "  # Regions     : " << region_list_.size() << "\n";
        str << tab << "  |-###\n";

        for (auto& m_region : get_regions()) {
            str << m_region.serialize(tab + "  | ");
            str << tab << "  |-###\n";
        }
    }

    if (!child_list_.empty()) {
        if (child_list_.size() == 1)
            str << tab << "  # Child : \n";
        else
            str << tab << "  # Children    : " << child_list_.size() << "\n";
        str << tab << "  |-###\n";

        for (const auto& m_child : get_children()) {
            str << m_child.serialize(tab + "  | ");
            str << tab << "  |-###\n";
        }
    }

    return str.str();
}

bool frame::can_use_script(const std::string& script_name) const {
    return (script_name == "OnChar") || (script_name == "OnDragStart") ||
           (script_name == "OnDragStop") || (script_name == "OnDragMove") ||
           (script_name == "OnEnter") || (script_name == "OnEvent") ||
           (script_name == "OnFocusGained") || (script_name == "OnFocusLost") ||
           (script_name == "OnHide") || (script_name == "OnKeyDown") ||
           (script_name == "OnKeyUp") || (script_name == "OnLeave") || (script_name == "OnLoad") ||
           (script_name == "OnMouseDown") || (script_name == "OnMouseUp") ||
           (script_name == "OnDoubleClick") || (script_name == "OnMouseWheel") ||
           (script_name == "OnReceiveDrag") || (script_name == "OnShow") ||
           (script_name == "OnSizeChanged") || (script_name == "OnUpdate");
}

void frame::copy_from(const region& m_obj) {
    base::copy_from(m_obj);

    const frame* p_frame = down_cast<frame>(&m_obj);
    if (!p_frame)
        return;

    for (const auto& m_item : p_frame->signal_list_) {
        for (const auto& m_function : p_frame->get_script(m_item.first)) {
            this->add_script(m_item.first, m_function);
        }
    }

    this->set_frame_strata(p_frame->get_frame_strata());

    utils::observer_ptr<const frame> p_high_parent = observer_from(this);

    for (int i = 0; i < p_frame->get_level(); ++i) {
        if (!p_high_parent->get_parent())
            break;

        p_high_parent = p_high_parent->get_parent();
    }

    this->set_level(p_high_parent->get_level() + p_frame->get_level());

    this->set_top_level(p_frame->is_top_level());

    this->enable_mouse_click(p_frame->is_mouse_click_enabled());
    this->enable_mouse_move(p_frame->is_mouse_move_enabled());
    this->enable_mouse_wheel(p_frame->is_mouse_wheel_enabled());

    this->set_movable(p_frame->is_movable());
    this->set_clamped_to_screen(p_frame->is_clamped_to_screen());
    this->set_resizable(p_frame->is_resizable());

    this->set_abs_hit_rect_insets(p_frame->get_abs_hit_rect_insets());
    this->set_rel_hit_rect_insets(p_frame->get_rel_hit_rect_insets());

    this->set_max_dimensions(p_frame->get_max_dimensions());
    this->set_min_dimensions(p_frame->get_min_dimensions());

    this->set_scale(p_frame->get_scale());

    for (const auto& p_art : p_frame->region_list_) {
        if (!p_art || p_art->is_special())
            continue;

        region_core_attributes m_attr;
        m_attr.object_type = p_art->get_object_type();
        m_attr.name        = p_art->get_raw_name();
        m_attr.inheritance = {p_art};

        auto p_new_art = create_layered_region(p_art->get_draw_layer(), std::move(m_attr));
        if (!p_new_art)
            continue;

        p_new_art->notify_loaded();
    }

    build_layer_list_flag_ = true;

    if (p_frame->p_backdrop_) {
        p_backdrop_ = std::unique_ptr<backdrop>(new backdrop(*this));
        p_backdrop_->copy_from(*p_frame->p_backdrop_);
    }

    if (p_frame->p_title_region_) {
        this->create_title_region();
        if (p_title_region_)
            p_title_region_->copy_from(*p_frame->p_title_region_);
    }

    for (const auto& p_child : p_frame->child_list_) {
        if (!p_child || p_child->is_special())
            continue;

        region_core_attributes m_attr;
        m_attr.object_type = p_child->get_object_type();
        m_attr.name        = p_child->get_raw_name();
        m_attr.inheritance = {p_child};

        auto p_new_child = create_child(std::move(m_attr));
        if (!p_new_child)
            continue;

        p_new_child->notify_loaded();
    }
}

void frame::create_title_region() {
    if (p_title_region_) {
        gui::out << gui::warning << "gui::" << type_.back()
                 << " : \"" + name_ + "\" already has a title region." << std::endl;
        return;
    }

    region_core_attributes m_attr;
    m_attr.object_type = "Region";
    m_attr.is_virtual  = is_virtual();
    m_attr.name        = "$parentTitleRegion";
    m_attr.p_parent    = observer_from(this);

    auto p_title_region = get_manager().get_factory().create_region(get_registry(), m_attr);

    if (!p_title_region)
        return;

    p_title_region->set_special();

    if (!p_title_region->is_virtual()) {
        // Add shortcut to region as entry in Lua table
        auto& m_lua                          = get_lua_();
        m_lua[get_lua_name()]["TitleRegion"] = m_lua[p_title_region->get_lua_name()];
    }

    p_title_region_ = std::move(p_title_region);
    p_title_region_->notify_loaded();
}

utils::observer_ptr<const frame> frame::get_child(const std::string& name) const {
    for (const auto& p_child : child_list_) {
        if (!p_child)
            continue;

        if (p_child->get_name() == name)
            return p_child;

        const std::string& raw_name = p_child->get_raw_name();
        if (utils::starts_with(raw_name, "$parent") && raw_name.substr(7) == name)
            return p_child;
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
    for (const auto& p_region : region_list_) {
        if (!p_region)
            continue;

        if (p_region->get_name() == name)
            return p_region;

        const std::string& raw_name = p_region->get_raw_name();
        if (utils::starts_with(raw_name, "$parent") && raw_name.substr(7) == name)
            return p_region;
    }

    return nullptr;
}

void frame::set_dimensions(const vector2f& m_dimensions) {
    base::set_dimensions(vector2f(
        std::min(std::max(m_dimensions.x, f_min_width_), f_max_width_),
        std::min(std::max(m_dimensions.y, f_min_height_), f_max_height_)));
}

void frame::set_width(float f_abs_width) {
    base::set_width(std::min(std::max(f_abs_width, f_min_width_), f_max_width_));
}

void frame::set_height(float f_abs_height) {
    base::set_height(std::min(std::max(f_abs_height, f_min_height_), f_max_height_));
}

void frame::check_position_() {
    if (border_list_.right - border_list_.left < f_min_width_) {
        border_list_.right = border_list_.left + f_min_width_;
    } else if (border_list_.right - border_list_.left > f_max_width_) {
        border_list_.right = border_list_.left + f_max_width_;
    }

    if (border_list_.bottom - border_list_.top < f_min_height_) {
        border_list_.bottom = border_list_.top + f_min_height_;
    } else if (border_list_.bottom - border_list_.top > f_max_height_) {
        border_list_.bottom = border_list_.top + f_max_height_;
    }

    if (is_clamped_to_screen_) {
        vector2f m_screen_dimensions = get_top_level_renderer()->get_target_dimensions();

        if (border_list_.right > m_screen_dimensions.x) {
            float f_width = border_list_.right - border_list_.left;
            if (f_width > m_screen_dimensions.x) {
                border_list_.left  = 0;
                border_list_.right = m_screen_dimensions.x;
            } else {
                border_list_.right = m_screen_dimensions.x;
                border_list_.left  = m_screen_dimensions.x - f_width;
            }
        }

        if (border_list_.left < 0) {
            float f_width = border_list_.right - border_list_.left;
            if (border_list_.right - border_list_.left > m_screen_dimensions.x) {
                border_list_.left  = 0;
                border_list_.right = m_screen_dimensions.x;
            } else {
                border_list_.left  = 0;
                border_list_.right = f_width;
            }
        }

        if (border_list_.bottom > m_screen_dimensions.y) {
            float f_height = border_list_.bottom - border_list_.top;
            if (f_height > m_screen_dimensions.y) {
                border_list_.top    = 0;
                border_list_.bottom = m_screen_dimensions.y;
            } else {
                border_list_.bottom = m_screen_dimensions.y;
                border_list_.top    = m_screen_dimensions.y - f_height;
            }
        }

        if (border_list_.top < 0) {
            float f_height = border_list_.bottom - border_list_.top;
            if (f_height > m_screen_dimensions.y) {
                border_list_.top    = 0;
                border_list_.bottom = m_screen_dimensions.y;
            } else {
                border_list_.top    = 0;
                border_list_.bottom = f_height;
            }
        }
    }
}

void frame::disable_draw_layer(layer m_layer_id) {
    layer_container& m_layer = layer_list_[static_cast<std::size_t>(m_layer_id)];
    if (!m_layer.is_disabled) {
        m_layer.is_disabled = true;
        notify_renderer_need_redraw();
    }
}

void frame::enable_draw_layer(layer m_layer_id) {
    layer_container& m_layer = layer_list_[static_cast<std::size_t>(m_layer_id)];
    if (!m_layer.is_disabled) {
        m_layer.is_disabled = false;
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

void frame::enable_key_capture(const std::string& key_name, bool is_capture_enabled) {
    if (is_capture_enabled)
        reg_key_list_.erase(key_name);
    else
        reg_key_list_.insert(key_name);
}

void frame::notify_loaded() {
    base::notify_loaded();

    if (!is_virtual_) {
        alive_checker m_checker(*this);
        fire_script("OnLoad");
        if (!m_checker.is_alive())
            return;
    }
}

void frame::notify_layers_need_update() {
    build_layer_list_flag_ = true;
}

bool frame::has_script(const std::string& script_name) const {
    const auto m_iter = signal_list_.find(script_name);
    if (m_iter == signal_list_.end())
        return false;

    return !m_iter->second.empty();
}

utils::observer_ptr<layered_region> frame::add_region(utils::owner_ptr<layered_region> p_region) {
    if (!p_region)
        return nullptr;

    p_region->set_parent_(observer_from(this));

    utils::observer_ptr<layered_region> p_added_region = p_region;
    region_list_.push_back(std::move(p_region));

    notify_layers_need_update();
    notify_renderer_need_redraw();

    if (!is_virtual_) {
        // Add shortcut to region as entry in Lua table
        std::string raw_name = p_added_region->get_raw_name();
        if (utils::starts_with(raw_name, "$parent")) {
            raw_name.erase(0, std::string("$parent").size());
            auto& m_lua                     = get_lua_();
            m_lua[get_lua_name()][raw_name] = m_lua[p_added_region->get_lua_name()];
        }
    }

    return p_added_region;
}

utils::owner_ptr<layered_region>
frame::remove_region(const utils::observer_ptr<layered_region>& p_region) {
    if (!p_region)
        return nullptr;

    layered_region* p_raw_pointer = p_region.get();

    auto m_iter =
        utils::find_if(region_list_, [&](auto& p_obj) { return p_obj.get() == p_raw_pointer; });

    if (m_iter == region_list_.end()) {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "Trying to remove \"" << p_region->get_name() << "\" from \"" << name_
                 << "\"'s children, "
                    "but it was not one of this frame's children."
                 << std::endl;
        return nullptr;
    }

    // NB: the iterator is not removed yet; it will be removed later in update().
    auto p_removed_region = std::move(*m_iter);

    notify_layers_need_update();
    notify_renderer_need_redraw();
    p_removed_region->set_parent_(nullptr);

    if (!is_virtual_) {
        // Remove shortcut to region
        std::string raw_name = p_removed_region->get_raw_name();
        if (utils::starts_with(raw_name, "$parent")) {
            raw_name.erase(0, std::string("$parent").size());
            sol::state& m_lua               = get_lua_();
            m_lua[get_lua_name()][raw_name] = sol::lua_nil;
        }
    }

    return p_removed_region;
}

utils::observer_ptr<layered_region>
frame::create_layered_region(layer m_layer, region_core_attributes m_attr) {
    m_attr.is_virtual = is_virtual();
    m_attr.p_parent   = observer_from(this);

    auto p_region = get_manager().get_factory().create_layered_region(get_registry(), m_attr);

    if (!p_region)
        return nullptr;

    p_region->set_draw_layer(m_layer);

    return add_region(std::move(p_region));
}

utils::observer_ptr<frame> frame::create_child(region_core_attributes m_attr) {
    m_attr.is_virtual = is_virtual();
    m_attr.p_parent   = observer_from(this);

    auto p_new_frame = get_manager().get_factory().create_frame(
        get_registry(), get_top_level_renderer().get(), m_attr);

    if (!p_new_frame)
        return nullptr;

    p_new_frame->set_level(get_level() + 1);

    return add_child(std::move(p_new_frame));
}

utils::observer_ptr<frame> frame::add_child(utils::owner_ptr<frame> p_child) {
    if (!p_child)
        return nullptr;

    p_child->set_parent_(observer_from(this));

    if (is_visible() && p_child->is_shown())
        p_child->notify_visible();
    else
        p_child->notify_invisible();

    utils::observer_ptr<frame> p_added_child = p_child;
    child_list_.push_back(std::move(p_child));

    if (!is_virtual_) {
        utils::observer_ptr<frame_renderer> p_old_top_level_renderer =
            p_added_child->get_top_level_renderer();
        utils::observer_ptr<frame_renderer> p_new_top_level_renderer = get_top_level_renderer();
        if (p_old_top_level_renderer != p_new_top_level_renderer) {
            p_old_top_level_renderer->notify_rendered_frame(p_added_child, false);
            p_new_top_level_renderer->notify_rendered_frame(p_added_child, true);
        }

        // Add shortcut to child as entry in Lua table
        std::string raw_name = p_added_child->get_raw_name();
        if (utils::starts_with(raw_name, "$parent")) {
            raw_name.erase(0, std::string("$parent").size());
            auto& m_lua                     = get_lua_();
            m_lua[get_lua_name()][raw_name] = m_lua[p_added_child->get_lua_name()];
        }
    }

    return p_added_child;
}

utils::owner_ptr<frame> frame::remove_child(const utils::observer_ptr<frame>& p_child) {
    if (!p_child)
        return nullptr;

    frame* p_raw_pointer = p_child.get();
    auto   m_iter =
        utils::find_if(child_list_, [&](auto& p_obj) { return p_obj.get() == p_raw_pointer; });

    if (m_iter == child_list_.end()) {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "Trying to remove \"" << p_child->get_name() << "\" from \"" << name_
                 << "\"'s children, but it was not one of this frame's children." << std::endl;
        return nullptr;
    }

    // NB: the iterator is not removed yet; it will be removed later in update().
    auto p_removed_child = std::move(*m_iter);

    bool notify_renderer = false;
    if (!is_virtual_) {
        utils::observer_ptr<frame_renderer> p_top_level_renderer = get_top_level_renderer();
        notify_renderer =
            !p_child->get_renderer() && p_top_level_renderer.get() != &get_manager().get_root();
        if (notify_renderer) {
            p_top_level_renderer->notify_rendered_frame(p_child, false);
            p_child->propagate_renderer_(false);
        }
    }

    p_removed_child->set_parent_(nullptr);

    if (!is_virtual_) {
        if (notify_renderer) {
            get_manager().get_root().notify_rendered_frame(p_child, true);
            p_child->propagate_renderer_(true);
        }

        // Remove shortcut to child
        std::string raw_name = p_removed_child->get_raw_name();
        if (utils::starts_with(raw_name, "$parent")) {
            raw_name.erase(0, std::string("$parent").size());
            sol::state& m_lua               = get_lua_();
            m_lua[get_lua_name()][raw_name] = sol::lua_nil;
        }
    }

    return p_removed_child;
}

frame::child_list_view frame::get_children() {
    return child_list_view(child_list_);
}

frame::const_child_list_view frame::get_children() const {
    return const_child_list_view(child_list_);
}

float frame::get_effective_alpha() const {
    if (p_parent_)
        return f_alpha_ * p_parent_->get_effective_alpha();
    else
        return f_alpha_;
}

float frame::get_effective_scale() const {
    if (p_parent_)
        return f_scale_ * p_parent_->get_effective_scale();
    else
        return f_scale_;
}

int frame::get_level() const {
    return level_;
}

frame_strata frame::get_frame_strata() const {
    return m_strata_;
}

utils::observer_ptr<const frame> frame::get_top_level_parent() const {
    auto p_frame = observer_from(this);
    do {
        if (p_frame->is_top_level())
            return p_frame;

        p_frame = p_frame->get_parent();
    } while (p_frame);

    return nullptr;
}

const backdrop* frame::get_backdrop() const {
    return p_backdrop_.get();
}

backdrop* frame::get_backdrop() {
    return p_backdrop_.get();
}

backdrop& frame::get_or_create_backdrop() {
    if (!p_backdrop_)
        p_backdrop_ = std::unique_ptr<backdrop>(new backdrop(*this));

    return *p_backdrop_;
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
    return vector2f(f_max_width_, f_max_height_);
}

vector2f frame::get_min_dimensions() const {
    return vector2f(f_min_width_, f_min_height_);
}

std::size_t frame::get_num_children() const {
    return std::count_if(child_list_.begin(), child_list_.end(), [](const auto& p_child) {
        return p_child != nullptr;
    });
}

std::size_t frame::get_rough_num_children() const {
    return child_list_.size();
}

std::size_t frame::get_num_regions() const {
    return std::count_if(region_list_.begin(), region_list_.end(), [](const auto& p_region) {
        return p_region != nullptr;
    });
}

std::size_t frame::get_rough_num_regions() const {
    return region_list_.size();
}

float frame::get_scale() const {
    return f_scale_;
}

bool frame::is_clamped_to_screen() const {
    return is_clamped_to_screen_;
}

bool frame::is_in_region(const vector2f& m_position) const {
    if (p_title_region_ && p_title_region_->is_in_region(m_position))
        return true;

    bool is_in_x_range = border_list_.left + abs_hit_rect_inset_list_.left <= m_position.x &&
                         m_position.x <= border_list_.right - abs_hit_rect_inset_list_.right - 1.0f;
    bool is_in_y_range =
        border_list_.top + abs_hit_rect_inset_list_.top <= m_position.y &&
        m_position.y <= border_list_.bottom - abs_hit_rect_inset_list_.bottom - 1.0f;

    return is_in_x_range && is_in_y_range;
}

utils::observer_ptr<const frame>
frame::find_topmost_frame(const std::function<bool(const frame&)>& m_predicate) const {
    if (m_predicate(*this))
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

bool frame::is_registered_for_drag(const std::string& button_name) const {
    return reg_drag_list_.find(button_name) != reg_drag_list_.end();
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

std::string hijack_sol_error_line(
    std::string original_message, const std::string& file, std::size_t ui_line_nbr) {
    auto ui_pos1 = original_message.find("[string \"" + file);
    if (ui_pos1 == std::string::npos)
        return original_message;

    auto ui_pos2 = original_message.find_first_of('"', ui_pos1 + 9);
    if (ui_pos2 == std::string::npos)
        return original_message;

    original_message.erase(ui_pos1, ui_pos2 - ui_pos1 + 2);
    original_message.insert(ui_pos1, file);

    auto ui_pos3 = original_message.find_first_of(':', ui_pos1 + file.size());
    if (ui_pos3 == std::string::npos)
        return original_message;

    auto ui_pos4 = original_message.find_first_of(":>", ui_pos3 + 1);
    if (ui_pos4 == std::string::npos)
        return original_message;

    std::size_t ui_offset = 0;
    if (!utils::from_string(original_message.substr(ui_pos3 + 1, ui_pos4 - ui_pos3 - 1), ui_offset))
        return original_message;

    original_message.erase(ui_pos3 + 1, ui_pos4 - ui_pos3 - 1);
    original_message.insert(ui_pos3 + 1, utils::to_string(ui_line_nbr + ui_offset - 1));
    ui_pos4 = original_message.find_first_of(':', ui_pos3 + 1);

    auto ui_pos5 = original_message.find("[string \"" + file, ui_pos4);
    if (ui_pos5 == std::string::npos)
        return original_message;

    std::string message = original_message.substr(ui_pos4 + 1);
    original_message.erase(ui_pos4 + 1);
    original_message += hijack_sol_error_line(message, file, ui_line_nbr);

    return original_message;
}

std::string hijack_sol_error_message(
    std::string_view original_message, const std::string& file, std::size_t ui_line_nbr) {
    std::string new_error;
    for (auto line : utils::cut(original_message, "\n")) {
        if (!new_error.empty())
            new_error += '\n';

        new_error += hijack_sol_error_line(std::string{line}, file, ui_line_nbr);
    }

    return new_error;
}

utils::connection frame::define_script_(
    const std::string& script_name,
    const std::string& content,
    bool               append,
    const script_info& m_info) {
    // Create the Lua function from the provided string
    sol::state& m_lua = get_lua_();

    std::string str = "return function(self";

    constexpr std::size_t ui_max_args = 9;
    for (std::size_t i = 0; i < ui_max_args; ++i)
        str += ", arg" + utils::to_string(i + 1);

    str += ") " + content + " end";

    auto m_result = m_lua.do_string(str, m_info.file_name);

    if (!m_result.valid()) {
        sol::error  m_error = m_result;
        std::string error =
            hijack_sol_error_message(m_error.what(), m_info.file_name, m_info.ui_line_nbr);

        gui::out << gui::error << error << std::endl;

        get_manager().get_event_emitter().fire_event("LUA_ERROR", {error});
        return {};
    }

    sol::protected_function m_handler = m_result;

    // Forward it as any other Lua function
    return define_script_(script_name, std::move(m_handler), append, m_info);
}

utils::connection frame::define_script_(
    const std::string&      script_name,
    sol::protected_function m_handler,
    bool                    append,
    const script_info&      m_info) {
    auto m_wrapped_handler = [m_handler = std::move(m_handler),
                              m_info](frame& m_self, const event_data& m_args) {
        sol::state& m_lua = m_self.get_manager().get_lua();
        lua_State*  p_lua = m_lua.lua_state();

        std::vector<sol::object> args;

        // Set arguments
        for (std::size_t i = 0; i < m_args.get_num_param(); ++i) {
            const utils::variant& m_arg = m_args.get(i);
            if (std::holds_alternative<utils::empty>(m_arg))
                args.emplace_back(sol::lua_nil);
            else
                args.emplace_back(p_lua, sol::in_place, m_arg);
        }

        // Get a reference to self
        sol::object m_self_lua = m_lua[m_self.get_lua_name()];
        if (m_self_lua == sol::lua_nil)
            throw gui::exception("Lua glue object is nil");

        // Call the function
        auto m_result = m_handler(m_self_lua, sol::as_args(args));
        // WARNING: after this point, the frame (mSelf) may be deleted.
        // Do not use any member variable or member function directly.

        // Handle errors
        if (!m_result.valid()) {
            sol::error  m_error = m_result;
            std::string error =
                hijack_sol_error_message(m_error.what(), m_info.file_name, m_info.ui_line_nbr);

            throw gui::exception(error);
        }
    };

    return define_script_(script_name, std::move(m_wrapped_handler), append, m_info);
}

utils::connection frame::define_script_(
    const std::string& script_name,
    script_function    m_handler,
    bool               append,
    const script_info& /*mInfo*/) {
    if (!is_virtual()) {
        // Register the function so it can be called directly from Lua
        std::string adjusted_name = get_adjusted_script_name(script_name);

        get_lua_()[get_lua_name()][adjusted_name].set_function(
            [=](frame& m_self, sol::variadic_args m_v_args) {
                event_data m_data;
                for (auto&& m_arg : m_v_args) {
                    lxgui::utils::variant m_variant;
                    if (!m_arg.is<sol::lua_nil_t>())
                        m_variant = m_arg;

                    m_data.add(std::move(m_variant));
                }

                m_self.fire_script(script_name, m_data);
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
    return handler_list.connect(std::move(m_handler));
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
        std::string adjusted_name                 = get_adjusted_script_name(script_name);
        get_lua_()[get_lua_name()][adjusted_name] = sol::lua_nil;
    }
}

void frame::on_event_(std::string_view event_name, const event_data& m_event) {
    event_data m_data;
    m_data.add(std::string(event_name));
    for (std::size_t i = 0; i < m_event.get_num_param(); ++i)
        m_data.add(m_event.get(i));

    fire_script("OnEvent", m_data);
}

void frame::fire_script(const std::string& script_name, const event_data& m_data) {
    if (!is_loaded())
        return;

    auto iter_h = signal_list_.find(script_name);
    if (iter_h == signal_list_.end())
        return;

    // Make a copy of useful pointers: in case the frame is deleted, we will need this
    auto&       m_event_emitter  = get_manager().get_event_emitter();
    auto&       m_addon_registry = *get_manager().get_addon_registry();
    const auto* p_old_add_on     = m_addon_registry.get_current_addon();
    m_addon_registry.set_current_addon(get_addon());

    try {
        // Call the handlers
        iter_h->second(*this, m_data);
    } catch (const std::exception& m_exception) {
        std::string error = m_exception.what();
        gui::out << gui::error << error << std::endl;
        m_event_emitter.fire_event("LUA_ERROR", {error});
    }

    m_addon_registry.set_current_addon(p_old_add_on);
}

void frame::register_event(const std::string& event_name) {
    if (is_virtual_)
        return;

    m_event_receiver_.register_event(
        event_name, [=](const event_data& m_event) { return on_event_(event_name, m_event); });
}

void frame::unregister_event(const std::string& event_name) {
    if (is_virtual_)
        return;

    m_event_receiver_.unregister_event(event_name);
}

void frame::register_for_drag(const std::vector<std::string>& button_list) {
    reg_drag_list_.clear();
    for (const auto& button : button_list)
        reg_drag_list_.insert(button);
}

void frame::set_clamped_to_screen(bool is_clamped_to_screen) {
    is_clamped_to_screen_ = is_clamped_to_screen;
}

void frame::set_frame_strata(frame_strata m_strata) {
    if (m_strata == frame_strata::parent) {
        if (!is_virtual_) {
            if (p_parent_)
                m_strata = p_parent_->get_frame_strata();
            else
                m_strata = frame_strata::medium;
        }
    }

    std::swap(m_strata_, m_strata);

    if (m_strata_ != m_strata && !is_virtual_) {
        get_top_level_renderer()->notify_frame_strata_changed(
            observer_from(this), m_strata, m_strata_);
    }
}

void frame::set_frame_strata(const std::string& strata_name) {
    frame_strata m_strata;

    if (strata_name == "BACKGROUND")
        m_strata = frame_strata::background;
    else if (strata_name == "LOW")
        m_strata = frame_strata::low;
    else if (strata_name == "MEDIUM")
        m_strata = frame_strata::medium;
    else if (strata_name == "HIGH")
        m_strata = frame_strata::high;
    else if (strata_name == "DIALOG")
        m_strata = frame_strata::dialog;
    else if (strata_name == "FULLSCREEN")
        m_strata = frame_strata::fullscreen;
    else if (strata_name == "FULLSCREEN_DIALOG")
        m_strata = frame_strata::fullscreen_dialog;
    else if (strata_name == "TOOLTIP")
        m_strata = frame_strata::tooltip;
    else if (strata_name == "PARENT") {
        if (is_virtual_) {
            m_strata = frame_strata::parent;
        } else {
            if (p_parent_)
                m_strata = p_parent_->get_frame_strata();
            else
                m_strata = frame_strata::medium;
        }
    } else {
        gui::out << gui::warning << "gui::" << type_.back()
                 << " : Unknown strata : \"" + strata_name + "\"." << std::endl;
        return;
    }

    set_frame_strata(m_strata);
}

void frame::set_backdrop(std::unique_ptr<backdrop> p_backdrop) {
    p_backdrop_ = std::move(p_backdrop);
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
        get_top_level_renderer()->notify_frame_level_changed(observer_from(this), level_id, level_);
    }
}

void frame::set_max_dimensions(const vector2f& m_max) {
    set_max_width(m_max.x);
    set_max_height(m_max.y);
}

void frame::set_min_dimensions(const vector2f& m_min) {
    set_min_width(m_min.x);
    set_min_height(m_min.y);
}

void frame::set_max_height(float f_max_height) {
    if (f_max_height < 0.0f)
        f_max_height = std::numeric_limits<float>::infinity();

    if (f_max_height >= f_min_height_)
        f_max_height_ = f_max_height;

    if (f_max_height_ != f_max_height && !is_virtual_)
        notify_borders_need_update();
}

void frame::set_max_width(float f_max_width) {
    if (f_max_width < 0.0f)
        f_max_width = std::numeric_limits<float>::infinity();

    if (f_max_width >= f_min_width_)
        f_max_width_ = f_max_width;

    if (f_max_width_ != f_max_width && !is_virtual_)
        notify_borders_need_update();
}

void frame::set_min_height(float f_min_height) {
    if (f_min_height <= f_max_height_)
        f_min_height_ = f_min_height;

    if (f_min_height_ != f_min_height && !is_virtual_)
        notify_borders_need_update();
}

void frame::set_min_width(float f_min_width) {
    if (f_min_width <= f_max_width_)
        f_min_width_ = f_min_width;

    if (f_min_width_ != f_min_width && !is_virtual_)
        notify_borders_need_update();
}

void frame::set_movable(bool is_movable) {
    is_movable_ = is_movable;
}

utils::owner_ptr<region> frame::release_from_parent() {
    utils::observer_ptr<frame> p_self = observer_from(this);
    if (p_parent_)
        return p_parent_->remove_child(p_self);
    else
        return get_manager().get_root().remove_root_frame(p_self);
}

void frame::set_resizable(bool is_resizable) {
    is_resizable_ = is_resizable;
}

void frame::set_scale(float f_scale) {
    f_scale_ = f_scale;
    if (f_scale_ != f_scale)
        notify_renderer_need_redraw();
}

void frame::set_top_level(bool is_top_level) {
    is_top_level_ = is_top_level;
}

void frame::raise() {
    if (!is_top_level_)
        return;

    int  old_level            = level_;
    auto p_top_level_renderer = get_top_level_renderer();
    level_                    = p_top_level_renderer->get_highest_level(m_strata_) + 1;

    if (level_ > old_level) {
        if (!is_virtual()) {
            p_top_level_renderer->notify_frame_level_changed(
                observer_from(this), old_level, level_);
        }

        int amount = level_ - old_level;

        for (auto& m_child : get_children())
            m_child.add_level_(amount);
    } else
        level_ = old_level;
}

void frame::enable_auto_focus(bool enable) {
    is_auto_focus_ = enable;
}

bool frame::is_auto_focus_enabled() const {
    return is_auto_focus_;
}

void frame::set_focus(bool focus) {
    auto& m_root = get_manager().get_root();
    if (focus)
        m_root.request_focus(observer_from(this));
    else
        m_root.release_focus(*this);
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
        get_top_level_renderer()->notify_frame_level_changed(
            observer_from(this), old_level, level_);
    }

    for (auto& m_child : get_children())
        m_child.add_level_(amount);
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

void frame::start_sizing(const anchor_point& m_point) {
    if (!is_resizable_)
        return;

    set_user_placed(true);
    get_manager().get_root().start_sizing(observer_from(this), m_point);
}

void frame::stop_sizing() {
    if (get_manager().get_root().is_sizing(*this))
        get_manager().get_root().stop_sizing();
}

void frame::propagate_renderer_(bool rendered) {
    auto p_top_level_renderer = get_top_level_renderer();
    for (const auto& p_child : child_list_) {
        if (!p_child)
            continue;

        if (!p_child->get_renderer())
            p_top_level_renderer->notify_rendered_frame(p_child, rendered);

        p_child->propagate_renderer_(rendered);
    }
}

void frame::set_renderer(utils::observer_ptr<frame_renderer> p_renderer) {
    if (p_renderer == p_renderer_)
        return;

    get_top_level_renderer()->notify_rendered_frame(observer_from(this), false);

    propagate_renderer_(false);

    p_renderer_ = std::move(p_renderer);

    get_top_level_renderer()->notify_rendered_frame(observer_from(this), true);

    propagate_renderer_(true);
}

utils::observer_ptr<const frame_renderer> frame::get_top_level_renderer() const {
    if (p_renderer_)
        return p_renderer_;
    else if (p_parent_)
        return p_parent_->get_top_level_renderer();
    else
        return get_manager().get_root().observer_from_this();
}

void frame::notify_visible() {
    alive_checker m_checker(*this);

    if (is_auto_focus_) {
        set_focus(true);
        if (!m_checker.is_alive())
            return;
    }

    base::notify_visible();

    for (auto& m_region : get_regions()) {
        if (m_region.is_shown()) {
            m_region.notify_visible();
            if (!m_checker.is_alive())
                return;
        }
    }

    for (auto& m_child : get_children()) {
        if (m_child.is_shown()) {
            m_child.notify_visible();
            if (!m_checker.is_alive())
                return;
        }
    }

    fire_script("OnShow");
    if (!m_checker.is_alive())
        return;

    notify_renderer_need_redraw();
}

void frame::notify_invisible() {
    alive_checker m_checker(*this);

    set_focus(false);
    if (!m_checker.is_alive())
        return;

    base::notify_invisible();

    for (auto& m_child : get_children()) {
        if (m_child.is_shown()) {
            m_child.notify_invisible();
            if (!m_checker.is_alive())
                return;
        }
    }

    fire_script("OnHide");
    if (!m_checker.is_alive())
        return;

    notify_renderer_need_redraw();
}

void frame::notify_renderer_need_redraw() {
    if (is_virtual_)
        return;

    get_top_level_renderer()->notify_strata_needs_redraw(m_strata_);
}

void frame::notify_scaling_factor_updated() {
    base::notify_scaling_factor_updated();

    if (p_title_region_)
        p_title_region_->notify_scaling_factor_updated();

    for (auto& m_child : get_children())
        m_child.notify_scaling_factor_updated();

    for (auto& m_region : get_regions())
        m_region.notify_scaling_factor_updated();
}

void frame::show() {
    if (is_shown_)
        return;

    bool was_visible = is_visible_;
    base::show();

    if (!was_visible)
        get_manager().get_root().notify_hovered_frame_dirty();
}

void frame::hide() {
    if (!is_shown_)
        return;

    bool was_visible = is_visible_;
    base::hide();

    if (was_visible)
        get_manager().get_root().notify_hovered_frame_dirty();
}

void frame::notify_mouse_in_frame(bool mouse_inframe, const vector2f& /*mPosition*/) {
    alive_checker m_checker(*this);

    if (mouse_inframe) {
        if (!is_mouse_in_frame_) {
            fire_script("OnEnter");
            if (!m_checker.is_alive())
                return;
        }

        is_mouse_in_frame_ = true;
    } else {
        if (is_mouse_in_frame_) {
            fire_script("OnLeave");
            if (!m_checker.is_alive())
                return;
        }

        is_mouse_in_frame_ = false;
    }
}

void frame::update_borders_() {
    const bool old_ready       = is_ready_;
    const auto old_border_list = border_list_;

    base::update_borders_();

    check_position_();

    if (border_list_ != old_border_list || is_ready_ != old_ready) {
        get_manager().get_root().notify_hovered_frame_dirty();
        if (p_backdrop_)
            p_backdrop_->notify_borders_updated();
    }
}

void frame::update(float f_delta) {
//#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

    alive_checker m_checker(*this);

    DEBUG_LOG("  ~");
    base::update(f_delta);
    DEBUG_LOG("   #");

    if (build_layer_list_flag_) {
        DEBUG_LOG("   Build layers");
        // Clear layers' content
        for (auto& m_layer : layer_list_)
            m_layer.region_list.clear();

        // Fill layers with regions (with font_string rendered last within the same layer)
        for (const auto& p_region : region_list_) {
            if (p_region && p_region->get_object_type() != "font_string")
                layer_list_[static_cast<std::size_t>(p_region->get_draw_layer())]
                    .region_list.push_back(p_region);
        }

        for (const auto& p_region : region_list_) {
            if (p_region && p_region->get_object_type() == "font_string")
                layer_list_[static_cast<std::size_t>(p_region->get_draw_layer())]
                    .region_list.push_back(p_region);
        }

        build_layer_list_flag_ = false;
    }

    if (is_visible()) {
        DEBUG_LOG("   On update");
        event_data m_data;
        m_data.add(f_delta);
        fire_script("OnUpdate", m_data);
        if (!m_checker.is_alive())
            return;
    }

    if (p_title_region_)
        p_title_region_->update(f_delta);

    // Update regions
    DEBUG_LOG("   Update regions");
    for (auto& m_region : get_regions())
        m_region.update(f_delta);

    // Remove deleted regions
    {
        auto m_iter_remove = std::remove_if(
            region_list_.begin(), region_list_.end(), [](auto& p_obj) { return p_obj == nullptr; });

        region_list_.erase(m_iter_remove, region_list_.end());
    }

    // Update children
    DEBUG_LOG("   Update children");
    for (auto& m_child : get_children()) {
        m_child.update(f_delta);
        if (!m_checker.is_alive())
            return;
    }

    // Remove deleted children
    {
        auto m_iter_remove = std::remove_if(
            child_list_.begin(), child_list_.end(), [](auto& p_obj) { return p_obj == nullptr; });

        child_list_.erase(m_iter_remove, child_list_.end());
    }

    // Remove empty handlers
    for (auto m_iter_list = signal_list_.begin(); m_iter_list != signal_list_.end();) {
        if (m_iter_list->second.empty())
            m_iter_list = signal_list_.erase(m_iter_list);
        else
            ++m_iter_list;
    }

    vector2f m_new_size = get_apparent_dimensions();
    if (m_old_size_ != m_new_size) {
        DEBUG_LOG("   On size changed");
        fire_script("OnSizeChanged");
        if (!m_checker.is_alive())
            return;

        m_old_size_ = m_new_size;
    }

    DEBUG_LOG("   .");
}

} // namespace lxgui::gui
