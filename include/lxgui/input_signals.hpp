#ifndef LXGUI_INPUT_SIGNALS_HPP
#define LXGUI_INPUT_SIGNALS_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_signal.hpp"

namespace lxgui::input {

/// Stores signals for input events.
class signals {
public:
    /// Default constructor.
    signals() = default;

    // Non-movable, non-copiable
    signals(const signals&) = delete;
    signals(signals&&)      = delete;
    signals& operator=(const signals&) = delete;
    signals& operator=(signals&&) = delete;

    /// Signal triggered when the mouse moves
    utils::signal<void(const gui::vector2f&, const gui::vector2f&)> on_mouse_moved;
    /// Signal triggered when the mouse wheel is moved
    utils::signal<void(float, const gui::vector2f&)> on_mouse_wheel;
    /// Signal triggered when a mouse button is pressed
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_pressed;
    /// Signal triggered when a mouse button is released
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_released;
    /// Signal triggered when a mouse button is double clicked
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_double_clicked;
    /// Signal triggered when the mouse starts a drag operation
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_drag_start;
    /// Signal triggered when the mouse ends a drag operation
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_drag_stop;
    /// Signal triggered when a keyboard key is pressed
    utils::signal<void(input::key)> on_key_pressed;
    /// Signal triggered when a keyboard key is released
    utils::signal<void(input::key)> on_key_released;
    /// Signal triggered when text is entered
    utils::signal<void(std::uint32_t)> on_text_entered;
};

} // namespace lxgui::input

#endif
