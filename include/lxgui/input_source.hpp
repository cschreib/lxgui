#ifndef LXGUI_INPUT_SOURCE_HPP
#define LXGUI_INPUT_SOURCE_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/input_keys.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_signal.hpp"
#include "lxgui/utils_string.hpp"

#include <array>
#include <string>
#include <vector>

namespace lxgui::input {

/**
 * \brief The base class for input source implementation
 * \details The implementation is responsible for generating the
 * following low-level events:
 *  - @ref on_mouse_moved
 *  - @ref on_mouse_wheel
 *  - @ref on_mouse_pressed
 *  - @ref on_mouse_released
 *  - @ref on_key_pressed
 *  - @ref on_key_pressed_repeat
 *  - @ref on_key_released
 *  - @ref on_text_entered
 *  - @ref on_window_resized
 *
 * These events are "raw", straight from the input implementation.
 * They are meant to be consumed by the @ref input::dispatcher, which
 * takes care of transforming them (apply scaling factors, etc.),
 * and generating more complex events (drag, double-click, etc.).
 * Therefore, do not use these events directly unless you are really
 * after the raw input events, and prefer using @ref input::dispatcher
 * instead.
 */
class source {
public:
    struct key_state {
        std::array<bool, key_number> is_key_down = {};
    };

    struct mouse_state {
        std::array<bool, mouse_button_number> is_button_down = {};
        gui::vector2f                         position;
        float                                 wheel = 0.0f;
    };

    /// Constructor.
    source() = default;

    /// Destructor.
    virtual ~source() = default;

    // Non-copiable, non-movable
    source(const source&) = delete;
    source(source&&)      = delete;
    source& operator=(const source&) = delete;
    source& operator=(source&&) = delete;

    /// Returns the keyboard state of this input source.
    const key_state& get_key_state() const;

    /// Returns the mouse state of this input source.
    const mouse_state& get_mouse_state() const;

    /**
     * \brief Get the window size (in pixels)
     * \return The window size
     */
    const gui::vector2ui& get_window_dimensions() const;

    /**
     * \brief Retrieve a copy of the clipboard content.
     * \return A copy of the clipboard content (empty string if clipboard is empty).
     */
    virtual utils::ustring get_clipboard_content() = 0;

    /**
     * \brief Replace the content of the clipboard.
     * \param content The new clipboard content
     */
    virtual void set_clipboard_content(const utils::ustring& content) = 0;

    /**
     * \brief Sets the mouse cursor to a given image on disk.
     * \param file_name The cursor image
     * \param hot_spot The pixel position of the tip of the pointer in the image
     * \note Use reset_mouse_cursor() to set the cursor back to the default.
     */
    virtual void set_mouse_cursor(const std::string& file_name, const gui::vector2i& hot_spot) = 0;

    /// Sets the mouse cursor back to the default (arrow).
    virtual void reset_mouse_cursor() = 0;

    /**
     * \brief Return the interface scaling factor suggested by the operating system.
     * \return The interface scaling factor suggested by the operating system
     * \note This is implementation-dependent; not all input implementations are able
     * to produce this hint, in which case the function always returns 1.
     * Consequently, it is recommended to not rely blindly on this hint, and
     * to offer a way for the user to change the scaling factor. But this can be used
     * for a good default value.
     */
    virtual float get_interface_scaling_factor_hint() const;

    /**
     * \brief Signal triggered when the mouse moves
     * \details Arguments:
     *  - mouse motion that generated this event, in pixels
     *  - mouse position, in pixels
     */
    utils::signal<void(const gui::vector2f&, const gui::vector2f&)> on_mouse_moved;

    /**
     * \brief Signal triggered when the mouse wheel is moved
     * \details Arguments:
     *  - mouse wheel motion that generated this event
     *  - mouse position, in pixels
     */
    utils::signal<void(float, const gui::vector2f&)> on_mouse_wheel;

    /**
     * \brief Signal triggered when a mouse button is pressed
     * \details Arguments:
     *  - mouse button that generated this event
     *  - mouse position, in pixels
     */
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_pressed;

    /**
     * \brief Signal triggered when a mouse button is released
     * \details Arguments:
     *  - mouse button that generated this event
     *  - mouse position, in pixels
     */
    utils::signal<void(input::mouse_button, const gui::vector2f&)> on_mouse_released;

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

    /**
     * \brief Signal triggered whenever the window is resized or changes resolution.
     * \details Arguments:
     *  - New size of the window, in pixels
     */
    utils::signal<void(const gui::vector2ui&)> on_window_resized;

protected:
    key_state   keyboard_;
    mouse_state mouse_;

    gui::vector2ui window_dimensions_;
};

} // namespace lxgui::input

#endif
