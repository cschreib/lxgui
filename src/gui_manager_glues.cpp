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

namespace lxgui { namespace gui {

void manager::register_lua_glues(std::function<void(gui::manager&)> pLuaRegs) {
    pLuaRegs_ = std::move(pLuaRegs);
}

void manager::create_lua_() {
    if (pLua_)
        return;

    pLua_ = std::unique_ptr<sol::state>(new sol::state());
    pLua_->open_libraries(
        sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::io, sol::lib::os,
        sol::lib::string, sol::lib::debug);

    auto& mLua = *pLua_;

    /** @function log
     */
    mLua.set_function(
        "log", [](const std::string& sMessage) { gui::out << sMessage << std::endl; });

    /** @function create_frame
     */
    mLua.set_function(
        "create_frame",
        [&](const std::string& sType, const std::string& sName, sol::optional<frame&> pParent,
            sol::optional<std::string> sInheritance) -> sol::object {
            region_core_attributes mAttr;
            mAttr.sName       = sName;
            mAttr.sObjectType = sType;
            if (sInheritance.has_value()) {
                mAttr.lInheritance =
                    get_virtual_root().get_registry().get_virtual_region_list(sInheritance.value());
            }

            utils::observer_ptr<frame> pNewFrame;
            if (pParent.has_value())
                pNewFrame = pParent.value().create_child(std::move(mAttr));
            else
                pNewFrame = pRoot_->create_root_frame(std::move(mAttr));

            if (pNewFrame) {
                pNewFrame->set_addon(get_addon_registry()->get_current_addon());
                pNewFrame->notify_loaded();
                return get_lua()[pNewFrame->get_lua_name()];
            } else
                return sol::lua_nil;
        });

    /** @function delete_frame
     */
    mLua.set_function("delete_frame", [&](frame& mFrame) { mFrame.destroy(); });

    /** @function register_key_binding
     */
    mLua.set_function(
        "register_key_binding", [&](std::string sID, sol::protected_function mFunction) {
            get_root().get_keybinder().register_key_binding(sID, mFunction);
        });

    /** @function set_key_binding
     */
    mLua.set_function("set_key_binding", [&](std::string sID, sol::optional<std::string> sKey) {
        if (sKey.has_value())
            get_root().get_keybinder().set_key_binding(sID, sKey.value());
        else
            get_root().get_keybinder().remove_key_binding(sID);
    });

    /** Closes the whole GUI and re-loads addons from files.
     * For safety reasons, the re-loading operation will not be triggered instantaneously.
     * The GUI will be reloaded at the end of the current update tick, when it is safe to do so.
     * @function reload_ui
     */
    mLua.set_function("reload_ui", [&]() { reload_ui(); });

    /** Sets the global interface scaling factor.
     *   @function set_interface_scaling_factor
     *   @tparam number factor The scaling factor (1: no scaling, 2: twice larger fonts and
     * textures, etc.)
     */
    mLua.set_function("set_interface_scaling_factor", [&](float fScaling) {
        set_interface_scaling_factor(fScaling);
    });

    /** Return the global interface scaling factor.
     *   @function get_interface_scaling_factor
     *   @treturn number The scaling factor (1: no scaling, 2: twice larger fonts and textures,
     * etc.)
     */
    mLua.set_function(
        "get_interface_scaling_factor", [&]() { return get_interface_scaling_factor(); });

    pLocalizer_->register_on_lua(mLua);

    // Base types
    pFactory_->register_region_type<region>();
    pFactory_->register_region_type<frame>();
    pFactory_->register_region_type<layered_region>();

    if (pLuaRegs_)
        pLuaRegs_(*this);
}

}} // namespace lxgui::gui
