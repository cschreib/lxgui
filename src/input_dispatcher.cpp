#include "lxgui/input_dispatcher.hpp"

#include "lxgui/gui_event_emitter.hpp"
#include "lxgui/gui_event_receiver.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input_source.hpp"
#include "lxgui/utils_exception.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <iostream>

namespace lxgui::input {

dispatcher::dispatcher(source& src) : source_(src) {
    connections_.push_back(src.on_key_pressed.connect([&](key key_id) {
        // Record press time
        key_pressed_time_[static_cast<std::size_t>(key_id)] = timer::now();
        // Forward
        on_key_pressed(key_id);
    }));

    connections_.push_back(src.on_key_released.connect([&](key key_id) {
        // Forward
        on_key_released(key_id);
    }));

    connections_.push_back(src.on_text_entered.connect([&](std::uint32_t c) {
        // Forward
        on_text_entered(c);
    }));

    connections_.push_back(
        src.on_mouse_pressed.connect([&](mouse_button button_id, gui::vector2f mouse_pos) {
            // Apply scaling factor to mouse coordinates
            mouse_pos /= scaling_factor_;

            // Record press time
            auto time_last = mouse_pressed_time_[static_cast<std::size_t>(button_id)];
            auto time_now  = timer::now();
            mouse_pressed_time_[static_cast<std::size_t>(button_id)] = time_now;
            double click_time = std::chrono::duration<double>(time_now - time_last).count();

            // Forward
            on_mouse_pressed(button_id, mouse_pos);

            if (click_time < double_click_time_)
                on_mouse_double_clicked(button_id, mouse_pos);
        }));

    connections_.push_back(
        src.on_mouse_released.connect([&](mouse_button button_id, gui::vector2f mouse_pos) {
            // Apply scaling factor to mouse coordinates
            mouse_pos /= scaling_factor_;

            // Forward
            on_mouse_released(button_id, mouse_pos);

            if (is_mouse_dragged_ && button_id == mouse_drag_button_) {
                is_mouse_dragged_ = false;
                on_mouse_drag_stop(button_id, mouse_pos);
            }
        }));

    connections_.push_back(src.on_mouse_wheel.connect([&](float wheel, gui::vector2f mouse_pos) {
        // Apply scaling factor to mouse coordinates
        mouse_pos /= scaling_factor_;
        // Forward
        on_mouse_wheel(wheel, mouse_pos);
    }));

    connections_.push_back(
        src.on_mouse_moved.connect([&](gui::vector2f movement, gui::vector2f mouse_pos) {
            // Apply scaling factor to mouse coordinates
            movement /= scaling_factor_;
            mouse_pos /= scaling_factor_;

            // Forward
            on_mouse_moved(movement, mouse_pos);

            if (!is_mouse_dragged_) {
                std::size_t mouse_button_pressed = std::numeric_limits<std::size_t>::max();
                for (std::size_t i = 0; i < mouse_button_number; ++i) {
                    if (mouse_is_down(static_cast<mouse_button>(i))) {
                        mouse_button_pressed = i;
                        break;
                    }
                }

                if (mouse_button_pressed != std::numeric_limits<std::size_t>::max()) {
                    is_mouse_dragged_  = true;
                    mouse_drag_button_ = static_cast<mouse_button>(mouse_button_pressed);
                    on_mouse_drag_start(mouse_drag_button_, mouse_pos);
                }
            }
        }));
}

bool dispatcher::any_key_is_down() const {
    const auto& is_key_down = source_.get_key_state().is_key_down;
    for (std::size_t i = 1; i < key_number; ++i) {
        if (is_key_down[i])
            return true;
    }

    return false;
}

bool dispatcher::key_is_down(key key_id) const {
    return source_.get_key_state().is_key_down[static_cast<std::size_t>(key_id)];
}

double dispatcher::get_key_down_duration(key key_id) const {
    if (!key_is_down(key_id))
        return 0.0;

    return std::chrono::duration<double>(
               timer::now() - key_pressed_time_[static_cast<std::size_t>(key_id)])
        .count();
}

bool dispatcher::mouse_is_down(mouse_button button_id) const {
    return source_.get_mouse_state().is_button_down[static_cast<std::size_t>(button_id)];
}

double dispatcher::get_mouse_down_duration(mouse_button button_id) const {
    if (!mouse_is_down(button_id))
        return 0.0;

    return std::chrono::duration<double>(
               timer::now() - mouse_pressed_time_[static_cast<std::size_t>(button_id)])
        .count();
}

void dispatcher::set_doubleclick_time(double double_click_time) {
    double_click_time_ = double_click_time;
}

double dispatcher::get_doubleclick_time() const {
    return double_click_time_;
}

bool dispatcher::alt_is_pressed() const {
    return key_is_down(key::k_lmenu) || key_is_down(key::k_rmenu);
}

bool dispatcher::shift_is_pressed() const {
    return key_is_down(key::k_lshift) || key_is_down(key::k_rshift);
}

bool dispatcher::ctrl_is_pressed() const {
    return key_is_down(key::k_lcontrol) || key_is_down(key::k_rcontrol);
}

gui::vector2f dispatcher::get_mouse_position() const {
    return source_.get_mouse_state().position / scaling_factor_;
}

float dispatcher::get_mouse_wheel() const {
    return source_.get_mouse_state().wheel;
}

const source& dispatcher::get_source() const {
    return source_;
}

source& dispatcher::get_source() {
    return source_;
}

void dispatcher::set_interface_scaling_factor(float scaling_factor) {
    scaling_factor_ = scaling_factor;
}

float dispatcher::get_interface_scaling_factor() const {
    return scaling_factor_;
}

} // namespace lxgui::input
