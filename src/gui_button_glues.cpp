#include "lxgui/gui_button.hpp"
#include "lxgui/gui_font_string.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

#include <lxgui/extern_sol2_state.hpp>

/** A @{Frame} with a button that can be clicked.
 * This class can handle three different states: "normal", "pushed"
 * and "disabled". You can provide a different texture for each of
 * these states, and two different fontstrings for "normal" and
 * "disabled".
 *
 * In addition, you can provide another texture/fontstring for the
 * "highlight" state (when the mouse is over the button region).
 *
 * Note that there is no fontstring for the "pushed" state: in this
 * case, the "normal" font is rendered with a slight offset that you
 * are free to define.
 *
 * Note that a @{Button} has @{Frame:enable_mouse} set to `true` by
 * default.
 *
 * __Events.__ Hard-coded events available to all @{Button}s, in
 * addition to those from @{Frame}:
 *
 * - `OnClick`: Triggered when the button is clicked, either when
 * @{Button:click} is called, or just when a mouse button is pressed
 * when the cursor is over the button.
 * - `OnDoubleClick`: Triggered when the button is double-clicked.
 * - `OnEnable`: Triggered by @{Button:enable}.
 * - `OnDisable`: Triggered by @{Button:disable}.
 *
 * Inherits all methods from: @{Region}, @{Frame}.
 *
 * Child classes: @{CheckButton}.
 * @classmod Button
 */

namespace lxgui::gui {

void button::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<button>(
        "Button", sol::base_classes, sol::bases<region, frame>(), sol::meta_function::index,
        member_function<&button::get_lua_member_>(), sol::meta_function::new_index,
        member_function<&button::set_lua_member_>());

    /** @function click
     */
    type.set_function("click", [](button& self) { self.fire_script("OnClick"); });

    /** @function disable
     */
    type.set_function("disable", member_function<&button::disable>());

    /** @function enable
     */
    type.set_function("enable", member_function<&button::enable>());

    /** @function get_button_state
     */
    type.set_function("get_button_state", [](const button& self) {
        switch (self.get_button_state()) {
        case button::state::up: return "NORMAL";
        case button::state::down: return "PUSHED";
        case button::state::disabled: return "DISABLED";
        default: return "UNKNOWN";
        }
    });

    /** @function get_disabled_font_object
     */
    type.set_function(
        "get_disabled_font_object",
        member_function< // select the right overload for Lua
            static_cast<const utils::observer_ptr<font_string>& (button::*)()>(
                &button::get_disabled_text)>());

    /** @function get_disabled_text_color
     */
    type.set_function(
        "get_disabled_text_color",
        [](const button& self) -> sol::optional<std::tuple<float, float, float, float>> {
            if (auto font_string = self.get_disabled_text()) {
                const color& color = font_string->get_text_color();
                return std::make_tuple(color.r, color.g, color.b, color.a);
            }

            return sol::nullopt;
        });

    /** @function get_disabled_texture
     */
    type.set_function(
        "get_disabled_texture", member_function< // select the right overload for Lua
                                    static_cast<const utils::observer_ptr<texture>& (button::*)()>(
                                        &button::get_disabled_texture)>());

    /** @function get_highlight_font_object
     */
    type.set_function(
        "get_highlight_font_object",
        member_function< // select the right overload for Lua
            static_cast<const utils::observer_ptr<font_string>& (button::*)()>(
                &button::get_highlight_text)>());

    /** @function get_highlight_text_color
     */
    type.set_function(
        "get_highlight_text_color",
        [](const button& self) -> sol::optional<std::tuple<float, float, float, float>> {
            if (auto font_string = self.get_highlight_text()) {
                const color& color = font_string->get_text_color();
                return std::make_tuple(color.r, color.g, color.b, color.a);
            }

            return sol::nullopt;
        });

    /** @function get_highlight_texture
     */
    type.set_function(
        "get_highlight_texture", member_function< // select the right overload for Lua
                                     static_cast<const utils::observer_ptr<texture>& (button::*)()>(
                                         &button::get_highlight_texture)>());

    /** @function get_normal_font_object
     */
    type.set_function(
        "get_normal_font_object",
        member_function< // select the right overload for Lua
            static_cast<const utils::observer_ptr<font_string>& (button::*)()>(
                &button::get_normal_text)>());

    /** @function get_normal_texture
     */
    type.set_function(
        "get_normal_texture", member_function< // select the right overload for Lua
                                  static_cast<const utils::observer_ptr<texture>& (button::*)()>(
                                      &button::get_normal_texture)>());

    /** @function get_pushed_text_offset
     */
    type.set_function("get_pushed_text_offset", [](const button& self) {
        vector2f offset = self.get_pushed_text_offset();
        return std::make_pair(offset.x, offset.y);
    });

    /** @function get_pushed_texture
     */
    type.set_function(
        "get_pushed_texture", member_function< // select the right overload for Lua
                                  static_cast<const utils::observer_ptr<texture>& (button::*)()>(
                                      &button::get_pushed_texture)>());

    /** @function get_text
     */
    type.set_function(
        "get_text", [](const button& self) { return utils::unicode_to_utf8(self.get_text()); });

    /** @function get_text_height
     */
    type.set_function("get_text_height", [](const button& self) -> sol::optional<float> {
        if (auto current_font = self.get_current_font_string())
            return current_font->get_string_height();

        return sol::nullopt;
    });

    /** @function get_text_width
     */
    type.set_function("get_text_width", [](const button& self) -> sol::optional<float> {
        if (auto current_font = self.get_current_font_string())
            return current_font->get_string_width();

        return sol::nullopt;
    });

    /** @function is_enabled
     */
    type.set_function("is_enabled", member_function<&button::is_enabled>());

    /** @function lock_highlight
     */
    type.set_function("lock_highlight", member_function<&button::lock_highlight>());

    /** @function set_button_state
     */
    type.set_function("set_button_state", [](button& self, const std::string& state) {
        if (state == "NORMAL") {
            self.enable();
            self.release();
        } else if (state == "PUSHED") {
            self.enable();
            self.push();
        } else if (state == "DISABLED") {
            self.disable();
            self.release();
        } else {
            gui::out << gui::warning << "Button:set_button_state"
                     << ": Unknown button state: \"" + state + "\"." << std::endl;
        }
    });

    /** @function set_disabled_font_object
     */
    type.set_function("set_disabled_font_object", member_function<&button::set_disabled_text>());

    /** @function set_disabled_text_color
     */
    type.set_function(
        "set_disabled_text_color",
        sol::overload(
            [](button& self, float r, float g, float b, sol::optional<float> a) {
                if (auto font_string = self.get_disabled_text())
                    font_string->set_text_color(color(r, g, b, a.value_or(1.0f)));
            },
            [](button& self, const std::string& s) {
                if (auto font_string = self.get_disabled_text())
                    font_string->set_text_color(color(s));
            }));

    /** @function set_disabled_texture
     */
    type.set_function("set_disabled_texture", member_function<&button::set_disabled_texture>());

    /** @function set_highlight_font_object
     */
    type.set_function("set_highlight_font_object", member_function<&button::set_highlight_text>());

    /** @function set_highlight_text_color
     */
    type.set_function(
        "set_highlight_text_color",
        sol::overload(
            [](button& self, float r, float g, float b, sol::optional<float> a) {
                if (auto font_string = self.get_highlight_text())
                    font_string->set_text_color(color(r, g, b, a.value_or(1.0f)));
            },
            [](button& self, const std::string& s) {
                if (auto font_string = self.get_highlight_text())
                    font_string->set_text_color(color(s));
            }));

    /** @function set_highlight_texture
     */
    type.set_function("set_highlight_texture", member_function<&button::set_highlight_texture>());

    /** @function set_normal_font_object
     */
    type.set_function("set_normal_font_object", member_function<&button::set_normal_text>());

    /** @function set_normal_text_color
     */
    type.set_function(
        "set_normal_text_color",
        sol::overload(
            [](button& self, float r, float g, float b, sol::optional<float> a) {
                if (auto font_string = self.get_normal_text())
                    font_string->set_text_color(color(r, g, b, a.value_or(1.0f)));
            },
            [](button& self, const std::string& s) {
                if (auto font_string = self.get_normal_text())
                    font_string->set_text_color(color(s));
            }));

    /** @function set_normal_texture
     */
    type.set_function("set_normal_texture", member_function<&button::set_normal_texture>());

    /** @function set_pushed_text_offset
     */
    type.set_function("set_pushed_text_offset", [](button& self, float x_offset, float y_offset) {
        self.set_pushed_text_offset(vector2f(x_offset, y_offset));
    });

    /** @function set_pushed_texture
     */
    type.set_function("set_pushed_texture", member_function<&button::set_pushed_texture>());

    /** @function set_text
     */
    type.set_function("set_text", [](button& self, const std::string& text) {
        self.set_text(utils::utf8_to_unicode(text));
    });

    /** @function unlock_highlight
     */
    type.set_function("unlock_highlight", member_function<&button::unlock_highlight>());
}

} // namespace lxgui::gui
