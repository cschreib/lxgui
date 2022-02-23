
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_event_emitter.hpp"
#include "lxgui/gui_key_binder.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input_dispatcher.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <lxgui/extern_sol2_state.hpp>

namespace lxgui::gui {

utils::connection
key_binder::register_key_binding(std::string_view name, sol::protected_function lua_function) {
    auto function = [lua_function = std::move(lua_function)]() {
        // Call function
        auto result = lua_function();

        // Handle errors
        if (!result.valid()) {
            throw gui::exception(result.get<sol::error>().what());
        }
    };

    return register_key_binding(name, std::move(function));
}

utils::connection key_binder::register_key_binding(std::string_view name, function_type function) {
    auto iter =
        utils::find_if(key_bindings_, [&](const auto& binding) { return binding.name == name; });

    if (iter == key_bindings_.end()) {
        gui::out << gui::error << "key_binder: a binding already exists with name '" << name << "'."
                 << std::endl;
        return {};
    }

    key_binding binding;
    binding.name    = std::string(name);
    auto connection = binding.signal.connect(std::move(function));
    key_bindings_.push_back(std::move(binding));

    return connection;
}

void key_binder::set_key_binding(
    std::string_view name,
    input::key       key_id,
    bool             shift_is_pressed,
    bool             ctrl_is_pressed,
    bool             alt_is_pressed) {
    auto iter =
        utils::find_if(key_bindings_, [&](const auto& binding) { return binding.name == name; });

    if (iter == key_bindings_.end()) {
        gui::out << gui::error << "key_binder: no binding with name '" << name << "'." << std::endl;
        return;
    }

    iter->key_id = key_id;
    iter->shift_is_pressed =
        shift_is_pressed || key_id == input::key::k_lshift || key_id == input::key::k_rshift;
    iter->ctrl_is_pressed =
        ctrl_is_pressed || key_id == input::key::k_lcontrol || key_id == input::key::k_rcontrol;
    iter->alt_is_pressed =
        alt_is_pressed || key_id == input::key::k_lmenu || key_id == input::key::k_rmenu;
}

void key_binder::set_key_binding(std::string_view name, std::string_view key_name) {
    bool shift_is_pressed = false;
    bool ctrl_is_pressed  = false;
    bool alt_is_pressed   = false;

    const auto tokens = utils::cut(key_name, "-");
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

void key_binder::remove_key_binding(std::string_view name) {
    auto iter =
        utils::find_if(key_bindings_, [&](const auto& binding) { return binding.name == name; });

    if (iter == key_bindings_.end()) {
        gui::out << gui::error << "key_binder: no binding with name '" << name << "'." << std::endl;
        return;
    }

    key_bindings_.erase(iter);
}

key_binder::key_binding* key_binder::find_binding_(
    input::key key_id, bool shift_is_pressed, bool ctrl_is_pressed, bool alt_is_pressed) {
    auto iter = utils::find_if(key_bindings_, [&](const auto& binding) {
        return binding.key_id == key_id && binding.shift_is_pressed == shift_is_pressed &&
               binding.ctrl_is_pressed == ctrl_is_pressed &&
               binding.alt_is_pressed == alt_is_pressed;
    });

    if (iter == key_bindings_.end())
        return nullptr;

    return &*iter;
}

bool key_binder::on_key_down(
    input::key key_id, bool shift_is_pressed, bool ctrl_is_pressed, bool alt_is_pressed) {
    auto* binding = find_binding_(key_id, shift_is_pressed, ctrl_is_pressed, alt_is_pressed);
    if (!binding)
        return false;

    try {
        binding->signal();
    } catch (const std::exception& e) {
        throw std::runtime_error("Bound action: " + binding->name + ": " + std::string(e.what()));
    }

    return true;
}

} // namespace lxgui::gui
