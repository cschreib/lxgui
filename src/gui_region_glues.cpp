#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/utils_string.hpp"

#include <sol/state.hpp>

/** The base class of all elements in the GUI.
 *   Objects of this class offers core functionalities needed by every element
 *   of the interface. They have a name, and a corresponding variable created
 *   in Lua to access them. They can have a parent @{Frame}. They can be placed
 *   on the screen at an absolute position, or relative to other @{Region}s.
 *   They can be shown or hidden.
 *
 *   Apart form this, a @{Region} does not contain anything, nor can it display
 *   anything on the screen. Any functionality beyond the list above is implemented
 *   in specialized subclasses (see the full list below).
 *
 *   __Interaction between C++, Lua, and layout files.__ When a @{Region} is created,
 *   it must be given a name, for example `"PlayerHealthBar"`. For as long as the
 *   object lives, this name will be used to refer to it. In particular, as soon
 *   as the object is created, regardless of whether this was done in C++, layout
 *   files, or Lua, a new variable will be created in the Lua state with the exact
 *   same name, `PlayerHealthBar`. This variable is a reference to the @{Region},
 *   and can be used to interact with it dynamically. Because of this, each object
 *   must have a unique name, otherwise it could not be accessible from Lua.
 *
 *   Note: Although you can destroy this Lua variable by setting it to nil, this is
 *   not recommended: the object will _not_ be destroyed (nor garbage-collected)
 *   because it still exists in the C++ memory space. The only way to truly destroy
 *   an object is to call @{Manager.delete_frame} (for @{Frame}s only). Destroying and
 *   creating objects has a cost however. If the object is likely to reappear later
 *   with the same content, simply hide it and show it again later on. If the
 *   content may change, you can also recycle the object, i.e., keep it alive and
 *   simply change its content when it later reappears.
 *
 *   __Parent-child relationship.__ Parents of @{Region}s are @{Frame}s. See
 *   the @{Frame} class documentation for more information. One important aspect
 *   of the parent-child relationship is related to the object name. If a
 *   @{Region} has a parent, it can be given a name starting with `"$parent"`.
 *   The name of the parent will automatically replace the `"$parent"` string.
 *   For example, if an object is named `"$parentButton"` and its parent is named
 *   `"ErrorMessage"`, the final name of the object will be `"ErrorMessageButton"`.
 *   It can be accessed from the Lua state as `ErrorMessageButton`, or as
 *   `ErrorMessage.Button`. Note that this is totally dynamic: if you later change
 *   the parent of this button to be another frame, for example `"ExitDialog"`
 *   its name will naturally change to `"ExitDialogButton"`, and it can be accessed
 *   from Lua as `ExitDialogButton`, or as `ExitDialog.Button`. This is particularly
 *   powerful for writing generic code which does not rely on the full names of
 *   objects, only on their child-parent relationship.
 *
 *   __Positioning.__ @{Region}s have a position on the screen, but this is
 *   not parametrized as a simple pair of X and Y coordinates. Instead, objects
 *   are positioned based on a list of "anchors". Anchors are links between
 *   objects, which force one edge or one corner of a given object to match with
 *   the edge or corner of another object. For example, given two objects A and B,
 *   you can create an anchor that links the top-left corner of A to the top-left
 *   corner of B. The position of A will automatically be linked to the position of
 *   B, hence if B moves, A will follow. To further refine this positioning, you
 *   can specify anchor offsets: for example, you may want A's top-left corner to
 *   be shifted from B's top-left corner by two pixels in the X direction, and
 *   five in the Y direction. This offset can be defined either as an absolute
 *   number of pixels, or as a relative fraction of the size of the object being
 *   anchored to. For example, you can specify that A's top-left corner links to
 *   B's top-left corner, with an horizontal offset equal to 30% of B's width.
 *   Read the "Anchors" section below for more information.
 *
 *   An object which has no anchor will be considered "invalid" and will not be
 *   displayed.
 *
 *   __Sizing.__ There are two ways to specify the size of a @{Region}. The
 *   first and most straightforward approach is to directly set its width and/or
 *   height. This must be specified as an absolute number of pixels. The second
 *   and more versatile method is to use more than one anchor for opposite sides
 *   of the object, for example an anchor for the "left" and another for the
 *   "right" edge. This will implicitly give a width to the object, depending on
 *   the position of the other objects to which it is anchored. Anchors will always
 *   override the absolute width and height of an object if they provide any
 *   constraint on the extents of the object in a given dimension.
 *
 *   An object which has neither a fixed absolute size, nor has it size implicitly
 *   constrained by anchors, is considered "invalid" and will not be displayed.
 *
 *   __Anchors.__ There are nine available anchor points:
 *
 *   - `TOP_LEFT`: constrains the max Y and min X.
 *   - `TOP_RIGHT`: constrains the max Y and max X.
 *   - `BOTTOM_LEFT`: constrains the min Y and min X.
 *   - `BOTTOMRIGH`: constrains the min Y and max X.
 *   - `LEFT`: constrains the min X and the midpoint in Y.
 *   - `RIGHT`: constrains the max X and the midpoint in Y.
 *   - `TOP`: constrains the max Y and the midpoint in X.
 *   - `BOTTOM`: constrains the min Y and the midpoint in X.
 *   - `CENTER`: constrains the midpoint in X and Y.
 *
 *   If you specify two constraints on the same point (for example: `TOP_LEFT`
 *   and `BOTTOM_LEFT` both constrain the min X coordinate), the most stringent
 *   constraint always wins. Constraints on the midpoints are more subtle however,
 *   as they will always be discarded when both the min and max are constrained.
 *   For example, consider an object `A` of fixed size 30x30 and some other object
 *   `B` of fixed size 40x40. If we anchor the `RIGHT` of `A` to the `LEFT` of `B`,
 *   `A`'s _vertical_ center will be automatically aligned with `B`'s vertical center.
 *   This is the effect of the midpoint constraint. Now, if we further anchor the
 *   `TOP` of `A` to the `TOP` of `B`, we have more than one anchor constraining
 *   the vertical extents of `A` (see "Sizing" above), therefore `A`'s fixed height
 *   of 30 pixels will be ignored from now on. It will shrink to a height of 20
 *   pixels, i.e., the distance between `B`'s top edge and its vertical center.
 *   Finally, if we further anchor the `BOTTOM` of `A` to the `BOTTOM` of `B`, the
 *   constraint on `A`'s midpoint will be ignored: `A` will be enlarged to a height
 *   of 40 pixels, i.e., the distance between `B`'s top and bottom edges.
 *
 *   Inherits all methods from: none.
 *
 *   Child classes: @{Frame}, @{LayeredRegion}, @{FontString}, @{Texture},
 *   @{Button}, @{CheckButton}, @{EditBox}, @{ScrollFrame},
 *   @{Slider}, @{StatusBar}.
 *   @classmod Region
 */

namespace lxgui::gui {

void region::set_lua_member_(std::string s_key, sol::stack_object m_value) {
    auto m_iter = l_lua_members_.find(s_key);
    if (m_iter == l_lua_members_.cend()) {
        l_lua_members_.insert(m_iter, {std::move(s_key), m_value});
    } else {
        m_iter->second = sol::object(m_value);
    }
}

sol::object region::get_lua_member_(const std::string& s_key) const {
    auto m_iter = l_lua_members_.find(s_key);
    if (m_iter == l_lua_members_.cend())
        return sol::lua_nil;

    return m_iter->second;
}

void region::register_on_lua(sol::state& m_lua) {
    auto m_class = m_lua.new_usertype<region>(
        "Region", sol::meta_function::index, member_function<&region::get_lua_member_>(),
        sol::meta_function::new_index, member_function<&region::set_lua_member_>());

    /** @function get_alpha
     */
    m_class.set_function("get_alpha", member_function<&region::get_alpha>());

    /** @function get_name
     */
    m_class.set_function("get_name", member_function<&region::get_name>());

    /** @function get_object_type
     */
    m_class.set_function("get_object_type", member_function<&region::get_object_type>());

    /** @function is_object_type
     */
    m_class.set_function(
        "is_object_type",
        member_function< // select the right overload for Lua
            static_cast<bool (region::*)(const std::string&) const>(&region::is_object_type)>());

    /** @function set_alpha
     */
    m_class.set_function("set_alpha", member_function<&region::set_alpha>());

    /** @function clear_all_points
     */
    m_class.set_function("clear_all_points", member_function<&region::clear_all_points>());

    /** @function get_bottom
     */
    m_class.set_function("get_bottom", member_function<&region::get_bottom>());

    /** @function get_center
     */
    m_class.set_function("get_center", [](const region& m_self) {
        vector2f m_p = m_self.get_center();
        return std::make_pair(m_p.x, m_p.y);
    });

    /** @function get_height
     */
    m_class.set_function(
        "get_height", [](const region& m_self) { return m_self.get_apparent_dimensions().y; });

    /** @function get_left
     */
    m_class.set_function("get_left", member_function<&region::get_left>());

    /** @function get_num_point
     */
    m_class.set_function("get_num_point", member_function<&region::get_num_point>());

    /** @function get_parent
     */
    m_class.set_function("get_parent", [](region& m_self) {
        sol::object m_parent;
        if (auto* p_parent = m_self.get_parent().get())
            m_parent = m_self.get_manager().get_lua()[p_parent->get_lua_name()];
        return m_parent;
    });

    /** @function get_point
     */
    m_class.set_function("get_point", [](const region& m_self, sol::optional<std::size_t> m_point) {
        anchor_point m_point_value = anchor_point::top_left;
        if (m_point.has_value()) {
            if (m_point.value() > static_cast<std::size_t>(anchor_point::center))
                throw sol::error("requested anchor point is invalid");

            m_point_value = static_cast<anchor_point>(m_point.value());
        }

        const anchor& m_anchor = m_self.get_point(m_point_value);

        return std::make_tuple(
            anchor::get_string_point(m_anchor.m_point), m_anchor.get_parent(),
            anchor::get_string_point(m_anchor.m_parent_point), m_anchor.m_offset.x, m_anchor.m_offset.y);
    });

    /** @function get_right
     */
    m_class.set_function("get_right", member_function<&region::get_right>());

    /** @function get_top
     */
    m_class.set_function("get_top", member_function<&region::get_top>());

    /** @function get_width
     */
    m_class.set_function(
        "get_width", [](const region& m_self) { return m_self.get_apparent_dimensions().x; });

    /** @function hide
     */
    m_class.set_function("hide", member_function<&region::hide>());

    /** @function is_shown
     */
    m_class.set_function("is_shown", member_function<&region::is_shown>());

    /** @function is_visible
     */
    m_class.set_function("is_visible", member_function<&region::is_visible>());

    /** @function set_all_points
     */
    m_class.set_function(
        "set_all_points",
        [](region& m_self, sol::optional<std::variant<std::string, region*>> m_target) {
            m_self.set_all_points(
                m_target.has_value() ? get_object<region>(m_self.get_manager(), m_target.value())
                                    : nullptr);
        });

    /** @function set_height
     */
    m_class.set_function("set_height", member_function<&region::set_height>());

    /** @function set_parent
     */
    m_class.set_function("set_parent", [](region& m_self, std::variant<std::string, frame*> m_parent) {
        utils::observer_ptr<frame> p_parent = get_object<frame>(m_self.get_manager(), m_parent);

        if (p_parent) {
            if (m_self.is_object_type<frame>())
                p_parent->add_child(utils::static_pointer_cast<frame>(m_self.release_from_parent()));
            else
                p_parent->add_region(
                    utils::static_pointer_cast<layered_region>(m_self.release_from_parent()));
        } else {
            if (m_self.is_object_type<frame>()) {
                m_self.get_manager().get_root().add_root_frame(
                    utils::static_pointer_cast<frame>(m_self.release_from_parent()));
            } else
                throw sol::error("set_parent(nil) can only be called on frames");
        }
    });

    /** @function set_point
     */
    m_class.set_function(
        "set_point", [](region& m_self, const std::string& s_point,
                        sol::optional<std::variant<std::string, region*>> m_parent,
                        sol::optional<std::string> s_relative_point, sol::optional<float> f_x_offset,
                        sol::optional<float> f_y_offset) {
            // point
            anchor_point m_point = anchor::get_anchor_point(s_point);

            // parent
            utils::observer_ptr<region> p_parent;
            if (m_parent.has_value()) {
                p_parent = get_object(m_self.get_manager(), m_parent.value());
            } else
                p_parent = m_self.get_parent();

            if (p_parent && p_parent->depends_on(m_self)) {
                throw sol::error(
                    "cyclic anchor dependency detected! \"" + m_self.get_name() + "\" and \"" +
                    p_parent->get_name() + "\" depend on eachothers (directly or indirectly)");
            }

            // relativePoint
            anchor_point m_parent_point = m_point;
            if (s_relative_point.has_value())
                m_parent_point = anchor::get_anchor_point(s_relative_point.value());

            // x, y
            float f_abs_x = f_x_offset.value_or(0.0f);
            float f_abs_y = f_y_offset.value_or(0.0f);

            m_self.set_point(
                m_point, p_parent ? p_parent->get_name() : "", m_parent_point, vector2f(f_abs_x, f_abs_y));
        });

    /** @function set_rel_point
     */
    m_class.set_function(
        "set_rel_point", [](region& m_self, const std::string& s_point,
                            sol::optional<std::variant<std::string, region*>> m_parent,
                            sol::optional<std::string>                        s_relative_point,
                            sol::optional<float> f_x_offset, sol::optional<float> f_y_offset) {
            // point
            anchor_point m_point = anchor::get_anchor_point(s_point);

            // parent
            utils::observer_ptr<region> p_parent;
            if (m_parent.has_value()) {
                p_parent = get_object(m_self.get_manager(), m_parent.value());
            } else
                p_parent = m_self.get_parent();

            if (p_parent && p_parent->depends_on(m_self)) {
                throw sol::error(
                    "cyclic anchor dependency detected! \"" + m_self.get_name() + "\" and \"" +
                    p_parent->get_name() + "\" depend on eachothers (directly or indirectly)");
            }

            // relativePoint
            anchor_point m_parent_point = m_point;
            if (s_relative_point.has_value())
                m_parent_point = anchor::get_anchor_point(s_relative_point.value());

            // x, y
            float f_rel_x = f_x_offset.value_or(0.0f);
            float f_rel_y = f_y_offset.value_or(0.0f);

            m_self.set_point(
                m_point, p_parent ? p_parent->get_name() : "", m_parent_point, vector2f(f_rel_x, f_rel_y),
                anchor_type::rel);
        });

    /** @function set_width
     */
    m_class.set_function("set_width", member_function<&region::set_width>());

    /** @function show
     */
    m_class.set_function("show", member_function<&region::show>());
}

} // namespace lxgui::gui
