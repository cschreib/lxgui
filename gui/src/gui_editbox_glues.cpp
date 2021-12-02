#include "lxgui/gui_editbox.hpp"

#include "lxgui/gui_uiobject_tpl.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_out.hpp"

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
*   - `OnChar`: Triggered whenever a new character is added to the
*   edit box. Will always be preceeded by `OnTextChanged`.
*   - `OnCursorChanged`: Triggered whenever the position of the edit
*   cursor is changed (not yet implemented).
*   - `OnEditFocusGained`: Triggered when the edit box gains focus,
*   see @{FocusFrame:set_focus}.
*   - `OnEditFocusLost`: Triggered when the edit box looses focus,
*   see @{FocusFrame:set_focus}.
*   - `OnEnterPressed`: Triggered when the `Enter` (or `Return`) key
*   is pressed while the edit box is focussed. This captures both
*   the main keyboard key and the smaller one on the numpad.
*   - `OnEscapePressed`: Triggered when the `Escape` key is *released*
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
*   etc.).
*   - `OnTextSet`: Triggered by @{EditBox:set_text}. Will always be
*   followed by `OnTextChanged`.
*
*   Inherits all methods from: @{UIObject}, @{Frame}, @{FocusFrame}.
*
*   Child classes: none.
*   @classmod EditBox
*/

namespace lxgui {
namespace gui
{
void edit_box::register_on_lua(sol::state& mLua)
{
    auto mClass = mLua.new_usertype<edit_box>("EditBox",
        sol::base_classes, sol::bases<uiobject, frame>(),
        sol::meta_function::index,
        member_function<&edit_box::get_lua_member_>(),
        sol::meta_function::new_index,
        member_function<&edit_box::set_lua_member_>());

    /** @function add_history_line
    */
    mClass.set_function("add_history_line", [](edit_box& mSelf, const std::string& sLine)
    {
        mSelf.add_history_line(utils::utf8_to_unicode(sLine));
    });

    /** @function clear_history
    */
    mClass.set_function("clear_history", member_function<&edit_box::clear_history>());

    /** @function get_blink_speed
    */
    mClass.set_function("get_blink_speed", member_function<&edit_box::get_blink_speed>());

    /** @function get_cursor_position
    */
    mClass.set_function("get_cursor_position", member_function<&edit_box::get_cursor_position>());

    /** @function get_history_lines
    */
    mClass.set_function("get_history_lines", [](const edit_box& mSelf)
    {
        return sol::as_table(mSelf.get_history_lines());
    });

    /** @function get_max_letters
    */
    mClass.set_function("get_max_letters", member_function<&edit_box::get_max_letters>());

    /** @function get_num_letters
    */
    mClass.set_function("get_num_letters", member_function<&edit_box::get_num_letters>());

    /** @function get_number
    */
    mClass.set_function("get_number", [](const edit_box& mSelf)
    {
        return utils::string_to_double(mSelf.get_text());
    });

    /** @function get_text
    */
    mClass.set_function("get_text", [](const edit_box& mSelf)
    {
        return utils::unicode_to_utf8(mSelf.get_text());
    });

    /** @function get_text_insets
    */
    mClass.set_function("get_text_insets", [](const edit_box& mSelf)
    {
        const bounds2f& lInsets = mSelf.get_text_insets();
        return std::make_tuple(lInsets.left, lInsets.right, lInsets.top, lInsets.bottom);
    });

    /** @function highlight_text
    */
    mClass.set_function("highlight_text", [](edit_box& mSelf,
        sol::optional<uint> uiStart, sol::optional<uint> uiEnd)
    {
        mSelf.highlight_text(
            uiStart.value_or(0u),
            uiEnd.value_or(std::numeric_limits<uint>::max()));
    });

    /** @function insert
    */
    mClass.set_function("insert", [](edit_box& mSelf, const std::string& sText)
    {
        mSelf.insert_after_cursor(utils::utf8_to_unicode(sText));
    });

    /** @function is_multi_line
    */
    mClass.set_function("is_multi_line", member_function<&edit_box::is_multi_line>());

    /** @function is_numeric
    */
    mClass.set_function("is_numeric", member_function<&edit_box::is_numeric_only>());

    /** @function is_password
    */
    mClass.set_function("is_password", member_function<&edit_box::is_password_mode_enabled>());

    /** @function set_blink_speed
    */
    mClass.set_function("set_blink_speed", member_function<&edit_box::set_blink_speed>());

    /** @function set_cursor_position
    */
    mClass.set_function("set_cursor_position", member_function<&edit_box::set_cursor_position>());

    /** @function set_font
    */
    mClass.set_function("set_font", [](edit_box& mSelf, const std::string& sFile,
        float fHeight, sol::optional<std::string> sFlags)
    {
        mSelf.set_font(sFile, fHeight);

        auto pFontString = mSelf.get_font_string().get();
        if (!pFontString)
            return;

        if (sFlags.has_value())
        {
            if (sFlags.value().find("OUTLINE") != std::string::npos ||
                sFlags.value().find("THICKOUTLINE") != std::string::npos)
                pFontString->set_outlined(true);
            else if (sFlags.value().empty())
                pFontString->set_outlined(false);
            else
            {
                gui::out << gui::warning << "EditBox:set_font : "
                    << "Unknown flags : \"" << sFlags.value() <<"\"." << std::endl;
            }
        }
        else
            pFontString->set_outlined(false);
    });

    /** @function set_max_history_lines
    */
    mClass.set_function("set_max_history_lines", member_function<&edit_box::set_max_history_lines>());

    /** @function set_max_letters
    */
    mClass.set_function("set_max_letters", member_function<&edit_box::set_max_letters>());

    /** @function set_multi_line
    */
    mClass.set_function("set_multi_line", member_function<&edit_box::set_multi_line>());

    /** @function set_number
    */
    mClass.set_function("set_number", sol::overload(
    [](edit_box& mSelf, int iValue)
    {
        mSelf.set_text(utils::to_ustring(iValue));
    },
    [](edit_box& mSelf, double dValue)
    {
        mSelf.set_text(utils::to_ustring(dValue));
    }));

    /** @function set_numeric
    */
    mClass.set_function("set_numeric", member_function<&edit_box::set_numeric_only>());

    /** @function set_password
    */
    mClass.set_function("set_password", member_function<&edit_box::enable_password_mode>());

    /** @function set_text
    */
    mClass.set_function("set_text", [](edit_box& mSelf, const std::string& sText)
    {
        mSelf.set_text(utils::utf8_to_unicode(sText));
    });

    /** @function set_text_insets
    */
    mClass.set_function("set_text_insets", [](edit_box& mSelf,
        float fLeft, float fRight, float fTop, float fBottom)
    {
        mSelf.set_text_insets(bounds2f(fLeft, fRight, fTop, fBottom));
    });
}

}
}
