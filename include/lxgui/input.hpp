#ifndef LXGUI_INPUT_HPP
#define LXGUI_INPUT_HPP

#include <lxgui/utils.hpp>
#include <string>
#include <vector>
#include <array>
#include <map>
#include "lxgui/input_keys.hpp"

namespace lxgui {
namespace gui
{
    class event;
    class event_receiver;
    class event_manager;
}

namespace input
{
    constexpr std::size_t MOUSE_BUTTON_NUMBER = 3u;
    constexpr std::size_t KEY_NUMBER = static_cast<uint>(key::K_MAXKEY);

    /// The base class for input source implementation
    /** \note In case you want to share the same source
    *         for multiple input::manager, you have to call
    *         set_manually_updated(true), and update the
    *         source yourself. Else, it will be updated
    *         by each input::manager (which may not be
    *         desirable).
    */
    class source_impl
    {
    public :

        struct key_state
        {
            std::array<bool, KEY_NUMBER> lKeyState = {};
        };

        struct mouse_state
        {
            std::array<bool, MOUSE_BUTTON_NUMBER> lButtonState = {};
            float fAbsX = 0.0f, fAbsY = 0.0f, fDX = 0.0f, fDY = 0.0f;
            float fRelX = 0.0f, fRelY = 0.0f, fRelDX = 0.0f, fRelDY = 0.0f;
            float fRelWheel = 0.0f;
            bool  bHasDelta = false;
        };

        /// Constructor.
        source_impl() = default;

        /// Destructor.
        virtual ~source_impl() = default;

        /// Updates this input source.
        void update();

        /// Checks if this source is manually updated.
        /** \return 'true' if this source is manually updated
        *   \note See set_manually_updated().
        */
        bool is_manually_updated() const;

        /// Marks this source as manually updated.
        /** \param bManuallyUpdated 'true' if this source is manually updated
        *   \note In case you want to share the same source
        *         for several input::manager, you have to call
        *         set_manually_updated(true), and update the
        *         source yourself. Else, it will be updated
        *         by each input::manager (which may not be
        *         desirable).
        */
        void set_manually_updated(bool bManuallyUpdated);

        /// Returns the keyboard state of this input source.
        const key_state& get_key_state() const;

        /// Returns the unicode characters that have been entered.
        /** \return The unicode characters entered with the keyboard
        */
        const std::vector<char32_t>& get_chars() const;

        /// Returns the mouse state of this input source.
        const mouse_state& get_mouse_state() const;

        /// Toggles mouse grab.
        /** When the mouse is grabbed, it is confined to the borders
        *   of the main window. The actual cursor behavior when reaching
        *   those borders is of no importance : what matters is that
        *   relative mouse movement is always aquired, i.e. the mouse is never
        *   blocked.
        *   The mouse is not grabbed by default.
        */
        virtual void toggle_mouse_grab() = 0;

        /// Checks if window has been resized.
        /** \return true if the window has been resized
        */
        bool has_window_resized() const;

        /// Resets the "window resized" flag.
        void reset_window_resized();

        /// Get the new window width
        /** \return The new window width
        */
        uint get_window_new_width() const;

        /// Get the new window height
        /** \return The new window height
        */
        uint get_window_new_height() const;

    protected:

        /// Updates this implementation input source.
        virtual void update_() = 0;

        key_state   mKeyboard_;
        mouse_state mMouse_;

        std::vector<char32_t> lChars_;
        std::vector<char32_t> lCharsCache_;

        bool bManuallyUpdated_ = false;

        bool bWindowResized_ = false;
        uint uiNewWindowHeight_ = 0u;
        uint uiNewWindowWidth_ = 0u;
    };

    /// Handles inputs (keyboard and mouse)
    class manager
    {
    public :

        /// Initializes this manager with a chosen input source.
        /** \param pSource The input source
        */
        explicit manager(std::unique_ptr<source_impl> pSource);

        manager(const manager& mMgr) = delete;
        manager(manager&& mMgr) = delete;
        manager& operator = (const manager& mMgr) = delete;
        manager& operator = (manager&& mMgr) = delete;

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
        double get_key_press_duration(key mKey) const;

        /// Checks if a key has been pressed.
        /** \param mKey   The ID code of the key you're interested in
        *   \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if the key has been pressed
        *   \note Happens just when the key is pressed.
        */
        bool key_is_pressed(key mKey, bool bForce = false) const;

        /// Checks if a key has been released.
        /** \param mKey   The ID code of the key you're interested in
        *   \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if the key has been released
        *   \note Happens just when the key is released.
        */
        bool key_is_released(key mKey, bool bForce = false) const;

        /// Returns the UTF8 (multibyte) character that has been entered.
        /** \return The multibyte UTF8 character just entered with the keyboard
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

        /// Returns the list of keys that have been released during this frame.
        /** \return The list of keys that have been released during this frame.
        */
        const std::vector<key>& get_key_release_stack() const;

        /// Returns the list of keys that have been pressed during this frame.
        /** \return The list of keys that have been pressed during this frame.
        */
        const std::vector<key>& get_key_press_stack() const;

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
        *   \return 'true' if the mouse button is being pressed
        */
        bool mouse_is_down(mouse_button mID) const;

        /// Checks if a mouse button is being pressed for a long time.
        /** \param mID    The ID code of the mouse button you're interested in
        *   \return 'true' if the mouse button is being pressed for a long time
        */
        bool mouse_is_down_long(mouse_button mID) const;

        /// Returns elapsed time since the mouse button has been pressed.
        /** \param mKey The ID code of the mouse button you're interested in
        *   \return Elapsed time since the mouse button has been pressed
        */
        double get_mouse_press_duration(mouse_button mKey) const;

        /// Checks if a mouse button has been pressed.
        /** \param mID    The ID code of the mouse button you're interested in
        *   \return 'true' if the mouse button has been pressed
        *   \note Happens just when the mouse button is pressed.
        */
        bool mouse_is_pressed(mouse_button mID) const;

        /// Checks if a mouse button has been released.
        /** \param mID    The ID code of the mouse button you're interested in
        *   \return 'true' if the mouse button has been released
        *   \note Happens just when the mouse button is released.
        */
        bool mouse_is_released(mouse_button mID) const;

        /// Checks if a mouse button has been double clicked.
        /** \param mID    The ID code of the mouse button you're interested in
        *   \return 'true' if the mouse button has been double clicked
        */
        bool mouse_is_doubleclicked(mouse_button mID) const;

        /// Checks if the mouse wheel has been rolled.
        /** \return 'true' if the mouse wheel has been rolled
        */
        bool wheel_is_rolled() const;

        /// Checks if the mouse has just started beeing dragged.
        /** \return 'true' if the mouse has just started beeing dragged
        */
        bool mouse_last_dragged() const;

        /// Returns a mouse button's state.
        /** \param mID The ID code of the mouse button you're interested in
        *   \return The mouse button's state
        */
        mouse_state get_mouse_state(mouse_button mID) const;

        /// Returns the horizontal position of the mouse.
        /** \return The horizontal position of the mouse
        */
        float get_mouse_x() const;

        /// Returns the vertical position of the mouse.
        /** \return The vertical position of the mouse
        */
        float get_mouse_y() const;

        /// Returns the horizontal position of the mouse in window units.
        /** \return The horizontal position of the mouse in window units
        *   \note This function returns the same thing as get_mouse_x(),
        *         but divided by the window's width
        */
        float get_mouse_rel_x() const;

        /// Returns the vertical position of the mouse in window units.
        /** \return The vertical position of the mouse in window units
        *   \note This function returns the same thing as get_mouse_y(),
        *         but divided by the window's height
        */
        float get_mouse_rel_y() const;

        /// Returns the horizontal position variation of the mouse.
        /** \return The horizontal position variation of the mouse
        *   \note This function returns values just as they are given
        *         by the mouse.
        */
        float get_mouse_raw_dx() const;

        /// Returns the vertical position variation of the mouse.
        /** \return The vertical position variation of the mouse
        *   \note This function returns values just as they are given
        *         by the mouse.
        */
        float get_mouse_raw_dy() const;

        /// Returns the horizontal position variation of the mouse.
        /** \return The horizontal position variation of the mouse
        *   \note This function returns the same thing as get_mouse_raw_dx(),
        *         but this time, the game's sensibility factor is applied.
        */
        float get_mouse_dx() const;

        /// Returns the vertical position variation of the mouse.
        /** \return The vertical position variation of the mouse
        *   \note This function returns the same thing as get_mouse_raw_dy(),
        *         but this time, the game's sensibility factor is applied.
        */
        float get_mouse_dy() const;

        /// Returns the horizontal position variation of the mouse in window units.
        /** \return The horizontal position variation of the mouse in window units
        *   \note This function returns the same thing as get_mouse_dx(),
        *         but divided by the window's width
        */
        float get_mouse_rel_dx() const;

        /// Returns the vertical position variation of the mouse in window units.
        /** \return The vertical position variation of the mouse in window units
        *   \note This function returns the same thing as get_mouse_dy(),
        *         but divided by the window's height
        */
        float get_mouse_rel_dy() const;

        /// Returns the horizontal position variation of the mouse.
        /** \return The horizontal position variation of the mouse
        *   \note A common mouse's update rate is 125Hz
        *         (it will report its movement every 8ms). In the cases
        *         where the game updates faster, the mouse will just
        *         report zero movement for some frames, which can be
        *         disturbing if you need a continuous movement.<br>
        *         If that's an issue for you, then use this function
        *         instead of get_mouse_dx().<br>
        *         Sensitivity factor is applied on the result.
        */
        float get_mouse_smooth_dx() const;

        /// Returns the vertical position variation of the mouse.
        /** \return The vertical position variation of the mouse
        *   \note A common mouse's update rate is 125Hz
        *         (it will report its movement every 8ms). In the cases
        *         where the game updates faster, the mouse will just
        *         report zero movement for some frames, which can be
        *         disturbing if you need a continuous movement.<br>
        *         If that's an issue for you, then use this function
        *         instead of get_mouse_dy().<br>
        *         Sensitivity factor is applied on the result.
        */
        float get_mouse_smooth_dy() const;

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

        /// Sets the ammount of mouse movement to be buffered.
        /** \param dMouseHistoryMaxLength The maximum buffer length (in seconds)
        *   \note If you experience jerky mouse movement, you can try to increase
        *         this value (default : 0.1s).<br> On the contrary, if you feel
        *         your mouse is not responsive enough, try to decrease it.
        */
        void set_mouse_buffer_duration(double dMouseHistoryMaxLength);

        /// Returns the ammount of mouse movement to be buffered.
        /** \return The ammount of mouse movement to be buffered
        */
        double get_mouse_buffer_duration() const;

        /// Sets the mouse movement factor.
        /** \param fMouseSensitivity The new movement factor
        *   \note Increase this parameter to make mouse controlled movement faster.
        */
        void set_mouse_sensibility(float fMouseSensitivity);

        /// Returns the mouse movement factor.
        /** \return The mouse movement factor
        */
        float get_mouse_sensibility() const;

        /// Sets the duration after which a key is considered as pressed for a long time.
        /** \param dLongPressDelay The "long pressed" duration
        *   \note This is used for key repeating for example.
        */
        void set_long_press_delay(double dLongPressDelay);

        /// Returns the duration after which a key is considered as pressed for a long time.
        /** \return The duration after which a key is considered as pressed for a long time
        */
        double get_long_press_delay() const;

        /// Sets whether input should be stopped.
        /** \param bFocus    'true' to stop inputs
        *   \param pReceiver The event receiver that requires focus (if any)
        *   \note This function is usefull if you need to implement
        *         an edit box : the user can type letters binded to
        *         actions in the game, and you should prevent them
        *         from happening. So, you just have to call this function
        *         and use the second argument of all input functions to
        *         force focus in your edit box.
        *   \note Calling set_focus(false) doesn't immediately remove focus.
        *         You have to wait for the next update() call.
        */
        void set_focus(bool bFocus, gui::event_receiver* pReceiver = nullptr);

        /// Checks whether input is focused somewhere, to prevent multiple inputs.
        /** \return 'true' if input is focused
        *   \note See set_focus() for more information. If you use some other source
        *         of input than this manager, you should check the result of this
        *         function before actually using it (if the manager is not focussed,
        *         it should not provide any input).
        */
        bool is_focused() const;

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

        /// Returns the input source.
        /** \return The input source
        */
        const source_impl* get_source() const;

        /// Returns the input source.
        /** \return The input source
        */
        source_impl* get_source();

    private :

        void fire_event_(const gui::event& mEvent, bool bForce = false);

        bool bRemoveFocus_ = false;
        bool bFocus_ = false;
        gui::event_receiver* pFocusReceiver_ = nullptr;

        std::vector<gui::event_manager*> lEventManagerList_;

        // Keyboard
        std::array<double, KEY_NUMBER> lKeyDelay_;
        std::array<bool,   KEY_NUMBER> lKeyLong_;
        std::array<bool,   KEY_NUMBER> lKeyBuf_;
        std::array<bool,   KEY_NUMBER> lKeyBufOld_;

        bool bCtrlPressed_ = false;
        bool bShiftPressed_ = false;
        bool bAltPressed_ = false;
        bool bKey_ = false;
        std::vector<char32_t> lChars_;

        std::vector<key> lDownStack_;
        std::vector<key> lUpStack_;

        // Mouse
        double                                       dDoubleClickTime_ = 0.25;
        std::array<double, MOUSE_BUTTON_NUMBER>      lDoubleClickDelay_;
        std::array<double, MOUSE_BUTTON_NUMBER>      lMouseDelay_;
        std::array<bool, MOUSE_BUTTON_NUMBER>        lMouseLong_;
        std::array<bool, MOUSE_BUTTON_NUMBER>        lMouseBuf_;
        std::array<bool, MOUSE_BUTTON_NUMBER>        lMouseBufOld_;
        std::array<mouse_state, MOUSE_BUTTON_NUMBER> lMouseState_;

        std::map<std::string, bool> lClickGroupList_;
        std::map<std::string, bool> lForcedClickGroupList_;

        float       fMX_ = 0.0f, fMY_ = 0.0f;
        float       fRelMX_ = 0.0f, fRelMY_ = 0.0f;
        float       fDMX_ = 0.0f, fDMY_ = 0.0f;
        float       fRelDMX_ = 0.0f, fRelDMY_ = 0.0f;
        float       fRawDMX_ = 0.0f, fRawDMY_ = 0.0f;
        float       fMouseSensitivity_ = 1.0f;
        double      dMouseHistoryMaxLength_ = 0.1;
        double      dLongPressDelay_ = 0.7;
        std::vector<std::pair<double, std::array<float,3>>> lMouseHistory_;
        float       fSmoothDMX_ = 0.0f, fSmoothDMY_ = 0.0f, fSmoothMWheel_ = 0.0f;
        float       fMWheel_ = 0.0f;
        bool        bWheelRolled_ = false;
        std::string sMouseButton_;
        bool        bLastDragged_ = false;

        double dTime_ = 0.0;

        std::unique_ptr<source_impl> pSource_;
    };
}
}

#endif
