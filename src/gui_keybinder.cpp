#include "lxgui/gui_keybinder.hpp"

#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input_dispatcher.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <sol/state.hpp>

namespace lxgui::gui {

utils::connection
keybinder::register_key_binding(std::string_view name, sol::protected_function m_lua_function) {
    auto m_function = [m_lua_function = std::move(m_lua_function)]() {
        // Call function
        auto m_result = m_lua_function();

        // Handle errors
        if (!m_result.valid()) {
            sol::error m_error = m_result;
            throw gui::exception(m_error.what());
        }
    };

    return register_key_binding(name, std::move(m_function));
}

utils::connection keybinder::register_key_binding(std::string_view name, function_type m_function) {
    auto m_iter = utils::find_if(
        key_bindings_, [&](const auto& m_binding) { return m_binding.name == name; });

    if (m_iter == key_bindings_.end()) {
        gui::out << gui::error << "keybinder: a binding already exists with name '" << name << "'."
                 << std::endl;
        return {};
    }

    key_binding m_binding;
    m_binding.name    = std::string(name);
    auto m_connection = m_binding.m_signal.connect(std::move(m_function));
    key_bindings_.push_back(std::move(m_binding));

    return m_connection;
}

void keybinder::set_key_binding(
    std::string_view name,
    input::key       m_key,
    bool             shift_is_pressed,
    bool             ctrl_is_pressed,
    bool             alt_is_pressed) {
    auto m_iter = utils::find_if(
        key_bindings_, [&](const auto& m_binding) { return m_binding.name == name; });

    if (m_iter == key_bindings_.end()) {
        gui::out << gui::error << "keybinder: no binding with name '" << name << "'." << std::endl;
        return;
    }

    m_iter->m_key = m_key;
    m_iter->shift_is_pressed =
        shift_is_pressed || m_key == input::key::k_lshift || m_key == input::key::k_rshift;
    m_iter->ctrl_is_pressed =
        ctrl_is_pressed || m_key == input::key::k_lcontrol || m_key == input::key::k_rcontrol;
    m_iter->alt_is_pressed =
        alt_is_pressed || m_key == input::key::k_lmenu || m_key == input::key::k_rmenu;
}

void keybinder::set_key_binding(std::string_view name, std::string_view key) {
    bool shift_is_pressed = false;
    bool ctrl_is_pressed  = false;
    bool alt_is_pressed   = false;

    const auto tokens = utils::cut(key, "-");
    for (auto token : tokens) {
        if (token == "Shift")
            shift_is_pressed = true;
        else if (token == "Ctrl")
            ctrl_is_pressed = true;
        else if (token == "Alt")
            alt_is_pressed = true;
    }

    set_key_binding(
        name, input::get_key_from_codename(tokens.back()), shift_is_pressed, ctrl_is_pressed,
        alt_is_pressed);
}

void keybinder::remove_key_binding(std::string_view name) {
    auto m_iter = utils::find_if(
        key_bindings_, [&](const auto& m_binding) { return m_binding.name == name; });

    if (m_iter == key_bindings_.end()) {
        gui::out << gui::error << "keybinder: no binding with name '" << name << "'." << std::endl;
        return;
    }

    key_bindings_.erase(m_iter);
}

keybinder::key_binding* keybinder::find_binding_(
    input::key mKey, bool shift_is_pressed, bool bCtrlIsPressed, bool bAltIsPressed) {
    auto m_iter = utils::find_if(key_bindings_, [&](const auto& m_binding) {
        return m_binding.m_key == mKey && m_binding.shift_is_pressed == shift_is_pressed &&
               m_binding.ctrl_is_pressed == bCtrlIsPressed &&
               m_binding.alt_is_pressed == bAltIsPressed;
    });

    if (m_iter == key_bindings_.end())
        return nullptr;

    return &*m_iter;
}

bool keybinder::on_key_down(
    input::key m_key, bool shift_is_pressed, bool ctrl_is_pressed, bool alt_is_pressed) {
    auto* p_key_binding = find_binding_(m_key, shift_is_pressed, ctrl_is_pressed, alt_is_pressed);
    if (!p_key_binding)
        return false;

    try {
        p_key_binding->m_signal();
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Bound action: " + p_key_binding->name + ": " + std::string(e.what()));
    }

    return true;
}

} // namespace lxgui::gui
