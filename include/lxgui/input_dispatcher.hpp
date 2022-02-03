#ifndef LXGUI_INPUT_DISPATCHER_HPP
#define LXGUI_INPUT_DISPATCHER_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/input_keys.hpp"
#include "lxgui/input_signals.hpp"
#include "lxgui/gui_vector2.hpp"

#include "lxgui/utils_signal.hpp"

#include <string>
#include <vector>
#include <array>
#include <chrono>

namespace lxgui {
namespace gui
{
    class event;
    class event_emitter;
}

namespace input
{
    class source;

    /// Handles inputs (keyboard and mouse)
    /** The implementation is responsible for generating the
    *   following events:
    *    - @ref on_mouse_moved
    *    - @ref on_mouse_wheel
    *    - @ref on_mouse_pressed
    *    - @ref on_mouse_released
    *    - @ref on_mouse_double_clicked
    *    - @ref on_mouse_drag_start
    *    - @ref on_mouse_drag_stop
    *    - @ref on_key_pressed
    *    - @ref on_key_released
    *    - @ref on_text_entered
    *
    *   These events are "global" and are not restricted by the UI.
    *   For example, @ref on_mouse_pressed will trigger whenever a mouse
    *   button is pressed, even if the mouse pointer is currently over
    *   a UI element that should capture mouse input. Likewise, @ref
    *   on_key_pressed will trigger even if a UI element has focus.
    *   These global events are meant to be consumed by the @ref gui::root,
    *   which takes care of forwarding them to UI elements, and determining
    *   if a particular event is allowed to propagate to the elements below
    *   the UI. If you need to react only to events that are not captured by
    *   the UI, use events from @ref input::world_dispatcher instead.
    */
    class dispatcher : public signals
    {
    public :

        /// Initializes this dispatcher with a chosen input source.
        /** \param mSource The input source
        */
        explicit dispatcher(source& mSource);

        // Non-copiable, non-movable
        dispatcher(const dispatcher&) = delete;
        dispatcher(dispatcher&&) = delete;
        dispatcher& operator=(const dispatcher&) = delete;
        dispatcher& operator=(dispatcher&&) = delete;

        /// Checks if any key is being pressed.
        /** \return 'true' if any key is being pressed
        */
        bool any_key_is_down() const;

        /// Checks if a key is being pressed.
        /** \param mKey   The ID code of the key you are interested in
        *   \return 'true' if the key is being pressed
        */
        bool key_is_down(key mKey) const;

        /// Returns elapsed time since the key has been pressed.
        /** \param mKey The ID code of the key you are interested in
        *   \return Elapsed time since the key has been pressed
        */
        double get_key_down_duration(key mKey) const;

        /// Checks if Alt is beeing pressed.
        /** \return 'true' if Alt is beeing pressed
        */
        bool alt_is_pressed() const;

        /// Checks if Shift is beeing pressed.
        /** \return 'true' if Shift is beeing pressed
        */
        bool shift_is_pressed() const;

        /// Checks if Control (Ctrl) is beeing pressed.
        /** \return 'true' if Control (Ctrl) is beeing pressed
        */
        bool ctrl_is_pressed() const;

        /// Checks if a mouse button is being pressed.
        /** \param mID The ID code of the mouse button you are interested in
        *   \return 'true' if the mouse button is being pressed
        */
        bool mouse_is_down(mouse_button mID) const;

        /// Returns elapsed time since the mouse button has been pressed.
        /** \param mKey The ID code of the mouse button you are interested in
        *   \return Elapsed time since the mouse button has been pressed
        */
        double get_mouse_down_duration(mouse_button mKey) const;

        /// Returns the position of the mouse in pixels.
        /** \return The position of the mouse in pixels
        */
        gui::vector2f get_mouse_position() const;

        /// Returns the accumulated rolling ammount of the mouse wheel.
        /** \return The accumulated rolling ammount of the mouse wheel
        */
        float get_mouse_wheel() const;

        /// Sets the double click maximum time.
        /** \param dDoubleClickTime Maximum amount of time between two clicks in a double click
        */
        void set_doubleclick_time(double dDoubleClickTime);

        /// Returns the double click maximum time.
        /** \return The double click maximum time
        */
        double get_doubleclick_time() const;

        /// Sets the scaling factor applied to the interface.
        /** \param fScalingFactor The new scaling factor (default: 1)
        *   \note This is the conversion factor between UI units and pixels in the display.
        *         This factor should match gui::renderer::get_interface_scaling_factor().
        */
        void set_interface_scaling_factor(float fScalingFactor);

        /// Return the current interface scaling factor.
        /** \return The current interface scaling factor
        */
        float get_interface_scaling_factor() const;

        /// Returns the input source.
        /** \return The input source
        */
        const source& get_source() const;

        /// Returns the input source.
        /** \return The input source
        */
        source& get_source();

    private :

        std::vector<utils::scoped_connection> lConnections_;

        using timer = std::chrono::high_resolution_clock;
        using time_point = timer::time_point;

        std::array<time_point, KEY_NUMBER>          lKeyPressedTime_ = {};
        std::array<time_point, MOUSE_BUTTON_NUMBER> lMousePressedTime_ = {};

        float fScalingFactor_ = 1.0f;

        double dDoubleClickTime_ = 0.25;

        bool          bMouseDragged_ = false;
        mouse_button  mMouseDragButton_ = mouse_button::LEFT;

        source& mSource_;
    };
}
}

#endif
