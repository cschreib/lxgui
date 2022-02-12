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
    l_type_.push_back(class_name);
}

frame::~frame() {
    // Disable callbacks
    l_signal_list_.clear();

    // Children must be destroyed first
    l_child_list_.clear();
    l_region_list_.clear();

    if (!b_virtual_) {
        // Tell the renderer to no longer render this region
        get_top_level_renderer()->notify_rendered_frame(observer_from(this), false);
        p_renderer_ = nullptr;
    }

    get_manager().get_root().notify_hovered_frame_dirty();

    set_focus(false);
}

void frame::render() const {
    if (!b_is_visible_ || !b_ready_)
        return;

    if (p_backdrop_)
        p_backdrop_->render();

    // Render child regions
    for (const auto& m_layer : l_layer_list_) {
        if (m_layer.b_disabled)
            continue;

        for (const auto& p_region : m_layer.l_region_list) {
            if (p_region->is_shown())
                p_region->render();
        }
    }
}

void frame::create_glue() {
    create_glue_(this);
}

std::string frame::serialize(const std::string& s_tab) const {
    std::ostringstream s_str;

    s_str << base::serialize(s_tab);
    if (auto p_frame_renderer = utils::dynamic_pointer_cast<frame>(p_renderer_))
        s_str << s_tab << "  # Man. render : " << p_frame_renderer->get_name() << "\n";
    s_str << s_tab << "  # Strata      : ";
    switch (m_strata_) {
    case frame_strata::parent: s_str << "PARENT\n"; break;
    case frame_strata::background: s_str << "BACKGROUND\n"; break;
    case frame_strata::low: s_str << "LOW\n"; break;
    case frame_strata::medium: s_str << "MEDIUM\n"; break;
    case frame_strata::high: s_str << "HIGH\n"; break;
    case frame_strata::dialog: s_str << "DIALOG\n"; break;
    case frame_strata::fullscreen: s_str << "FULLSCREEN\n"; break;
    case frame_strata::fullscreen_dialog: s_str << "FULLSCREEN_DIALOG\n"; break;
    case frame_strata::tooltip: s_str << "TOOLTIP\n"; break;
    }
    s_str << s_tab << "  # Level       : " << i_level_ << "\n";
    s_str << s_tab << "  # TopLevel    : " << b_is_top_level_;
    if (!b_is_top_level_ && get_top_level_parent())
        s_str << " (" << get_top_level_parent()->get_name() << ")\n";
    else
        s_str << "\n";
    if (!b_is_mouse_click_enabled_ && !b_is_mouse_move_enabled_ && !!b_is_mouse_wheel_enabled_)
        s_str << s_tab << "  # Inputs      : none\n";
    else {
        s_str << s_tab << "  # Inputs      :\n";
        s_str << s_tab << "  |-###\n";
        if (b_is_mouse_click_enabled_)
            s_str << s_tab << "  |   # mouse click\n";
        if (b_is_mouse_move_enabled_)
            s_str << s_tab << "  |   # mouse move\n";
        if (b_is_mouse_wheel_enabled_)
            s_str << s_tab << "  |   # mouse wheel\n";
        s_str << s_tab << "  |-###\n";
    }
    s_str << s_tab << "  # Movable     : " << b_is_movable_ << "\n";
    s_str << s_tab << "  # Resizable   : " << b_is_resizable_ << "\n";
    s_str << s_tab << "  # Clamped     : " << b_is_clamped_to_screen_ << "\n";
    s_str << s_tab << "  # HRect inset :\n";
    s_str << s_tab << "  |-###\n";
    s_str << s_tab << "  |   # left   : " << l_abs_hit_rect_inset_list_.left << "\n";
    s_str << s_tab << "  |   # right  : " << l_abs_hit_rect_inset_list_.right << "\n";
    s_str << s_tab << "  |   # top    : " << l_abs_hit_rect_inset_list_.top << "\n";
    s_str << s_tab << "  |   # bottom : " << l_abs_hit_rect_inset_list_.bottom << "\n";
    s_str << s_tab << "  |-###\n";
    s_str << s_tab << "  # Min width   : " << f_min_width_ << "\n";
    s_str << s_tab << "  # Max width   : " << f_max_width_ << "\n";
    s_str << s_tab << "  # Min height  : " << f_min_height_ << "\n";
    s_str << s_tab << "  # Max height  : " << f_max_height_ << "\n";
    s_str << s_tab << "  # Scale       : " << f_scale_ << "\n";
    if (p_title_region_) {
        s_str << s_tab << "  # Title reg.  :\n";
        s_str << s_tab << "  |-###\n";
        s_str << p_title_region_->serialize(s_tab + "  | ");
        s_str << s_tab << "  |-###\n";
    }
    if (p_backdrop_) {
        const bounds2f& l_insets = p_backdrop_->get_background_insets();

        s_str << s_tab << "  # Backdrop    :\n";
        s_str << s_tab << "  |-###\n";
        s_str << s_tab << "  |   # Background : " << p_backdrop_->get_background_file() << "\n";
        s_str << s_tab << "  |   # Tilling    : " << p_backdrop_->is_background_tilling() << "\n";
        if (p_backdrop_->is_background_tilling())
            s_str << s_tab << "  |   # Tile size  : " << p_backdrop_->get_tile_size() << "\n";
        s_str << s_tab << "  |   # BG Insets  :\n";
        s_str << s_tab << "  |   |-###\n";
        s_str << s_tab << "  |   |   # left   : " << l_insets.left << "\n";
        s_str << s_tab << "  |   |   # right  : " << l_insets.right << "\n";
        s_str << s_tab << "  |   |   # top    : " << l_insets.top << "\n";
        s_str << s_tab << "  |   |   # bottom : " << l_insets.bottom << "\n";
        s_str << s_tab << "  |   |-###\n";
        s_str << s_tab << "  |   # Edge       : " << p_backdrop_->get_edge_file() << "\n";
        s_str << s_tab << "  |   # Edge size  : " << p_backdrop_->get_edge_size() << "\n";
        s_str << s_tab << "  |-###\n";
    }

    if (!l_region_list_.empty()) {
        if (l_child_list_.size() == 1)
            s_str << s_tab << "  # Region : \n";
        else
            s_str << s_tab << "  # Regions     : " << l_region_list_.size() << "\n";
        s_str << s_tab << "  |-###\n";

        for (auto& m_region : get_regions()) {
            s_str << m_region.serialize(s_tab + "  | ");
            s_str << s_tab << "  |-###\n";
        }
    }

    if (!l_child_list_.empty()) {
        if (l_child_list_.size() == 1)
            s_str << s_tab << "  # Child : \n";
        else
            s_str << s_tab << "  # Children    : " << l_child_list_.size() << "\n";
        s_str << s_tab << "  |-###\n";

        for (const auto& m_child : get_children()) {
            s_str << m_child.serialize(s_tab + "  | ");
            s_str << s_tab << "  |-###\n";
        }
    }

    return s_str.str();
}

bool frame::can_use_script(const std::string& s_script_name) const {
    return (s_script_name == "OnChar") || (s_script_name == "OnDragStart") ||
           (s_script_name == "OnDragStop") || (s_script_name == "OnDragMove") ||
           (s_script_name == "OnEnter") || (s_script_name == "OnEvent") ||
           (s_script_name == "OnFocusGained") || (s_script_name == "OnFocusLost") ||
           (s_script_name == "OnHide") || (s_script_name == "OnKeyDown") ||
           (s_script_name == "OnKeyUp") || (s_script_name == "OnLeave") ||
           (s_script_name == "OnLoad") || (s_script_name == "OnMouseDown") ||
           (s_script_name == "OnMouseUp") || (s_script_name == "OnDoubleClick") ||
           (s_script_name == "OnMouseWheel") || (s_script_name == "OnReceiveDrag") ||
           (s_script_name == "OnShow") || (s_script_name == "OnSizeChanged") ||
           (s_script_name == "OnUpdate");
}

void frame::copy_from(const region& m_obj) {
    base::copy_from(m_obj);

    const frame* p_frame = down_cast<frame>(&m_obj);
    if (!p_frame)
        return;

    for (const auto& m_item : p_frame->l_signal_list_) {
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

    for (const auto& p_art : p_frame->l_region_list_) {
        if (!p_art || p_art->is_special())
            continue;

        region_core_attributes m_attr;
        m_attr.s_object_type = p_art->get_object_type();
        m_attr.s_name        = p_art->get_raw_name();
        m_attr.l_inheritance = {p_art};

        auto p_new_art = create_layered_region(p_art->get_draw_layer(), std::move(m_attr));
        if (!p_new_art)
            continue;

        p_new_art->notify_loaded();
    }

    b_build_layer_list_ = true;

    if (p_frame->p_backdrop_) {
        p_backdrop_ = std::unique_ptr<backdrop>(new backdrop(*this));
        p_backdrop_->copy_from(*p_frame->p_backdrop_);
    }

    if (p_frame->p_title_region_) {
        this->create_title_region();
        if (p_title_region_)
            p_title_region_->copy_from(*p_frame->p_title_region_);
    }

    for (const auto& p_child : p_frame->l_child_list_) {
        if (!p_child || p_child->is_special())
            continue;

        region_core_attributes m_attr;
        m_attr.s_object_type = p_child->get_object_type();
        m_attr.s_name        = p_child->get_raw_name();
        m_attr.l_inheritance = {p_child};

        auto p_new_child = create_child(std::move(m_attr));
        if (!p_new_child)
            continue;

        p_new_child->notify_loaded();
    }
}

void frame::create_title_region() {
    if (p_title_region_) {
        gui::out << gui::warning << "gui::" << l_type_.back()
                 << " : \"" + s_name_ + "\" already has a title region." << std::endl;
        return;
    }

    region_core_attributes m_attr;
    m_attr.s_object_type = "Region";
    m_attr.b_virtual     = is_virtual();
    m_attr.s_name        = "$parentTitleRegion";
    m_attr.p_parent      = observer_from(this);

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

utils::observer_ptr<const frame> frame::get_child(const std::string& s_name) const {
    for (const auto& p_child : l_child_list_) {
        if (!p_child)
            continue;

        if (p_child->get_name() == s_name)
            return p_child;

        const std::string& s_raw_name = p_child->get_raw_name();
        if (utils::starts_with(s_raw_name, "$parent") && s_raw_name.substr(7) == s_name)
            return p_child;
    }

    return nullptr;
}

frame::region_list_view frame::get_regions() {
    return region_list_view(l_region_list_);
}

frame::const_region_list_view frame::get_regions() const {
    return const_region_list_view(l_region_list_);
}

utils::observer_ptr<const layered_region> frame::get_region(const std::string& s_name) const {
    for (const auto& p_region : l_region_list_) {
        if (!p_region)
            continue;

        if (p_region->get_name() == s_name)
            return p_region;

        const std::string& s_raw_name = p_region->get_raw_name();
        if (utils::starts_with(s_raw_name, "$parent") && s_raw_name.substr(7) == s_name)
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
    if (l_border_list_.right - l_border_list_.left < f_min_width_) {
        l_border_list_.right = l_border_list_.left + f_min_width_;
    } else if (l_border_list_.right - l_border_list_.left > f_max_width_) {
        l_border_list_.right = l_border_list_.left + f_max_width_;
    }

    if (l_border_list_.bottom - l_border_list_.top < f_min_height_) {
        l_border_list_.bottom = l_border_list_.top + f_min_height_;
    } else if (l_border_list_.bottom - l_border_list_.top > f_max_height_) {
        l_border_list_.bottom = l_border_list_.top + f_max_height_;
    }

    if (b_is_clamped_to_screen_) {
        vector2f m_screen_dimensions = get_top_level_renderer()->get_target_dimensions();

        if (l_border_list_.right > m_screen_dimensions.x) {
            float f_width = l_border_list_.right - l_border_list_.left;
            if (f_width > m_screen_dimensions.x) {
                l_border_list_.left  = 0;
                l_border_list_.right = m_screen_dimensions.x;
            } else {
                l_border_list_.right = m_screen_dimensions.x;
                l_border_list_.left  = m_screen_dimensions.x - f_width;
            }
        }

        if (l_border_list_.left < 0) {
            float f_width = l_border_list_.right - l_border_list_.left;
            if (l_border_list_.right - l_border_list_.left > m_screen_dimensions.x) {
                l_border_list_.left  = 0;
                l_border_list_.right = m_screen_dimensions.x;
            } else {
                l_border_list_.left  = 0;
                l_border_list_.right = f_width;
            }
        }

        if (l_border_list_.bottom > m_screen_dimensions.y) {
            float f_height = l_border_list_.bottom - l_border_list_.top;
            if (f_height > m_screen_dimensions.y) {
                l_border_list_.top    = 0;
                l_border_list_.bottom = m_screen_dimensions.y;
            } else {
                l_border_list_.bottom = m_screen_dimensions.y;
                l_border_list_.top    = m_screen_dimensions.y - f_height;
            }
        }

        if (l_border_list_.top < 0) {
            float f_height = l_border_list_.bottom - l_border_list_.top;
            if (f_height > m_screen_dimensions.y) {
                l_border_list_.top    = 0;
                l_border_list_.bottom = m_screen_dimensions.y;
            } else {
                l_border_list_.top    = 0;
                l_border_list_.bottom = f_height;
            }
        }
    }
}

void frame::disable_draw_layer(layer m_layer_id) {
    layer_container& m_layer = l_layer_list_[static_cast<std::size_t>(m_layer_id)];
    if (!m_layer.b_disabled) {
        m_layer.b_disabled = true;
        notify_renderer_need_redraw();
    }
}

void frame::enable_draw_layer(layer m_layer_id) {
    layer_container& m_layer = l_layer_list_[static_cast<std::size_t>(m_layer_id)];
    if (!m_layer.b_disabled) {
        m_layer.b_disabled = false;
        notify_renderer_need_redraw();
    }
}

void frame::enable_mouse(bool b_is_mouse_enabled) {
    enable_mouse_click(b_is_mouse_enabled);
    enable_mouse_move(b_is_mouse_enabled);
}

void frame::enable_mouse_click(bool b_is_mouse_enabled) {
    b_is_mouse_click_enabled_ = b_is_mouse_enabled;
}

void frame::enable_mouse_move(bool b_is_mouse_enabled) {
    b_is_mouse_move_enabled_ = b_is_mouse_enabled;
}

void frame::enable_mouse_wheel(bool b_is_mouse_wheel_enabled) {
    b_is_mouse_wheel_enabled_ = b_is_mouse_wheel_enabled;
}

void frame::enable_key_capture(const std::string& s_key, bool b_is_capture_enabled) {
    if (b_is_capture_enabled)
        l_reg_key_list_.erase(s_key);
    else
        l_reg_key_list_.insert(s_key);
}

void frame::notify_loaded() {
    base::notify_loaded();

    if (!b_virtual_) {
        alive_checker m_checker(*this);
        fire_script("OnLoad");
        if (!m_checker.is_alive())
            return;
    }
}

void frame::notify_layers_need_update() {
    b_build_layer_list_ = true;
}

bool frame::has_script(const std::string& s_script_name) const {
    const auto m_iter = l_signal_list_.find(s_script_name);
    if (m_iter == l_signal_list_.end())
        return false;

    return !m_iter->second.empty();
}

utils::observer_ptr<layered_region> frame::add_region(utils::owner_ptr<layered_region> p_region) {
    if (!p_region)
        return nullptr;

    p_region->set_parent_(observer_from(this));

    utils::observer_ptr<layered_region> p_added_region = p_region;
    l_region_list_.push_back(std::move(p_region));

    notify_layers_need_update();
    notify_renderer_need_redraw();

    if (!b_virtual_) {
        // Add shortcut to region as entry in Lua table
        std::string s_raw_name = p_added_region->get_raw_name();
        if (utils::starts_with(s_raw_name, "$parent")) {
            s_raw_name.erase(0, std::string("$parent").size());
            auto& m_lua                       = get_lua_();
            m_lua[get_lua_name()][s_raw_name] = m_lua[p_added_region->get_lua_name()];
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
        utils::find_if(l_region_list_, [&](auto& p_obj) { return p_obj.get() == p_raw_pointer; });

    if (m_iter == l_region_list_.end()) {
        gui::out << gui::warning << "gui::" << l_type_.back() << " : "
                 << "Trying to remove \"" << p_region->get_name() << "\" from \"" << s_name_
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

    if (!b_virtual_) {
        // Remove shortcut to region
        std::string s_raw_name = p_removed_region->get_raw_name();
        if (utils::starts_with(s_raw_name, "$parent")) {
            s_raw_name.erase(0, std::string("$parent").size());
            sol::state& m_lua                 = get_lua_();
            m_lua[get_lua_name()][s_raw_name] = sol::lua_nil;
        }
    }

    return p_removed_region;
}

utils::observer_ptr<layered_region>
frame::create_layered_region(layer m_layer, region_core_attributes m_attr) {
    m_attr.b_virtual = is_virtual();
    m_attr.p_parent  = observer_from(this);

    auto p_region = get_manager().get_factory().create_layered_region(get_registry(), m_attr);

    if (!p_region)
        return nullptr;

    p_region->set_draw_layer(m_layer);

    return add_region(std::move(p_region));
}

utils::observer_ptr<frame> frame::create_child(region_core_attributes m_attr) {
    m_attr.b_virtual = is_virtual();
    m_attr.p_parent  = observer_from(this);

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
    l_child_list_.push_back(std::move(p_child));

    if (!b_virtual_) {
        utils::observer_ptr<frame_renderer> p_old_top_level_renderer =
            p_added_child->get_top_level_renderer();
        utils::observer_ptr<frame_renderer> p_new_top_level_renderer = get_top_level_renderer();
        if (p_old_top_level_renderer != p_new_top_level_renderer) {
            p_old_top_level_renderer->notify_rendered_frame(p_added_child, false);
            p_new_top_level_renderer->notify_rendered_frame(p_added_child, true);
        }

        // Add shortcut to child as entry in Lua table
        std::string s_raw_name = p_added_child->get_raw_name();
        if (utils::starts_with(s_raw_name, "$parent")) {
            s_raw_name.erase(0, std::string("$parent").size());
            auto& m_lua                       = get_lua_();
            m_lua[get_lua_name()][s_raw_name] = m_lua[p_added_child->get_lua_name()];
        }
    }

    return p_added_child;
}

utils::owner_ptr<frame> frame::remove_child(const utils::observer_ptr<frame>& p_child) {
    if (!p_child)
        return nullptr;

    frame* p_raw_pointer = p_child.get();
    auto   m_iter =
        utils::find_if(l_child_list_, [&](auto& p_obj) { return p_obj.get() == p_raw_pointer; });

    if (m_iter == l_child_list_.end()) {
        gui::out << gui::warning << "gui::" << l_type_.back() << " : "
                 << "Trying to remove \"" << p_child->get_name() << "\" from \"" << s_name_
                 << "\"'s children, but it was not one of this frame's children." << std::endl;
        return nullptr;
    }

    // NB: the iterator is not removed yet; it will be removed later in update().
    auto p_removed_child = std::move(*m_iter);

    bool b_notify_renderer = false;
    if (!b_virtual_) {
        utils::observer_ptr<frame_renderer> p_top_level_renderer = get_top_level_renderer();
        b_notify_renderer =
            !p_child->get_renderer() && p_top_level_renderer.get() != &get_manager().get_root();
        if (b_notify_renderer) {
            p_top_level_renderer->notify_rendered_frame(p_child, false);
            p_child->propagate_renderer_(false);
        }
    }

    p_removed_child->set_parent_(nullptr);

    if (!b_virtual_) {
        if (b_notify_renderer) {
            get_manager().get_root().notify_rendered_frame(p_child, true);
            p_child->propagate_renderer_(true);
        }

        // Remove shortcut to child
        std::string s_raw_name = p_removed_child->get_raw_name();
        if (utils::starts_with(s_raw_name, "$parent")) {
            s_raw_name.erase(0, std::string("$parent").size());
            sol::state& m_lua                 = get_lua_();
            m_lua[get_lua_name()][s_raw_name] = sol::lua_nil;
        }
    }

    return p_removed_child;
}

frame::child_list_view frame::get_children() {
    return child_list_view(l_child_list_);
}

frame::const_child_list_view frame::get_children() const {
    return const_child_list_view(l_child_list_);
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
    return i_level_;
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
    return l_type_.back();
}

const bounds2f& frame::get_abs_hit_rect_insets() const {
    return l_abs_hit_rect_inset_list_;
}

const bounds2f& frame::get_rel_hit_rect_insets() const {
    return l_rel_hit_rect_inset_list_;
}

vector2f frame::get_max_dimensions() const {
    return vector2f(f_max_width_, f_max_height_);
}

vector2f frame::get_min_dimensions() const {
    return vector2f(f_min_width_, f_min_height_);
}

std::size_t frame::get_num_children() const {
    return std::count_if(l_child_list_.begin(), l_child_list_.end(), [](const auto& p_child) {
        return p_child != nullptr;
    });
}

std::size_t frame::get_rough_num_children() const {
    return l_child_list_.size();
}

std::size_t frame::get_num_regions() const {
    return std::count_if(l_region_list_.begin(), l_region_list_.end(), [](const auto& p_region) {
        return p_region != nullptr;
    });
}

std::size_t frame::get_rough_num_regions() const {
    return l_region_list_.size();
}

float frame::get_scale() const {
    return f_scale_;
}

bool frame::is_clamped_to_screen() const {
    return b_is_clamped_to_screen_;
}

bool frame::is_in_region(const vector2f& m_position) const {
    if (p_title_region_ && p_title_region_->is_in_region(m_position))
        return true;

    bool b_is_in_x_range =
        l_border_list_.left + l_abs_hit_rect_inset_list_.left <= m_position.x &&
        m_position.x <= l_border_list_.right - l_abs_hit_rect_inset_list_.right - 1.0f;
    bool b_is_in_y_range =
        l_border_list_.top + l_abs_hit_rect_inset_list_.top <= m_position.y &&
        m_position.y <= l_border_list_.bottom - l_abs_hit_rect_inset_list_.bottom - 1.0f;

    return b_is_in_x_range && b_is_in_y_range;
}

utils::observer_ptr<const frame>
frame::find_topmost_frame(const std::function<bool(const frame&)>& m_predicate) const {
    if (m_predicate(*this))
        return observer_from(this);

    return nullptr;
}

bool frame::is_mouse_click_enabled() const {
    return b_is_mouse_click_enabled_;
}

bool frame::is_mouse_move_enabled() const {
    return b_is_mouse_move_enabled_;
}

bool frame::is_mouse_wheel_enabled() const {
    return b_is_mouse_wheel_enabled_;
}

bool frame::is_registered_for_drag(const std::string& s_button) const {
    return l_reg_drag_list_.find(s_button) != l_reg_drag_list_.end();
}

bool frame::is_key_capture_enabled(const std::string& s_key) const {
    return l_reg_key_list_.find(s_key) != l_reg_key_list_.end();
}

bool frame::is_movable() const {
    return b_is_movable_;
}

bool frame::is_resizable() const {
    return b_is_resizable_;
}

bool frame::is_top_level() const {
    return b_is_top_level_;
}

bool frame::is_user_placed() const {
    return b_is_user_placed_;
}

std::string frame::get_adjusted_script_name(const std::string& s_script_name) {
    std::string s_adjusted_name = s_script_name;
    for (auto iter = s_adjusted_name.begin(); iter != s_adjusted_name.end(); ++iter) {
        if ('A' <= *iter && *iter <= 'Z') {
            *iter = std::tolower(*iter);
            if (iter != s_adjusted_name.begin())
                iter = s_adjusted_name.insert(iter, '_');
        }
    }

    return s_adjusted_name;
}

std::string hijack_sol_error_line(
    std::string s_original_message, const std::string& s_file, std::size_t ui_line_nbr) {
    auto ui_pos1 = s_original_message.find("[string \"" + s_file);
    if (ui_pos1 == std::string::npos)
        return s_original_message;

    auto ui_pos2 = s_original_message.find_first_of('"', ui_pos1 + 9);
    if (ui_pos2 == std::string::npos)
        return s_original_message;

    s_original_message.erase(ui_pos1, ui_pos2 - ui_pos1 + 2);
    s_original_message.insert(ui_pos1, s_file);

    auto ui_pos3 = s_original_message.find_first_of(':', ui_pos1 + s_file.size());
    if (ui_pos3 == std::string::npos)
        return s_original_message;

    auto ui_pos4 = s_original_message.find_first_of(":>", ui_pos3 + 1);
    if (ui_pos4 == std::string::npos)
        return s_original_message;

    std::size_t ui_offset = 0;
    if (!utils::from_string(
            s_original_message.substr(ui_pos3 + 1, ui_pos4 - ui_pos3 - 1), ui_offset))
        return s_original_message;

    s_original_message.erase(ui_pos3 + 1, ui_pos4 - ui_pos3 - 1);
    s_original_message.insert(ui_pos3 + 1, utils::to_string(ui_line_nbr + ui_offset - 1));
    ui_pos4 = s_original_message.find_first_of(':', ui_pos3 + 1);

    auto ui_pos5 = s_original_message.find("[string \"" + s_file, ui_pos4);
    if (ui_pos5 == std::string::npos)
        return s_original_message;

    std::string s_message = s_original_message.substr(ui_pos4 + 1);
    s_original_message.erase(ui_pos4 + 1);
    s_original_message += hijack_sol_error_line(s_message, s_file, ui_line_nbr);

    return s_original_message;
}

std::string hijack_sol_error_message(
    std::string_view s_original_message, const std::string& s_file, std::size_t ui_line_nbr) {
    std::string s_new_error;
    for (auto s_line : utils::cut(s_original_message, "\n")) {
        if (!s_new_error.empty())
            s_new_error += '\n';

        s_new_error += hijack_sol_error_line(std::string{s_line}, s_file, ui_line_nbr);
    }

    return s_new_error;
}

utils::connection frame::define_script_(
    const std::string& s_script_name,
    const std::string& s_content,
    bool               b_append,
    const script_info& m_info) {
    // Create the Lua function from the provided string
    sol::state& m_lua = get_lua_();

    std::string s_str = "return function(self";

    constexpr std::size_t ui_max_args = 9;
    for (std::size_t i = 0; i < ui_max_args; ++i)
        s_str += ", arg" + utils::to_string(i + 1);

    s_str += ") " + s_content + " end";

    auto m_result = m_lua.do_string(s_str, m_info.s_file_name);

    if (!m_result.valid()) {
        sol::error  m_error = m_result;
        std::string s_error =
            hijack_sol_error_message(m_error.what(), m_info.s_file_name, m_info.ui_line_nbr);

        gui::out << gui::error << s_error << std::endl;

        get_manager().get_event_emitter().fire_event("LUA_ERROR", {s_error});
        return {};
    }

    sol::protected_function m_handler = m_result;

    // Forward it as any other Lua function
    return define_script_(s_script_name, std::move(m_handler), b_append, m_info);
}

utils::connection frame::define_script_(
    const std::string&      s_script_name,
    sol::protected_function m_handler,
    bool                    b_append,
    const script_info&      m_info) {
    auto m_wrapped_handler = [m_handler = std::move(m_handler),
                              m_info](frame& m_self, const event_data& m_args) {
        sol::state& m_lua = m_self.get_manager().get_lua();
        lua_State*  p_lua = m_lua.lua_state();

        std::vector<sol::object> l_args;

        // Set arguments
        for (std::size_t i = 0; i < m_args.get_num_param(); ++i) {
            const utils::variant& m_arg = m_args.get(i);
            if (std::holds_alternative<utils::empty>(m_arg))
                l_args.emplace_back(sol::lua_nil);
            else
                l_args.emplace_back(p_lua, sol::in_place, m_arg);
        }

        // Get a reference to self
        sol::object m_self_lua = m_lua[m_self.get_lua_name()];
        if (m_self_lua == sol::lua_nil)
            throw gui::exception("Lua glue object is nil");

        // Call the function
        auto m_result = m_handler(m_self_lua, sol::as_args(l_args));
        // WARNING: after this point, the frame (mSelf) may be deleted.
        // Do not use any member variable or member function directly.

        // Handle errors
        if (!m_result.valid()) {
            sol::error  m_error = m_result;
            std::string s_error =
                hijack_sol_error_message(m_error.what(), m_info.s_file_name, m_info.ui_line_nbr);

            throw gui::exception(s_error);
        }
    };

    return define_script_(s_script_name, std::move(m_wrapped_handler), b_append, m_info);
}

utils::connection frame::define_script_(
    const std::string& s_script_name,
    script_function    m_handler,
    bool               b_append,
    const script_info& /*mInfo*/) {
    if (!is_virtual()) {
        // Register the function so it can be called directly from Lua
        std::string s_adjusted_name = get_adjusted_script_name(s_script_name);

        get_lua_()[get_lua_name()][s_adjusted_name].set_function(
            [=](frame& m_self, sol::variadic_args m_v_args) {
                event_data m_data;
                for (auto&& m_arg : m_v_args) {
                    lxgui::utils::variant m_variant;
                    if (!m_arg.is<sol::lua_nil_t>())
                        m_variant = m_arg;

                    m_data.add(std::move(m_variant));
                }

                m_self.fire_script(s_script_name, m_data);
            });
    }

    auto& l_handler_list = l_signal_list_[s_script_name];
    if (!b_append) {
        // Just disable existing scripts, it may not be safe to modify the handler list
        // if this script is being defined during a handler execution.
        // They will be deleted later, when we know it is safe.
        l_handler_list.disconnect_all();
    }

    // TODO: add file/line info if the handler comes from C++
    // https://github.com/cschreib/lxgui/issues/96
    return l_handler_list.connect(std::move(m_handler));
}

script_list_view frame::get_script(const std::string& s_script_name) const {
    auto iter_h = l_signal_list_.find(s_script_name);
    if (iter_h == l_signal_list_.end())
        throw gui::exception(l_type_.back(), "no script registered for " + s_script_name);

    return iter_h->second.slots();
}

void frame::remove_script(const std::string& s_script_name) {
    auto iter_h = l_signal_list_.find(s_script_name);
    if (iter_h == l_signal_list_.end())
        return;

    // Just disable existing scripts, it may not be safe to modify the handler list
    // if this script is being defined during a handler execution.
    // They will be deleted later, when we know it is safe.
    iter_h->second.disconnect_all();

    if (!is_virtual()) {
        std::string s_adjusted_name                 = get_adjusted_script_name(s_script_name);
        get_lua_()[get_lua_name()][s_adjusted_name] = sol::lua_nil;
    }
}

void frame::on_event_(std::string_view s_event_name, const event_data& m_event) {
    event_data m_data;
    m_data.add(std::string(s_event_name));
    for (std::size_t i = 0; i < m_event.get_num_param(); ++i)
        m_data.add(m_event.get(i));

    fire_script("OnEvent", m_data);
}

void frame::fire_script(const std::string& s_script_name, const event_data& m_data) {
    if (!is_loaded())
        return;

    auto iter_h = l_signal_list_.find(s_script_name);
    if (iter_h == l_signal_list_.end())
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
        std::string s_error = m_exception.what();
        gui::out << gui::error << s_error << std::endl;
        m_event_emitter.fire_event("LUA_ERROR", {s_error});
    }

    m_addon_registry.set_current_addon(p_old_add_on);
}

void frame::register_event(const std::string& s_event_name) {
    if (b_virtual_)
        return;

    m_event_receiver_.register_event(
        s_event_name, [=](const event_data& m_event) { return on_event_(s_event_name, m_event); });
}

void frame::unregister_event(const std::string& s_event_name) {
    if (b_virtual_)
        return;

    m_event_receiver_.unregister_event(s_event_name);
}

void frame::register_for_drag(const std::vector<std::string>& l_button_list) {
    l_reg_drag_list_.clear();
    for (const auto& s_button : l_button_list)
        l_reg_drag_list_.insert(s_button);
}

void frame::set_clamped_to_screen(bool b_is_clamped_to_screen) {
    b_is_clamped_to_screen_ = b_is_clamped_to_screen;
}

void frame::set_frame_strata(frame_strata m_strata) {
    if (m_strata == frame_strata::parent) {
        if (!b_virtual_) {
            if (p_parent_)
                m_strata = p_parent_->get_frame_strata();
            else
                m_strata = frame_strata::medium;
        }
    }

    std::swap(m_strata_, m_strata);

    if (m_strata_ != m_strata && !b_virtual_) {
        get_top_level_renderer()->notify_frame_strata_changed(
            observer_from(this), m_strata, m_strata_);
    }
}

void frame::set_frame_strata(const std::string& s_strata) {
    frame_strata m_strata;

    if (s_strata == "BACKGROUND")
        m_strata = frame_strata::background;
    else if (s_strata == "LOW")
        m_strata = frame_strata::low;
    else if (s_strata == "MEDIUM")
        m_strata = frame_strata::medium;
    else if (s_strata == "HIGH")
        m_strata = frame_strata::high;
    else if (s_strata == "DIALOG")
        m_strata = frame_strata::dialog;
    else if (s_strata == "FULLSCREEN")
        m_strata = frame_strata::fullscreen;
    else if (s_strata == "FULLSCREEN_DIALOG")
        m_strata = frame_strata::fullscreen_dialog;
    else if (s_strata == "TOOLTIP")
        m_strata = frame_strata::tooltip;
    else if (s_strata == "PARENT") {
        if (b_virtual_) {
            m_strata = frame_strata::parent;
        } else {
            if (p_parent_)
                m_strata = p_parent_->get_frame_strata();
            else
                m_strata = frame_strata::medium;
        }
    } else {
        gui::out << gui::warning << "gui::" << l_type_.back()
                 << " : Unknown strata : \"" + s_strata + "\"." << std::endl;
        return;
    }

    set_frame_strata(m_strata);
}

void frame::set_backdrop(std::unique_ptr<backdrop> p_backdrop) {
    p_backdrop_ = std::move(p_backdrop);
    notify_renderer_need_redraw();
}

void frame::set_abs_hit_rect_insets(const bounds2f& l_insets) {
    l_abs_hit_rect_inset_list_ = l_insets;
}

void frame::set_rel_hit_rect_insets(const bounds2f& l_insets) {
    l_rel_hit_rect_inset_list_ = l_insets;
}

void frame::set_level(int i_level) {
    if (i_level == i_level_)
        return;

    std::swap(i_level, i_level_);

    if (!b_virtual_) {
        get_top_level_renderer()->notify_frame_level_changed(
            observer_from(this), i_level, i_level_);
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

    if (f_max_height_ != f_max_height && !b_virtual_)
        notify_borders_need_update();
}

void frame::set_max_width(float f_max_width) {
    if (f_max_width < 0.0f)
        f_max_width = std::numeric_limits<float>::infinity();

    if (f_max_width >= f_min_width_)
        f_max_width_ = f_max_width;

    if (f_max_width_ != f_max_width && !b_virtual_)
        notify_borders_need_update();
}

void frame::set_min_height(float f_min_height) {
    if (f_min_height <= f_max_height_)
        f_min_height_ = f_min_height;

    if (f_min_height_ != f_min_height && !b_virtual_)
        notify_borders_need_update();
}

void frame::set_min_width(float f_min_width) {
    if (f_min_width <= f_max_width_)
        f_min_width_ = f_min_width;

    if (f_min_width_ != f_min_width && !b_virtual_)
        notify_borders_need_update();
}

void frame::set_movable(bool b_is_movable) {
    b_is_movable_ = b_is_movable;
}

utils::owner_ptr<region> frame::release_from_parent() {
    utils::observer_ptr<frame> p_self = observer_from(this);
    if (p_parent_)
        return p_parent_->remove_child(p_self);
    else
        return get_manager().get_root().remove_root_frame(p_self);
}

void frame::set_resizable(bool b_is_resizable) {
    b_is_resizable_ = b_is_resizable;
}

void frame::set_scale(float f_scale) {
    f_scale_ = f_scale;
    if (f_scale_ != f_scale)
        notify_renderer_need_redraw();
}

void frame::set_top_level(bool b_is_top_level) {
    b_is_top_level_ = b_is_top_level;
}

void frame::raise() {
    if (!b_is_top_level_)
        return;

    int  i_old_level          = i_level_;
    auto p_top_level_renderer = get_top_level_renderer();
    i_level_                  = p_top_level_renderer->get_highest_level(m_strata_) + 1;

    if (i_level_ > i_old_level) {
        if (!is_virtual()) {
            p_top_level_renderer->notify_frame_level_changed(
                observer_from(this), i_old_level, i_level_);
        }

        int i_amount = i_level_ - i_old_level;

        for (auto& m_child : get_children())
            m_child.add_level_(i_amount);
    } else
        i_level_ = i_old_level;
}

void frame::enable_auto_focus(bool b_enable) {
    b_auto_focus_ = b_enable;
}

bool frame::is_auto_focus_enabled() const {
    return b_auto_focus_;
}

void frame::set_focus(bool b_focus) {
    auto& m_root = get_manager().get_root();
    if (b_focus)
        m_root.request_focus(observer_from(this));
    else
        m_root.release_focus(*this);
}

bool frame::has_focus() const {
    return b_focus_;
}

void frame::notify_focus(bool b_focus) {
    if (b_focus_ == b_focus)
        return;

    b_focus_ = b_focus;

    if (b_focus_)
        fire_script("OnFocusGained");
    else
        fire_script("OnFocusLost");
}

void frame::add_level_(int i_amount) {
    int i_old_level = i_level_;
    i_level_ += i_amount;

    if (!is_virtual()) {
        get_top_level_renderer()->notify_frame_level_changed(
            observer_from(this), i_old_level, i_level_);
    }

    for (auto& m_child : get_children())
        m_child.add_level_(i_amount);
}

void frame::set_user_placed(bool b_is_user_placed) {
    b_is_user_placed_ = b_is_user_placed;
}

void frame::start_moving() {
    if (!b_is_movable_)
        return;

    set_user_placed(true);
    get_manager().get_root().start_moving(observer_from(this));
}

void frame::stop_moving() {
    if (get_manager().get_root().is_moving(*this))
        get_manager().get_root().stop_moving();
}

void frame::start_sizing(const anchor_point& m_point) {
    if (!b_is_resizable_)
        return;

    set_user_placed(true);
    get_manager().get_root().start_sizing(observer_from(this), m_point);
}

void frame::stop_sizing() {
    if (get_manager().get_root().is_sizing(*this))
        get_manager().get_root().stop_sizing();
}

void frame::propagate_renderer_(bool b_rendered) {
    auto p_top_level_renderer = get_top_level_renderer();
    for (const auto& p_child : l_child_list_) {
        if (!p_child)
            continue;

        if (!p_child->get_renderer())
            p_top_level_renderer->notify_rendered_frame(p_child, b_rendered);

        p_child->propagate_renderer_(b_rendered);
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

    if (b_auto_focus_) {
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
    if (b_virtual_)
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
    if (b_is_shown_)
        return;

    bool b_was_visible = b_is_visible_;
    base::show();

    if (!b_was_visible)
        get_manager().get_root().notify_hovered_frame_dirty();
}

void frame::hide() {
    if (!b_is_shown_)
        return;

    bool b_was_visible = b_is_visible_;
    base::hide();

    if (b_was_visible)
        get_manager().get_root().notify_hovered_frame_dirty();
}

void frame::notify_mouse_in_frame(bool b_mouse_inframe, const vector2f& /*mPosition*/) {
    alive_checker m_checker(*this);

    if (b_mouse_inframe) {
        if (!b_mouse_in_frame_) {
            fire_script("OnEnter");
            if (!m_checker.is_alive())
                return;
        }

        b_mouse_in_frame_ = true;
    } else {
        if (b_mouse_in_frame_) {
            fire_script("OnLeave");
            if (!m_checker.is_alive())
                return;
        }

        b_mouse_in_frame_ = false;
    }
}

void frame::update_borders_() {
    const bool b_old_ready       = b_ready_;
    const auto l_old_border_list = l_border_list_;

    base::update_borders_();

    check_position_();

    if (l_border_list_ != l_old_border_list || b_ready_ != b_old_ready) {
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

    if (b_build_layer_list_) {
        DEBUG_LOG("   Build layers");
        // Clear layers' content
        for (auto& m_layer : l_layer_list_)
            m_layer.l_region_list.clear();

        // Fill layers with regions (with font_string rendered last within the same layer)
        for (const auto& p_region : l_region_list_) {
            if (p_region && p_region->get_object_type() != "font_string")
                l_layer_list_[static_cast<std::size_t>(p_region->get_draw_layer())]
                    .l_region_list.push_back(p_region);
        }

        for (const auto& p_region : l_region_list_) {
            if (p_region && p_region->get_object_type() == "font_string")
                l_layer_list_[static_cast<std::size_t>(p_region->get_draw_layer())]
                    .l_region_list.push_back(p_region);
        }

        b_build_layer_list_ = false;
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
        auto m_iter_remove =
            std::remove_if(l_region_list_.begin(), l_region_list_.end(), [](auto& p_obj) {
                return p_obj == nullptr;
            });

        l_region_list_.erase(m_iter_remove, l_region_list_.end());
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
        auto m_iter_remove =
            std::remove_if(l_child_list_.begin(), l_child_list_.end(), [](auto& p_obj) {
                return p_obj == nullptr;
            });

        l_child_list_.erase(m_iter_remove, l_child_list_.end());
    }

    // Remove empty handlers
    for (auto m_iter_list = l_signal_list_.begin(); m_iter_list != l_signal_list_.end();) {
        if (m_iter_list->second.empty())
            m_iter_list = l_signal_list_.erase(m_iter_list);
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
