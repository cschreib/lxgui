#include "lxgui/gui_editbox.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"

#include <sol/state.hpp>

/** A @{Frame} with an editable text box.
 *   This frame lets the user input arbitrary text into a box,
 *   which can be read and used by the rest of the interface.
 *   The text box can be either single-line or multi-line.
 *   Text can be selected by holding the Shift key, and natural
 *   navigation is available with the Left, Right, Up, Down, Home,
 *   End, Page Up, and Page Down keys. Copy and paste operations
 *   are also supported. The edit box can also remember the history
 *   of previously entered values or commands, which can be brought
 *   back at will. The characters entered as handled by the
 *   operating system, hence this class will use whatever keyboard
 *   layout is currently in use. Finally, the edit box can be
 *   configured to only accept numeric values (of either sign, or
 *   positive only), and to hide the input characters to simulate a
 *   password box (no encryption or other safety measure is used).
 *
 *   Note that an @{EditBox} has @{Frame:enable_mouse} set to `true`
 *   and @{Frame:register_for_drag} set to `"LeftButton"` by default.
 *
 *   __Events.__ Hard-coded events available to all @{EditBox}es,
 *   in addition to those from @{Frame}:
 *
 *   - `OnCursorChanged`: Triggered whenever the position of the edit
 *   cursor is changed (not yet implemented).
 *   - `OnEnterPressed`: Triggered when the `Enter` (or `Return`) key
 *   is pressed while the edit box is focussed. This captures both
 *   the main keyboard key and the smaller one on the numpad.
 *   - `OnEscapePressed`: Triggered when the `Escape` key is pressed
 *   while the edit box is focussed.
 *   - `OnSpacePressed`: Triggered when the `Space` key is pressed
 *   while the edit box is focussed.
 *   - `OnTabPressed`: Triggered when the `Tab` key is pressed
 *   while the edit box is focussed.
 *   - `OnUpPressed`: Triggered when the `Up` key is pressed
 *   while the edit box is focussed.
 *   - `OnDownPressed`: Triggered when the `Down` key is pressed
 *   while the edit box is focussed.
 *   - `OnTextChanged`: Triggered whenever the text contained in the
 *   edit box changes (character added or deleted, text set or pasted,
 *   etc.). Triggered after `OnChar`.
 *   - `OnTextSet`: Triggered by @{EditBox:set_text}. Will always be
 *   followed by `OnTextChanged`.
 *
 *   Inherits all methods from: @{Region}, @{Frame}.
 *
 *   Child classes: none.
 *   @classmod EditBox
 */

namespace lxgui::gui {

void edit_box::register_on_lua(sol::state& m_lua) {
    auto m_class = m_lua.new_usertype<edit_box>(
        "EditBox", sol::base_classes, sol::bases<region, frame>(), sol::meta_function::index,
        member_function<&edit_box::get_lua_member_>(), sol::meta_function::new_index,
        member_function<&edit_box::set_lua_member_>());

    /** @function add_history_line
     */
    m_class.set_function("add_history_line", [](edit_box& m_self, const std::string& s_line) {
        m_self.add_history_line(utils::utf8_to_unicode(s_line));
    });

    /** @function clear_history
     */
    m_class.set_function("clear_history", member_function<&edit_box::clear_history>());

    /** @function get_blink_speed
     */
    m_class.set_function("get_blink_speed", member_function<&edit_box::get_blink_speed>());

    /** @function get_cursor_position
     */
    m_class.set_function("get_cursor_position", member_function<&edit_box::get_cursor_position>());

    /** @function get_history_lines
     */
    m_class.set_function("get_history_lines", [](const edit_box& m_self) {
        return sol::as_table(m_self.get_history_lines());
    });

    /** @function get_max_letters
     */
    m_class.set_function("get_max_letters", member_function<&edit_box::get_max_letters>());

    /** @function get_num_letters
     */
    m_class.set_function("get_num_letters", member_function<&edit_box::get_num_letters>());

    /** @function get_number
     */
    m_class.set_function("get_number", [](const edit_box& m_self) {
        // TODO: use localizer's locale for that
        // https://github.com/cschreib/lxgui/issues/88
        double d_number = 0.0;
        utils::from_string(m_self.get_text(), d_number);
        return d_number;
    });

    /** @function get_text
     */
    m_class.set_function(
        "get_text", [](const edit_box& m_self) { return utils::unicode_to_utf8(m_self.get_text()); });

    /** @function get_text_insets
     */
    m_class.set_function("get_text_insets", [](const edit_box& m_self) {
        const bounds2f& l_insets = m_self.get_text_insets();
        return std::make_tuple(l_insets.left, l_insets.right, l_insets.top, l_insets.bottom);
    });

    /** @function highlight_text
     */
    m_class.set_function(
        "highlight_text",
        [](edit_box& m_self, sol::optional<std::size_t> ui_start, sol::optional<std::size_t> ui_end) {
            m_self.highlight_text(
                ui_start.value_or(0u), ui_end.value_or(std::numeric_limits<std::size_t>::max()));
        });

    /** @function insert
     */
    m_class.set_function("insert", [](edit_box& m_self, const std::string& s_text) {
        m_self.insert_after_cursor(utils::utf8_to_unicode(s_text));
    });

    /** @function is_multi_line
     */
    m_class.set_function("is_multi_line", member_function<&edit_box::is_multi_line>());

    /** @function is_numeric
     */
    m_class.set_function("is_numeric", member_function<&edit_box::is_numeric_only>());

    /** @function is_password
     */
    m_class.set_function("is_password", member_function<&edit_box::is_password_mode_enabled>());

    /** @function set_blink_speed
     */
    m_class.set_function("set_blink_speed", member_function<&edit_box::set_blink_speed>());

    /** @function set_cursor_position
     */
    m_class.set_function("set_cursor_position", member_function<&edit_box::set_cursor_position>());

    /** @function set_font
     */
    m_class.set_function(
        "set_font", [](edit_box& m_self, const std::string& s_file, float f_height,
                       sol::optional<std::string> s_flags) {
            m_self.set_font(s_file, f_height);

            auto* p_font_string = m_self.get_font_string().get();
            if (!p_font_string)
                return;

            if (s_flags.has_value()) {
                if (s_flags.value().find("OUTLINE") != std::string::npos ||
                    s_flags.value().find("THICKOUTLINE") != std::string::npos)
                    p_font_string->set_outlined(true);
                else if (s_flags.value().empty())
                    p_font_string->set_outlined(false);
                else {
                    gui::out << gui::warning << "EditBox:set_font : "
                             << "Unknown flags : \"" << s_flags.value() << "\"." << std::endl;
                }
            } else
                p_font_string->set_outlined(false);
        });

    /** @function set_max_history_lines
     */
    m_class.set_function(
        "set_max_history_lines", member_function<&edit_box::set_max_history_lines>());

    /** @function set_max_letters
     */
    m_class.set_function("set_max_letters", member_function<&edit_box::set_max_letters>());

    /** @function set_multi_line
     */
    m_class.set_function("set_multi_line", member_function<&edit_box::set_multi_line>());

    /** @function set_number
     */
    m_class.set_function(
        "set_number",
        sol::overload(
            [](edit_box& m_self, int i_value) {
                m_self.set_text(utils::utf8_to_unicode(
                    m_self.get_manager().get_localizer().format_string("{:L}", i_value)));
            },
            [](edit_box& m_self, double d_value) {
                m_self.set_text(utils::utf8_to_unicode(
                    m_self.get_manager().get_localizer().format_string("{:L}", d_value)));
            }));

    /** @function set_numeric
     */
    m_class.set_function("set_numeric", member_function<&edit_box::set_numeric_only>());

    /** @function set_password
     */
    m_class.set_function("set_password", member_function<&edit_box::enable_password_mode>());

    /** @function set_text
     */
    m_class.set_function("set_text", [](edit_box& m_self, const std::string& s_text) {
        m_self.set_text(utils::utf8_to_unicode(s_text));
    });

    /** @function set_text_insets
     */
    m_class.set_function(
        "set_text_insets",
        [](edit_box& m_self, float f_left, float f_right, float f_top, float f_bottom) {
            m_self.set_text_insets(bounds2f(f_left, f_right, f_top, f_bottom));
        });
}

} // namespace lxgui::gui
