#ifndef INPUT_HPP
#define INPUT_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_refptr.hpp>
#include <lxgui/utils_wptr.hpp>
#include <string>
#include <vector>
#include <deque>
#include <array>
#include <map>
#include "lxgui/input_keys.hpp"

#define INPUT_MOUSE_BUTTON_NUMBER 3

namespace gui
{
    class event;
    class event_receiver;
    class event_manager;
}

namespace input
{
    /// The base class for input implementation
    class handler_impl
    {
    public :

        /// Constructor.
        handler_impl();

        /// Destructor.
        virtual ~handler_impl() {}

        /// Updates this implementation handler.
        virtual void update() = 0;

        /// Toggles mouse grab.
        /** When the mouse is grabbed, it is confined to the borders
        *   of the main window. The actual cursor behavior when reaching
        *   those borders is of no importance : what matters is that
        *   relative mouse movement is always aquired, i.e. the mouse is never
        *   blocked.
        *   The mouse is not grabbed by default.
        */
        virtual void toggle_mouse_grab() = 0;

        /// Returns the name of the key as it appears on the keyboard.
        /** \return The name of the key as it appears on the keyboard
        */
        virtual std::string get_key_name(key::code mKey) const = 0;

        struct key_state
        {
            std::array<bool, key::K_MAXKEY> lKeyState;
        } mKeyboard;

        struct mouse_state
        {
            std::array<bool, INPUT_MOUSE_BUTTON_NUMBER> lButtonState;
            float fAbsX, fAbsY, fDX, fDY;
            float fRelX, fRelY, fRelDX, fRelDY;
            float fRelWheel;
            bool  bHasDelta;
        } mMouse;

        std::vector<char32_t> lChars;
        std::vector<char32_t> lCharsCache_;
    };

    /// A reference to an input implementation
    /** \note In case you want to share the same handler for
    *         several input::managers, you have to call
    *         set_manually_updated(true), and update the
    *         handler yourself. Else, it will be updated
    *         by each input::manager (and that may be something
    *         you don't want to happen).
    */
    class handler
    {
    public :

        /// Default constructor (empty handler).
        handler();

        /// Implementation constructor.
        /** \param pImpl A pointer to the implementation handler
        */
        template<class T>
        handler(utils::refptr<T> pImpl) : bManuallyUpdated_(false), pImpl_(pImpl) {
        }

        /// Updates this handler.
        /** \note This function calls pImpl->update().
        */
        void update();

        /// Checks if this handler is manually updated.
        /** \return 'true' if this handler is manually updated
        *   \note See set_manually_updated().
        */
        bool is_manually_updated() const;

        /// Marks this handler as manually updated.
        /** \param bManuallyUpdated 'true' if this handler is manually updated
        *   \note In case you want to share the same handler for
        *         several input::managers, you have to call
        *         set_manually_updated(true), and update the
        *         handler yourself. Else, it will be updated
        *         by each input::manager (and that may be something
        *         you don't want to happen).
        */
        void set_manually_updated(bool bManuallyUpdated);

        /// Returns the keyboard state of this input handler.
        const handler_impl::key_state& get_key_state() const;

        /// Returns the unicode characters that has been entered.
        /** \return The unicode characters entered with the keyboard
        */
        std::vector<char32_t> get_chars() const;

        /// Returns the name of the key as it appears on the keyboard.
        /** \return The name of the key as it appears on the keyboard
        */
        std::string get_key_name(key::code mKey) const;

        /// Returns the mouse state of this input handler.
        const handler_impl::mouse_state& get_mouse_state() const;

        /// Returns the implementation specific handler.
        utils::wptr<handler_impl> get_impl();

    private :

        bool                        bManuallyUpdated_;
        utils::refptr<handler_impl> pImpl_;
    };

    /// Handles inputs (keyboard and mouse)
    class manager
    {
    public :

        /// Initializes this manager with the proper input source.
        /** \param mHandler The input source (from another library)
        */
        explicit manager(const handler& mHandler);

        #ifndef NO_CPP11_DELETE_FUNCTION
        /// This class is non copiable.
        manager(const manager& mMgr) = delete;

        /// This class is non copiable.
        manager& operator = (const manager& mMgr) = delete;
        #else
    private :
        /// This class is non copiable.
        manager(const manager& mMgr);

        /// This class is non copiable.
        manager& operator = (const manager& mMgr);

    public :
        #endif

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
        bool key_is_down(key::code mKey, bool bForce = false) const;

        /// Checks if a key is being pressed for a long time.
        /** \param mKey   The ID code of the key you're interested in
        *   \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if the key is being pressed for a long time
        */
        bool key_is_down_long(key::code mKey, bool bForce = false) const;

        /// Returns elapsed time since the key has been pressed.
        /** \param mKey The ID code of the key you're interested in
        *   \return Elapsed time since the key has been pressed
        */
        double get_key_press_duration(key::code mKey) const;

        /// Checks if a key has been pressed.
        /** \param mKey   The ID code of the key you're interested in
        *   \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if the key has been pressed
        *   \note Happens just when the key is pressed.
        */
        bool key_is_pressed(key::code mKey, bool bForce = false) const;

        /// Checks if a key has been released.
        /** \param mKey   The ID code of the key you're interested in
        *   \param bForce 'true' to bypass focus (see set_focus())
        *   \return 'true' if the key has been released
        *   \note Happens just when the key is released.
        */
        bool key_is_released(key::code mKey, bool bForce = false) const;

        /// Returns the UTF8 (multibyte) character that has been entered.
        /** \return The multibyte UTF8 character just entered with the keyboard
        */
        std::vector<char32_t> get_chars() const;

        /// Returns the name of the provided key, as it appears on your keyboard.
        /** \param mKey The key
        *   \return The name of the provided key, as it appears on your keyboard
        */
        std::string get_key_name(key::code mKey) const;

        /// Returns the name of the provided key combination.
        /** \param mKey      The main key
        *   \param mModifier The modifier key (shift, ctrl, ...)
        *   \return The name of key combination, example : "Ctrl + A"
        */
        std::string get_key_name(key::code mKey, key::code mModifier) const;

        /// Returns the name of the provided key combination.
        /** \param mKey       The main key
        *   \param mModifier1 The first modifier key (shift, ctrl, ...)
        *   \param mModifier2 The second modifier key (shift, ctrl, ...)
        *   \return The name of key combination, example : "Ctrl + Shift + A"
        */
        std::string get_key_name(key::code mKey, key::code mModifier1, key::code mModifier2) const;

        /// Returns the list of keys that have been released during this frame.
        /** \return The list of keys that have been released during this frame.
        */
        const std::deque<key::code>& get_key_release_stack() const;

        /// Returns the list of keys that have been pressed during this frame.
        /** \return The list of keys that have been pressed during this frame.
        */
        const std::deque<key::code>& get_key_press_stack() const;

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
        bool mouse_is_down(mouse::button mID) const;

        /// Checks if a mouse button is being pressed for a long time.
        /** \param mID    The ID code of the mouse button you're interested in
        *   \return 'true' if the mouse button is being pressed for a long time
        */
        bool mouse_is_down_long(mouse::button mID) const;

        /// Returns elapsed time since the mouse button has been pressed.
        /** \param mKey The ID code of the mouse button you're interested in
        *   \return Elapsed time since the mouse button has been pressed
        */
        double get_mouse_press_duration(mouse::button mKey) const;

        /// Checks if a mouse button has been pressed.
        /** \param mID    The ID code of the mouse button you're interested in
        *   \return 'true' if the mouse button has been pressed
        *   \note Happens just when the mouse button is pressed.
        */
        bool mouse_is_pressed(mouse::button mID) const;

        /// Checks if a mouse button has been released.
        /** \param mID    The ID code of the mouse button you're interested in
        *   \return 'true' if the mouse button has been released
        *   \note Happens just when the mouse button is released.
        */
        bool mouse_is_released(mouse::button mID) const;

        /// Checks if a mouse button has been double clicked.
        /** \param mID    The ID code of the mouse button you're interested in
        *   \return 'true' if the mouse button has been double clicked
        */
        bool mouse_is_doubleclicked(mouse::button mID) const;

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
        mouse::state get_mouse_state(mouse::button mID) const;

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
        *         Sensibility factor is applied on the result.
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
        *         Sensibility factor is applied on the result.
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
        std::string get_mouse_button_string(mouse::button mID) const;

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
        /** \param fMouseSensibility The new movement factor
        *   \note Increase this parameter to make mouse controlled movement faster.
        */
        void set_mouse_sensibility(float fMouseSensibility);

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
        *   \note See set_focus() for more information. If you use another input
        *         source than this manager, you should check the result of this
        *         function before actually using it.
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

        /// Returns this manager's handler.
        /** \return This manager's handler
        */
        const handler& get_handler() const;

        /// Returns this manager's handler.
        /** \return This manager's handler
        */
        handler& get_handler();

    private :

        void fire_event_(const gui::event& mEvent, bool bForce = false);

        bool bRemoveFocus_;
        bool bFocus_;
        gui::event_receiver* pFocusReceiver_;

        std::vector<gui::event_manager*> lEventManagerList_;

        // Keyboard
        std::array<double, key::K_MAXKEY> lKeyDelay_;
        std::array<bool,   key::K_MAXKEY> lKeyLong_;
        std::array<bool,   key::K_MAXKEY> lKeyBuf_;
        std::array<bool,   key::K_MAXKEY> lKeyBufOld_;

        bool bCtrlPressed_;
        bool bShiftPressed_;
        bool bAltPressed_;
        bool bKey_;
        std::vector<char32_t> lChars_;

        std::deque<key::code> lDownStack_;
        std::deque<key::code> lUpStack_;

        // Mouse
        double                                              dDoubleClickTime_;
        std::array<double, INPUT_MOUSE_BUTTON_NUMBER>       lDoubleClickDelay_;
        std::array<double, INPUT_MOUSE_BUTTON_NUMBER>       lMouseDelay_;
        std::array<bool, INPUT_MOUSE_BUTTON_NUMBER>         lMouseLong_;
        std::array<bool, INPUT_MOUSE_BUTTON_NUMBER>         lMouseBuf_;
        std::array<bool, INPUT_MOUSE_BUTTON_NUMBER>         lMouseBufOld_;
        std::array<mouse::state, INPUT_MOUSE_BUTTON_NUMBER> lMouseState_;

        std::map<std::string, bool> lClickGroupList_;
        std::map<std::string, bool> lForcedClickGroupList_;

        float       fMX_, fMY_;
        float       fRelMX_, fRelMY_;
        float       fDMX_, fDMY_;
        float       fRelDMX_, fRelDMY_;
        float       fRawDMX_, fRawDMY_;
        float       fMouseSensibility_;
        double      dMouseHistoryMaxLength_;
        double      dLongPressDelay_;
        std::deque<std::pair<double, std::array<float,3>>> lMouseHistory_;
        float       fSmoothDMX_, fSmoothDMY_, fSmoothMWheel_;
        float       fMWheel_;
        bool        bWheelRolled_;
        std::string sMouseButton_;
        bool        bLastDragged_;

        double dTime_;

        handler mHandler_;
    };
}

#endif
