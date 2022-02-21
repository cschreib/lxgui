#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_virtual_registry.hpp"
#include "lxgui/gui_virtual_root.hpp"

#include <lxgui/extern_sol2_state.hpp>
#include <lxgui/extern_sol2_variadic_args.hpp>

/** A @{Region} that can contain other regions and react to events.
 * This class, which is at the core of the UI design, can contain
 * other @{Frame}s as "children", and @{LayeredRegion}s sorted by layers
 * (text, images, ...). A frame can also react to events, and register
 * callbacks to be executed on particular events (key presses, etc.)
 * or on every tick.
 *
 * Each frame has an optional "title region", which can be used to
 * define and draw a title bar. This title bar can then be used to
 * move the frame around the screen using mouse click and drag.
 * Furthermore, frames have optional support for resizing by click
 * and drag on corners or edges (opt in).
 *
 * Frames can either move freely on the screen, or be "clamped" to the
 * screen so they cannot be partly outside of their render area.
 *
 * __Rendering.__ Frames are grouped into different "strata", which are
 * rendered sequentially. Frames in a high strata will always be rendered
 * above frames in a low strata. Then, within a strata, frames are further
 * sorted by "level"; within this particular strata, a frame with a high
 * level will always be rendered above all frames with a lower level, but
 * it will still remain below other frames in a higher strata. The level
 * of a frame is automatically set to the maximum level inside the strata
 * when the frame is clicked, which effectively brings the frame to the
 * front.
 *
 * __Children and regions.__ When a frame is hidden, all its children
 * and regions will also be hidden. Likewise, deleting a frame will
 * automatically delete all its children and regions, unless they are
 * detached first. Other than this, children and regions do not need to
 * be located inside the frame; this is controlled purely by their anchors.
 * Therefore, if a child is not anchored to its parent, moving the parent
 * will not automatically move the child.
 *
 * __Events.__ Frames can react to events. For this to happen, a callback
 * function must be registered to handle the corresponding event. There are
 * two types of events. First: hard-coded UI events such as `OnKeyPress`
 * or `OnUpdate`, which are automatically triggered by lxgui. Second:
 * generic events, which can be triggered from various sources and all
 * forwarded to the `OnEvent` callback. Generic events are typically
 * generated by whatever application is being driven by lxgui (i.e., your
 * game), and they enable application-specific behavior (for example:
 * changing the UI when the player is under attack will likely require an
 * `"UNDER_ATTACK"` event).
 *
 * To use the first type of events (hard-coded events), all you have to
 * do in general is register a callback function using @{Frame:add_script}
 * or @{Frame:set_script}. However, some hard-coded events require explicit
 * enabling. In particular:
 *
 * - Events related to keyboard input (`OnKeyDown`, `OnKeyUp`) require
 * focus, see @{Frame:set_focus}, or @{Frame:enable_key_capture}.
 * - Events related to mouse click input (`OnDragStart`, `OnDragStop`,
 * `OnMouseUp`, `OnMouseDown`) require @{Frame:enable_mouse_click}.
 * - Events related to mouse move input (`OnEnter`, `OnLeave`)
 * require @{Frame:enable_mouse_move}.
 * - Events related to mouse wheel input (`OnMouseWheel`) require
 * @{Frame:enable_mouse_wheel}.
 *
 * To use the second type of events (generic events), you have to register
 * a callback for `OnEvent` _and_ register the frame for each generic event
 * you wish to listen to. This is done with @{Frame:register_event}.
 *
 * Some events provide arguments to the registered callback function. For
 * example, the application can fire a `"UNIT_ATTACKED"` event when a unit
 * is under attack, and pass the ID of the attacked unit as a first argument,
 * and the ID of the attacker as a second argument. If a callback
 * function is registered using @{Frame:add_script} or @{Frame:set_script},
 * these arguments can be handled and named like regular function parameters.
 * In layout files "scripts" handlers, they can be accessed with the
 * hard-coded generic names `arg1`, `arg2`, etc.
 *
 * Hard-coded events available to all @{Frame}s:
 *
 * - `OnChar`: Triggered whenever a character is typed into the frame, and
 * the frame has focus (see @{Frame:set_focus}).
 * - `OnDragStart`: Triggered when one of the mouse button registered for
 * dragging (see @{Frame:register_for_drag}) has been pressed inside the
 * area of the screen occupied by the frame, and a mouse movement is first
 * recorded.
 * - `OnDragMove`: Triggered after `OnDragStart`, each time the mouse moves,
 * until `OnDragStop` is triggered.
 * - `OnDragStop`: Triggered after `OnDragStart`, when the mouse button is
 * released.
 * - `OnEnter`: Triggered when the mouse pointer enters into the area of
 * the screen occupied by the frame. Note: this only takes into account the
 * position and size of the frame and its title region, but not the space
 * occupied by its children or layered regions. Will not trigger if the
 * frame is hidden.
 * - `OnEvent`: Triggered when a registered generic event occurs. See
 * @{Frame:register_event}. To allow distinguishing which event has just
 * been fired, the registered callback function is always provided with a
 * first argument that is set to a string matching the event name. Further
 * arguments can be passed to the callback and are handled as for other events.
 * - `OnFocusGained`: Triggered when the frame gains focus, see
 * @{Frame:set_focus}.
 * - `OnFocusLost`: Triggered when the frame looses focus, see
 * @{Frame:set_focus}.
 * - `OnHide`: Triggered when @{Region:hide} is called, or when the frame
 * is hidden indirectly (for example if its parent is itself hidden). This
 * will only fire if the frame was previously shown.
 * - `OnKeyDown`: Triggered when any keyboard key is pressed. Will only
 * trigger if the frame has focus (see @{Frame:set_focus}) or if the key has
 * been registered for capture using @{Frame:enable_key_capture}. If no
 * frame is focused, only the topmost frame with
 * @{Frame:enable_key_capture} will receive the event. If no frame has
 * captured the key, then the key is tested for existing key bindings (see
 * @{Manager:set_key_binding}). This event provides two arguments to the registered
 * callback: a number identifying the key, and the human-readable name of the
 * key. If you need to react to simultaneous key presses (e.g., Shift+A), use
 * the @{Manager:set_key_binding}.
 * - `OnKeyUp`: Triggered when any keyboard key is released. Will only
 * trigger if the frame has focus (see @{Frame:set_focus}) or if the key has
 * been registered for capture using @{Frame:enable_key_capture}. If no
 * frame is focused, only the topmost frame with
 * @{Frame:enable_key_capture} will receive the event. If no frame has
 * captured the key, then the key is tested for existing key bindings (see
 * @{Manager:set_key_binding}). This event provides two arguments to the registered
 * callback: a number identifying the key, and the human-readable name of the
 * key. If you need to react to simultaneous key presses (e.g., Shift+A), use
 * the @{Manager:set_key_binding}.
 * - `OnLeave`: Triggered when the mouse pointer leaves the area of the
 * screen occupied by the frame. Note: this only takes into account the
 * position and size of the frame and its title region, but not the space
 * occupied by its children or layered regions. Will not trigger if the
 * frame is hidden, unless the frame was just hidden with the mouse
 * previously inside the frame. Finally, this _will_ trigger whenever
 * the mouse enters another mouse-enabled frame with a higher level/strata,
 * even if the mouse is still technically within this frame's region.
 * - `OnLoad`: Triggered just after the frame is created. This is where
 * you would normally register for events and specific inputs, set up
 * initial states for extra logic, or do localization. When this event is
 * triggered, you can assume that all the frame's regions and children
 * have already been loaded. The same is true for other frames and regions
 * that are defined *earlier* in the same layout file, and those that are
 * defined in an addon listed *earlier* than the current addon in the
 * 'addons.txt' file. In all other cases, frames or regions will not yet
 * be loaded when `OnLoad` is called, hence they cannot be refered to
 * (directly or indirectly).
 * - `OnMouseDown`: Triggered when any mouse button is pressedand this frame is
 * the topmost mouse-click-enabled frame under the mouse pointer. Will not
 * trigger if the frame is hidden. This event provides one argument to
 * the registered callback: a string identifying the mouse button
 * (`"LeftButton"`, `"RightButton"`, or `"MiddleButton"`).
 * - `OnMouseUp`: Triggered when any mouse button is releasedand this frame is
 * the topmost mouse-click-enabled frame under the mouse pointer. Will not
 * trigger if the frame is hidden. This event provides one argument to
 * the registered callback: a string identifying the mouse button
 * (`"LeftButton"`, `"RightButton"`, or `"MiddleButton"`).
 * - `OnMouseWheel`: Triggered when the mouse wheel is moved and this frame is
 * the topmost mouse-wheel-enabled frame under the mouse pointer. This event
 * provides one argument to the registered callback: a number indicating by
 * how many "notches" the wheel has turned in this event. A positive value
 * means the wheel has been moved "away" from the user (this would normally
 * scroll *up* in a document).
 * - `OnReceiveDrag`: Triggered when the mouse pointer was previously
 * dragged onto the frame, and when one of the mouse button registered for
 * dragging (see @{Frame:register_for_drag}) is released. This enables
 * the "drop" in "drag and drop" operations.
 * - `OnShow`: Triggered when @{Region:show} is called, or when the frame
 * is shown indirectly (for example if its parent is itself shown). This
 * will only fire if the frame was previously hidden.
 * - `OnSizeChanged`: Triggered whenever the size of the frame changes, either
 * directly or indirectly. Be very careful not to call any function that could
 * change the size of the frame inside this callback, as this would generate
 * an infinite loop.
 * - `OnUpdate`: Triggered on every tick of the game loop. This event provides
 * one argument to the registered callback: a floating point number indicating
 * how much time has passed since the last call to `OnUpdate` (in seconds).
 * For optimal performance, prefer using other events types whenever possible.
 * `OnUpdate` callbacks will be executed over and over again, and can quickly
 * consume a lot of resources if user unreasonably. If you have to use
 * `OnUpdate`, you can mitigate performance problems by artificially reducing
 * the update rate: let the callback function only accumulate the time passed,
 * and wait until enough time has passed (say, half a second) to execute any
 * expensive operation. Then reset the accumulated time, and wait again.
 *
 * Generic events fired natively by lxgui:
 *
 * - `"LUA_ERROR"`: Triggered whenever a callback function or an addon script
 * file generates a Lua error. This event provides one argument to the
 * registered callback: a string containing the error message.
 * - `"ADDON_LOADED"`: Triggered when an addon is fully loaded. This event
 * provides one argument to the registered callback: a string containing the
 * name of the loaded addon.
 * - `"ENTERING_WORLD"`: Triggered once at the start of the program, at the
 * end of the first update tick.
 *
 * __Virtual frames.__ Virtual frames are not displayed on the screen,
 * and technically are not part of the interface. They are only available
 * as "templates" that can be reused by other (virtual or non-virtual)
 * frames. This is useful for defining a few frame templates with a
 * particular style, and then reuse these templates across the interface
 * to ensure a consistent look. When inheriting from a virtual frame,
 * the inheriting frame will copy all the registered callbacks, all the
 * child frames, and all the layered regions of the virtual frame.
 *
 * This inheritance mechanism can be chained: a virtual frame itself can
 * inherit from another virtual frame. It is also possible to inherit from
 * multiple virtual frames at once, which will copy their respective content
 * in the order they are specified.
 *
 * Inherits all methods from: @{Region}.
 *
 * Child classes: @{Button}, @{CheckButton}, @{EditBox},
 * @{ScrollFrame}, @{Slider}, @{StatusBar}.
 * @classmod Frame
 */

namespace lxgui::gui {

void frame::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<frame>(
        "Frame", sol::base_classes, sol::bases<region>(), sol::meta_function::index,
        member_function<&frame::get_lua_member_>(), sol::meta_function::new_index,
        member_function<&frame::set_lua_member_>());

    /** @function add_script
     */
    type.set_function(
        "add_script", [](frame& self, const std::string& name, sol::protected_function func) {
            self.add_script(name, std::move(func));
        });

    /** @function clear_focus
     */
    type.set_function("clear_focus", [](frame& self) { self.set_focus(false); });

    /** @function create_font_string
     */
    type.set_function(
        "create_font_string",
        [](frame& self, const std::string& name, sol::optional<std::string> layer_name,
           sol::optional<std::string> inheritance) {
            layer layer = layer::artwork;
            if (layer_name.has_value())
                layer = parse_layer_type(layer_name.value());

            region_core_attributes attr;
            attr.name = name;
            attr.inheritance =
                self.get_manager().get_virtual_root().get_registry().get_virtual_region_list(
                    inheritance.value_or(""));

            return self.create_layered_region<font_string>(layer, std::move(attr));
        });

    /** @function create_texture
     */
    type.set_function(
        "create_texture",
        [](frame& self, const std::string& name, sol::optional<std::string> layer_name,
           sol::optional<std::string> inheritance) {
            layer layer = layer::artwork;
            if (layer_name.has_value())
                layer = parse_layer_type(layer_name.value());

            region_core_attributes attr;
            attr.name = name;
            attr.inheritance =
                self.get_manager().get_virtual_root().get_registry().get_virtual_region_list(
                    inheritance.value_or(""));

            return self.create_layered_region<texture>(layer, std::move(attr));
        });

    /** @function create_title_region
     */
    type.set_function("create_title_region", member_function<&frame::create_title_region>());

    /** @function disable_draw_layer
     */
    type.set_function("disable_draw_layer", [](frame& self, const std::string& layer_name) {
        self.disable_draw_layer(parse_layer_type(layer_name));
    });

    /** @function enable_draw_layer
     */
    type.set_function("enable_draw_layer", [](frame& self, const std::string& layer_name) {
        self.enable_draw_layer(parse_layer_type(layer_name));
    });

    /** @function enable_mouse
     */
    type.set_function("enable_mouse", member_function<&frame::enable_mouse>());

    /** @function enable_mouse_click
     */
    type.set_function("enable_mouse_click", member_function<&frame::enable_mouse_click>());

    /** @function enable_mouse_move
     */
    type.set_function("enable_mouse_move", member_function<&frame::enable_mouse_move>());

    /** @function enable_mouse_wheel
     */
    type.set_function("enable_mouse_wheel", member_function<&frame::enable_mouse_wheel>());

    /** @function enable_key_capture
     */
    type.set_function("enable_key_capture", member_function<&frame::enable_key_capture>());

    /** @function get_backdrop
     */
    type.set_function(
        "get_backdrop",
        [](sol::this_state this_lua, const frame& self) -> sol::optional<sol::table> {
            const backdrop* backdrop = self.get_backdrop();
            if (!backdrop)
                return sol::nullopt;

            sol::table backdrop_table = sol::state_view(this_lua).create_table();

            backdrop_table["bgFile"]   = backdrop->get_background_file();
            backdrop_table["edgeFile"] = backdrop->get_edge_file();
            backdrop_table["tile"]     = backdrop->is_background_tilling();

            backdrop_table["tileSize"] = backdrop->get_tile_size();
            backdrop_table["edgeSize"] = backdrop->get_edge_size();

            const auto& insets                 = backdrop->get_background_insets();
            backdrop_table["insets"]["left"]   = insets.left;
            backdrop_table["insets"]["right"]  = insets.right;
            backdrop_table["insets"]["top"]    = insets.top;
            backdrop_table["insets"]["bottom"] = insets.bottom;

            return std::move(backdrop_table);
        });

    /** @function get_backdrop_border_color
     */
    type.set_function(
        "get_backdrop_border_color",
        [](const frame& self) -> sol::optional<std::tuple<float, float, float, float>> {
            if (!self.get_backdrop())
                return sol::nullopt;

            const color& color = self.get_backdrop()->get_edge_color();
            return std::make_tuple(color.r, color.g, color.b, color.a);
        });

    /** @function get_backdrop_color
     */
    type.set_function(
        "get_backdrop_color",
        [](const frame& self) -> sol::optional<std::tuple<float, float, float, float>> {
            if (!self.get_backdrop())
                return sol::nullopt;

            const color& color = self.get_backdrop()->get_background_color();
            return std::make_tuple(color.r, color.g, color.b, color.a);
        });

    /** @function get_children
     */
    type.set_function("get_children", [](sol::this_state this_lua, const frame& self) {
        std::vector<sol::object> children;
        children.reserve(self.get_rough_num_children());

        auto lua_state = sol::state_view(this_lua);
        for (const auto& child : self.get_children()) {
            children.push_back(lua_state[child.get_lua_name()]);
        }

        return sol::as_table(std::move(children));
    });

    /** @function get_effective_alpha
     */
    type.set_function("get_effective_alpha", member_function<&frame::get_effective_alpha>());

    /** @function get_effective_scale
     */
    type.set_function("get_effective_scale", member_function<&frame::get_effective_scale>());

    /** @function get_frame_level
     */
    type.set_function("get_frame_level", member_function<&frame::get_level>());

    /** @function get_frame_strata
     */
    type.set_function("get_frame_strata", [](const frame& self) -> sol::optional<std::string> {
        frame_strata strata_id = self.get_frame_strata();
        if (strata_id == frame_strata::background)
            return std::string("BACKGROUND");
        else if (strata_id == frame_strata::low)
            return std::string("LOW");
        else if (strata_id == frame_strata::medium)
            return std::string("MEDIUM");
        else if (strata_id == frame_strata::high)
            return std::string("HIGH");
        else if (strata_id == frame_strata::dialog)
            return std::string("DIALOG");
        else if (strata_id == frame_strata::fullscreen)
            return std::string("FULLSCREEN");
        else if (strata_id == frame_strata::fullscreen_dialog)
            return std::string("FULLSCREEN_DIALOG");
        else if (strata_id == frame_strata::tooltip)
            return std::string("TOOLTIP");
        else
            return {};
    });

    /** @function get_frame_type
     */
    type.set_function("get_frame_type", member_function<&frame::get_frame_type>());

    /** @function get_hit_rect_insets
     */
    type.set_function("get_hit_rect_insets", [](const frame& self) {
        const bounds2f& insets = self.get_abs_hit_rect_insets();
        return std::make_tuple(insets.left, insets.right, insets.top, insets.bottom);
    });

    /** @function get_max_dimensions
     */
    type.set_function("get_max_dimensions", [](const frame& self) {
        const vector2f& max = self.get_max_dimensions();
        return std::make_tuple(max.x, max.y);
    });

    /** @function get_min_dimensions
     */
    type.set_function("get_min_dimensions", [](const frame& self) {
        const vector2f& min = self.get_min_dimensions();
        return std::make_tuple(min.x, min.y);
    });

    /** @function get_num_children
     */
    type.set_function("get_num_children", member_function<&frame::get_num_children>());

    /** @function get_num_regions
     */
    type.set_function("get_num_regions", member_function<&frame::get_num_regions>());

    /** @function get_scale
     */
    type.set_function("get_scale", member_function<&frame::get_scale>());

    /** @function get_script
     */
    type.set_function(
        "get_script", [](const frame& self, const std::string& script_name) -> sol::object {
            if (!self.has_script(script_name))
                return sol::lua_nil;

            std::string adjusted_name = get_adjusted_script_name(script_name);
            return self.get_manager().get_lua()[self.get_lua_name()][adjusted_name];
        });

    /** @function get_title_region
     */
    type.set_function(
        "get_title_region",
        member_function< // select the right overload for Lua
            static_cast<utils::observer_ptr<region> (frame::*)()>(&frame::get_title_region)>());

    /** @function has_script
     */
    type.set_function("has_script", member_function<&frame::has_script>());

    /** @function is_auto_focus
     */
    type.set_function("is_auto_focus", member_function<&frame::is_auto_focus_enabled>());

    /** @function is_clamped_to_screen
     */
    type.set_function("is_clamped_to_screen", member_function<&frame::is_clamped_to_screen>());

    /** @function is_frame_type
     */
    type.set_function("is_frame_type", [](const frame& self, const std::string& type_name) {
        return self.is_object_type(type_name);
    });

    /** @function is_mouse_click_enabled
     */
    type.set_function("is_mouse_click_enabled", member_function<&frame::is_mouse_click_enabled>());

    /** @function is_mouse_move_enabled
     */
    type.set_function("is_mouse_move_enabled", member_function<&frame::is_mouse_move_enabled>());

    /** @function is_mouse_wheel_enabled
     */
    type.set_function("is_mouse_wheel_enabled", member_function<&frame::is_mouse_wheel_enabled>());

    /** @function is_key_capture_enabled
     */
    type.set_function("is_key_capture_enabled", member_function<&frame::is_key_capture_enabled>());

    /** @function is_movable
     */
    type.set_function("is_movable", member_function<&frame::is_movable>());

    /** @function is_resizable
     */
    type.set_function("is_resizable", member_function<&frame::is_resizable>());

    /** @function is_top_level
     */
    type.set_function("is_top_level", member_function<&frame::is_top_level>());

    /** @function is_user_placed
     */
    type.set_function("is_user_placed", member_function<&frame::is_user_placed>());

    /** @function raise
     */
    type.set_function("raise", member_function<&frame::raise>());

    /** @function register_event
     */
    type.set_function("register_event", member_function<&frame::register_event>());

    /** @function register_for_drag
     */
    type.set_function(
        "register_for_drag",
        [](frame& self, sol::optional<std::string> button1, sol::optional<std::string> button2,
           sol::optional<std::string> button3) {
            std::vector<std::string> button_list;
            if (button1.has_value())
                button_list.push_back(button1.value());
            if (button2.has_value())
                button_list.push_back(button2.value());
            if (button3.has_value())
                button_list.push_back(button3.value());

            self.register_for_drag(button_list);
        });

    /** @function set_auto_focus
     */
    type.set_function("set_auto_focus", member_function<&frame::enable_auto_focus>());

    /** @function set_backdrop
     */
    type.set_function("set_backdrop", [](frame& self, sol::optional<sol::table> table_opt) {
        if (!table_opt.has_value()) {
            self.set_backdrop(nullptr);
            return;
        }

        std::unique_ptr<backdrop> bdrop(new backdrop(self));

        sol::table& table = table_opt.value();

        bdrop->set_background(self.parse_file_name(table["bgFile"].get_or<std::string>("")));
        bdrop->set_edge(self.parse_file_name(table["edgeFile"].get_or<std::string>("")));
        bdrop->set_background_tilling(table["tile"].get_or(false));

        float tile_size = table["tileSize"].get_or<float>(0.0);
        if (tile_size != 0)
            bdrop->set_tile_size(tile_size);

        float edge_size = table["edgeSize"].get_or<float>(0.0);
        if (edge_size != 0)
            bdrop->set_edge_size(edge_size);

        if (table["insets"] != sol::lua_nil) {
            bdrop->set_background_insets(bounds2f(
                table["insets"]["left"].get_or<float>(0), table["insets"]["right"].get_or<float>(0),
                table["insets"]["top"].get_or<float>(0),
                table["insets"]["bottom"].get_or<float>(0)));
        }

        self.set_backdrop(std::move(bdrop));
    });

    /** @function set_backdrop_border_color
     */
    type.set_function(
        "set_backdrop_border_color",
        sol::overload(
            [](frame& self, float r, float g, float b, sol::optional<float> a) {
                self.get_or_create_backdrop().set_edge_color(color(r, g, b, a.value_or(1.0f)));
            },
            [](frame& self, const std::string& s) {
                self.get_or_create_backdrop().set_edge_color(color(s));
            }));

    /** @function set_backdrop_color
     */
    type.set_function(
        "set_backdrop_color",
        sol::overload(
            [](frame& self, float r, float g, float b, sol::optional<float> a) {
                self.get_or_create_backdrop().set_background_color(
                    color(r, g, b, a.value_or(1.0f)));
            },
            [](frame& self, const std::string& s) {
                self.get_or_create_backdrop().set_background_color(color(s));
            }));

    /** @function set_clamped_to_screen
     */
    type.set_function("set_clamped_to_screen", member_function<&frame::set_clamped_to_screen>());

    /** @function set_focus
     */
    type.set_function("set_focus", [](frame& self) { self.set_focus(true); });

    /** @function set_frame_level
     */
    type.set_function("set_frame_level", member_function<&frame::set_level>());

    /** @function set_frame_strata
     */
    type.set_function(
        "set_frame_strata",
        member_function< // select the right overload for Lua
            static_cast<void (frame::*)(const std::string&)>(&frame::set_frame_strata)>());

    /** @function set_hit_rect_insets
     */
    type.set_function(
        "set_hit_rect_insets", [](frame& self, float left, float right, float top, float bottom) {
            self.set_abs_hit_rect_insets(bounds2f(left, right, top, bottom));
        });

    /** @function set_max_dimensions
     */
    type.set_function("set_max_dimensions", [](frame& self, float width, float height) {
        self.set_max_dimensions(vector2f(width, height));
    });

    /** @function set_min_dimensions
     */
    type.set_function("set_min_dimensions", [](frame& self, float width, float height) {
        self.set_min_dimensions(vector2f(width, height));
    });

    /** @function set_max_width
     */
    type.set_function("set_max_width", member_function<&frame::set_max_width>());

    /** @function set_max_height
     */
    type.set_function("set_max_height", member_function<&frame::set_max_height>());

    /** @function set_min_width
     */
    type.set_function("set_min_width", member_function<&frame::set_min_width>());

    /** @function set_min_height
     */
    type.set_function("set_min_height", member_function<&frame::set_min_height>());

    /** @function set_movable
     */
    type.set_function("set_movable", member_function<&frame::set_movable>());

    /** @function set_resizable
     */
    type.set_function("set_resizable", member_function<&frame::set_resizable>());

    /** @function set_scale
     */
    type.set_function("set_scale", member_function<&frame::set_scale>());

    /** @function set_script
     */
    type.set_function(
        "set_script", [](frame& self, const std::string& script_name,
                         sol::optional<sol::protected_function> script) {
            if (!self.can_use_script(script_name)) {
                gui::out << gui::error << self.get_frame_type() << ": "
                         << "\"" << self.get_name() << "\" cannot use script \"" << script_name
                         << "\"." << std::endl;
                return;
            }

            if (script.has_value())
                self.set_script(script_name, script.value());
            else
                self.remove_script(script_name);
        });

    /** @function set_top_level
     */
    type.set_function("set_top_level", member_function<&frame::set_top_level>());

    /** @function set_user_placed
     */
    type.set_function("set_user_placed", member_function<&frame::set_user_placed>());

    /** @function start_moving
     */
    type.set_function("start_moving", member_function<&frame::start_moving>());

    /** @function start_sizing
     */
    type.set_function("start_sizing", [](frame& self, const std::string& point) {
        self.start_sizing(anchor::get_anchor_point(point));
    });

    /** @function stop_moving_or_sizing
     */
    type.set_function("stop_moving_or_sizing", [](frame& self) {
        self.stop_moving();
        self.stop_sizing();
    });

    /** @function unregister_event
     */
    type.set_function("unregister_event", member_function<&frame::unregister_event>());
}

} // namespace lxgui::gui
