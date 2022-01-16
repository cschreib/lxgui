#ifndef LXGUI_INPUT_DISPATCHER_HPP
#define LXGUI_INPUT_DISPATCHER_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/input_keys.hpp"
#include "lxgui/gui_vector2.hpp"
#include "lxgui/gui_eventreceiver.hpp"

#include <lxgui/utils_observer.hpp>

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
    class dispatcher : public gui::event_receiver
    {
    public :

        /// Initializes this dispatcher with a chosen input source.
        /** \param mBlock        The owner pointer control block
        *   \param mSource       The input source
        *   \param mEventEmitter The event emitter that will generate input events
        */
        explicit dispatcher(utils::control_block& mBlock, source& mSource,
            gui::event_emitter& mEventEmitter);

        // Non-copiable, non-movable
        dispatcher(const dispatcher&) = delete;
        dispatcher(dispatcher&&) = delete;
        dispatcher& operator=(const dispatcher&) = delete;
        dispatcher& operator=(dispatcher&&) = delete;

        /// Called whenever an event occurs.
        /** \param mEvent The event which has occured
        *   \note Only registered events will cause this
        *         function to be called.
        */
        void on_event(const gui::event& mEvent) override;

        /// Enable/disable mouse inputs
        /** \param bBlock 'true' to block mouse input events from being generated, 'true' to allow all
        *   \note This only blocks mouse events; the state of the keyboard and mouse can always be
        *         queried using @ref key_is_down, @ref mouse_is_down, etc. Use
        *         @ref is_mouse_blocked() before using direct state queries if you want to fully
        *         honor mouse input blocking.
        */
        void block_mouse_events(bool bBlock);

        /// Check if mouse events are blocked
        /** \return 'true' if blocked, 'false' otherwise
        *   \see block_events
        */
        bool is_mouse_blocked() const;

        /// Enable/disable keyboard inputs
        /** \param bBlock 'true' to block keyboard events from being generated, 'true' to allow all
        *   \note This only blocks keyboard events; the state of the keyboard and keyboard can
        *         always be queried using @ref key_is_down, etc. Use @ref is_keyboard_blocked()
        *         before using direct state queries if you want to fully honor keyboard input
        *         blocking.
        */
        void block_keyboard_events(bool bBlock);

        /// Check if keyboard events are blocked
        /** \return 'true' if blocked, 'false' otherwise
        *   \see block_events
        */
        bool is_keyboard_blocked() const;

        /// Checks if any key is being pressed.
        /** \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if any key is being pressed
        */
        bool any_key_is_down() const;

        /// Checks if a key is being pressed.
        /** \param mKey   The ID code of the key you are interested in
        *   \return 'true' if the key is being pressed
        *   \note This will report the keyboard state regardless of focus.
        *         If supporting focus is necessary, respond to input events instead.
        */
        bool key_is_down(key mKey) const;

        /// Returns elapsed time since the key has been pressed.
        /** \param mKey The ID code of the key you are interested in
        *   \return Elapsed time since the key has been pressed
        *   \note This will report the keyboard state regardless of focus.
        *         If supporting focus is necessary, respond to input events instead.
        */
        double get_key_down_duration(key mKey) const;

        /// Checks if Alt is beeing pressed.
        /** \return 'true' if Alt is beeing pressed
        *   \note This will report the keyboard state regardless of focus.
        *         If supporting focus is necessary, respond to input events instead.
        */
        bool alt_is_pressed() const;

        /// Checks if Shift is beeing pressed.
        /** \return 'true' if Shift is beeing pressed
        *   \note This will report the keyboard state regardless of focus.
        *         If supporting focus is necessary, respond to input events instead.
        */
        bool shift_is_pressed() const;

        /// Checks if Control (Ctrl) is beeing pressed.
        /** \return 'true' if Control (Ctrl) is beeing pressed
        *   \note This will report the keyboard state regardless of focus.
        *         If supporting focus is necessary, respond to input events instead.
        */
        bool ctrl_is_pressed() const;

        /// Checks if a mouse button is being pressed.
        /** \param mID The ID code of the mouse button you are interested in
        *   \return 'true' if the mouse button is being pressed
        *   \note This will report the mouse state regardless of focus.
        *         If supporting focus is necessary, respond to input events instead.
        */
        bool mouse_is_down(mouse_button mID) const;

        /// Returns elapsed time since the mouse button has been pressed.
        /** \param mKey The ID code of the mouse button you are interested in
        *   \return Elapsed time since the mouse button has been pressed
        *   \note This will report the mouse state regardless of focus.
        *         If supporting focus is necessary, respond to input events instead.
        */
        double get_mouse_down_duration(mouse_button mKey) const;

        /// Returns the position of the mouse in pixels.
        /** \return The position of the mouse in pixels
        *   \note This will report the mouse state regardless of focus.
        *         If supporting focus is necessary, respond to input events instead.
        */
        gui::vector2f get_mouse_position() const;

        /// Returns the accumulated rolling ammount of the mouse wheel.
        /** \return The accumulated rolling ammount of the mouse wheel
        *   \note This will report the mouse state regardless of focus.
        *         If supporting focus is necessary, respond to input events instead.
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

        void fire_event_(const gui::event& mEvent);

        gui::event_emitter& mEventEmitter_;

        using timer = std::chrono::high_resolution_clock;
        using time_point = timer::time_point;

        std::array<time_point, KEY_NUMBER>          lKeyPressedTime_ = {};
        std::array<time_point, MOUSE_BUTTON_NUMBER> lMousePressedTime_ = {};

        float         fScalingFactor_ = 1.0f;

        bool          bMouseDragged_ = false;
        mouse_button  mMouseDragButton_ = mouse_button::LEFT;

        source& mSource_;

        bool bMouseBlocked_ = false;
        bool bKeyboardBlocked_ = false;
    };
}
}

#endif
