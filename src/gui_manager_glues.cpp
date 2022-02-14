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

#include <sol/state.hpp>

/** Global functions for interacting with the GUI.
 *   The functions listed on this page are registered in the
 *   Lua state as globals, and as such are accessible from
 *   anywhere automatically. They offer various functionalities
 *   for creating or destroying @{Frame}s, and accessing a few
 *   other global properties or objects.
 *
 *   @module Manager
 */

namespace lxgui::gui {

void manager::register_lua_glues(std::function<void(gui::manager&)> p_lua_regs) {
    p_lua_regs_ = std::move(p_lua_regs);
}

void manager::create_lua_() {
    if (p_lua_)
        return;

    p_lua_ = std::unique_ptr<sol::state>(new sol::state());
    p_lua_->open_libraries(
        sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::io, sol::lib::os,
        sol::lib::string, sol::lib::debug);

    auto& m_lua = *p_lua_;

    /** @function log
     */
    m_lua.set_function(
        "log", [](const std::string& s_message) { gui::out << s_message << std::endl; });

    /** @function create_frame
     */
    m_lua.set_function(
        "create_frame",
        [&](const std::string& s_type, const std::string& s_name, sol::optional<frame&> p_parent,
            sol::optional<std::string> s_inheritance) -> sol::object {
            region_core_attributes m_attr;
            m_attr.s_name        = s_name;
            m_attr.s_object_type = s_type;
            if (s_inheritance.has_value()) {
                m_attr.inheritance = get_virtual_root().get_registry().get_virtual_region_list(
                    s_inheritance.value());
            }

            utils::observer_ptr<frame> p_new_frame;
            if (p_parent.has_value())
                p_new_frame = p_parent.value().create_child(std::move(m_attr));
            else
                p_new_frame = p_root_->create_root_frame(std::move(m_attr));

            if (p_new_frame) {
                p_new_frame->set_addon(get_addon_registry()->get_current_addon());
                p_new_frame->notify_loaded();
                return get_lua()[p_new_frame->get_lua_name()];
            } else
                return sol::lua_nil;
        });

    /** @function delete_frame
     */
    m_lua.set_function("delete_frame", [&](frame& m_frame) { m_frame.destroy(); });

    /** @function register_key_binding
     */
    m_lua.set_function(
        "register_key_binding", [&](std::string s_id, sol::protected_function m_function) {
            get_root().get_keybinder().register_key_binding(s_id, m_function);
        });

    /** @function set_key_binding
     */
    m_lua.set_function("set_key_binding", [&](std::string s_id, sol::optional<std::string> s_key) {
        if (s_key.has_value())
            get_root().get_keybinder().set_key_binding(s_id, s_key.value());
        else
            get_root().get_keybinder().remove_key_binding(s_id);
    });

    /** Closes the whole GUI and re-loads addons from files.
     * For safety reasons, the re-loading operation will not be triggered instantaneously.
     * The GUI will be reloaded at the end of the current update tick, when it is safe to do so.
     * @function reload_ui
     */
    m_lua.set_function("reload_ui", [&]() { reload_ui(); });

    /** Sets the global interface scaling factor.
     *   @function set_interface_scaling_factor
     *   @tparam number factor The scaling factor (1: no scaling, 2: twice larger fonts and
     * textures, etc.)
     */
    m_lua.set_function("set_interface_scaling_factor", [&](float f_scaling) {
        set_interface_scaling_factor(f_scaling);
    });

    /** Return the global interface scaling factor.
     *   @function get_interface_scaling_factor
     *   @treturn number The scaling factor (1: no scaling, 2: twice larger fonts and textures,
     * etc.)
     */
    m_lua.set_function(
        "get_interface_scaling_factor", [&]() { return get_interface_scaling_factor(); });

    p_localizer_->register_on_lua(m_lua);

    // Base types
    p_factory_->register_region_type<region>();
    p_factory_->register_region_type<frame>();
    p_factory_->register_region_type<layered_region>();

    if (p_lua_regs_)
        p_lua_regs_(*this);
}

} // namespace lxgui::gui
