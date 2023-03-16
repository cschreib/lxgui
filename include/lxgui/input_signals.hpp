#ifndef LXGUI_INPUT_SIGNALS_HPP
#define LXGUI_INPUT_SIGNALS_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_signal.hpp"

namespace lxgui::input {

/// Data for on_mouse_moved signal
struct mouse_moved_data {
    gui::vector2f motion; /// Mouse motion that generated this event, in points
    gui::vector2f position; /// Mouse position, in points
};

/// Data for on_mouse_wheel signal
struct mouse_wheel_data {
    float         motion; /// Mouse wheel motion that generated this event
    gui::vector2f position; /// Mouse position, in points
};

/// Data for on_mouse_pressed signal
struct mouse_pressed_data {
    input::mouse_button button; /// Mouse button that generated this event
    gui::vector2f       position; /// Mouse position, in points
};

/// Data for on_mouse_released signal
struct mouse_released_data {
    input::mouse_button button; /// Mouse button that generated this event
    gui::vector2f       position; /// Mouse position, in points
    bool                was_dragged; /// Was mouse dragged before release?
};

/// Data for on_mouse_double_clicked signal
struct mouse_double_clicked_data {
    input::mouse_button button; /// Mouse button that generated this event
    gui::vector2f       position; /// Mouse position, in points
};

/// Data for on_mouse_drag_start signal
struct mouse_drag_start_data {
    input::mouse_button button; /// Mouse button that generated this event (if more than one, only
                                /// the first pressed)
    gui::vector2f position; /// Mouse position, in points
};

/// Data for on_mouse_drag_stop signal
struct mouse_drag_stop_data {
    input::mouse_button button; /// Mouse button that generated this event (if more than one, only
                                /// the first pressed)
    gui::vector2f position; /// Mouse position, in points
};

/// Data for on_key_pressed signal
struct key_pressed_data {
    input::key key; /// Keyboard key that generated this event
};

/// Data for on_key_pressed_repeat signal
struct key_pressed_repeat_data {
    input::key key; /// Keyboard key that generated this event
};

/// Data for on_key_released signal
struct key_released_data {
    input::key key; /// Keyboard key that generated this event
};

/// Data for on_text_entered signal
struct text_entered_data {
    std::uint32_t character; /// Unicode UTF-32 code point of the typed character
};

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

    /**
     * \brief Signal triggered when the mouse moves
     */
    utils::signal<void(const mouse_moved_data&)> on_mouse_moved;

    /**
     * \brief Signal triggered when the mouse wheel is moved
     */
    utils::signal<void(const mouse_wheel_data&)> on_mouse_wheel;

    /**
     * \brief Signal triggered when a mouse button is pressed
     */
    utils::signal<void(const mouse_pressed_data&)> on_mouse_pressed;

    /**
     * \brief Signal triggered when a mouse button is released
     */
    utils::signal<void(const mouse_released_data&)> on_mouse_released;

    /**
     * \brief Signal triggered when a mouse button is double clicked
     */
    utils::signal<void(const mouse_double_clicked_data&)> on_mouse_double_clicked;

    /**
     * \brief Signal triggered when the mouse starts a drag operation
     */
    utils::signal<void(const mouse_drag_start_data&)> on_mouse_drag_start;

    /**
     * \brief Signal triggered when the mouse ends a drag operation
     */
    utils::signal<void(const mouse_drag_stop_data&)> on_mouse_drag_stop;

    /**
     * \brief Signal triggered when a keyboard key is pressed
     */
    utils::signal<void(const key_pressed_data&)> on_key_pressed;

    /**
     * \brief Signal triggered when a keyboard key is long-pressed and repeats
     */
    utils::signal<void(const key_pressed_repeat_data&)> on_key_pressed_repeat;

    /**
     * \brief Signal triggered when a keyboard key is released
     */
    utils::signal<void(const key_released_data&)> on_key_released;

    /**
     * \brief Signal triggered when text is entered
     * \note The event will trigger repeatedly if more than one character is generated.
     */
    utils::signal<void(const text_entered_data&)> on_text_entered;
};

} // namespace lxgui::input

#endif
