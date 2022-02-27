#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_texture.hpp"

#include <lxgui/extern_sol2_state.hpp>

/** A @{Frame} with a movable texture.
 * This frame contains a special texture, the "slider thumb".
 * It can be moved along a single axis (X or Y) and its position
 * can be used to represent a value (for configuration menus, or
 * scroll bars).
 *
 * __Events.__ Hard-coded events available to all @{Slider}s,
 * in addition to those from @{Frame}:
 *
 * - `OnValueChanged`: Triggered whenever the value controlled by
 * the slider changes. This is triggered whenever the user moves
 * the slider thumb, and by @{Slider:set_value}. This can also be
 * triggered by @{Slider:set_min_value}, @{Slider:set_max_value},
 * @{Slider:set_min_max_values}, and @{Slider:set_value_step} if the
 * previous value would not satisfy the new constraints.
 *
 * Inherits all methods from: @{Region}, @{Frame}.
 *
 * Child classes: none.
 * @classmod Slider
 */

namespace lxgui::gui {

void slider::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<slider>(
        "Slider", sol::base_classes, sol::bases<region, frame>(), sol::meta_function::index,
        member_function<&slider::get_lua_member_>(), sol::meta_function::new_index,
        member_function<&slider::set_lua_member_>());

    /** @function allow_clicks_outside_thumb
     */
    type.set_function(
        "allow_clicks_outside_thumb", member_function<&slider::set_allow_clicks_outside_thumb>());

    /** @function get_max_value
     */
    type.set_function("get_max_value", member_function<&slider::get_max_value>());

    /** @function get_min_value
     */
    type.set_function("get_min_value", member_function<&slider::get_min_value>());

    /** @function get_min_max_values
     */
    type.set_function("get_min_max_values", [](const slider& self) {
        return std::make_pair(self.get_min_value(), self.get_max_value());
    });

    /** @function get_orientation
     */
    type.set_function("get_orientation", [](const slider& self) {
        return utils::to_string(self.get_orientation());
    });

    /** @function get_thumb_texture
     */
    type.set_function(
        "get_thumb_texture", member_function< // select the right overload for Lua
                                 static_cast<const utils::observer_ptr<texture>& (slider::*)()>(
                                     &slider::get_thumb_texture)>());

    /** @function get_value
     */
    type.set_function("get_value", member_function<&slider::get_value>());

    /** @function get_value_step
     */
    type.set_function("get_value_step", member_function<&slider::get_value_step>());

    /** @function set_max_value
     */
    type.set_function("set_max_value", member_function<&slider::set_max_value>());

    /** @function set_min_value
     */
    type.set_function("set_min_value", member_function<&slider::set_min_value>());

    /** @function set_min_max_values
     */
    type.set_function("set_min_max_values", member_function<&slider::set_min_max_values>());

    /** @function set_orientation
     */
    type.set_function("set_orientation", [](slider& self, std::string orientation_name) {
        if (auto converted = utils::from_string<orientation>(orientation_name);
            converted.has_value()) {
            self.set_orientation(converted.value());
        } else {
            gui::out << gui::warning << "Slider:set_orientation: Unknown orientation: \""
                     << orientation_name << "\"." << std::endl;
        }
    });

    /** @function set_thumb_texture
     */
    type.set_function("set_thumb_texture", member_function<&slider::set_thumb_texture>());

    /** @function set_value_step
     */
    type.set_function("set_value_step", member_function<&slider::set_value_step>());

    /** @function set_value
     */
    type.set_function("set_value", [](slider& self, float value, sol::optional<bool> silent) {
        self.set_value(value, silent.value_or(false));
    });
}

} // namespace lxgui::gui
