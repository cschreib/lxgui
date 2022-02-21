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
    frame_container(mgr.get_factory(), object_registry_, this),
    utils::enable_observer_from_this<root>(block),
    manager_(mgr),
    renderer_(mgr.get_renderer()),
    world_input_dispatcher_(mgr.get_world_input_dispatcher()) {
    auto& window       = get_manager().get_window();
    screen_dimensions_ = window.get_dimensions();

    connections_.push_back(
        window.on_window_resized.connect([&](auto... args) { on_window_resized_(args...); }));

    auto& input_dispatcher = get_manager().get_input_dispatcher();

    connections_.push_back(
        input_dispatcher.on_mouse_moved.connect([&](auto... args) { on_mouse_moved_(args...); }));

    connections_.push_back(
        input_dispatcher.on_mouse_wheel.connect([&](auto... args) { on_mouse_wheel_(args...); }));

    connections_.push_back(input_dispatcher.on_mouse_drag_start.connect(
        [&](auto... args) { on_drag_start_(args...); }));

    connections_.push_back(
        input_dispatcher.on_mouse_drag_stop.connect([&](auto... args) { on_drag_stop_(args...); }));

    connections_.push_back(
        input_dispatcher.on_text_entered.connect([&](auto... args) { on_text_entered_(args...); }));

    connections_.push_back(input_dispatcher.on_mouse_pressed.connect(
        [&](input::mouse_button button, const vector2f& mouse_pos) {
            on_mouse_button_state_changed_(button, true, false, mouse_pos);
        }));

    connections_.push_back(input_dispatcher.on_mouse_released.connect(
        [&](input::mouse_button button, const vector2f& mouse_pos) {
            on_mouse_button_state_changed_(button, false, false, mouse_pos);
        }));

    connections_.push_back(input_dispatcher.on_mouse_double_clicked.connect(
        [&](input::mouse_button button, const vector2f& mouse_pos) {
            on_mouse_button_state_changed_(button, true, true, mouse_pos);
        }));

    connections_.push_back(input_dispatcher.on_key_pressed.connect(
        [&](input::key key) { on_key_state_changed_(key, true); }));

    connections_.push_back(input_dispatcher.on_key_released.connect(
        [&](input::key key) { on_key_state_changed_(key, false); }));
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

void root::create_strata_cache_render_target_(strata& strata_obj) {
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

            moved_object_->clear_all_points();
            moved_object_->set_point(anchor_point::top_left, "", borders.top_left());

            moved_anchor_ = &moved_object_->modify_point(anchor_point::top_left);

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

void root::start_sizing(utils::observer_ptr<region> obj, anchor_point point) {
    moved_object_   = nullptr;
    sized_object_   = std::move(obj);
    mouse_movement_ = vector2f::zero;

    if (sized_object_) {
        const bounds2f borders = sized_object_->get_borders();

        anchor_point opposite_point = anchor_point::center;
        vector2f     offset;

        switch (point) {
        case anchor_point::top_left:
        case anchor_point::top:
            opposite_point           = anchor_point::bottom_right;
            offset                   = borders.bottom_right();
            is_resizing_from_right_  = false;
            is_resizing_from_bottom_ = false;
            break;
        case anchor_point::top_right:
        case anchor_point::right:
            opposite_point           = anchor_point::bottom_left;
            offset                   = borders.bottom_left();
            is_resizing_from_right_  = true;
            is_resizing_from_bottom_ = false;
            break;
        case anchor_point::bottom_right:
        case anchor_point::bottom:
            opposite_point           = anchor_point::top_left;
            offset                   = borders.top_left();
            is_resizing_from_right_  = true;
            is_resizing_from_bottom_ = true;
            break;
        case anchor_point::bottom_left:
        case anchor_point::left:
            opposite_point           = anchor_point::top_right;
            offset                   = borders.top_right();
            is_resizing_from_right_  = false;
            is_resizing_from_bottom_ = true;
            break;
        case anchor_point::center:
            gui::out << gui::error << "gui::manager: "
                     << "Cannot resize \"" << sized_object_->get_name() << "\" from its center."
                     << std::endl;
            sized_object_ = nullptr;
            return;
        }

        sized_object_->clear_all_points();
        sized_object_->set_point(opposite_point, "", anchor_point::top_left, offset);

        resize_start_ = sized_object_->get_apparent_dimensions();

        if (point == anchor_point::left || point == anchor_point::right) {
            is_resizing_width_  = true;
            is_resizing_height_ = false;
        } else if (point == anchor_point::top || point == anchor_point::bottom) {
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
    if (hovered_frame_ && obj != hovered_frame_)
        hovered_frame_->notify_mouse_in_frame(false, mouse_pos);

    if (obj) {
        hovered_frame_ = std::move(obj);
        hovered_frame_->notify_mouse_in_frame(true, mouse_pos);
    } else
        clear_hovered_frame_();
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

void root::on_mouse_moved_(const vector2f& movement, const vector2f& mouse_pos) {
    notify_hovered_frame_dirty();

    if (moved_object_ || sized_object_) {
        DEBUG_LOG(" Moved object...");
        mouse_movement_ += movement;
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
        data.add(mouse_pos.x);
        data.add(mouse_pos.y);
        dragged_frame_->fire_script("OnDragMove", data);
    }

    if (!hovered_frame_) {
        // Forward to the world
        world_input_dispatcher_.on_mouse_moved(movement, mouse_pos);
    }
}

void root::on_mouse_wheel_(float wheel_scroll, const vector2f& mouse_pos) {
    utils::observer_ptr<frame> hovered_frame = find_topmost_frame([&](const frame& obj) {
        return obj.is_in_region(mouse_pos) && obj.is_mouse_wheel_enabled();
    });

    if (!hovered_frame) {
        // Forward to the world
        world_input_dispatcher_.on_mouse_wheel(wheel_scroll, mouse_pos);
        return;
    }

    event_data data;
    data.add(wheel_scroll);
    data.add(mouse_pos.x);
    data.add(mouse_pos.y);
    hovered_frame->fire_script("OnMouseWheel", data);
}

void root::on_drag_start_(input::mouse_button button, const vector2f& mouse_pos) {
    utils::observer_ptr<frame> hovered_frame = find_topmost_frame([&](const frame& obj) {
        return obj.is_in_region(mouse_pos) && obj.is_mouse_click_enabled();
    });

    if (!hovered_frame) {
        // Forward to the world
        world_input_dispatcher_.on_mouse_drag_start(button, mouse_pos);
        return;
    }

    if (auto* reg = hovered_frame->get_title_region().get(); reg && reg->is_in_region(mouse_pos)) {
        hovered_frame->start_moving();
    }

    std::string mouse_button = std::string(input::get_mouse_button_codename(button));

    if (hovered_frame->is_registered_for_drag(mouse_button)) {
        event_data data;
        data.add(mouse_button);
        data.add(mouse_pos.x);
        data.add(mouse_pos.y);

        dragged_frame_ = std::move(hovered_frame);
        dragged_frame_->fire_script("OnDragStart", data);
    }
}

void root::on_drag_stop_(input::mouse_button button, const vector2f& mouse_pos) {
    stop_moving();
    stop_sizing();

    if (dragged_frame_) {
        dragged_frame_->fire_script("OnDragStop");
        dragged_frame_ = nullptr;
    }

    utils::observer_ptr<frame> hovered_frame = find_topmost_frame([&](const frame& obj) {
        return obj.is_in_region(mouse_pos) && obj.is_mouse_click_enabled();
    });

    if (!hovered_frame) {
        // Forward to the world
        world_input_dispatcher_.on_mouse_drag_stop(button, mouse_pos);
        return;
    }

    std::string mouse_button = std::string(input::get_mouse_button_codename(button));

    if (hovered_frame->is_registered_for_drag(mouse_button)) {
        event_data data;
        data.add(mouse_button);
        data.add(mouse_pos.x);
        data.add(mouse_pos.y);

        hovered_frame->fire_script("OnReceiveDrag", data);
    }
}

void root::on_text_entered_(std::uint32_t c) {
    if (auto focus = get_focused_frame()) {
        event_data data;
        data.add(utils::unicode_to_utf8(utils::ustring(1, c)));
        data.add(c);

        focus->fire_script("OnChar", data);
    } else {
        // Forward to the world
        world_input_dispatcher_.on_text_entered(c);
    }
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

void root::on_key_state_changed_(input::key key_id, bool is_down) {
    const auto& input_dispatcher = get_manager().get_input_dispatcher();
    bool        is_shift_pressed = input_dispatcher.shift_is_pressed();
    bool        is_ctrl_pressed  = input_dispatcher.ctrl_is_pressed();
    bool        is_alt_pressed   = input_dispatcher.alt_is_pressed();

    std::string key_name = get_key_name(key_id, is_shift_pressed, is_ctrl_pressed, is_alt_pressed);

    // First, give priority to the focused frame
    utils::observer_ptr<frame> topmost_frame = get_focused_frame();

    // If no focused frame, look top-down for a frame that captures this key
    if (!topmost_frame) {
        topmost_frame = find_topmost_frame(
            [&](const frame& frame) { return frame.is_key_capture_enabled(key_name); });
    }

    // If a frame is found, capture input and return
    if (topmost_frame) {
        event_data data;
        data.add(static_cast<std::underlying_type_t<input::key>>(key_id));
        data.add(key_name);
        data.add(is_shift_pressed);
        data.add(is_ctrl_pressed);
        data.add(is_alt_pressed);

        if (is_down)
            topmost_frame->fire_script("OnKeyDown", data);
        else
            topmost_frame->fire_script("OnKeyUp", data);

        return;
    }

    if (is_down) {
        // If no frame is found, try the keybinder
        try {
            if (get_keybinder().on_key_down(
                    key_id, is_shift_pressed, is_ctrl_pressed, is_alt_pressed)) {
                return;
            }
        } catch (const std::exception& e) {
            std::string err = e.what();
            gui::out << gui::error << err << std::endl;
            get_manager().get_event_emitter().fire_event("LUA_ERROR", {err});
            return;
        }
    }

    // Forward to the world
    if (is_down)
        world_input_dispatcher_.on_key_pressed(key_id);
    else
        world_input_dispatcher_.on_key_released(key_id);
}

void root::on_mouse_button_state_changed_(
    input::mouse_button button, bool is_down, bool is_double_click, const vector2f& mouse_pos) {
    utils::observer_ptr<frame> hovered_frame = find_topmost_frame([&](const frame& frame) {
        return frame.is_in_region(mouse_pos) && frame.is_mouse_click_enabled();
    });

    if (is_down && !is_double_click) {
        if (!hovered_frame || hovered_frame != get_focused_frame())
            clear_focus();
    }

    if (!hovered_frame) {
        // Forward to the world
        if (is_double_click)
            world_input_dispatcher_.on_mouse_double_clicked(button, mouse_pos);
        else if (is_down)
            world_input_dispatcher_.on_mouse_pressed(button, mouse_pos);
        else
            world_input_dispatcher_.on_mouse_released(button, mouse_pos);
        return;
    }

    event_data data;
    data.add(std::string(input::get_mouse_button_codename(button)));
    data.add(mouse_pos.x);
    data.add(mouse_pos.y);

    if (is_double_click) {
        hovered_frame->fire_script("OnDoubleClicked", data);
    } else if (is_down) {
        if (auto* top_level = hovered_frame->get_top_level_parent().get())
            top_level->raise();

        hovered_frame->fire_script("OnMouseDown", data);
    } else {
        hovered_frame->fire_script("OnMouseUp", data);
    }
}

} // namespace lxgui::gui
