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
keybinder::register_key_binding(std::string_view s_name, sol::protected_function m_lua_function) {
    auto m_function = [m_lua_function = std::move(m_lua_function)]() {
        // Call function
        auto m_result = m_lua_function();

        // Handle errors
        if (!m_result.valid()) {
            sol::error m_error = m_result;
            throw gui::exception(m_error.what());
        }
    };

    return register_key_binding(s_name, std::move(m_function));
}

utils::connection
keybinder::register_key_binding(std::string_view s_name, function_type m_function) {
    auto m_iter = utils::find_if(
        l_key_bindings_, [&](const auto& m_binding) { return m_binding.s_name == s_name; });

    if (m_iter == l_key_bindings_.end()) {
        gui::out << gui::error << "keybinder: a binding already exists with name '" << s_name
                 << "'." << std::endl;
        return {};
    }

    key_binding m_binding;
    m_binding.s_name  = std::string(s_name);
    auto m_connection = m_binding.m_signal.connect(std::move(m_function));
    l_key_bindings_.push_back(std::move(m_binding));

    return m_connection;
}

void keybinder::set_key_binding(
    std::string_view s_name,
    input::key       m_key,
    bool             b_shift_is_pressed,
    bool             b_ctrl_is_pressed,
    bool             b_alt_is_pressed) {
    auto m_iter = utils::find_if(
        l_key_bindings_, [&](const auto& m_binding) { return m_binding.s_name == s_name; });

    if (m_iter == l_key_bindings_.end()) {
        gui::out << gui::error << "keybinder: no binding with name '" << s_name << "'."
                 << std::endl;
        return;
    }

    m_iter->m_key = m_key;
    m_iter->b_shift_is_pressed =
        b_shift_is_pressed || m_key == input::key::k_lshift || m_key == input::key::k_rshift;
    m_iter->b_ctrl_is_pressed =
        b_ctrl_is_pressed || m_key == input::key::k_lcontrol || m_key == input::key::k_rcontrol;
    m_iter->b_alt_is_pressed =
        b_alt_is_pressed || m_key == input::key::k_lmenu || m_key == input::key::k_rmenu;
}

void keybinder::set_key_binding(std::string_view s_name, std::string_view s_key) {
    bool b_shift_is_pressed = false;
    bool b_ctrl_is_pressed  = false;
    bool b_alt_is_pressed   = false;

    const auto l_tokens = utils::cut(s_key, "-");
    for (auto s_token : l_tokens) {
        if (s_token == "Shift")
            b_shift_is_pressed = true;
        else if (s_token == "Ctrl")
            b_ctrl_is_pressed = true;
        else if (s_token == "Alt")
            b_alt_is_pressed = true;
    }

    set_key_binding(
        s_name, input::get_key_from_codename(l_tokens.back()), b_shift_is_pressed,
        b_ctrl_is_pressed, b_alt_is_pressed);
}

void keybinder::remove_key_binding(std::string_view s_name) {
    auto m_iter = utils::find_if(
        l_key_bindings_, [&](const auto& m_binding) { return m_binding.s_name == s_name; });

    if (m_iter == l_key_bindings_.end()) {
        gui::out << gui::error << "keybinder: no binding with name '" << s_name << "'."
                 << std::endl;
        return;
    }

    l_key_bindings_.erase(m_iter);
}

keybinder::key_binding* keybinder::find_binding_(
    input::key mKey, bool b_shift_is_pressed, bool bCtrlIsPressed, bool bAltIsPressed) {
    auto m_iter = utils::find_if(l_key_bindings_, [&](const auto& m_binding) {
        return m_binding.m_key == mKey && m_binding.b_shift_is_pressed == b_shift_is_pressed &&
               m_binding.b_ctrl_is_pressed == bCtrlIsPressed &&
               m_binding.b_alt_is_pressed == bAltIsPressed;
    });

    if (m_iter == l_key_bindings_.end())
        return nullptr;

    return &*m_iter;
}

bool keybinder::on_key_down(
    input::key m_key, bool b_shift_is_pressed, bool b_ctrl_is_pressed, bool b_alt_is_pressed) {
    auto* p_key_binding =
        find_binding_(m_key, b_shift_is_pressed, b_ctrl_is_pressed, b_alt_is_pressed);
    if (!p_key_binding)
        return false;

    try {
        p_key_binding->m_signal();
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Bound action: " + p_key_binding->s_name + ": " + std::string(e.what()));
    }

    return true;
}

} // namespace lxgui::gui
