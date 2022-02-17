#include "lxgui/gui_button.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

#include <sol/state.hpp>

/** A @{Frame} with a button that can be clicked.
 *   This class can handle three different states: "normal", "pushed"
 *   and "disabled". You can provide a different texture for each of
 *   these states, and two different fontstrings for "normal" and
 *   "disabled".
 *
 *   In addition, you can provide another texture/fontstring for the
 *   "highlight" state (when the mouse is over the button region).
 *
 *   Note that there is no fontstring for the "pushed" state: in this
 *   case, the "normal" font is rendered with a slight offset that you
 *   are free to define.
 *
 *   Note that a @{Button} has @{Frame:enable_mouse} set to `true` by
 *   default.
 *
 *   __Events.__ Hard-coded events available to all @{Button}s, in
 *   addition to those from @{Frame}:
 *
 *   - `OnClick`: Triggered when the button is clicked, either when
 *   @{Button:click} is called, or just when a mouse button is pressed
 *   when the cursor is over the button.
 *   - `OnDoubleClick`: Triggered when the button is double-clicked.
 *   - `OnEnable`: Triggered by @{Button:enable}.
 *   - `OnDisable`: Triggered by @{Button:disable}.
 *
 *   Inherits all methods from: @{Region}, @{Frame}.
 *
 *   Child classes: @{CheckButton}.
 *   @classmod Button
 */

namespace lxgui::gui {

void button::register_on_lua(sol::state& m_lua) {
    auto m_class = m_lua.new_usertype<button>(
        "Button", sol::base_classes, sol::bases<region, frame>(), sol::meta_function::index,
        member_function<&button::get_lua_member_>(), sol::meta_function::new_index,
        member_function<&button::set_lua_member_>());

    /** @function click
     */
    m_class.set_function("click", [](button& m_self) { m_self.fire_script("OnClick"); });

    /** @function disable
     */
    m_class.set_function("disable", member_function<&button::disable>());

    /** @function enable
     */
    m_class.set_function("enable", member_function<&button::enable>());

    /** @function get_button_state
     */
    m_class.set_function("get_button_state", [](const button& m_self) {
        switch (m_self.get_button_state()) {
        case button::state::up: return "NORMAL";
        case button::state::down: return "PUSHED";
        case button::state::disabled: return "DISABLED";
        default: return "UNKNOWN";
        }
    });

    /** @function get_disabled_font_object
     */
    m_class.set_function(
        "get_disabled_font_object",
        member_function< // select the right overload for Lua
            static_cast<const utils::observer_ptr<font_string>& (button::*)()>(
                &button::get_disabled_text)>());

    /** @function get_disabled_text_color
     */
    m_class.set_function(
        "get_disabled_text_color",
        [](const button& m_self) -> sol::optional<std::tuple<float, float, float, float>> {
            if (auto p_font_string = m_self.get_disabled_text()) {
                const color& m_color = p_font_string->get_text_color();
                return std::make_tuple(m_color.r, m_color.g, m_color.b, m_color.a);
            }

            return sol::nullopt;
        });

    /** @function get_disabled_texture
     */
    m_class.set_function(
        "get_disabled_texture", member_function< // select the right overload for Lua
                                    static_cast<const utils::observer_ptr<texture>& (button::*)()>(
                                        &button::get_disabled_texture)>());

    /** @function get_highlight_font_object
     */
    m_class.set_function(
        "get_highlight_font_object",
        member_function< // select the right overload for Lua
            static_cast<const utils::observer_ptr<font_string>& (button::*)()>(
                &button::get_highlight_text)>());

    /** @function get_highlight_text_color
     */
    m_class.set_function(
        "get_highlight_text_color",
        [](const button& m_self) -> sol::optional<std::tuple<float, float, float, float>> {
            if (auto p_font_string = m_self.get_highlight_text()) {
                const color& m_color = p_font_string->get_text_color();
                return std::make_tuple(m_color.r, m_color.g, m_color.b, m_color.a);
            }

            return sol::nullopt;
        });

    /** @function get_highlight_texture
     */
    m_class.set_function(
        "get_highlight_texture", member_function< // select the right overload for Lua
                                     static_cast<const utils::observer_ptr<texture>& (button::*)()>(
                                         &button::get_highlight_texture)>());

    /** @function get_normal_font_object
     */
    m_class.set_function(
        "get_normal_font_object",
        member_function< // select the right overload for Lua
            static_cast<const utils::observer_ptr<font_string>& (button::*)()>(
                &button::get_normal_text)>());

    /** @function get_normal_texture
     */
    m_class.set_function(
        "get_normal_texture", member_function< // select the right overload for Lua
                                  static_cast<const utils::observer_ptr<texture>& (button::*)()>(
                                      &button::get_normal_texture)>());

    /** @function get_pushed_text_offset
     */
    m_class.set_function("get_pushed_text_offset", [](const button& m_self) {
        vector2f offset = m_self.get_pushed_text_offset();
        return std::make_pair(offset.x, offset.y);
    });

    /** @function get_pushed_texture
     */
    m_class.set_function(
        "get_pushed_texture", member_function< // select the right overload for Lua
                                  static_cast<const utils::observer_ptr<texture>& (button::*)()>(
                                      &button::get_pushed_texture)>());

    /** @function get_text
     */
    m_class.set_function(
        "get_text", [](const button& m_self) { return utils::unicode_to_utf8(m_self.get_text()); });

    /** @function get_text_height
     */
    m_class.set_function("get_text_height", [](const button& m_self) -> sol::optional<float> {
        if (auto p_current_font = m_self.get_current_font_string())
            return p_current_font->get_string_height();

        return sol::nullopt;
    });

    /** @function get_text_width
     */
    m_class.set_function("get_text_width", [](const button& m_self) -> sol::optional<float> {
        if (auto p_current_font = m_self.get_current_font_string())
            return p_current_font->get_string_width();

        return sol::nullopt;
    });

    /** @function is_enabled
     */
    m_class.set_function("is_enabled", member_function<&button::is_enabled>());

    /** @function lock_highlight
     */
    m_class.set_function("lock_highlight", member_function<&button::lock_highlight>());

    /** @function set_button_state
     */
    m_class.set_function("set_button_state", [](button& m_self, const std::string& state) {
        if (state == "NORMAL") {
            m_self.enable();
            m_self.release();
        } else if (state == "PUSHED") {
            m_self.enable();
            m_self.push();
        } else if (state == "DISABLED") {
            m_self.disable();
            m_self.release();
        } else {
            gui::out << gui::warning << "Button:set_button_state"
                     << " : Unknown button state : \"" + state + "\"." << std::endl;
        }
    });

    /** @function set_disabled_font_object
     */
    m_class.set_function("set_disabled_font_object", member_function<&button::set_disabled_text>());

    /** @function set_disabled_text_color
     */
    m_class.set_function(
        "set_disabled_text_color",
        sol::overload(
            [](button& m_self, float r, float g, float b, sol::optional<float> a) {
                if (auto p_font_string = m_self.get_disabled_text())
                    p_font_string->set_text_color(color(r, g, b, a.value_or(1.0f)));
            },
            [](button& m_self, const std::string& s) {
                if (auto p_font_string = m_self.get_disabled_text())
                    p_font_string->set_text_color(color(s));
            }));

    /** @function set_disabled_texture
     */
    m_class.set_function("set_disabled_texture", member_function<&button::set_disabled_texture>());

    /** @function set_highlight_font_object
     */
    m_class.set_function(
        "set_highlight_font_object", member_function<&button::set_highlight_text>());

    /** @function set_highlight_text_color
     */
    m_class.set_function(
        "set_highlight_text_color",
        sol::overload(
            [](button& m_self, float r, float g, float b, sol::optional<float> a) {
                if (auto p_font_string = m_self.get_highlight_text())
                    p_font_string->set_text_color(color(r, g, b, a.value_or(1.0f)));
            },
            [](button& m_self, const std::string& s) {
                if (auto p_font_string = m_self.get_highlight_text())
                    p_font_string->set_text_color(color(s));
            }));

    /** @function set_highlight_texture
     */
    m_class.set_function(
        "set_highlight_texture", member_function<&button::set_highlight_texture>());

    /** @function set_normal_font_object
     */
    m_class.set_function("set_normal_font_object", member_function<&button::set_normal_text>());

    /** @function set_normal_text_color
     */
    m_class.set_function(
        "set_normal_text_color",
        sol::overload(
            [](button& m_self, float r, float g, float b, sol::optional<float> a) {
                if (auto p_font_string = m_self.get_normal_text())
                    p_font_string->set_text_color(color(r, g, b, a.value_or(1.0f)));
            },
            [](button& m_self, const std::string& s) {
                if (auto p_font_string = m_self.get_normal_text())
                    p_font_string->set_text_color(color(s));
            }));

    /** @function set_normal_texture
     */
    m_class.set_function("set_normal_texture", member_function<&button::set_normal_texture>());

    /** @function set_pushed_text_offset
     */
    m_class.set_function(
        "set_pushed_text_offset", [](button& m_self, float x_offset, float y_offset) {
            m_self.set_pushed_text_offset(vector2f(x_offset, y_offset));
        });

    /** @function set_pushed_texture
     */
    m_class.set_function("set_pushed_texture", member_function<&button::set_pushed_texture>());

    /** @function set_text
     */
    m_class.set_function("set_text", [](button& m_self, const std::string& text) {
        m_self.set_text(utils::utf8_to_unicode(text));
    });

    /** @function unlock_highlight
     */
    m_class.set_function("unlock_highlight", member_function<&button::unlock_highlight>());
}

} // namespace lxgui::gui
