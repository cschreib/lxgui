#ifndef LXGUI_INPUT_HPP
#define LXGUI_INPUT_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_observer.hpp>

#include "lxgui/gui_vector2.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/input_keys.hpp"
#include "lxgui/input_source.hpp"

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <memory>

namespace lxgui {
namespace gui
{
    class event;
    class event_receiver;
    class event_manager;
}

namespace input
{
    /// Handles inputs (keyboard and mouse)
    class manager
    {
    public :

        /// Initializes this manager with a chosen input source.
        /** \param pSource The input source
        */
        explicit manager(std::unique_ptr<source> pSource);

        /// Non-copiable
        manager(const manager&) = delete;

        /// Non-movable
        manager(manager&&) = delete;

        /// Non-copiable
        manager& operator=(const manager&) = delete;

        /// Non-movable
        manager& operator=(manager&&) = delete;

        /// Updates input (keyboard and mouse).
        void update(float fDelta);

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

        /// Checks if a key has been pressed.
        /** \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if a key has been pressed
        */
        bool get_key(bool bForce = false) const;

        /// Checks if a key is being pressed.
        /** \param mKey   The ID code of the key you're interested in
        *   \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if the key is being pressed
        */
        bool key_is_down(key mKey, bool bForce = false) const;

        /// Checks if a key is being pressed for a long time.
        /** \param mKey   The ID code of the key you're interested in
        *   \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if the key is being pressed for a long time
        */
        bool key_is_down_long(key mKey, bool bForce = false) const;

        /// Returns elapsed time since the key has been pressed.
        /** \param mKey The ID code of the key you're interested in
        *   \return Elapsed time since the key has been pressed
        */
        double get_key_down_duration(key mKey) const;

        /// Returns the UTF32 characters that have been entered.
        /** \return The UTF32 characters entered with the keyboard on the last update() call
        */
        std::vector<char32_t> get_chars() const;

        /// Returns the name of the provided key, as it appears on your keyboard.
        /** \param mKey The key
        *   \return The name of the provided key, as it appears on your keyboard
        *   \note This will only return English key names.
        */
        std::string get_key_name(key mKey) const;

        /// Returns the name of the provided key combination.
        /** \param mKey      The main key
        *   \param mModifier The modifier key (shift, ctrl, ...)
        *   \return The name of key combination, example : "Ctrl + A"
        *   \note This will only return English key names.
        */
        std::string get_key_name(key mKey, key mModifier) const;

        /// Returns the name of the provided key combination.
        /** \param mKey       The main key
        *   \param mModifier1 The first modifier key (shift, ctrl, ...)
        *   \param mModifier2 The second modifier key (shift, ctrl, ...)
        *   \return The name of key combination, example : "Ctrl + Shift + A"
        *   \note This will only return English key names.
        */
        std::string get_key_name(key mKey, key mModifier1, key mModifier2) const;

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
        /** \param mID    The ID code of the mouse button you're interested in
        *   \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if the mouse button is being pressed
        */
        bool mouse_is_down(mouse_button mID, bool bForce = false) const;

        /// Checks if a mouse button is being pressed for a long time.
        /** \param mID    The ID code of the mouse button you're interested in
        *   \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if the mouse button is being pressed for a long time
        */
        bool mouse_is_down_long(mouse_button mID, bool bForce = false) const;

        /// Returns elapsed time since the mouse button has been pressed.
        /** \param mKey The ID code of the mouse button you're interested in
        *   \return Elapsed time since the mouse button has been pressed
        */
        double get_mouse_down_duration(mouse_button mKey) const;

        /// Checks if the mouse wheel has been rolled.
        /** \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if the mouse wheel has been rolled
        */
        bool wheel_is_rolled(bool bForce = false) const;

        /// Returns the position of the mouse in pixels.
        /** \return The position of the mouse in pixels
        */
        const gui::vector2f& get_mouse_position() const;

        /// Returns the position variation of the mouse.
        /** \return The position variation of the mouse
        */
        const gui::vector2f& get_mouse_delta() const;

        /// Returns the rolling ammount of the mouse wheel.
        /** \return The rolling ammount of the mouse wheel
        */
        float get_mouse_wheel() const;

        /// Returns the string associated to a mouse button.
        /** \param mID The ID code of the mouse button you're interested in
        *   \return The string associated with the mouse button
        */
        std::string get_mouse_button_string(mouse_button mID) const;

        /// Sets the double click maximum time.
        /** \param dDoubleClickTime Maximum amount of time between two clicks in a double click
        */
        void set_doubleclick_time(double dDoubleClickTime);

        /// Returns the double click maximum time.
        /** \return The double click maximum time
        */
        double get_doubleclick_time() const;

        /// Sets the duration after which a key is considered as pressed for a long time.
        /** \param dLongPressDelay The "long pressed" duration
        *   \note This is used for key repeating for example.
        */
        void set_long_press_delay(double dLongPressDelay);

        /// Returns the duration after which a key is considered as pressed for a long time.
        /** \return The duration after which a key is considered as pressed for a long time
        */
        double get_long_press_delay() const;

        /// Sets whether input should be focussed.
        /** \param bFocus    'true' to stop general inputs and focus on one receiver
        *   \param pReceiver The event receiver that requires focus (if any)
        *   \note This function is usefull if you need to implement
        *         an edit box: the user can type letters binded to
        *         actions in the game, and you should prevent them
        *         from happening. So, you just have to call this function
        *         and use the second argument of all input functions to
        *         force focus in your edit box.
        *   \note Calling set_focus(false) doesn't immediately remove focus.
        *         You have to wait for the next update() call.
        *   \note This function will forward all events (mouse and keyboard)
        *         to the new receiver. See set_keyboard_focus() and
        *         set_mouse_focus() for partial forwarding.
        */
        void set_focus(bool bFocus,
            utils::observer_ptr<gui::event_receiver> pReceiver = nullptr);

        /// Sets whether keyboard input should be focussed.
        /** \param bFocus    'true' to stop keyboard inputs and focus on one receiver
        *   \param pReceiver The event receiver that requires focus (if any)
        *   \note This function will forward all keyboard events to the new receiver.
        *         See set_focus() for more information.
        */
        void set_keyboard_focus(bool bFocus,
            utils::observer_ptr<gui::event_receiver> pReceiver = nullptr);

        /// Sets whether mouse input should be focussed.
        /** \param bFocus    'true' to stop mouse inputs and focus on one receiver
        *   \param pReceiver The event receiver that requires focus (if any)
        *   \note This function will forward all mouse events to the new receiver.
        *         See set_focus() for more information.
        */
        void set_mouse_focus(bool bFocus,
            utils::observer_ptr<gui::event_receiver> pReceiver = nullptr);

        /// Checks whether all input is focused somewhere, to prevent multiple inputs.
        /** \return 'true' if input is focused
        *   \note See set_focus() for more information. If you use some other source
        *         of input than this manager, you should check the result of this
        *         function before actually using it (if the manager is not focussed,
        *         it should not provide any input).
        */
        bool is_focused() const;

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

        /// Registers a new event manager that will listen to input events.
        /** \param pManager The new event manager
        *   \note There can be as many event managers connected to this input
        *         manager. If you need to remove one from the list, see
        *         remove_event_manager().
        */
        void register_event_manager(gui::event_manager* pManager);

        /// Unregisters an event manager.
        /** \param pManager The manager to unregister
        *   \note For more details, see register_event_manager().
        */
        void unregister_event_manager(gui::event_manager* pManager);

        /// Retrieve a copy of the clipboard content.
        /** \return A copy of the clipboard content (empty string is clipboard is empty).
        */
        utils::ustring get_clipboard_content();

        /// Replace the content of the clipboard.
        /** \param sContent The new clipboard content
        */
        void set_clipboard_content(const utils::ustring& sContent);

        /// Sets the mouse cursor to a given image on disk.
        /** \param sFileName The cursor image
        *   \param mHotSpot The pixel position of the tip of the pointer in the image
        *   \note Use reset_mouse_cursor() to set the cursor back to the default.
        */
        void set_mouse_cursor(const std::string& sFileName, const gui::vector2i& mHotSpot);

        /// Sets the mouse cursor back to the default (arrow).
        void reset_mouse_cursor();

        /// Get the window size (in pixels)
        /** \return The window size
        */
        const gui::vector2ui& get_window_dimensions() const;

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

        /// Return the interface scaling factor suggested by the operating system.
        /** \return The interface scaling factor suggested by the operating system
        *   \note This is implementation-dependent; not all input implementations are able
        *         to produce this hint, in which case the function always returns 1.
        *         Consequently, it is recommended to not rely blindly on this hint, and
        *         to offer a way for the user to change the scaling factor.
        */
        float get_interface_scaling_factor_hint() const;

        /// Returns the input source.
        /** \return The input source
        */
        const source& get_source() const;

        /// Returns the input source.
        /** \return The input source
        */
        source& get_source();

    private :

        void fire_event_(const gui::event& mEvent, bool bForce = false);

        bool bRemoveKeyboardFocus_ = false;
        bool bRemoveMouseFocus_ = false;
        bool bKeyboardFocus_ = false;
        bool bMouseFocus_ = false;
        utils::observer_ptr<gui::event_receiver> pKeyboardFocusReceiver_ = nullptr;
        utils::observer_ptr<gui::event_receiver> pMouseFocusReceiver_ = nullptr;

        std::vector<gui::event_manager*> lEventManagerList_;

        // Keyboard
        double                         dLongPressDelay_ = 0.7;
        std::array<double, KEY_NUMBER> lKeyDelay_ = {};
        std::array<bool,   KEY_NUMBER> lKeyLong_ = {};

        bool bCtrlPressed_ = false;
        bool bShiftPressed_ = false;
        bool bAltPressed_ = false;
        bool bKey_ = false;
        std::vector<char32_t> lChars_;

        // Mouse
        std::array<double, MOUSE_BUTTON_NUMBER> lMouseDelay_ = {};
        std::array<bool,   MOUSE_BUTTON_NUMBER> lMouseLong_ = {};

        std::unordered_map<std::string, bool> lClickGroupList_;
        std::unordered_map<std::string, bool> lForcedClickGroupList_;

        float         fScalingFactor_ = 1.0f;
        gui::vector2f mMousePos_;
        gui::vector2f mMouseDelta_;
        float         fMWheel_ = 0.0f;
        bool          bWheelRolled_ = false;
        bool          bMouseDragged_ = false;
        mouse_button  mMouseDragButton_ = mouse_button::LEFT;

        double dTime_ = 0.0;

        std::unique_ptr<source> pSource_;
    };
}
}

#endif
