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

    /**
     * \brief Signal triggered when the mouse moves
     * \details Arguments:
     *  - mouse motion that generated this event, in points
     *  - mouse position, in points
     */
    utils::signal<void(const gui::vector2f&, const gui::vector2f&)> on_mouse_moved;

    /**
     * \brief Signal triggered when the mouse wheel is moved
     * \details Arguments:
     *  - mouse wheel motion that generated this event
     *  - mouse position, in points
     */
    utils::signal<void(float, const gui::vector2f&)> on_mouse_wheel;

    /**
     * \brief Signal triggered when a mouse button is pressed
     * \details Arguments:
     *  - mouse button that generated this event
     *  - mouse position, in points
     */
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_pressed;

    /**
     * \brief Signal triggered when a mouse button is released
     * \details Arguments:
     *  - mouse button that generated this event
     *  - mouse position, in points
     */
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_released;

    /**
     * \brief Signal triggered when a mouse button is double clicked
     * \details Arguments:
     *  - mouse button that generated this event
     *  - mouse position, in points
     */
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_double_clicked;

    /**
     * \brief Signal triggered when the mouse starts a drag operation
     * \details Arguments:
     *  - mouse button that is pressed (if more than one, only the first pressed)
     *  - mouse position, in points
     */
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_drag_start;

    /**
     * \brief Signal triggered when the mouse ends a drag operation
     * \details Arguments:
     *  - mouse button that was pressed (if more than one, only the first pressed)
     *  - mouse position, in points
     */
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_drag_stop;

    /**
     * \brief Signal triggered when a keyboard key is pressed
     * \details Arguments:
     *  - keyboard key that generated this event
     */
    utils::signal<void(input::key)> on_key_pressed;

    /**
     * \brief Signal triggered when a keyboard key is long-pressed and repeats
     * \details Arguments:
     *  - keyboard key that generated this event
     */
    utils::signal<void(input::key)> on_key_pressed_repeat;

    /**
     * \brief Signal triggered when a keyboard key is released
     * \details Arguments:
     *  - keyboard key that generated this event
     */
    utils::signal<void(input::key)> on_key_released;

    /**
     * \brief Signal triggered when text is entered
     * \details Arguments:
     *  - Unicode UTF-32 code point of the typed character
     * \note The event will trigger repeatedly if more than one character is generated.
     */
    utils::signal<void(std::uint32_t)> on_text_entered;
};

} // namespace lxgui::input

#endif
