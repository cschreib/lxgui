#include "lxgui/gui_root.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/input_dispatcher.hpp"
#include "lxgui/input_window.hpp"
#include "lxgui/input_world_dispatcher.hpp"
#include "lxgui/utils_range.hpp"
#include "lxgui/utils_std.hpp"

// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace lxgui::gui {

root::root(utils::control_block& m_block, manager& m_manager) :
    frame_container(m_manager.get_factory(), m_object_registry_, this),
    utils::enable_observer_from_this<root>(m_block),
    m_manager_(m_manager),
    m_renderer_(m_manager.get_renderer()),
    m_world_input_dispatcher_(m_manager.get_world_input_dispatcher()) {
    auto& m_window       = get_manager().get_window();
    m_screen_dimensions_ = m_window.get_dimensions();

    connections_.push_back(
        m_window.on_window_resized.connect([&](auto... m_args) { on_window_resized_(m_args...); }));

    auto& m_input_dispatcher = get_manager().get_input_dispatcher();

    connections_.push_back(m_input_dispatcher.on_mouse_moved.connect(
        [&](auto... m_args) { on_mouse_moved_(m_args...); }));

    connections_.push_back(m_input_dispatcher.on_mouse_wheel.connect(
        [&](auto... m_args) { on_mouse_wheel_(m_args...); }));

    connections_.push_back(m_input_dispatcher.on_mouse_drag_start.connect(
        [&](auto... m_args) { on_drag_start_(m_args...); }));

    connections_.push_back(m_input_dispatcher.on_mouse_drag_stop.connect(
        [&](auto... m_args) { on_drag_stop_(m_args...); }));

    connections_.push_back(m_input_dispatcher.on_text_entered.connect(
        [&](auto... m_args) { on_text_entered_(m_args...); }));

    connections_.push_back(m_input_dispatcher.on_mouse_pressed.connect(
        [&](input::mouse_button m_button, const vector2f& m_mouse_pos) {
            on_mouse_button_state_changed_(m_button, true, false, m_mouse_pos);
        }));

    connections_.push_back(m_input_dispatcher.on_mouse_released.connect(
        [&](input::mouse_button m_button, const vector2f& m_mouse_pos) {
            on_mouse_button_state_changed_(m_button, false, false, m_mouse_pos);
        }));

    connections_.push_back(m_input_dispatcher.on_mouse_double_clicked.connect(
        [&](input::mouse_button m_button, const vector2f& m_mouse_pos) {
            on_mouse_button_state_changed_(m_button, true, true, m_mouse_pos);
        }));

    connections_.push_back(m_input_dispatcher.on_key_pressed.connect(
        [&](input::key m_key) { on_key_state_changed_(m_key, true); }));

    connections_.push_back(m_input_dispatcher.on_key_released.connect(
        [&](input::key m_key) { on_key_state_changed_(m_key, false); }));
}

root::~root() {
    // Must be done before we destroy the registry
    clear_frames_();
}

vector2f root::get_target_dimensions() const {
    return vector2f(m_screen_dimensions_) / get_manager().get_interface_scaling_factor();
}

void root::render() const {
    m_renderer_.set_view(matrix4f::view(get_target_dimensions()));

    if (b_enable_caching_) {
        m_renderer_.render_quad(m_screen_quad_);
    } else {
        for (const auto& m_strata : strata_list_) {
            render_strata_(m_strata);
        }
    }
}

void root::create_caching_render_target_() {
    try {
        if (p_render_target_)
            p_render_target_->set_dimensions(m_screen_dimensions_);
        else
            p_render_target_ = m_renderer_.create_render_target(m_screen_dimensions_);
    } catch (const utils::exception& e) {
        gui::out << gui::error << "gui::root : "
                 << "Unable to create render_target for GUI caching :\n"
                 << e.get_description() << std::endl;

        b_enable_caching_ = false;
        return;
    }

    vector2f m_scaled_dimensions = get_target_dimensions();

    m_screen_quad_.mat      = m_renderer_.create_material(p_render_target_);
    m_screen_quad_.v[0].pos = vector2f::zero;
    m_screen_quad_.v[1].pos = vector2f(m_scaled_dimensions.x, 0);
    m_screen_quad_.v[2].pos = m_scaled_dimensions;
    m_screen_quad_.v[3].pos = vector2f(0, m_scaled_dimensions.y);

    m_screen_quad_.v[0].uvs = m_screen_quad_.mat->get_canvas_uv(vector2f(0, 0), true);
    m_screen_quad_.v[1].uvs = m_screen_quad_.mat->get_canvas_uv(vector2f(1, 0), true);
    m_screen_quad_.v[2].uvs = m_screen_quad_.mat->get_canvas_uv(vector2f(1, 1), true);
    m_screen_quad_.v[3].uvs = m_screen_quad_.mat->get_canvas_uv(vector2f(0, 1), true);
}

void root::create_strata_cache_render_target_(strata& m_strata) {
    if (m_strata.p_render_target)
        m_strata.p_render_target->set_dimensions(m_screen_dimensions_);
    else
        m_strata.p_render_target = m_renderer_.create_render_target(m_screen_dimensions_);

    vector2f m_scaled_dimensions = get_target_dimensions();

    m_strata.m_quad.mat      = m_renderer_.create_material(m_strata.p_render_target);
    m_strata.m_quad.v[0].pos = vector2f::zero;
    m_strata.m_quad.v[1].pos = vector2f(m_scaled_dimensions.x, 0);
    m_strata.m_quad.v[2].pos = m_scaled_dimensions;
    m_strata.m_quad.v[3].pos = vector2f(0, m_scaled_dimensions.y);

    m_strata.m_quad.v[0].uvs = m_strata.m_quad.mat->get_canvas_uv(vector2f(0, 0), true);
    m_strata.m_quad.v[1].uvs = m_strata.m_quad.mat->get_canvas_uv(vector2f(1, 0), true);
    m_strata.m_quad.v[2].uvs = m_strata.m_quad.mat->get_canvas_uv(vector2f(1, 1), true);
    m_strata.m_quad.v[3].uvs = m_strata.m_quad.mat->get_canvas_uv(vector2f(0, 1), true);
}

void root::update(float f_delta) {
    // Update logics on root frames from parent to children.
    for (auto& m_frame : get_root_frames()) {
        m_frame.update(f_delta);
    }

    // Removed destroyed frames
    garbage_collect();

    bool b_redraw = has_strata_list_changed_();
    reset_strata_list_changed_flag_();

    if (b_redraw)
        notify_hovered_frame_dirty();

    if (b_enable_caching_) {
        DEBUG_LOG(" Redraw strata...");

        try {
            for (auto& m_strata : strata_list_) {
                if (m_strata.b_redraw) {
                    if (!m_strata.p_render_target)
                        create_strata_cache_render_target_(m_strata);

                    if (m_strata.p_render_target) {
                        m_renderer_.begin(m_strata.p_render_target);

                        vector2f m_view =
                            vector2f(m_strata.p_render_target->get_canvas_dimensions()) /
                            get_manager().get_interface_scaling_factor();

                        m_renderer_.set_view(matrix4f::view(m_view));

                        m_strata.p_render_target->clear(color::empty);
                        render_strata_(m_strata);

                        m_renderer_.end();
                    }

                    b_redraw = true;
                }

                m_strata.b_redraw = false;
            }

            if (!p_render_target_)
                create_caching_render_target_();

            if (b_redraw && p_render_target_) {
                m_renderer_.begin(p_render_target_);

                vector2f m_view = vector2f(p_render_target_->get_canvas_dimensions()) /
                                  get_manager().get_interface_scaling_factor();

                m_renderer_.set_view(matrix4f::view(m_view));

                p_render_target_->clear(color::empty);

                for (auto& m_strata : strata_list_) {
                    m_renderer_.render_quad(m_strata.m_quad);
                }

                m_renderer_.end();
            }
        } catch (const utils::exception& e) {
            gui::out << gui::error << "gui::root : "
                     << "Unable to create render_target for strata :\n"
                     << e.get_description() << std::endl;

            b_enable_caching_ = false;
        }
    }
}

void root::toggle_caching() {
    b_enable_caching_ = !b_enable_caching_;

    if (b_enable_caching_) {
        for (auto& m_strata : strata_list_)
            m_strata.b_redraw = true;
    }
}

void root::enable_caching(bool b_enable) {
    if (b_enable_caching_ != b_enable)
        toggle_caching();
}

bool root::is_caching_enabled() const {
    return b_enable_caching_;
}

void root::notify_scaling_factor_updated() {
    for (auto& m_frame : get_root_frames()) {
        m_frame.notify_scaling_factor_updated();
    }

    if (p_render_target_)
        create_caching_render_target_();

    for (auto& m_strata : strata_list_) {
        if (m_strata.p_render_target)
            create_strata_cache_render_target_(m_strata);
    }
}

void root::update_hovered_frame_() {
    const auto m_mouse_pos = get_manager().get_input_dispatcher().get_mouse_position();

    utils::observer_ptr<frame> p_hovered_frame = find_topmost_frame([&](const frame& m_frame) {
        return m_frame.is_in_region(m_mouse_pos) && m_frame.is_mouse_move_enabled();
    });

    set_hovered_frame_(std::move(p_hovered_frame), m_mouse_pos);
}

void root::notify_hovered_frame_dirty() {
    update_hovered_frame_();
}

void root::start_moving(
    utils::observer_ptr<region> p_obj,
    anchor*                     p_anchor,
    constraint                  m_constraint,
    std::function<void()>       m_apply_constraint_func) {
    p_sized_object_   = nullptr;
    p_moved_object_   = std::move(p_obj);
    m_mouse_movement_ = vector2f::zero;

    if (p_moved_object_) {
        m_constraint_            = m_constraint;
        m_apply_constraint_func_ = std::move(m_apply_constraint_func);
        if (p_anchor) {
            p_moved_anchor_            = p_anchor;
            m_movement_start_position_ = p_moved_anchor_->m_offset;
        } else {
            const bounds2f borders = p_moved_object_->get_borders();

            p_moved_object_->clear_all_points();
            p_moved_object_->set_point(anchor_point::top_left, "", borders.top_left());

            p_moved_anchor_ = &p_moved_object_->modify_point(anchor_point::top_left);

            m_movement_start_position_ = borders.top_left();
        }
    }
}

void root::stop_moving() {
    p_moved_object_ = nullptr;
    p_moved_anchor_ = nullptr;
}

bool root::is_moving(const region& m_obj) const {
    return p_moved_object_.get() == &m_obj;
}

void root::start_sizing(utils::observer_ptr<region> p_obj, anchor_point m_point) {
    p_moved_object_   = nullptr;
    p_sized_object_   = std::move(p_obj);
    m_mouse_movement_ = vector2f::zero;

    if (p_sized_object_) {
        const bounds2f borders = p_sized_object_->get_borders();

        anchor_point m_opposite_point = anchor_point::center;
        vector2f     m_offset;

        switch (m_point) {
        case anchor_point::top_left:
        case anchor_point::top:
            m_opposite_point      = anchor_point::bottom_right;
            m_offset              = borders.bottom_right();
            b_resize_from_right_  = false;
            b_resize_from_bottom_ = false;
            break;
        case anchor_point::top_right:
        case anchor_point::right:
            m_opposite_point      = anchor_point::bottom_left;
            m_offset              = borders.bottom_left();
            b_resize_from_right_  = true;
            b_resize_from_bottom_ = false;
            break;
        case anchor_point::bottom_right:
        case anchor_point::bottom:
            m_opposite_point      = anchor_point::top_left;
            m_offset              = borders.top_left();
            b_resize_from_right_  = true;
            b_resize_from_bottom_ = true;
            break;
        case anchor_point::bottom_left:
        case anchor_point::left:
            m_opposite_point      = anchor_point::top_right;
            m_offset              = borders.top_right();
            b_resize_from_right_  = false;
            b_resize_from_bottom_ = true;
            break;
        case anchor_point::center:
            gui::out << gui::error << "gui::manager : "
                     << "Cannot resize \"" << p_sized_object_->get_name() << "\" from its center."
                     << std::endl;
            p_sized_object_ = nullptr;
            return;
        }

        p_sized_object_->clear_all_points();
        p_sized_object_->set_point(m_opposite_point, "", anchor_point::top_left, m_offset);

        m_resize_start_ = p_sized_object_->get_apparent_dimensions();

        if (m_point == anchor_point::left || m_point == anchor_point::right) {
            b_resize_width_  = true;
            b_resize_height_ = false;
        } else if (m_point == anchor_point::top || m_point == anchor_point::bottom) {
            b_resize_width_  = false;
            b_resize_height_ = true;
        } else {
            b_resize_width_  = true;
            b_resize_height_ = true;
        }
    }
}

void root::stop_sizing() {
    p_sized_object_ = nullptr;
}

bool root::is_sizing(const region& m_obj) const {
    return p_sized_object_.get() == &m_obj;
}

void release_focus_to_list(const frame& m_receiver, std::vector<utils::observer_ptr<frame>>& list) {
    if (list.empty())
        return;

    // Find receiver in the list
    auto m_iter =
        utils::find_if(list, [&](const auto& p_ptr) { return p_ptr.get() == &m_receiver; });

    if (m_iter == list.end())
        return;

    // Set it to null
    *m_iter = nullptr;

    // Clean up null entries
    auto m_end_iter = std::remove_if(
        list.begin(), list.end(), [](const auto& p_ptr) { return p_ptr == nullptr; });

    list.erase(m_end_iter, list.end());
}

void request_focus_to_list(
    utils::observer_ptr<frame> p_receiver, std::vector<utils::observer_ptr<frame>>& list) {
    auto* p_raw_pointer = p_receiver.get();
    if (!p_raw_pointer)
        return;

    // Check if this receiver was already in the focus stack and remove it
    release_focus_to_list(*p_raw_pointer, list);

    // Add receiver at the top of the stack
    list.push_back(std::move(p_receiver));
}

void root::request_focus(utils::observer_ptr<frame> p_receiver) {
    auto p_old_focus = get_focussed_frame();
    request_focus_to_list(std::move(p_receiver), focus_stack_);
    auto p_new_focus = get_focussed_frame();

    if (p_old_focus != p_new_focus) {
        if (p_old_focus)
            p_old_focus->notify_focus(false);

        if (p_new_focus)
            p_new_focus->notify_focus(true);
    }
}

void root::release_focus(const frame& m_receiver) {
    auto p_old_focus = get_focussed_frame();
    release_focus_to_list(m_receiver, focus_stack_);
    auto p_new_focus = get_focussed_frame();

    if (p_old_focus != p_new_focus) {
        if (p_old_focus)
            p_old_focus->notify_focus(false);

        if (p_new_focus)
            p_new_focus->notify_focus(true);
    }
}

void root::clear_focus() {
    auto p_old_focus = get_focussed_frame();
    focus_stack_.clear();

    if (p_old_focus)
        p_old_focus->notify_focus(false);
}

bool root::is_focused() const {
    return get_focussed_frame() != nullptr;
}

utils::observer_ptr<const frame> root::get_focussed_frame() const {
    for (const auto& p_ptr : utils::range::reverse(focus_stack_)) {
        if (p_ptr)
            return p_ptr;
    }

    return nullptr;
}

void root::clear_hovered_frame_() {
    p_hovered_frame_ = nullptr;
}

void root::set_hovered_frame_(utils::observer_ptr<frame> p_frame, const vector2f& m_mouse_pos) {
    if (p_hovered_frame_ && p_frame != p_hovered_frame_)
        p_hovered_frame_->notify_mouse_in_frame(false, m_mouse_pos);

    if (p_frame) {
        p_hovered_frame_ = std::move(p_frame);
        p_hovered_frame_->notify_mouse_in_frame(true, m_mouse_pos);
    } else
        clear_hovered_frame_();
}

void root::on_window_resized_(const vector2ui& m_dimensions) {
    // Update internal window size
    m_screen_dimensions_ = m_dimensions;

    // Notify all frames anchored to the window edges
    for (auto& m_frame : get_root_frames()) {
        m_frame.notify_borders_need_update();
        m_frame.notify_renderer_need_redraw();
    }

    // Resize caching render targets
    if (p_render_target_)
        create_caching_render_target_();

    for (auto& m_strata : strata_list_) {
        if (m_strata.p_render_target)
            create_strata_cache_render_target_(m_strata);
    }

    notify_hovered_frame_dirty();
}

void root::on_mouse_moved_(const vector2f& m_movement, const vector2f& m_mouse_pos) {
    notify_hovered_frame_dirty();

    if (p_moved_object_ || p_sized_object_) {
        DEBUG_LOG(" Moved object...");
        m_mouse_movement_ += m_movement;
    }

    if (p_moved_object_) {
        switch (m_constraint_) {
        case constraint::none:
            p_moved_anchor_->m_offset = m_movement_start_position_ + m_mouse_movement_;
            break;
        case constraint::x:
            p_moved_anchor_->m_offset =
                m_movement_start_position_ + vector2f(m_mouse_movement_.x, 0.0f);
            break;
        case constraint::y:
            p_moved_anchor_->m_offset =
                m_movement_start_position_ + vector2f(0.0f, m_mouse_movement_.y);
            break;
        default: break;
        }

        if (m_apply_constraint_func_)
            m_apply_constraint_func_();

        // As a result of applying constraints, object may have been deleted,
        // so check again before use
        if (p_moved_object_)
            p_moved_object_->notify_borders_need_update();
    } else if (p_sized_object_) {
        float f_width;
        if (b_resize_from_right_)
            f_width = std::max(0.0f, m_resize_start_.x + m_mouse_movement_.x);
        else
            f_width = std::max(0.0f, m_resize_start_.x - m_mouse_movement_.x);

        float f_height;
        if (b_resize_from_bottom_)
            f_height = std::max(0.0f, m_resize_start_.y + m_mouse_movement_.y);
        else
            f_height = std::max(0.0f, m_resize_start_.y - m_mouse_movement_.y);

        if (b_resize_width_ && b_resize_height_)
            p_sized_object_->set_dimensions(vector2f(f_width, f_height));
        else if (b_resize_width_)
            p_sized_object_->set_width(f_width);
        else if (b_resize_height_)
            p_sized_object_->set_height(f_height);
    }

    if (p_dragged_frame_) {
        event_data m_data;
        m_data.add(m_mouse_pos.x);
        m_data.add(m_mouse_pos.y);
        p_dragged_frame_->fire_script("OnDragMove", m_data);
    }

    if (!p_hovered_frame_) {
        // Forward to the world
        m_world_input_dispatcher_.on_mouse_moved(m_movement, m_mouse_pos);
    }
}

void root::on_mouse_wheel_(float f_wheel_scroll, const vector2f& m_mouse_pos) {
    utils::observer_ptr<frame> p_hovered_frame = find_topmost_frame([&](const frame& m_frame) {
        return m_frame.is_in_region(m_mouse_pos) && m_frame.is_mouse_wheel_enabled();
    });

    if (!p_hovered_frame) {
        // Forward to the world
        m_world_input_dispatcher_.on_mouse_wheel(f_wheel_scroll, m_mouse_pos);
        return;
    }

    event_data m_data;
    m_data.add(f_wheel_scroll);
    m_data.add(m_mouse_pos.x);
    m_data.add(m_mouse_pos.y);
    p_hovered_frame->fire_script("OnMouseWheel", m_data);
}

void root::on_drag_start_(input::mouse_button m_button, const vector2f& m_mouse_pos) {
    utils::observer_ptr<frame> p_hovered_frame = find_topmost_frame([&](const frame& m_frame) {
        return m_frame.is_in_region(m_mouse_pos) && m_frame.is_mouse_click_enabled();
    });

    if (!p_hovered_frame) {
        // Forward to the world
        m_world_input_dispatcher_.on_mouse_drag_start(m_button, m_mouse_pos);
        return;
    }

    if (auto* p_region = p_hovered_frame->get_title_region().get();
        p_region && p_region->is_in_region(m_mouse_pos)) {
        p_hovered_frame->start_moving();
    }

    std::string mouse_button = std::string(input::get_mouse_button_codename(m_button));

    if (p_hovered_frame->is_registered_for_drag(mouse_button)) {
        event_data m_data;
        m_data.add(mouse_button);
        m_data.add(m_mouse_pos.x);
        m_data.add(m_mouse_pos.y);

        p_dragged_frame_ = std::move(p_hovered_frame);
        p_dragged_frame_->fire_script("OnDragStart", m_data);
    }
}

void root::on_drag_stop_(input::mouse_button m_button, const vector2f& m_mouse_pos) {
    stop_moving();
    stop_sizing();

    if (p_dragged_frame_) {
        p_dragged_frame_->fire_script("OnDragStop");
        p_dragged_frame_ = nullptr;
    }

    utils::observer_ptr<frame> p_hovered_frame = find_topmost_frame([&](const frame& m_frame) {
        return m_frame.is_in_region(m_mouse_pos) && m_frame.is_mouse_click_enabled();
    });

    if (!p_hovered_frame) {
        // Forward to the world
        m_world_input_dispatcher_.on_mouse_drag_stop(m_button, m_mouse_pos);
        return;
    }

    std::string mouse_button = std::string(input::get_mouse_button_codename(m_button));

    if (p_hovered_frame->is_registered_for_drag(mouse_button)) {
        event_data m_data;
        m_data.add(mouse_button);
        m_data.add(m_mouse_pos.x);
        m_data.add(m_mouse_pos.y);

        p_hovered_frame->fire_script("OnReceiveDrag", m_data);
    }
}

void root::on_text_entered_(std::uint32_t c) {
    if (auto p_focus = get_focussed_frame()) {
        event_data m_data;
        m_data.add(utils::unicode_to_utf8(utils::ustring(1, c)));
        m_data.add(c);

        p_focus->fire_script("OnChar", m_data);
    } else {
        // Forward to the world
        m_world_input_dispatcher_.on_text_entered(c);
    }
}

std::string get_key_name(
    input::key m_key, bool b_is_shift_pressed, bool b_is_ctrl_pressed, bool b_is_alt_pressed) {
    std::string name;

    if (m_key != input::key::k_lcontrol && m_key != input::key::k_rcontrol &&
        m_key != input::key::k_lshift && m_key != input::key::k_rshift &&
        m_key != input::key::k_lmenu && m_key != input::key::k_rmenu) {
        if (b_is_ctrl_pressed)
            name = "Ctrl-";
        if (b_is_alt_pressed)
            name.append("Alt-");
        if (b_is_shift_pressed)
            name.append("Shift-");
    }

    name.append(input::get_key_codename(m_key));

    return name;
}

void root::on_key_state_changed_(input::key m_key, bool b_is_down) {
    const auto& m_input_dispatcher = get_manager().get_input_dispatcher();
    bool        b_is_shift_pressed = m_input_dispatcher.shift_is_pressed();
    bool        b_is_ctrl_pressed  = m_input_dispatcher.ctrl_is_pressed();
    bool        b_is_alt_pressed   = m_input_dispatcher.alt_is_pressed();

    std::string key_name =
        get_key_name(m_key, b_is_shift_pressed, b_is_ctrl_pressed, b_is_alt_pressed);

    // First, give priority to the focussed frame
    utils::observer_ptr<frame> p_topmost_frame = get_focussed_frame();

    // If no focussed frame, look top-down for a frame that captures this key
    if (!p_topmost_frame) {
        p_topmost_frame = find_topmost_frame(
            [&](const frame& m_frame) { return m_frame.is_key_capture_enabled(key_name); });
    }

    // If a frame is found, capture input and return
    if (p_topmost_frame) {
        event_data m_data;
        m_data.add(static_cast<std::underlying_type_t<input::key>>(m_key));
        m_data.add(key_name);
        m_data.add(b_is_shift_pressed);
        m_data.add(b_is_ctrl_pressed);
        m_data.add(b_is_alt_pressed);

        if (b_is_down)
            p_topmost_frame->fire_script("OnKeyDown", m_data);
        else
            p_topmost_frame->fire_script("OnKeyUp", m_data);

        return;
    }

    if (b_is_down) {
        // If no frame is found, try the keybinder
        try {
            if (get_keybinder().on_key_down(
                    m_key, b_is_shift_pressed, b_is_ctrl_pressed, b_is_alt_pressed))
                return;
        } catch (const std::exception& m_exception) {
            std::string error = m_exception.what();
            gui::out << gui::error << error << std::endl;
            get_manager().get_event_emitter().fire_event("LUA_ERROR", {error});
            return;
        }
    }

    // Forward to the world
    if (b_is_down)
        m_world_input_dispatcher_.on_key_pressed(m_key);
    else
        m_world_input_dispatcher_.on_key_released(m_key);
}

void root::on_mouse_button_state_changed_(
    input::mouse_button m_button,
    bool                b_is_down,
    bool                b_is_double_click,
    const vector2f&     m_mouse_pos) {
    utils::observer_ptr<frame> p_hovered_frame = find_topmost_frame([&](const frame& m_frame) {
        return m_frame.is_in_region(m_mouse_pos) && m_frame.is_mouse_click_enabled();
    });

    if (b_is_down && !b_is_double_click) {
        if (!p_hovered_frame || p_hovered_frame != get_focussed_frame())
            clear_focus();
    }

    if (!p_hovered_frame) {
        // Forward to the world
        if (b_is_double_click)
            m_world_input_dispatcher_.on_mouse_double_clicked(m_button, m_mouse_pos);
        else if (b_is_down)
            m_world_input_dispatcher_.on_mouse_pressed(m_button, m_mouse_pos);
        else
            m_world_input_dispatcher_.on_mouse_released(m_button, m_mouse_pos);
        return;
    }

    event_data m_data;
    m_data.add(std::string(input::get_mouse_button_codename(m_button)));
    m_data.add(m_mouse_pos.x);
    m_data.add(m_mouse_pos.y);

    if (b_is_double_click) {
        p_hovered_frame->fire_script("OnDoubleClicked", m_data);
    } else if (b_is_down) {
        if (auto* p_top_level = p_hovered_frame->get_top_level_parent().get())
            p_top_level->raise();

        p_hovered_frame->fire_script("OnMouseDown", m_data);
    } else {
        p_hovered_frame->fire_script("OnMouseUp", m_data);
    }
}

} // namespace lxgui::gui
