#include "lxgui/input_source.hpp"

namespace lxgui { namespace input {

const source::key_state& source::get_key_state() const {
    return mKeyboard_;
}

const source::mouse_state& source::get_mouse_state() const {
    return mMouse_;
}

const gui::vector2ui& source::get_window_dimensions() const {
    return mWindowDimensions_;
}

float source::get_interface_scaling_factor_hint() const {
    return 1.0f;
}

}} // namespace lxgui::input
