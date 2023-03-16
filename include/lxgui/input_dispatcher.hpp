#ifndef LXGUI_INPUT_DISPATCHER_HPP
#define LXGUI_INPUT_DISPATCHER_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/input_keys.hpp"
#include "lxgui/input_signals.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_signal.hpp"

#include <array>
#include <chrono>
#include <string>
#include <vector>

namespace lxgui::input {

class source;

/**
 * \brief Handles inputs (keyboard and mouse)
 * \details The implementation is responsible for generating the
 * following events:
 *  - @ref on_mouse_moved
 *  - @ref on_mouse_wheel
 *  - @ref on_mouse_pressed
 *  - @ref on_mouse_released
 *  - @ref on_mouse_double_clicked
 *  - @ref on_mouse_drag_start
 *  - @ref on_mouse_drag_stop
 *  - @ref on_key_pressed
 *  - @ref on_key_pressed_repeat
 *  - @ref on_key_released
 *  - @ref on_text_entered
 *
 * These events are "global" and are not restricted by the UI.
 * For example, @ref on_mouse_pressed will trigger whenever a mouse
 * button is pressed, even if the mouse pointer is currently over
 * a UI element that should capture mouse input. Likewise, @ref
 * on_key_pressed will trigger even if a UI element has focus.
 * These global events are meant to be consumed by the @ref gui::root,
 * which takes care of forwarding them to UI elements, and determining
 * if a particular event is allowed to propagate to the elements below
 * the UI. If you need to react only to events that are not captured by
 * the UI, use events from @ref input::world_dispatcher instead.
 */
class dispatcher : public signals {
public:
    /**
     * \brief Initializes this dispatcher with a chosen input source.
     * \param src The input source
     */
    explicit dispatcher(source& src);

    // Non-copiable, non-movable
    dispatcher(const dispatcher&) = delete;
    dispatcher(dispatcher&&)      = delete;
    dispatcher& operator=(const dispatcher&) = delete;
    dispatcher& operator=(dispatcher&&) = delete;

    /**
     * \brief Checks if any key is being pressed.
     * \return 'true' if any key is being pressed
     */
    bool any_key_is_down() const;

    /**
     * \brief Checks if a key is being pressed.
     * \param key_id The ID code of the key you are interested in
     * \return 'true' if the key is being pressed
     */
    bool key_is_down(key key_id) const;

    /**
     * \brief Returns elapsed time since the key has been pressed.
     * \param key_id The ID code of the key you are interested in
     * \return Elapsed time since the key has been pressed
     */
    double get_key_down_duration(key key_id) const;

    /**
     * \brief Checks if Alt is being pressed.
     * \return 'true' if Alt is being pressed
     */
    bool alt_is_pressed() const;

    /**
     * \brief Checks if Shift is being pressed.
     * \return 'true' if Shift is being pressed
     */
    bool shift_is_pressed() const;

    /**
     * \brief Checks if Control (Ctrl) is being pressed.
     * \return 'true' if Control (Ctrl) is being pressed
     */
    bool ctrl_is_pressed() const;

    /**
     * \brief Checks if a mouse button is being pressed.
     * \param button_id The ID code of the mouse button you are interested in
     * \return 'true' if the mouse button is being pressed
     */
    bool mouse_is_down(mouse_button button_id) const;

    /**
     * \brief Returns elapsed time since the mouse button has been pressed.
     * \param key_id The ID code of the mouse button you are interested in
     * \return Elapsed time since the mouse button has been pressed
     */
    double get_mouse_down_duration(mouse_button key_id) const;

    /**
     * \brief Returns the position of the mouse in pixels.
     * \return The position of the mouse in pixels
     */
    gui::vector2f get_mouse_position() const;

    /**
     * \brief Returns the accumulated rolling amount of the mouse wheel.
     * \return The accumulated rolling amount of the mouse wheel
     */
    float get_mouse_wheel() const;

    /**
     * \brief Sets the double click maximum time.
     * \param double_click_time Maximum amount of time between two clicks in a double click
     */
    void set_doubleclick_time(double double_click_time);

    /**
     * \brief Returns the double click maximum time.
     * \return The double click maximum time
     */
    double get_doubleclick_time() const;

    /**
     * \brief Sets the scaling factor applied to the interface.
     * \param scaling_factor The new scaling factor (default: 1)
     * \note This is the conversion factor between UI units and pixels in the display.
     * This factor should match gui::renderer::get_interface_scaling_factor().
     */
    void set_interface_scaling_factor(float scaling_factor);

    /**
     * \brief Return the current interface scaling factor.
     * \return The current interface scaling factor
     */
    float get_interface_scaling_factor() const;

    /**
     * \brief Returns the input source.
     * \return The input source
     */
    const source& get_source() const;

    /**
     * \brief Returns the input source.
     * \return The input source
     */
    source& get_source();

private:
    std::vector<utils::scoped_connection> connections_;

    using timer      = std::chrono::high_resolution_clock;
    using time_point = timer::time_point;

    std::array<time_point, key_number>          key_pressed_time_   = {};
    std::array<time_point, mouse_button_number> mouse_pressed_time_ = {};

    float scaling_factor_ = 1.0f;

    double double_click_time_ = 0.25;

    bool         is_mouse_dragged_  = false;
    mouse_button mouse_drag_button_ = mouse_button::left;

    source& source_;
};

} // namespace lxgui::input

#endif
