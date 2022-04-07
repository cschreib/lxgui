#include "lxgui/gui_edit_box.hpp"
#include "lxgui/gui_font_string.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"

#include <lxgui/extern_sol2_state.hpp>

/** A @{Frame} with an editable text box.
 * This frame lets the user input arbitrary text into a box,
 * which can be read and used by the rest of the interface.
 * The text box can be either single-line or multi-line.
 * Text can be selected by holding the Shift key, and natural
 * navigation is available with the Left, Right, Up, Down, Home,
 * End, Page Up, and Page Down keys. Copy and paste operations
 * are also supported. The edit box can also remember the history
 * of previously entered values or commands, which can be brought
 * back at will. The characters entered as handled by the
 * operating system, hence this class will use whatever keyboard
 * layout is currently in use. Finally, the edit box can be
 * configured to only accept numeric values (of either sign, or
 * positive only), and to hide the input characters to simulate a
 * password box (no encryption or other safety measure is used).
 *
 * Note that an @{EditBox} has @{Frame:enable_mouse} set to `true`
 * and @{Frame:enable_drag} set to `"LeftButton"` by default.
 *
 * __Events.__ Hard-coded events available to all @{EditBox}es,
 * in addition to those from @{Frame}:
 *
 * - `OnCursorChanged`: Triggered whenever the position of the edit
 * cursor is changed (not yet implemented).
 * - `OnEnterPressed`: Triggered when the `Enter` (or `Return`) key
 * is pressed while the edit box is focused. This captures both
 * the main keyboard key and the smaller one on the numpad.
 * - `OnEscapePressed`: Triggered when the `Escape` key is pressed
 * while the edit box is focused.
 * - `OnSpacePressed`: Triggered when the `Space` key is pressed
 * while the edit box is focused.
 * - `OnTabPressed`: Triggered when the `Tab` key is pressed
 * while the edit box is focused.
 * - `OnUpPressed`: Triggered when the `Up` key is pressed
 * while the edit box is focused.
 * - `OnDownPressed`: Triggered when the `Down` key is pressed
 * while the edit box is focused.
 * - `OnTextChanged`: Triggered whenever the text contained in the
 * edit box changes (character added or deleted, text set or pasted,
 * etc.). Triggered after `OnChar`.
 * - `OnTextSet`: Triggered by @{EditBox:set_text}. Will always be
 * followed by `OnTextChanged`.
 *
 * Inherits all methods from: @{Region}, @{Frame}.
 *
 * Child classes: none.
 * @classmod EditBox
 */

namespace lxgui::gui {

void edit_box::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<edit_box>(
        edit_box::class_name, sol::base_classes, sol::bases<region, frame>(),
        sol::meta_function::index, member_function<&edit_box::get_lua_member_>(),
        sol::meta_function::new_index, member_function<&edit_box::set_lua_member_>());

    /** @function add_history_line
     */
    type.set_function("add_history_line", [](edit_box& self, const std::string& line) {
        self.add_history_line(utils::utf8_to_unicode(line));
    });

    /** @function clear_history
     */
    type.set_function("clear_history", member_function<&edit_box::clear_history>());

    /** @function disable_password_mode
     */
    type.set_function("disable_password_mode", member_function<&edit_box::disable_password_mode>());

    /** @function enable_password_mode
     */
    type.set_function("enable_password_mode", member_function<&edit_box::enable_password_mode>());

    /** @function get_blink_period
     */
    type.set_function("get_blink_period", member_function<&edit_box::get_blink_period>());

    /** @function get_cursor_position
     */
    type.set_function("get_cursor_position", member_function<&edit_box::get_cursor_position>());

    /** @function get_history_lines
     */
    type.set_function("get_history_lines", [](const edit_box& self) {
        return sol::as_table(self.get_history_lines());
    });

    /** @function get_max_letters
     */
    type.set_function("get_max_letters", member_function<&edit_box::get_max_letters>());

    /** @function get_letter_count
     */
    type.set_function("get_letter_count", member_function<&edit_box::get_letter_count>());

    /** @function get_number
     */
    type.set_function("get_number", [](const edit_box& self) {
        // TODO: use localizer's locale for that
        // https://github.com/cschreib/lxgui/issues/88
        return utils::from_string<double>(self.get_text()).value_or(0.0);
    });

    /** @function get_text
     */
    type.set_function(
        "get_text", [](const edit_box& self) { return utils::unicode_to_utf8(self.get_text()); });

    /** @function get_text_insets
     */
    type.set_function("get_text_insets", [](const edit_box& self) {
        const bounds2f& insets = self.get_text_insets();
        return std::make_tuple(insets.left, insets.right, insets.top, insets.bottom);
    });

    /** @function highlight_text
     */
    type.set_function(
        "highlight_text",
        [](edit_box& self, sol::optional<std::size_t> start, sol::optional<std::size_t> end) {
            self.highlight_text(
                start.value_or(0u), end.value_or(std::numeric_limits<std::size_t>::max()));
        });

    /** @function insert
     */
    type.set_function("insert", [](edit_box& self, const std::string& text) {
        self.insert_after_cursor(utils::utf8_to_unicode(text));
    });

    /** @function is_multi_line
     */
    type.set_function("is_multi_line", member_function<&edit_box::is_multi_line>());

    /** @function is_numeric
     */
    type.set_function("is_numeric", member_function<&edit_box::is_numeric_only>());

    /** @function is_password_mode_enabled
     */
    type.set_function(
        "is_password_mode_enabled", member_function<&edit_box::is_password_mode_enabled>());

    /** @function set_blink_period
     */
    type.set_function("set_blink_period", member_function<&edit_box::set_blink_period>());

    /** @function set_cursor_position
     */
    type.set_function("set_cursor_position", member_function<&edit_box::set_cursor_position>());

    /** @function set_font
     */
    type.set_function(
        "set_font", [](edit_box& self, const std::string& file, float height,
                       sol::optional<std::string> flags) {
            self.set_font(file, height);

            auto* font_string = self.get_font_string().get();
            if (!font_string)
                return;

            if (flags.has_value()) {
                if (flags.value().find("OUTLINE") != std::string::npos ||
                    flags.value().find("THICKOUTLINE") != std::string::npos)
                    font_string->set_outlined(true);
                else if (flags.value().empty())
                    font_string->set_outlined(false);
                else {
                    gui::out << gui::warning << "EditBox:set_font: "
                             << "Unknown flags: \"" << flags.value() << "\"." << std::endl;
                }
            } else
                font_string->set_outlined(false);
        });

    /** @function set_max_history_lines
     */
    type.set_function("set_max_history_lines", member_function<&edit_box::set_max_history_lines>());

    /** @function set_max_letters
     */
    type.set_function("set_max_letters", member_function<&edit_box::set_max_letters>());

    /** @function set_multi_line
     */
    type.set_function("set_multi_line", member_function<&edit_box::set_multi_line>());

    /** @function set_number
     */
    type.set_function(
        "set_number", sol::overload(
                          [](edit_box& self, int value) {
                              self.set_text(utils::utf8_to_unicode(
                                  self.get_manager().get_localizer().format_string("{:L}", value)));
                          },
                          [](edit_box& self, double value) {
                              self.set_text(utils::utf8_to_unicode(
                                  self.get_manager().get_localizer().format_string("{:L}", value)));
                          }));

    /** @function set_numeric
     */
    type.set_function("set_numeric", member_function<&edit_box::set_numeric_only>());

    /** @function set_password_mode_enabled
     */
    type.set_function(
        "set_password_mode_enabled", member_function<&edit_box::set_password_mode_enabled>());

    /** @function set_text
     */
    type.set_function("set_text", [](edit_box& self, const std::string& text) {
        self.set_text(utils::utf8_to_unicode(text));
    });

    /** @function set_text_insets
     */
    type.set_function(
        "set_text_insets", [](edit_box& self, float left, float right, float top, float bottom) {
            self.set_text_insets(bounds2f(left, right, top, bottom));
        });
}

} // namespace lxgui::gui
