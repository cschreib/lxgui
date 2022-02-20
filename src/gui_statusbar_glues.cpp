#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_statusbar.hpp"
#include "lxgui/gui_texture.hpp"

#include <lxgui/extern_sol2_state.hpp>

/** A @{Frame} representing a variable-length bar.
 *   This frame has three main properties: a minimum value, a
 *   maximum value, and a current value that must be contained
 *   between the minimum and maximum values. The frame will
 *   render a textured bar that will either be full, empty, or
 *   anything in between depending on the current value.
 *
 *   This can be used to display health bars, or progress bars.
 *
 *   __Events.__ Hard-coded events available to all @{StatusBar}s,
 *   in addition to those from @{Frame}:
 *
 *   - `OnValueChanged`: Triggered whenever the value represented by
 *   the status bar changes. This is triggered by @{StatusBar:set_value}.
 *   This can also be triggered by @{StatusBar:set_min_max_values} if
 *   the previous value would not satisfy the new constraints.
 *
 *   Inherits all methods from: @{Region}, @{Frame}.
 *
 *   Child classes: none.
 *   @classmod StatusBar
 */

namespace lxgui::gui {

void status_bar::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<status_bar>(
        "StatusBar", sol::base_classes, sol::bases<region, frame>(), sol::meta_function::index,
        member_function<&status_bar::get_lua_member_>(), sol::meta_function::new_index,
        member_function<&status_bar::set_lua_member_>());

    /** @function get_min_max_values
     */
    type.set_function("get_min_max_values", [](const status_bar& self) {
        return std::make_pair(self.get_min_value(), self.get_max_value());
    });

    /** @function get_orientation
     */
    type.set_function("get_orientation", [](const status_bar& self) {
        switch (self.get_orientation()) {
        case status_bar::orientation::vertical: return "VERTICAL";
        case status_bar::orientation::horizontal: return "HORIZONTAL";
        default: return "";
        }
    });

    /** @function get_status_bar_color
     */
    type.set_function("get_status_bar_color", [](const status_bar& self) {
        const color& color = self.get_bar_color();
        return std::make_tuple(color.r, color.g, color.b, color.a);
    });

    /** @function get_status_bar_texture
     */
    type.set_function(
        "get_status_bar_texture",
        member_function< // select the right overload for Lua
            static_cast<const utils::observer_ptr<texture>& (status_bar::*)()>(
                &status_bar::get_bar_texture)>());

    /** @function get_value
     */
    type.set_function("get_value", member_function<&status_bar::get_value>());

    /** @function is_reversed
     */
    type.set_function("is_reversed", member_function<&status_bar::is_reversed>());

    /** @function set_min_max_values
     */
    type.set_function("set_min_max_values", member_function<&status_bar::set_min_max_values>());

    /** @function set_orientation
     */
    type.set_function(
        "set_orientation",
        member_function< // select the right overload for Lua
            static_cast<void (status_bar::*)(const std::string&)>(&status_bar::set_orientation)>());

    /** @function set_status_bar_color
     */
    type.set_function(
        "set_status_bar_color",
        sol::overload(
            [](status_bar& self, float r, float g, float b, sol::optional<float> a) {
                self.set_bar_color(color(r, g, b, a.value_or(1.0f)));
            },
            [](status_bar& self, const std::string& s) { self.set_bar_color(color(s)); }));

    /** @function set_status_bar_texture
     */
    type.set_function("set_status_bar_texture", member_function<&status_bar::set_bar_texture>());

    /** @function set_value
     */
    type.set_function("set_value", member_function<&status_bar::set_value>());

    /** @function set_reversed
     */
    type.set_function("set_reversed", member_function<&status_bar::set_reversed>());
}

} // namespace lxgui::gui
