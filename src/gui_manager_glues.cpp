#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_keybinder.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_virtual_registry.hpp"
#include "lxgui/gui_virtual_root.hpp"
#include "lxgui/input_keys.hpp"

#include <lxgui/extern_sol2_state.hpp>

/** Global functions for interacting with the GUI.
 * The functions listed on this page are registered in the
 * Lua state as globals, and as such are accessible from
 * anywhere automatically. They offer various functionalities
 * for creating or destroying @{Frame}s, and accessing a few
 * other global properties or objects.
 *
 * @module Manager
 */

namespace lxgui::gui {

void manager::register_lua_glues(std::function<void(gui::manager&)> lua_regs) {
    lua_regs_ = std::move(lua_regs);
}

void manager::create_lua_() {
    if (lua_)
        return;

    lua_ = std::unique_ptr<sol::state>(new sol::state());
    lua_->open_libraries(
        sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::io, sol::lib::os,
        sol::lib::string, sol::lib::debug);

    auto& lua = *lua_;

    /** @function log
     */
    lua.set_function("log", [](const std::string& message) { gui::out << message << std::endl; });

    /** @function create_frame
     */
    lua.set_function(
        "create_frame",
        [&](const std::string& type, const std::string& name, sol::optional<frame&> parent,
            sol::optional<std::string> inheritance) -> sol::object {
            region_core_attributes attr;
            attr.name        = name;
            attr.object_type = type;
            if (inheritance.has_value()) {
                attr.inheritance =
                    get_virtual_root().get_registry().get_virtual_region_list(inheritance.value());
            }

            utils::observer_ptr<frame> new_frame;
            if (parent.has_value())
                new_frame = parent.value().create_child(std::move(attr));
            else
                new_frame = root_->create_root_frame(std::move(attr));

            if (new_frame) {
                new_frame->set_addon(get_addon_registry()->get_current_addon());
                new_frame->notify_loaded();
                return get_lua()[new_frame->get_lua_name()];
            } else
                return sol::lua_nil;
        });

    /** @function delete_frame
     */
    lua.set_function("delete_frame", [&](frame& frame) { frame.destroy(); });

    /** @function register_key_binding
     */
    lua.set_function("register_key_binding", [&](std::string id, sol::protected_function function) {
        get_root().get_keybinder().register_key_binding(id, function);
    });

    /** @function set_key_binding
     */
    lua.set_function("set_key_binding", [&](std::string id, sol::optional<std::string> key) {
        if (key.has_value())
            get_root().get_keybinder().set_key_binding(id, key.value());
        else
            get_root().get_keybinder().remove_key_binding(id);
    });

    /** Closes the whole GUI and re-loads addons from files.
     * For safety reasons, the re-loading operation will not be triggered instantaneously.
     * The GUI will be reloaded at the end of the current update tick, when it is safe to do so.
     * @function reload_ui
     */
    lua.set_function("reload_ui", [&]() { reload_ui(); });

    /** Sets the global interface scaling factor.
     * @function set_interface_scaling_factor
     * @tparam number factor The scaling factor (1: no scaling, 2: twice larger fonts and
     * textures, etc.)
     */
    lua.set_function("set_interface_scaling_factor", [&](float scaling) {
        set_interface_scaling_factor(scaling);
    });

    /** Return the global interface scaling factor.
     * @function get_interface_scaling_factor
     * @treturn number The scaling factor (1: no scaling, 2: twice larger fonts and textures,
     * etc.)
     */
    lua.set_function(
        "get_interface_scaling_factor", [&]() { return get_interface_scaling_factor(); });

    localizer_->register_on_lua(lua);

    // Base types
    factory_->register_region_type<region>();
    factory_->register_region_type<frame>();
    factory_->register_region_type<layered_region>();

    if (lua_regs_)
        lua_regs_(*this);
}

} // namespace lxgui::gui
