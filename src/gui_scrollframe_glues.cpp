#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_scrollframe.hpp"

#include <sol/state.hpp>

/** A @{Frame} with scrollable content.
 *   This frame has a special child frame, the "scroll child". The scroll
 *   child is rendered on a separate render target, which is then rendered
 *   on the screen. This allows clipping the content of the scroll child
 *   and only display a portion of it (as if scrolling on a page). The
 *   displayed portion is controlled by the scroll value, which can be
 *   changed in both the vertical and horizontal directions.
 *
 *   By default, the mouse wheel movement will not trigger any scrolling;
 *   this has to be explicitly implemented using the `OnMouseWheel` callback
 *   and the @{ScrollFrame:set_horizontal_scroll} function.
 *
 *   __Events.__ Hard-coded events available to all @{ScrollFrame}s,
 *   in addition to those from @{Frame}:
 *
 *   - `OnHorizontalScroll`: Triggered by @{ScrollFrame:set_horizontal_scroll}.
 *   - `OnScrollRangeChanged`: Triggered whenever the range of the scroll value
 *   changes. This happens either when the size of the scrollable content
 *   changes, or when the size of the scroll frame changes.
 *   - `OnVerticalScroll`: Triggered by @{ScrollFrame:set_vertical_scroll}.
 *
 *   Inherits all methods from: @{Region}, @{Frame}.
 *
 *   Child classes: none.
 *   @classmod ScrollFrame
 */

namespace lxgui::gui {

void scroll_frame::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<scroll_frame>(
        "ScrollFrame", sol::base_classes, sol::bases<region, frame>(), sol::meta_function::index,
        member_function<&scroll_frame::get_lua_member_>(), sol::meta_function::new_index,
        member_function<&scroll_frame::set_lua_member_>());

    /** @function get_horizontal_scroll
     */
    type.set_function(
        "get_horizontal_scroll", member_function<&scroll_frame::get_horizontal_scroll>());

    /** @function get_horizontal_scroll_range
     */
    type.set_function(
        "get_horizontal_scroll_range",
        member_function<&scroll_frame::get_horizontal_scroll_range>());

    /** @function get_scroll_child
     */
    type.set_function(
        "get_scroll_child", member_function< // select the right overload for Lua
                                static_cast<const utils::observer_ptr<frame>& (scroll_frame::*)()>(
                                    &scroll_frame::get_scroll_child)>());

    /** @function get_vertical_scroll
     */
    type.set_function("get_vertical_scroll", member_function<&scroll_frame::get_vertical_scroll>());

    /** @function get_vertical_scroll_range
     */
    type.set_function(
        "get_vertical_scroll_range", member_function<&scroll_frame::get_vertical_scroll_range>());

    /** @function set_horizontal_scroll
     */
    type.set_function(
        "set_horizontal_scroll", member_function<&scroll_frame::set_horizontal_scroll>());

    /** @function set_scroll_child
     */
    type.set_function(
        "set_scroll_child", [](scroll_frame& self, std::variant<std::string, frame*> child) {
            utils::observer_ptr<frame> p_child = get_object<frame>(self.get_manager(), child);

            utils::owner_ptr<frame> p_scroll_child;
            if (p_child)
                p_scroll_child = utils::static_pointer_cast<frame>(p_child->release_from_parent());

            self.set_scroll_child(std::move(p_scroll_child));
        });

    /** @function set_vertical_scroll
     */
    type.set_function("set_vertical_scroll", member_function<&scroll_frame::set_vertical_scroll>());
}

} // namespace lxgui::gui
