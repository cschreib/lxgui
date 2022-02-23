#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/utils_string.hpp"

#include <lxgui/extern_sol2_state.hpp>

/** The base class of all elements in the GUI.
 * Objects of this class offers core functionalities needed by every element
 * of the interface. They have a name, and a corresponding variable created
 * in Lua to access them. They can have a parent @{Frame}. They can be placed
 * on the screen at an absolute position, or relative to other @{Region}s.
 * They can be shown or hidden.
 *
 * Apart form this, a @{Region} does not contain anything, nor can it display
 * anything on the screen. Any functionality beyond the list above is implemented
 * in specialized subclasses (see the full list below).
 *
 * __Interaction between C++, Lua, and layout files.__ When a @{Region} is created,
 * it must be given a name, for example `"PlayerHealthBar"`. For as long as the
 * object lives, this name will be used to refer to it. In particular, as soon
 * as the object is created, regardless of whether this was done in C++, layout
 * files, or Lua, a new variable will be created in the Lua state with the exact
 * same name, `PlayerHealthBar`. This variable is a reference to the @{Region},
 * and can be used to interact with it dynamically. Because of this, each object
 * must have a unique name, otherwise it could not be accessible from Lua.
 *
 * Note: Although you can destroy this Lua variable by setting it to nil, this is
 * not recommended: the object will _not_ be destroyed (nor garbage-collected)
 * because it still exists in the C++ memory space. The only way to truly destroy
 * an object is to call @{Manager.delete_frame} (for @{Frame}s only). Destroying and
 * creating objects has a cost however. If the object is likely to reappear later
 * with the same content, simply hide it and show it again later on. If the
 * content may change, you can also recycle the object, i.e., keep it alive and
 * simply change its content when it later reappears.
 *
 * __Parent-child relationship.__ Parents of @{Region}s are @{Frame}s. See
 * the @{Frame} class documentation for more information. One important aspect
 * of the parent-child relationship is related to the object name. If a
 * @{Region} has a parent, it can be given a name starting with `"$parent"`.
 * The name of the parent will automatically replace the `"$parent"` string.
 * For example, if an object is named `"$parentButton"` and its parent is named
 * `"ErrorMessage"`, the final name of the object will be `"ErrorMessageButton"`.
 * It can be accessed from the Lua state as `ErrorMessageButton`, or as
 * `ErrorMessage.Button`. Note that this is totally dynamic: if you later change
 * the parent of this button to be another frame, for example `"ExitDialog"`
 * its name will naturally change to `"ExitDialogButton"`, and it can be accessed
 * from Lua as `ExitDialogButton`, or as `ExitDialog.Button`. This is particularly
 * powerful for writing generic code which does not rely on the full names of
 * objects, only on their child-parent relationship.
 *
 * __Positioning.__ @{Region}s have a position on the screen, but this is
 * not parametrized as a simple pair of X and Y coordinates. Instead, objects
 * are positioned based on a list of "anchors". Anchors are links between
 * objects, which force one edge or one corner of a given object to match with
 * the edge or corner of another object. For example, given two objects A and B,
 * you can create an anchor that links the top-left corner of A to the top-left
 * corner of B. The position of A will automatically be linked to the position of
 * B, hence if B moves, A will follow. To further refine this positioning, you
 * can specify anchor offsets: for example, you may want A's top-left corner to
 * be shifted from B's top-left corner by two pixels in the X direction, and
 * five in the Y direction. This offset can be defined either as an absolute
 * number of pixels, or as a relative fraction of the size of the object being
 * anchored to. For example, you can specify that A's top-left corner links to
 * B's top-left corner, with an horizontal offset equal to 30% of B's width.
 * Read the "Anchors" section below for more information.
 *
 * An object which has no anchor will be considered "invalid" and will not be
 * displayed.
 *
 * __Sizing.__ There are two ways to specify the size of a @{Region}. The
 * first and most straightforward approach is to directly set its width and/or
 * height. This must be specified as an absolute number of pixels. The second
 * and more versatile method is to use more than one anchor for opposite sides
 * of the object, for example an anchor for the "left" and another for the
 * "right" edge. This will implicitly give a width to the object, depending on
 * the position of the other objects to which it is anchored. Anchors will always
 * override the absolute width and height of an object if they provide any
 * constraint on the extents of the object in a given dimension.
 *
 * An object which has neither a fixed absolute size, nor has it size implicitly
 * constrained by anchors, is considered "invalid" and will not be displayed.
 *
 * __Anchors.__ There are nine available anchor points:
 *
 * - `TOP_LEFT`: constrains the max Y and min X.
 * - `TOP_RIGHT`: constrains the max Y and max X.
 * - `BOTTOM_LEFT`: constrains the min Y and min X.
 * - `BOTTOM_RIGHT`: constrains the min Y and max X.
 * - `LEFT`: constrains the min X and the midpoint in Y.
 * - `RIGHT`: constrains the max X and the midpoint in Y.
 * - `TOP`: constrains the max Y and the midpoint in X.
 * - `BOTTOM`: constrains the min Y and the midpoint in X.
 * - `CENTER`: constrains the midpoint in X and Y.
 *
 * If you specify two constraints on the same point (for example: `TOP_LEFT`
 * and `BOTTOM_LEFT` both constrain the min X coordinate), the most stringent
 * constraint always wins. Constraints on the midpoints are more subtle however,
 * as they will always be discarded when both the min and max are constrained.
 * For example, consider an object `A` of fixed size 30x30 and some other object
 * `B` of fixed size 40x40. If we anchor the `RIGHT` of `A` to the `LEFT` of `B`,
 * `A`'s _vertical_ center will be automatically aligned with `B`'s vertical center.
 * This is the effect of the midpoint constraint. Now, if we further anchor the
 * `TOP` of `A` to the `TOP` of `B`, we have more than one anchor constraining
 * the vertical extents of `A` (see "Sizing" above), therefore `A`'s fixed height
 * of 30 pixels will be ignored from now on. It will shrink to a height of 20
 * pixels, i.e., the distance between `B`'s top edge and its vertical center.
 * Finally, if we further anchor the `BOTTOM` of `A` to the `BOTTOM` of `B`, the
 * constraint on `A`'s midpoint will be ignored: `A` will be enlarged to a height
 * of 40 pixels, i.e., the distance between `B`'s top and bottom edges.
 *
 * Inherits all methods from: none.
 *
 * Child classes: @{Frame}, @{LayeredRegion}, @{FontString}, @{Texture},
 * @{Button}, @{CheckButton}, @{EditBox}, @{ScrollFrame},
 * @{Slider}, @{StatusBar}.
 * @classmod Region
 */

namespace lxgui::gui {

void region::set_lua_member_(std::string key, sol::stack_object value) {
    auto iter = lua_members_.find(key);
    if (iter == lua_members_.cend()) {
        lua_members_.insert(iter, {std::move(key), value});
    } else {
        iter->second = sol::object(value);
    }
}

sol::object region::get_lua_member_(const std::string& key) const {
    auto iter = lua_members_.find(key);
    if (iter == lua_members_.cend())
        return sol::lua_nil;

    return iter->second;
}

void region::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<region>(
        "Region", sol::meta_function::index, member_function<&region::get_lua_member_>(),
        sol::meta_function::new_index, member_function<&region::set_lua_member_>());

    /** @function get_alpha
     */
    type.set_function("get_alpha", member_function<&region::get_alpha>());

    /** @function get_name
     */
    type.set_function("get_name", member_function<&region::get_name>());

    /** @function get_object_type
     */
    type.set_function("get_object_type", member_function<&region::get_object_type>());

    /** @function is_object_type
     */
    type.set_function(
        "is_object_type",
        member_function< // select the right overload for Lua
            static_cast<bool (region::*)(const std::string&) const>(&region::is_object_type)>());

    /** @function set_alpha
     */
    type.set_function("set_alpha", member_function<&region::set_alpha>());

    /** @function clear_all_points
     */
    type.set_function("clear_all_points", member_function<&region::clear_all_points>());

    /** @function get_bottom
     */
    type.set_function("get_bottom", member_function<&region::get_bottom>());

    /** @function get_center
     */
    type.set_function("get_center", [](const region& self) {
        vector2f p = self.get_center();
        return std::make_pair(p.x, p.y);
    });

    /** @function get_height
     */
    type.set_function(
        "get_height", [](const region& self) { return self.get_apparent_dimensions().y; });

    /** @function get_left
     */
    type.set_function("get_left", member_function<&region::get_left>());

    /** @function get_num_point
     */
    type.set_function("get_num_point", member_function<&region::get_num_point>());

    /** @function get_parent
     */
    type.set_function("get_parent", [](region& self) {
        sol::object parent;
        if (auto* self_parent = self.get_parent().get())
            parent = self.get_manager().get_lua()[self_parent->get_lua_name()];
        return parent;
    });

    /** @function get_point
     */
    type.set_function("get_point", [](const region& self, sol::optional<std::size_t> p) {
        point point_value = point::top_left;
        if (p.has_value()) {
            if (p.value() > static_cast<std::size_t>(point::center))
                throw sol::error("requested anchor point is invalid");

            point_value = static_cast<point>(p.value());
        }

        const anchor& a = self.get_point(point_value);

        return std::make_tuple(
            anchor::get_anchor_point_name(a.object_point), a.get_parent(),
            anchor::get_anchor_point_name(a.parent_point), a.offset.x, a.offset.y);
    });

    /** @function get_right
     */
    type.set_function("get_right", member_function<&region::get_right>());

    /** @function get_top
     */
    type.set_function("get_top", member_function<&region::get_top>());

    /** @function get_width
     */
    type.set_function(
        "get_width", [](const region& self) { return self.get_apparent_dimensions().x; });

    /** @function hide
     */
    type.set_function("hide", member_function<&region::hide>());

    /** @function is_shown
     */
    type.set_function("is_shown", member_function<&region::is_shown>());

    /** @function is_visible
     */
    type.set_function("is_visible", member_function<&region::is_visible>());

    /** @function set_all_points
     */
    type.set_function(
        "set_all_points",
        [](region& self, sol::optional<std::variant<std::string, region*>> target) {
            self.set_all_points(
                target.has_value() ? get_object<region>(self.get_manager(), target.value())
                                   : nullptr);
        });

    /** @function set_height
     */
    type.set_function("set_height", member_function<&region::set_height>());

    /** @function set_parent
     */
    type.set_function("set_parent", [](region& self, std::variant<std::string, frame*> parent) {
        utils::observer_ptr<frame> parent_obj = get_object<frame>(self.get_manager(), parent);

        if (parent_obj) {
            if (self.is_object_type<frame>())
                parent_obj->add_child(
                    utils::static_pointer_cast<frame>(self.release_from_parent()));
            else
                parent_obj->add_region(
                    utils::static_pointer_cast<layered_region>(self.release_from_parent()));
        } else {
            if (self.is_object_type<frame>()) {
                self.get_manager().get_root().add_root_frame(
                    utils::static_pointer_cast<frame>(self.release_from_parent()));
            } else
                throw sol::error("set_parent(nil) can only be called on frames");
        }
    });

    /** @function set_point
     */
    type.set_function(
        "set_point", [](region& self, const std::string& point_name,
                        sol::optional<std::variant<std::string, region*>> parent,
                        sol::optional<std::string> relative_point, sol::optional<float> x_offset,
                        sol::optional<float> y_offset) {
            // point
            anchor_point p = anchor::get_anchor_point(point_name);

            // parent
            utils::observer_ptr<region> parent_obj;
            if (parent.has_value()) {
                parent_obj = get_object(self.get_manager(), parent.value());
            } else
                parent_obj = self.get_parent();

            if (parent_obj && parent_obj->depends_on(self)) {
                throw sol::error(
                    "cyclic anchor dependency detected! \"" + self.get_name() + "\" and \"" +
                    parent_obj->get_name() + "\" depend on eachothers (directly or indirectly)");
            }

            // relativePoint
            anchor_point parent_point = p;
            if (relative_point.has_value())
                parent_point = anchor::get_anchor_point(relative_point.value());

            // x, y
            float abs_x = x_offset.value_or(0.0f);
            float abs_y = y_offset.value_or(0.0f);

            self.set_point(
                p, parent_obj ? parent_obj->get_name() : "", parent_point, vector2f(abs_x, abs_y));
        });

    /** @function set_rel_point
     */
    type.set_function(
        "set_rel_point", [](region& self, const std::string& point_name,
                            sol::optional<std::variant<std::string, region*>> parent,
                            sol::optional<std::string>                        relative_point,
                            sol::optional<float> x_offset, sol::optional<float> y_offset) {
            // point
            anchor_point p = anchor::get_anchor_point(point_name);

            // parent
            utils::observer_ptr<region> parent_obj;
            if (parent.has_value()) {
                parent_obj = get_object(self.get_manager(), parent.value());
            } else
                parent_obj = self.get_parent();

            if (parent_obj && parent_obj->depends_on(self)) {
                throw sol::error(
                    "cyclic anchor dependency detected! \"" + self.get_name() + "\" and \"" +
                    parent_obj->get_name() + "\" depend on eachothers (directly or indirectly)");
            }

            // relativePoint
            anchor_point parent_point = p;
            if (relative_point.has_value())
                parent_point = anchor::get_anchor_point(relative_point.value());

            // x, y
            float rel_x = x_offset.value_or(0.0f);
            float rel_y = y_offset.value_or(0.0f);

            self.set_point(
                p, parent_obj ? parent_obj->get_name() : "", parent_point, vector2f(rel_x, rel_y),
                anchor_type::rel);
        });

    /** @function set_width
     */
    type.set_function("set_width", member_function<&region::set_width>());

    /** @function show
     */
    type.set_function("show", member_function<&region::show>());
}

} // namespace lxgui::gui
