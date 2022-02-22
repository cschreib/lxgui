#ifndef LXGUI_GUI_KEYBINDER_HPP
#define LXGUI_GUI_KEYBINDER_HPP

#include "lxgui/input_keys.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_signal.hpp"

#include <lxgui/extern_sol2_protected_function.hpp>
#include <string>
#include <vector>

namespace lxgui::gui {

class event_emitter;

/// Binds global actions to key presses
class keybinder {
    using signal_type = utils::signal<void()>;

public:
    /// Type of a keybinding callback.
    using function_type = signal_type::function_type;

    /// Constructor.
    keybinder() = default;

    // Non-copiable, non-movable.
    keybinder(const keybinder&) = delete;
    keybinder(keybinder&&)      = delete;
    keybinder& operator=(const keybinder&) = delete;
    keybinder& operator=(keybinder&&) = delete;

    /**
     * \brief Registers an action as a possible key binding.
     * \param name The name of the key binding (e.g., "JUMP")
     * \param lua_function The Lua function that will be executed
     * \note The name of the key binding can be anything, but it must be unique.
     * This only registers the action as "available" for a key binding.
     * You must then call @ref set_key_binding() to actually bind it to a key.
     * \return A connection object representing the registered callback function, can be used
     * to gracefully disconnect the callback.
     */
    utils::connection
    register_key_binding(std::string_view name, sol::protected_function lua_function);

    /**
     * \brief Registers an action as a possible key binding.
     * \param name The name of the key binding (e.g., "JUMP")
     * \param function The C++ function that will be executed
     * \note The name of the key binding can be anything, but it must be unique.
     * This only registers the action as "available" for a key binding.
     * You must then call @ref set_key_binding() to actually bind it to a key.
     * \return A connection object representing the registered callback function, can be used
     * to gracefully disconnect the callback.
     */
    utils::connection register_key_binding(std::string_view name, function_type function);

    /**
     * \brief Binds an action to a key.
     * \param name The action to bind
     * \param key_name The key to bind it to (e.g., "Shift-T")
     * \note The format of the `key_name` parameter is any key name as returned from @ref
     * input::get_key_codename(), preceded by optional modifiers (any
     * combination of "Shift-", "Ctrl-", "Alt-"). This corresponds to the key name given
     * to frames in the "OnKeyDown" and "OnKeyUp" scripts.
     */
    void set_key_binding(std::string_view name, std::string_view key_name);

    /**
     * \brief Binds an action to a key.
     * \param name The action to bind
     * \param key_id The key to bind
     * \param shift_is_pressed 'true' if the Shift key must be pressed
     * \param ctrl_is_pressed 'true' if the Ctrl key must be pressed
     * \param alt_is_pressed 'true' if the Alt key must be pressed
     */
    void set_key_binding(
        std::string_view name,
        input::key       key_id,
        bool             shift_is_pressed,
        bool             ctrl_is_pressed,
        bool             alt_is_pressed);

    /**
     * \brief Unbinds an action.
     * \param name The action to unbind
     */
    void remove_key_binding(std::string_view name);

    /**
     * \brief Called when a key is pressed.
     * \param key_id The key that is pressed
     * \param shift_is_pressed Is the Shift key pressed
     * \param ctrl_is_pressed Is the Ctrl key pressed
     * \param alt_is_pressed Is the Alt key pressed
     * \return 'true' if a key binding was found matching this key combination,
     * 'false' otherwise.
     */
    bool on_key_down(
        input::key key_id, bool shift_is_pressed, bool ctrl_is_pressed, bool alt_is_pressed);

private:
    struct key_binding {
        std::string name;

        input::key key_id           = input::key::k_unassigned;
        bool       shift_is_pressed = false;
        bool       ctrl_is_pressed  = false;
        bool       alt_is_pressed   = false;

        signal_type signal;
    };

    key_binding* find_binding_(
        input::key key_id, bool shift_is_pressed, bool ctrl_is_pressed, bool alt_is_pressed);

    std::vector<key_binding> key_bindings_;
};

} // namespace lxgui::gui

#endif
