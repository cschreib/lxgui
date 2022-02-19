#include "lxgui/gui_checkbutton.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

#include <sol/state.hpp>

/** A @{Button} with two additional states: checked and unchecked.
 *   This region works exactly like a classic @{Button}, but is has two
 *   additional special textures for the check sign.
 *
 *   Inherits all methods from: @{Region}, @{Frame}, @{Button}.
 *
 *   Child classes: none.
 *   @classmod CheckButton
 */

namespace lxgui::gui {

void check_button::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<check_button>(
        "CheckButton", sol::base_classes, sol::bases<region, frame, button>(),
        sol::meta_function::index, member_function<&check_button::get_lua_member_>(),
        sol::meta_function::new_index, member_function<&check_button::set_lua_member_>());

    /** Checks if this CheckButton is checked.
     *   @function is_checked
     *   @treturn boolean 'true' if checked, 'false' otherwise
     */
    type.set_function("is_checked", member_function<&check_button::is_checked>());

    /** Returns this button's checked texture.
     *   @function get_checked_texture
     *   @treturn Texture This button's checked texture
     */
    type.set_function(
        "get_checked_texture",
        member_function< // select the right overload for Lua
            static_cast<const utils::observer_ptr<texture>& (check_button::*)()>(
                &check_button::get_checked_texture)>());

    /** Returns this button's disabled checked texture.
     *   @function get_disabled_checked_texture
     *   @treturn Texture This button's disabled checked texture
     */
    type.set_function(
        "get_disabled_checked_texture",
        member_function< // select the right overload for Lua
            static_cast<const utils::observer_ptr<texture>& (check_button::*)()>(
                &check_button::get_disabled_checked_texture)>());

    /** Check or uncheck the button.
     *   @function set_checked
     *   @tparam boolean is_checked 'true' to check, 'false' to uncheck (if nil or missing,
     * equivalent to 'true')
     */
    /** Checks the button.
     *   @function set_checked
     */
    type.set_function("set_checked", [](check_button& self, sol::optional<bool> is_checked) {
        if (is_checked.value_or(true))
            self.check();
        else
            self.uncheck();
    });

    /** Sets this button's checked texture.
     *   @function set_checked_texture
     *   @tparam Texture tex The new texture
     */
    type.set_function("set_checked_texture", member_function<&check_button::set_checked_texture>());

    /** Sets this button's disabled checked texture.
     *   @function set_disabled_checked_texture
     *   @tparam Texture tex The new texture
     */
    type.set_function(
        "set_disabled_checked_texture",
        member_function<&check_button::set_disabled_checked_texture>());
}

} // namespace lxgui::gui
