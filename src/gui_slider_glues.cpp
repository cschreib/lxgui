#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_texture.hpp"

#include <sol/state.hpp>

/** A @{Frame} with a movable texture.
 *   This frame contains a special texture, the "slider thumb".
 *   It can be moved along a single axis (X or Y) and its position
 *   can be used to represent a value (for configuration menus, or
 *   scroll bars).
 *
 *   __Events.__ Hard-coded events available to all @{Slider}s,
 *   in addition to those from @{Frame}:
 *
 *   - `OnValueChanged`: Triggered whenever the value controlled by
 *   the slider changes. This is triggered whenever the user moves
 *   the slider thumb, and by @{Slider:set_value}. This can also be
 *   triggered by @{Slider:set_min_value}, @{Slider:set_max_value},
 *   @{Slider:set_min_max_values}, and @{Slider:set_value_step} if the
 *   previous value would not satisfy the new constraints.
 *
 *   Inherits all methods from: @{Region}, @{Frame}.
 *
 *   Child classes: none.
 *   @classmod Slider
 */

namespace lxgui::gui {

void slider::register_on_lua(sol::state& m_lua) {
    auto m_class = m_lua.new_usertype<slider>(
        "Slider", sol::base_classes, sol::bases<region, frame>(), sol::meta_function::index,
        member_function<&slider::get_lua_member_>(), sol::meta_function::new_index,
        member_function<&slider::set_lua_member_>());

    /** @function allow_clicks_outside_thumb
     */
    m_class.set_function(
        "allow_clicks_outside_thumb", member_function<&slider::set_allow_clicks_outside_thumb>());

    /** @function get_max_value
     */
    m_class.set_function("get_max_value", member_function<&slider::get_max_value>());

    /** @function get_min_value
     */
    m_class.set_function("get_min_value", member_function<&slider::get_min_value>());

    /** @function get_min_max_values
     */
    m_class.set_function("get_min_max_values", [](const slider& m_self) {
        return std::make_pair(m_self.get_min_value(), m_self.get_max_value());
    });

    /** @function get_orientation
     */
    m_class.set_function("get_orientation", [](const slider& m_self) {
        switch (m_self.get_orientation()) {
        case slider::orientation::vertical: return "VERTICAL";
        case slider::orientation::horizontal: return "HORIZONTAL";
        default: return "";
        }
    });

    /** @function get_thumb_texture
     */
    m_class.set_function(
        "get_thumb_texture", member_function< // select the right overload for Lua
                                 static_cast<const utils::observer_ptr<texture>& (slider::*)()>(
                                     &slider::get_thumb_texture)>());

    /** @function get_value
     */
    m_class.set_function("get_value", member_function<&slider::get_value>());

    /** @function get_value_step
     */
    m_class.set_function("get_value_step", member_function<&slider::get_value_step>());

    /** @function set_max_value
     */
    m_class.set_function("set_max_value", member_function<&slider::set_max_value>());

    /** @function set_min_value
     */
    m_class.set_function("set_min_value", member_function<&slider::set_min_value>());

    /** @function set_min_max_values
     */
    m_class.set_function("set_min_max_values", member_function<&slider::set_min_max_values>());

    /** @function set_orientation
     */
    m_class.set_function(
        "set_orientation",
        member_function< // select the right overload for Lua
            static_cast<void (slider::*)(const std::string&)>(&slider::set_orientation)>());

    /** @function set_thumb_texture
     */
    m_class.set_function("set_thumb_texture", member_function<&slider::set_thumb_texture>());

    /** @function set_value_step
     */
    m_class.set_function("set_value_step", member_function<&slider::set_value_step>());

    /** @function set_value
     */
    m_class.set_function("set_value", [](slider& m_self, float value, sol::optional<bool> silent) {
        m_self.set_value(value, silent.value_or(false));
    });
}

} // namespace lxgui::gui
