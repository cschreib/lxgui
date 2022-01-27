#ifndef LXGUI_GUI_KEYBINDER_HPP
#define LXGUI_GUI_KEYBINDER_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/input_keys.hpp"

#include <sol/protected_function.hpp>

#include <string>
#include <vector>
#include <functional>

namespace lxgui {
namespace gui
{
    class event_emitter;

    /// Manages the user interface
    class keybinder
    {
    public :

        /// Type of a keybinding callback.
        using function_type = std::function<void()>;

        /// Constructor.
        keybinder() = default;

        // Non-copiable, non-movable.
        keybinder(const keybinder&) = delete;
        keybinder(keybinder&&) = delete;
        keybinder& operator= (const keybinder&) = delete;
        keybinder& operator= (keybinder&&) = delete;

        /// Binds some Lua code to a key.
        /** \param sName       The key to bind
        *   \param uiModifier The modifier key (shift, ctrl, ...)
        *   \param mHandler   The Lua function that will be executed
        */
        void register_key_binding(std::string_view sName, sol::protected_function mHandler);

        /// Binds some Lua code to a key.
        /** \param sName        The key to bind
        *   \param uiModifier1 The first modifier key (shift, ctrl, ...)
        *   \param uiModifier2 The second modifier key (shift, ctrl, ...)
        *   \param mHandler    The Lua function that will be executed
        */
        void register_key_binding(std::string_view sName, function_type mFunction);

        /// Binds some Lua code to a key.
        /** \param sName       The key to bind
        *   \param sKey   The Lua function that will be executed
        */
        void set_key_binding(std::string_view sName, std::string_view sKey);

        /// Binds some Lua code to a key.
        /** \param mKey       The key to bind
        *   \param mHandler   The Lua function that will be executed
        */
        void set_key_binding(std::string_view sName, input::key mKey,
            bool bShiftIsPressed, bool bCtrlIsPressed, bool bAltIsPressed);

        /// Unbinds a key.
        /** \param mKey        The key to unbind
        *   \param uiModifier1 The first modifier key (shift, ctrl, ...), default is no modifier
        *   \param uiModifier2 The second modifier key (shift, ctrl, ...), default is no modified
        */
        void remove_key_binding(std::string_view sName);

        /// Called when a key is pressed.
        /** \param mKey            The key that is pressed
        *   \param bShiftIsPressed Is the Shift key pressed
        *   \param bCtrlIsPressed  Is the Ctrl key pressed
        *   \param bAltIsPressed   Is the Alt key pressed
        */
        bool on_key_down(input::key mKey,
            bool bShiftIsPressed, bool bCtrlIsPressed, bool bAltIsPressed) const;

        /// Registers this class to the provided Lua state
        void register_on_lua(sol::state& mLua);

    private :

        struct key_binding
        {
            std::string sName;

            input::key mKey = input::key::K_UNASSIGNED;
            bool       bShiftIsPressed = false;
            bool       bCtrlIsPressed = false;
            bool       bAltIsPressed = false;

            function_type mCallback;
        };

        const key_binding* find_binding_(input::key mKey,
            bool bShiftIsPressed, bool bCtrlIsPressed, bool bAltIsPressed) const;

        std::vector<key_binding> lKeyBindings_;
    };
}
}


#endif
