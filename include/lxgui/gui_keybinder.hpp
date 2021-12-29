#ifndef LXGUI_GUI_KEYBINDER_HPP
#define LXGUI_GUI_KEYBINDER_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/input_keys.hpp"

#include <sol/protected_function.hpp>

#include <string>
#include <unordered_map>

namespace sol {
    class state;
}

namespace lxgui {

namespace input {
    class manager;
}

namespace gui
{
    class event_manager;

    /// Manages the user interface
    class keybinder : public event_receiver
    {
    public :

        /// Constructor.
        explicit keybinder(input::manager& mInputManager, event_manager& mEventManager);

        keybinder(const keybinder&) = delete;
        keybinder(keybinder&&) = delete;
        keybinder& operator= (const keybinder&) = delete;
        keybinder& operator= (keybinder&&) = delete;

        /// Binds some Lua code to a key.
        /** \param uiKey      The key to bind
        *   \param mHandler   The Lua function that will be executed
        */
        void set_key_binding(input::key uiKey, sol::protected_function mHandler);

        /// Binds some Lua code to a key.
        /** \param uiKey      The key to bind
        *   \param uiModifier The modifier key (shift, ctrl, ...)
        *   \param mHandler   The Lua function that will be executed
        */
        void set_key_binding(input::key uiKey, input::key uiModifier, sol::protected_function mHandler);

        /// Binds some Lua code to a key.
        /** \param uiKey       The key to bind
        *   \param uiModifier1 The first modifier key (shift, ctrl, ...)
        *   \param uiModifier2 The second modifier key (shift, ctrl, ...)
        *   \param mHandler    The Lua function that will be executed
        */
        void set_key_binding(
            input::key uiKey, input::key uiModifier1, input::key uiModifier2,
            sol::protected_function mHandler);

        /// Unbinds a key.
        /** \param uiKey       The key to unbind
        *   \param uiModifier1 The first modifier key (shift, ctrl, ...), default is no modifier
        *   \param uiModifier2 The second modifier key (shift, ctrl, ...), default is no modified
        */
        void remove_key_binding(
            input::key uiKey, input::key uiModifier1 = input::key::K_UNASSIGNED,
            input::key uiModifier2 = input::key::K_UNASSIGNED);

        /// Called whenever an Event occurs.
        /** \param mEvent The Event which has occured
        */
        void on_event(const event& mEvent) override;

        /// Registers this class to the provided Lua state
        void register_on_lua(sol::state& mLua);

    private :

        std::pair<const sol::protected_function*, std::string> find_handler_(input::key mKey) const;

        template<typename T>
        using key_map = std::unordered_map<input::key,T>;

        input::manager& mInputManager_;

        key_map<key_map<key_map<sol::protected_function>>> lKeyBindingList_;
    };
}
}


#endif
