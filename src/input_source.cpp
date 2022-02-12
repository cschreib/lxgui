#include "lxgui/input_source.hpp"

namespace lxgui::input {

const source::key_state& source::get_key_state() const {
    return m_keyboard_;
}

const source::mouse_state& source::get_mouse_state() const {
    return m_mouse_;
}

const gui::vector2ui& source::get_window_dimensions() const {
    return m_window_dimensions_;
}

float source::get_interface_scaling_factor_hint() const {
    return 1.0f;
}

} // namespace lxgui::input
