#ifndef LXGUI_INPUT_HPP
#define LXGUI_INPUT_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_observer.hpp>

#include "lxgui/gui_vector2.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/input_keys.hpp"
#include "lxgui/input_source.hpp"

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <memory>
#include <chrono>

namespace lxgui {
namespace gui
{
    class event;
    class event_emitter;
}

namespace input
{
    /// Handles inputs (keyboard and mouse)
    class manager : public gui::event_receiver
    {
    public :

        /// Initializes this manager with a chosen input source.
        /** \param mBlock  The owner pointer control block
        *   \param mSource The input source
        */
        explicit manager(utils::control_block& mBlock, source& mSource);

        // Non-copiable, non-movable
        manager(const manager&) = delete;
        manager(manager&&) = delete;
        manager& operator=(const manager&) = delete;
        manager& operator=(manager&&) = delete;

        /// Called whenever an event occurs.
        /** \param mEvent The event which has occured
        *   \note Only registered events will cause this
        *         function to be called.
        */
        void on_event(const gui::event& mEvent) override;

        /// Allows a particular input group to receive input events.
        /** \param sGroupName The name of the group to enable
        */
        void allow_input(const std::string& sGroupName);

        /// Prevents a particular input group from receiving input events.
        /** \param sGroupName The name of the group to disable
        */
        void block_input(const std::string& sGroupName);

        /// Checks if a particular input group can receive input events.
        /** \param sGroupName The name of the group to check
        *   \return 'true' if the group can receive input events
        */
        bool can_receive_input(const std::string& sGroupName) const;

        /// Makes sure a particular input group receives input events.
        /** \param sGroupName The name of the group to force
        *   \param bForce     'true' to force input
        *   \note Even if you call block_input() with the same group name,
        *         can_receive_input() will return true.
        */
        void force_input_allowed(const std::string& sGroupName, bool bForce);

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

        /// Sets whether keyboard input should be focussed.
        /** \param pReceiver The event receiver that requires focus
        *   \note This function will forward all keyboard events to the new receiver.
        *         This is usefull to implement an edit box: the user can type letters using keys
        *         that can be bound to special actions in the game, and these should be prevented
        *         from happening. This can be achieved by calling this function and using the
        *         edit box as second argument, which will ensure that input events are only sent
        *         to the edit box exclusively.
        */
        void request_keyboard_focus(utils::observer_ptr<gui::event_receiver> pReceiver);

        /// Give up focus of keyboard input.
        /** \param mReceiver The event receiver that releases focus
        */
        void release_keyboard_focus(const gui::event_receiver& mReceiver);

        /// Sets whether mouse input should be focussed.
        /** \param pReceiver The event receiver that requires focus
        *   \note This function will forward all mouse events to the new receiver.
        */
        void request_mouse_focus(utils::observer_ptr<gui::event_receiver> pReceiver);

        /// Give up focus of mouse input.
        /** \param mReceiver The event receiver that releases focus
        */
        void release_mouse_focus(const gui::event_receiver& mReceiver);

        /// Checks whether keyboard input is focused somewhere, to prevent multiple inputs.
        /** \return 'true' if input is focused
        *   \note See set_keyboard_focus() for more information. If you use some other source
        *         of input than this manager, you should check the result of this
        *         function before actually using it (if the manager is not focussed,
        *         it should not provide any input).
        */
        bool is_keyboard_focused() const;

        /// Checks whether mouse input is focused somewhere, to prevent multiple inputs.
        /** \return 'true' if input is focused
        *   \note See set_mouse_focus() for more information. If you use some other source
        *         of input than this manager, you should check the result of this
        *         function before actually using it (if the manager is not focussed,
        *         it should not provide any input).
        */
        bool is_mouse_focused() const;

        /// Registers a new event emitter that will forward input events.
        /** \param pEmitter The new event emitter
        *   \note There can be as many event emitters connected to this input
        *         manager. If you need to remove one from the list, see
        *         @ref unregister_event_emitter().
        */
        void register_event_emitter(utils::observer_ptr<gui::event_emitter> pEmitter);

        /// Unregisters an event emitter.
        /** \param mEmitter The emitter to unregister
        *   \note For more details, see @ref register_event_emitter().
        */
        void unregister_event_emitter(gui::event_emitter& mEmitter);

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

        gui::event_receiver* get_keyboard_focus_() const;
        gui::event_receiver* get_mouse_focus_() const;

        std::vector<utils::observer_ptr<gui::event_receiver>> lKeyboardFocusStack_;
        std::vector<utils::observer_ptr<gui::event_receiver>> lMouseFocusStack_;

        std::vector<utils::observer_ptr<gui::event_emitter>> lEventEmitterList_;

        using timer = std::chrono::high_resolution_clock;
        using time_point = timer::time_point;

        std::array<time_point, KEY_NUMBER>          lKeyPressedTime_ = {};
        std::array<time_point, MOUSE_BUTTON_NUMBER> lMousePressedTime_ = {};

        std::unordered_map<std::string, bool> lClickGroupList_;
        std::unordered_map<std::string, bool> lForcedClickGroupList_;

        float         fScalingFactor_ = 1.0f;

        bool          bMouseDragged_ = false;
        mouse_button  mMouseDragButton_ = mouse_button::LEFT;

        source& mSource_;
    };
}
}

#endif
