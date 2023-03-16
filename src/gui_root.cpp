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

root::root(utils::control_block& block, manager& mgr) :
    utils::enable_observer_from_this<root>(block),
    frame_container(mgr.get_factory(), object_registry_, observer_from_this()),
    manager_(mgr),
    renderer_(mgr.get_renderer()),
    world_input_dispatcher_(mgr.get_world_input_dispatcher()) {
    auto& window       = get_manager().get_window();
    screen_dimensions_ = window.get_dimensions();

    connections_.push_back(
        window.on_window_resized.connect([&](auto... args) { on_window_resized_(args...); }));

    auto& input_dispatcher = get_manager().get_input_dispatcher();

    connections_.push_back(
        input_dispatcher.on_mouse_moved.connect([&](const input::mouse_moved_data& args) {
            if (!on_mouse_moved_(args)) {
                world_input_dispatcher_.on_mouse_moved(args);
            }
        }));

    connections_.push_back(
        input_dispatcher.on_mouse_wheel.connect([&](const input::mouse_wheel_data& args) {
            if (!on_mouse_wheel_(args)) {
                world_input_dispatcher_.on_mouse_wheel(args);
            }
        }));

    connections_.push_back(
        input_dispatcher.on_mouse_drag_start.connect([&](const input::mouse_drag_start_data& args) {
            if (!on_drag_start_(args)) {
                world_input_dispatcher_.on_mouse_drag_start(args);
            }
        }));

    connections_.push_back(
        input_dispatcher.on_mouse_drag_stop.connect([&](const input::mouse_drag_stop_data& args) {
            if (!on_drag_stop_(args)) {
                world_input_dispatcher_.on_mouse_drag_stop(args);
            }
        }));

    connections_.push_back(
        input_dispatcher.on_text_entered.connect([&](const input::text_entered_data& args) {
            if (!on_text_entered_(args)) {
                world_input_dispatcher_.on_text_entered(args);
            }
        }));

    connections_.push_back(
        input_dispatcher.on_mouse_pressed.connect([&](const input::mouse_pressed_data& args) {
            if (!on_mouse_button_state_changed_(args.button, true, false, false, args.position)) {
                world_input_dispatcher_.on_mouse_pressed(args);
            }
        }));

    connections_.push_back(
        input_dispatcher.on_mouse_released.connect([&](const input::mouse_released_data& args) {
            if (!on_mouse_button_state_changed_(
                    args.button, false, false, args.was_dragged, args.position)) {
                world_input_dispatcher_.on_mouse_released(args);
            }
        }));

    connections_.push_back(input_dispatcher.on_mouse_double_clicked.connect(
        [&](const input::mouse_double_clicked_data& args) {
            if (!on_mouse_button_state_changed_(args.button, true, true, false, args.position)) {
                world_input_dispatcher_.on_mouse_double_clicked(args);
            }
        }));

    connections_.push_back(
        input_dispatcher.on_key_pressed.connect([&](const input::key_pressed_data& args) {
            if (!on_key_state_changed_(args.key, true, false)) {
                world_input_dispatcher_.on_key_pressed(args);
            }
        }));

    connections_.push_back(input_dispatcher.on_key_pressed_repeat.connect(
        [&](const input::key_pressed_repeat_data& args) {
            if (!on_key_state_changed_(args.key, true, true)) {
                world_input_dispatcher_.on_key_pressed_repeat(args);
            }
        }));

    connections_.push_back(
        input_dispatcher.on_key_released.connect([&](const input::key_released_data& args) {
            if (!on_key_state_changed_(args.key, false, false)) {
                world_input_dispatcher_.on_key_released(args);
            }
        }));
}

root::~root() {
    // Must be done before we destroy the registry
    clear_frames_();
}

vector2f root::get_target_dimensions() const {
    return vector2f(screen_dimensions_) / get_manager().get_interface_scaling_factor();
}

void root::render() const {
    renderer_.set_view(matrix4f::view(get_target_dimensions()));

    if (caching_enabled_) {
        renderer_.render_quad(screen_quad_);
    } else {
        for (const auto& s : strata_list_) {
            render_strata_(s);
        }
    }
}

void root::create_caching_render_target_() {
    try {
        if (target_)
            target_->set_dimensions(screen_dimensions_);
        else
            target_ = renderer_.create_render_target(screen_dimensions_);
    } catch (const utils::exception& e) {
        gui::out << gui::error << "gui::root: "
                 << "Unable to create render_target for GUI caching: " << e.get_description()
                 << std::endl;

        caching_enabled_ = false;
        return;
    }

    vector2f scaled_dimensions = get_target_dimensions();

    screen_quad_.mat      = renderer_.create_material(target_);
    screen_quad_.v[0].pos = vector2f::zero;
    screen_quad_.v[1].pos = vector2f(scaled_dimensions.x, 0);
    screen_quad_.v[2].pos = scaled_dimensions;
    screen_quad_.v[3].pos = vector2f(0, scaled_dimensions.y);

    screen_quad_.v[0].uvs = screen_quad_.mat->get_canvas_uv(vector2f(0, 0), true);
    screen_quad_.v[1].uvs = screen_quad_.mat->get_canvas_uv(vector2f(1, 0), true);
    screen_quad_.v[2].uvs = screen_quad_.mat->get_canvas_uv(vector2f(1, 1), true);
    screen_quad_.v[3].uvs = screen_quad_.mat->get_canvas_uv(vector2f(0, 1), true);
}

void root::create_strata_cache_render_target_(strata_data& strata_obj) {
    if (strata_obj.target)
        strata_obj.target->set_dimensions(screen_dimensions_);
    else
        strata_obj.target = renderer_.create_render_target(screen_dimensions_);

    vector2f scaled_dimensions = get_target_dimensions();

    auto& q = strata_obj.target_quad;

    q.mat      = renderer_.create_material(strata_obj.target);
    q.v[0].pos = vector2f::zero;
    q.v[1].pos = vector2f(scaled_dimensions.x, 0);
    q.v[2].pos = scaled_dimensions;
    q.v[3].pos = vector2f(0, scaled_dimensions.y);

    q.v[0].uvs = q.mat->get_canvas_uv(vector2f(0, 0), true);
    q.v[1].uvs = q.mat->get_canvas_uv(vector2f(1, 0), true);
    q.v[2].uvs = q.mat->get_canvas_uv(vector2f(1, 1), true);
    q.v[3].uvs = q.mat->get_canvas_uv(vector2f(0, 1), true);
}

void root::update(float delta) {
    // Update logics on root frames from parent to children.
    for (auto& obj : get_root_frames()) {
        obj.update(delta);
    }

    // Removed destroyed frames
    garbage_collect();

    bool redraw_flag = has_strata_list_changed_();
    reset_strata_list_changed_flag_();

    if (redraw_flag)
        notify_hovered_frame_dirty();

    if (caching_enabled_) {
        DEBUG_LOG(" Redraw strata...");

        try {
            for (auto& s : strata_list_) {
                if (s.redraw_flag) {
                    if (!s.target)
                        create_strata_cache_render_target_(s);

                    if (s.target) {
                        renderer_.begin(s.target);

                        vector2f view = vector2f(s.target->get_canvas_dimensions()) /
                                        get_manager().get_interface_scaling_factor();

                        renderer_.set_view(matrix4f::view(view));

                        s.target->clear(color::empty);
                        render_strata_(s);

                        renderer_.end();
                    }

                    redraw_flag = true;
                }

                s.redraw_flag = false;
            }

            if (!target_)
                create_caching_render_target_();

            if (redraw_flag && target_) {
                renderer_.begin(target_);

                vector2f view = vector2f(target_->get_canvas_dimensions()) /
                                get_manager().get_interface_scaling_factor();

                renderer_.set_view(matrix4f::view(view));

                target_->clear(color::empty);

                for (auto& strata : strata_list_) {
                    renderer_.render_quad(strata.target_quad);
                }

                renderer_.end();
            }
        } catch (const utils::exception& e) {
            gui::out << gui::error << "gui::root: "
                     << "Unable to create render_target for strata: " << e.get_description()
                     << std::endl;

            caching_enabled_ = false;
        }
    }
}

void root::toggle_caching() {
    caching_enabled_ = !caching_enabled_;

    if (caching_enabled_) {
        for (auto& s : strata_list_)
            s.redraw_flag = true;
    }
}

void root::enable_caching(bool enable) {
    if (caching_enabled_ != enable)
        toggle_caching();
}

bool root::is_caching_enabled() const {
    return caching_enabled_;
}

void root::notify_scaling_factor_updated() {
    for (auto& obj : get_root_frames()) {
        obj.notify_scaling_factor_updated();
    }

    if (target_)
        create_caching_render_target_();

    for (auto& s : strata_list_) {
        if (s.target)
            create_strata_cache_render_target_(s);
    }
}

void root::update_hovered_frame_() {
    const auto mouse_pos = get_manager().get_input_dispatcher().get_mouse_position();

    utils::observer_ptr<frame> hovered_frame = find_topmost_frame([&](const frame& obj) {
        return obj.is_in_region(mouse_pos) && obj.is_mouse_move_enabled();
    });

    set_hovered_frame_(std::move(hovered_frame), mouse_pos);
}

void root::notify_hovered_frame_dirty() {
    update_hovered_frame_();
}

void root::start_moving(
    utils::observer_ptr<region> obj,
    anchor*                     a,
    constraint                  constraint,
    std::function<void()>       apply_constraint_func) {
    sized_object_   = nullptr;
    moved_object_   = std::move(obj);
    mouse_movement_ = vector2f::zero;

    if (moved_object_) {
        constraint_            = constraint;
        apply_constraint_func_ = std::move(apply_constraint_func);
        if (a) {
            moved_anchor_            = a;
            movement_start_position_ = moved_anchor_->offset;
        } else {
            const bounds2f borders = moved_object_->get_borders();

            moved_object_->clear_all_anchors();
            moved_object_->set_anchor(point::top_left, "", borders.top_left());

            moved_anchor_ = &moved_object_->modify_anchor(point::top_left);

            movement_start_position_ = borders.top_left();
        }
    }
}

void root::stop_moving() {
    moved_object_ = nullptr;
    moved_anchor_ = nullptr;
}

bool root::is_moving(const region& obj) const {
    return moved_object_.get() == &obj;
}

void root::start_sizing(utils::observer_ptr<region> obj, point p) {
    moved_object_   = nullptr;
    sized_object_   = std::move(obj);
    mouse_movement_ = vector2f::zero;

    if (sized_object_) {
        const bounds2f borders = sized_object_->get_borders();

        point    opposite_point = point::center;
        vector2f offset;

        switch (p) {
        case point::top_left:
        case point::top:
            opposite_point           = point::bottom_right;
            offset                   = borders.bottom_right();
            is_resizing_from_right_  = false;
            is_resizing_from_bottom_ = false;
            break;
        case point::top_right:
        case point::right:
            opposite_point           = point::bottom_left;
            offset                   = borders.bottom_left();
            is_resizing_from_right_  = true;
            is_resizing_from_bottom_ = false;
            break;
        case point::bottom_right:
        case point::bottom:
            opposite_point           = point::top_left;
            offset                   = borders.top_left();
            is_resizing_from_right_  = true;
            is_resizing_from_bottom_ = true;
            break;
        case point::bottom_left:
        case point::left:
            opposite_point           = point::top_right;
            offset                   = borders.top_right();
            is_resizing_from_right_  = false;
            is_resizing_from_bottom_ = true;
            break;
        case point::center:
            gui::out << gui::error << "gui::manager: "
                     << "Cannot resize \"" << sized_object_->get_name() << "\" from its center."
                     << std::endl;
            sized_object_ = nullptr;
            return;
        }

        sized_object_->clear_all_anchors();
        sized_object_->set_anchor(opposite_point, "", point::top_left, offset);

        resize_start_ = sized_object_->get_apparent_dimensions();

        if (p == point::left || p == point::right) {
            is_resizing_width_  = true;
            is_resizing_height_ = false;
        } else if (p == point::top || p == point::bottom) {
            is_resizing_width_  = false;
            is_resizing_height_ = true;
        } else {
            is_resizing_width_  = true;
            is_resizing_height_ = true;
        }
    }
}

void root::stop_sizing() {
    sized_object_ = nullptr;
}

bool root::is_sizing(const region& obj) const {
    return sized_object_.get() == &obj;
}

void release_focus_to_list(const frame& receiver, std::vector<utils::observer_ptr<frame>>& list) {
    if (list.empty())
        return;

    // Find receiver in the list
    auto iter = utils::find_if(list, [&](const auto& ptr) { return ptr.get() == &receiver; });

    if (iter == list.end())
        return;

    // Set it to null
    *iter = nullptr;

    // Clean up null entries
    auto end_iter =
        std::remove_if(list.begin(), list.end(), [](const auto& ptr) { return ptr == nullptr; });

    list.erase(end_iter, list.end());
}

void request_focus_to_list(
    utils::observer_ptr<frame> receiver, std::vector<utils::observer_ptr<frame>>& list) {
    auto* raw_pointer = receiver.get();
    if (!raw_pointer)
        return;

    // Check if this receiver was already in the focus stack and remove it
    release_focus_to_list(*raw_pointer, list);

    // Add receiver at the top of the stack
    list.push_back(std::move(receiver));
}

void root::request_focus(utils::observer_ptr<frame> receiver) {
    auto old_focus = get_focused_frame();
    request_focus_to_list(std::move(receiver), focus_stack_);
    auto new_focus = get_focused_frame();

    if (old_focus != new_focus) {
        if (old_focus)
            old_focus->notify_focus(false);

        if (new_focus)
            new_focus->notify_focus(true);
    }
}

void root::release_focus(const frame& receiver) {
    auto old_focus = get_focused_frame();
    release_focus_to_list(receiver, focus_stack_);
    auto new_focus = get_focused_frame();

    if (old_focus != new_focus) {
        if (old_focus)
            old_focus->notify_focus(false);

        if (new_focus)
            new_focus->notify_focus(true);
    }
}

void root::clear_focus() {
    auto old_focus = get_focused_frame();
    focus_stack_.clear();

    if (old_focus)
        old_focus->notify_focus(false);
}

bool root::is_focused() const {
    return get_focused_frame() != nullptr;
}

utils::observer_ptr<const frame> root::get_focused_frame() const {
    for (const auto& ptr : utils::range::reverse(focus_stack_)) {
        if (ptr)
            return ptr;
    }

    return nullptr;
}

void root::clear_hovered_frame_() {
    hovered_frame_ = nullptr;
}

void root::set_hovered_frame_(utils::observer_ptr<frame> obj, const vector2f& mouse_pos) {
    if (obj == hovered_frame_)
        return;

    auto old_hovered_frame = hovered_frame_;
    hovered_frame_         = std::move(obj);

    if (old_hovered_frame) {
        old_hovered_frame->notify_mouse_in_frame(false, mouse_pos);
    }

    if (hovered_frame_) {
        hovered_frame_->notify_mouse_in_frame(true, mouse_pos);
    }
}

void root::on_window_resized_(const vector2ui& dimensions) {
    // Update internal window size
    screen_dimensions_ = dimensions;

    // Notify all frames anchored to the window edges
    for (auto& frame : get_root_frames()) {
        frame.notify_borders_need_update();
        frame.notify_renderer_need_redraw();
    }

    // Resize caching render targets
    if (target_)
        create_caching_render_target_();

    for (auto& strata : strata_list_) {
        if (strata.target)
            create_strata_cache_render_target_(strata);
    }

    notify_hovered_frame_dirty();
}

bool root::on_mouse_moved_(const input::mouse_moved_data& args) {
    notify_hovered_frame_dirty();

    if (moved_object_ || sized_object_) {
        DEBUG_LOG(" Moved object...");
        mouse_movement_ += args.motion;
    }

    if (moved_object_) {
        switch (constraint_) {
        case constraint::none:
            moved_anchor_->offset = movement_start_position_ + mouse_movement_;
            break;
        case constraint::x:
            moved_anchor_->offset = movement_start_position_ + vector2f(mouse_movement_.x, 0.0f);
            break;
        case constraint::y:
            moved_anchor_->offset = movement_start_position_ + vector2f(0.0f, mouse_movement_.y);
            break;
        default: break;
        }

        if (apply_constraint_func_)
            apply_constraint_func_();

        // As a result of applying constraints, object may have been deleted,
        // so check again before use
        if (moved_object_)
            moved_object_->notify_borders_need_update();
    } else if (sized_object_) {
        float width;
        if (is_resizing_from_right_)
            width = std::max(0.0f, resize_start_.x + mouse_movement_.x);
        else
            width = std::max(0.0f, resize_start_.x - mouse_movement_.x);

        float height;
        if (is_resizing_from_bottom_)
            height = std::max(0.0f, resize_start_.y + mouse_movement_.y);
        else
            height = std::max(0.0f, resize_start_.y - mouse_movement_.y);

        if (is_resizing_width_ && is_resizing_height_)
            sized_object_->set_dimensions(vector2f(width, height));
        else if (is_resizing_width_)
            sized_object_->set_width(width);
        else if (is_resizing_height_)
            sized_object_->set_height(height);
    }

    if (dragged_frame_) {
        event_data data;
        data.add(args.motion.x);
        data.add(args.motion.y);
        data.add(args.position.x);
        data.add(args.position.y);
        dragged_frame_->fire_script("OnDragMove", data);
    }

    if (hovered_frame_) {
        event_data data;
        data.add(args.motion.x);
        data.add(args.motion.y);
        data.add(args.position.x);
        data.add(args.position.y);
        hovered_frame_->fire_script("OnMouseMove", data);
        return true;
    }

    // Forward to the world
    return false;
}

bool root::on_mouse_wheel_(const input::mouse_wheel_data& args) {
    utils::observer_ptr<frame> hovered_frame = find_topmost_frame([&](const frame& obj) {
        return obj.is_in_region(args.position) && obj.is_mouse_wheel_enabled();
    });

    if (hovered_frame) {
        event_data data;
        data.add(args.motion);
        data.add(args.position.x);
        data.add(args.position.y);
        hovered_frame->fire_script("OnMouseWheel", data);
        return true;
    }

    // Forward to the world
    return false;
}

bool root::on_drag_start_(const input::mouse_drag_start_data& args) {
    utils::observer_ptr<frame> hovered_frame = find_topmost_frame([&](const frame& obj) {
        return obj.is_in_region(args.position) && obj.is_mouse_click_enabled();
    });

    if (!hovered_frame) {
        // Forward to the world
        return false;
    }

    if (auto* reg = hovered_frame->get_title_region().get();
        reg && reg->is_in_region(args.position)) {
        hovered_frame->start_moving();
    }

    std::string button_name = std::string(input::get_mouse_button_codename(args.button));

    if (hovered_frame->is_drag_enabled(button_name)) {
        event_data data;
        data.add(static_cast<std::underlying_type_t<input::key>>(args.button));
        data.add(button_name);
        data.add(args.position.x);
        data.add(args.position.y);

        dragged_frame_ = std::move(hovered_frame);
        dragged_frame_->fire_script("OnDragStart", data);
    }

    return true;
}

bool root::on_drag_stop_(const input::mouse_drag_stop_data& args) {
    stop_moving();
    stop_sizing();

    if (dragged_frame_) {
        dragged_frame_->fire_script("OnDragStop");
        dragged_frame_ = nullptr;
    }

    utils::observer_ptr<frame> hovered_frame = find_topmost_frame([&](const frame& obj) {
        return obj.is_in_region(args.position) && obj.is_mouse_click_enabled();
    });

    if (!hovered_frame) {
        // Forward to the world
        return false;
    }

    std::string button_name = std::string(input::get_mouse_button_codename(args.button));

    if (hovered_frame->is_drag_enabled(button_name)) {
        event_data data;
        data.add(static_cast<std::underlying_type_t<input::key>>(args.button));
        data.add(button_name);
        data.add(args.position.x);
        data.add(args.position.y);

        hovered_frame->fire_script("OnReceiveDrag", data);
    }

    return true;
}

bool root::on_text_entered_(const input::text_entered_data& args) {
    if (auto focus = get_focused_frame()) {
        event_data data;
        data.add(utils::unicode_to_utf8(utils::ustring(1, args.character)));
        data.add(args.character);

        focus->fire_script("OnChar", data);
        return true;
    }

    // Forward to the world
    return false;
}

std::string
get_key_name(input::key key_id, bool is_shift_pressed, bool is_ctrl_pressed, bool is_alt_pressed) {
    std::string name;

    if (key_id != input::key::k_lcontrol && key_id != input::key::k_rcontrol &&
        key_id != input::key::k_lshift && key_id != input::key::k_rshift &&
        key_id != input::key::k_lmenu && key_id != input::key::k_rmenu) {
        if (is_ctrl_pressed)
            name = "Ctrl-";
        if (is_alt_pressed)
            name.append("Alt-");
        if (is_shift_pressed)
            name.append("Shift-");
    }

    name.append(input::get_key_codename(key_id));

    return name;
}

bool root::on_key_state_changed_(input::key key_id, bool is_down, bool is_repeat) {
    const auto& input_dispatcher = get_manager().get_input_dispatcher();
    bool        is_shift_pressed = input_dispatcher.shift_is_pressed();
    bool        is_ctrl_pressed  = input_dispatcher.ctrl_is_pressed();
    bool        is_alt_pressed   = input_dispatcher.alt_is_pressed();

    std::string key_name = get_key_name(key_id, is_shift_pressed, is_ctrl_pressed, is_alt_pressed);

    // First, give priority to the focused frame
    utils::observer_ptr<frame> topmost_frame = get_focused_frame();

    // If no focused frame with keyboard enabled, look top-down for a frame that captures this key
    if (!topmost_frame || !topmost_frame->is_keyboard_enabled()) {
        topmost_frame = find_topmost_frame([&](const frame& frame) {
            return frame.is_keyboard_enabled() && frame.is_key_capture_enabled(key_name);
        });
    }

    // If a frame is found, capture input and return
    if (topmost_frame) {
        event_data data;
        data.add(static_cast<std::underlying_type_t<input::key>>(key_id));
        data.add(is_shift_pressed);
        data.add(is_ctrl_pressed);
        data.add(is_alt_pressed);
        data.add(key_name);

        if (is_down) {
            if (is_repeat) {
                topmost_frame->fire_script("OnKeyRepeat", data);
            } else {
                topmost_frame->fire_script("OnKeyDown", data);
            }
        } else {
            topmost_frame->fire_script("OnKeyUp", data);
        }

        return true;
    }

    if (is_down && !is_repeat) {
        // If no frame is found, try the key_binder
        try {
            if (get_key_binder().on_key_down(
                    key_id, is_shift_pressed, is_ctrl_pressed, is_alt_pressed)) {
                return true;
            }
        } catch (const std::exception& e) {
            std::string err = e.what();
            gui::out << gui::error << err << std::endl;
            get_manager().get_event_emitter().fire_event("LUA_ERROR", {err});
            return true;
        }
    }

    // Forward to the world
    return false;
}

bool root::on_mouse_button_state_changed_(
    input::mouse_button button_id,
    bool                is_down,
    bool                is_double_click,
    bool                was_dragged,
    const vector2f&     mouse_pos) {
    utils::observer_ptr<frame> hovered_frame = find_topmost_frame([&](const frame& frame) {
        return frame.is_in_region(mouse_pos) && frame.is_mouse_click_enabled();
    });

    if (is_down && !is_double_click) {
        if (!hovered_frame || hovered_frame != get_focused_frame())
            clear_focus();
    }

    if (!hovered_frame) {
        // Forward to the world
        return false;
    }

    event_data data;
    data.add(static_cast<std::underlying_type_t<input::key>>(button_id));
    data.add(std::string(input::get_mouse_button_codename(button_id)));
    data.add(mouse_pos.x);
    data.add(mouse_pos.y);

    if (is_double_click) {
        hovered_frame->fire_script("OnDoubleClicked", data);
    } else if (is_down) {
        if (auto* top_level = hovered_frame->get_top_level_parent().get())
            top_level->raise();

        hovered_frame->fire_script("OnMouseDown", data);
    } else {
        data.add(was_dragged);
        hovered_frame->fire_script("OnMouseUp", data);
    }

    return true;
}

} // namespace lxgui::gui
