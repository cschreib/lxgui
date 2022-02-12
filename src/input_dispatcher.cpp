#include "lxgui/input_dispatcher.hpp"

#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input_source.hpp"
#include "lxgui/utils_exception.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <iostream>

namespace lxgui::input {

dispatcher::dispatcher(source& m_source) : m_source_(m_source) {
    l_connections_.push_back(m_source.on_key_pressed.connect([&](input::key m_key) {
        // Record press time
        l_key_pressed_time_[static_cast<std::size_t>(m_key)] = timer::now();
        // Forward
        on_key_pressed(m_key);
    }));

    l_connections_.push_back(m_source.on_key_released.connect([&](input::key m_key) {
        // Forward
        on_key_released(m_key);
    }));

    l_connections_.push_back(m_source.on_text_entered.connect([&](std::uint32_t ui_char) {
        // Forward
        on_text_entered(ui_char);
    }));

    l_connections_.push_back(
        m_source.on_mouse_pressed.connect([&](input::mouse_button m_button, gui::vector2f m_mouse_pos) {
            // Apply scaling factor to mouse coordinates
            m_mouse_pos /= f_scaling_factor_;

            // Record press time
            auto m_time_last = l_mouse_pressed_time_[static_cast<std::size_t>(m_button)];
            auto m_time_now  = timer::now();
            l_mouse_pressed_time_[static_cast<std::size_t>(m_button)] = m_time_now;
            double d_click_time = std::chrono::duration<double>(m_time_now - m_time_last).count();

            // Forward
            on_mouse_pressed(m_button, m_mouse_pos);

            if (d_click_time < d_double_click_time_)
                on_mouse_double_clicked(m_button, m_mouse_pos);
        }));

    l_connections_.push_back(m_source.on_mouse_released.connect(
        [&](input::mouse_button m_button, gui::vector2f m_mouse_pos) {
            // Apply scaling factor to mouse coordinates
            m_mouse_pos /= f_scaling_factor_;

            // Forward
            on_mouse_released(m_button, m_mouse_pos);

            if (b_mouse_dragged_ && m_button == m_mouse_drag_button_) {
                b_mouse_dragged_ = false;
                on_mouse_drag_stop(m_button, m_mouse_pos);
            }
        }));

    l_connections_.push_back(
        m_source.on_mouse_wheel.connect([&](float f_wheel, gui::vector2f m_mouse_pos) {
            // Apply scaling factor to mouse coordinates
            m_mouse_pos /= f_scaling_factor_;
            // Forward
            on_mouse_wheel(f_wheel, m_mouse_pos);
        }));

    l_connections_.push_back(
        m_source.on_mouse_moved.connect([&](gui::vector2f m_movement, gui::vector2f m_mouse_pos) {
            // Apply scaling factor to mouse coordinates
            m_movement /= f_scaling_factor_;
            m_mouse_pos /= f_scaling_factor_;

            // Forward
            on_mouse_moved(m_movement, m_mouse_pos);

            if (!b_mouse_dragged_) {
                std::size_t ui_mouse_button_pressed = std::numeric_limits<std::size_t>::max();
                for (std::size_t i = 0; i < mouse_button_number; ++i) {
                    if (mouse_is_down(static_cast<mouse_button>(i))) {
                        ui_mouse_button_pressed = i;
                        break;
                    }
                }

                if (ui_mouse_button_pressed != std::numeric_limits<std::size_t>::max()) {
                    b_mouse_dragged_    = true;
                    m_mouse_drag_button_ = static_cast<mouse_button>(ui_mouse_button_pressed);
                    on_mouse_drag_start(m_mouse_drag_button_, m_mouse_pos);
                }
            }
        }));
}

bool dispatcher::any_key_is_down() const {
    const auto& l_key_state = m_source_.get_key_state().l_key_state;
    for (std::size_t i = 1; i < key_number; ++i) {
        if (l_key_state[i])
            return true;
    }

    return false;
}

bool dispatcher::key_is_down(key m_key) const {
    return m_source_.get_key_state().l_key_state[static_cast<std::size_t>(m_key)];
}

double dispatcher::get_key_down_duration(key m_key) const {
    if (!key_is_down(m_key))
        return 0.0;

    return std::chrono::duration<double>(
               timer::now() - l_key_pressed_time_[static_cast<std::size_t>(m_key)])
        .count();
}

bool dispatcher::mouse_is_down(mouse_button m_id) const {
    return m_source_.get_mouse_state().l_button_state[static_cast<std::size_t>(m_id)];
}

double dispatcher::get_mouse_down_duration(mouse_button m_id) const {
    if (!mouse_is_down(m_id))
        return 0.0;

    return std::chrono::duration<double>(
               timer::now() - l_mouse_pressed_time_[static_cast<std::size_t>(m_id)])
        .count();
}

void dispatcher::set_doubleclick_time(double d_double_click_time) {
    d_double_click_time_ = d_double_click_time;
}

double dispatcher::get_doubleclick_time() const {
    return d_double_click_time_;
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
    return m_source_.get_mouse_state().m_position / f_scaling_factor_;
}

float dispatcher::get_mouse_wheel() const {
    return m_source_.get_mouse_state().f_wheel;
}

const source& dispatcher::get_source() const {
    return m_source_;
}

source& dispatcher::get_source() {
    return m_source_;
}

void dispatcher::set_interface_scaling_factor(float f_scaling_factor) {
    f_scaling_factor_ = f_scaling_factor;
}

float dispatcher::get_interface_scaling_factor() const {
    return f_scaling_factor_;
}

} // namespace lxgui::input
