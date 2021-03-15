#include "lxgui/gui_editbox.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

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
void edit_box::register_glue(lua::state& mLua)
{
    mLua.reg<lua_edit_box>();
}

lua_edit_box::lua_edit_box(lua_State* pLua) : lua_focus_frame(pLua)
{
}

/** @function add_history_line
*/
int lua_edit_box::_add_history_line(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:add_history_line", pLua);
    mFunc.add(0, "line", lua::type::STRING);
    if (mFunc.check())
        get_object()->add_history_line(mFunc.get(0)->get_string());

    return mFunc.on_return();
}

/** @function clear_history
*/
int lua_edit_box::_clear_history(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:clear_history", pLua);

    get_object()->clear_history();

    return mFunc.on_return();
}

/** @function get_blink_speed
*/
int lua_edit_box::_get_blink_speed(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:get_blink_speed", pLua, 1);

    mFunc.push(get_object()->get_blink_speed());

    return mFunc.on_return();
}

/** @function get_cursor_position
*/
int lua_edit_box::_get_cursor_position(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:get_cursor_position", pLua, 1);

    mFunc.push(get_object()->get_cursor_position());

    return mFunc.on_return();
}

/** @function get_history_lines
*/
int lua_edit_box::_get_history_lines(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    const auto& lHistoryLine = get_object()->get_history_lines();
    lua::function mFunc("EditBox:get_history_lines", pLua, lHistoryLine.size());

    for (const auto& sLine : lHistoryLine)
        mFunc.push(sLine);

    return mFunc.on_return();
}

/** @function get_max_letters
*/
int lua_edit_box::_get_max_letters(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:get_max_letters", pLua, 1);

    mFunc.push(get_object()->get_max_letters());

    return mFunc.on_return();
}

/** @function get_num_letters
*/
int lua_edit_box::_get_num_letters(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:get_num_letters", pLua, 1);

    mFunc.push(get_object()->get_num_letters());

    return mFunc.on_return();
}

/** @function get_number
*/
int lua_edit_box::_get_number(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:get_number", pLua, 1);

    mFunc.push(utils::string_to_float(get_object()->get_text()));

    return mFunc.on_return();
}

/** @function get_text
*/
int lua_edit_box::_get_text(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:get_text", pLua, 1);

    mFunc.push(get_object()->get_text());

    return mFunc.on_return();
}

/** @function get_text_insets
*/
int lua_edit_box::_get_text_insets(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:get_text_insets", pLua, 4);

    const quad2i& lInsets = get_object()->get_text_insets();

    mFunc.push(lInsets.left);
    mFunc.push(lInsets.right);
    mFunc.push(lInsets.top);
    mFunc.push(lInsets.bottom);

    return mFunc.on_return();
}

/** @function highlight_text
*/
int lua_edit_box::_highlight_text(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:highlight_text", pLua);
    mFunc.add(0, "start", lua::type::NUMBER, true);
    mFunc.add(1, "end", lua::type::NUMBER, true);
    if (mFunc.check())
    {
        uint uiStart = 0;
        uint uiEnd = uint(-1);

        if (mFunc.is_provided(0))
            uiStart = uint(mFunc.get(0)->get_number());
        if (mFunc.is_provided(1))
            uiEnd = uint(mFunc.get(1)->get_number());

        get_object()->highlight_text(uiStart, uiEnd);
    }

    return mFunc.on_return();
}

/** @function insert
*/
int lua_edit_box::_insert(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:insert", pLua);
    mFunc.add(0, "text", lua::type::STRING);
    if (mFunc.check())
        get_object()->insert_after_cursor(mFunc.get(0)->get_string());

    return mFunc.on_return();
}

/** @function is_multi_line
*/
int lua_edit_box::_is_multi_line(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:is_multi_line", pLua);

    mFunc.push(get_object()->is_multi_line());

    return mFunc.on_return();
}

/** @function is_numeric
*/
int lua_edit_box::_is_numeric(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:is_numeric", pLua);

    mFunc.push(get_object()->is_numeric_only());

    return mFunc.on_return();
}

/** @function is_password
*/
int lua_edit_box::_is_password(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:is_password", pLua);

    mFunc.push(get_object()->is_password_mode_enabled());

    return mFunc.on_return();
}

/** @function set_blink_speed
*/
int lua_edit_box::_set_blink_speed(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:set_blink_speed", pLua);
    mFunc.add(0, "blink speed", lua::type::NUMBER);
    if (mFunc.check())
        get_object()->set_blink_speed(double(mFunc.get(0)->get_number()));

    return mFunc.on_return();
}

/** @function set_cursor_position
*/
int lua_edit_box::_set_cursor_position(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:set_cursor_position", pLua);
    mFunc.add(0, "cursor position", lua::type::NUMBER);
    if (mFunc.check())
        get_object()->set_cursor_position(mFunc.get(0)->get_number());

    return mFunc.on_return();
}

/** @function set_max_history_lines
*/
int lua_edit_box::_set_max_history_lines(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:set_max_history_lines", pLua);
    mFunc.add(0, "max lines", lua::type::NUMBER);
    if (mFunc.check())
        get_object()->set_max_history_lines(mFunc.get(0)->get_number());

    return mFunc.on_return();
}

/** @function set_max_letters
*/
int lua_edit_box::_set_max_letters(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:set_max_letters", pLua);
    mFunc.add(0, "max letters", lua::type::NUMBER);
    if (mFunc.check())
        get_object()->set_max_letters(mFunc.get(0)->get_number());

    return mFunc.on_return();
}

/** @function set_multi_line
*/
int lua_edit_box::_set_multi_line(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:set_multi_line", pLua);
    mFunc.add(0, "multiLine", lua::type::BOOLEAN);
    if (mFunc.check())
        get_object()->set_multi_line(mFunc.get(0)->get_bool());

    return mFunc.on_return();
}

/** @function set_number
*/
int lua_edit_box::_set_number(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:set_number", pLua);
    mFunc.add(0, "number", lua::type::NUMBER);
    if (mFunc.check())
        get_object()->set_text(utils::to_string(mFunc.get(0)->get_number()));

    return mFunc.on_return();
}

/** @function set_numeric
*/
int lua_edit_box::_set_numeric(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:set_numeric", pLua);
    mFunc.add(0, "numeric", lua::type::NUMBER);
    if (mFunc.check())
        get_object()->set_numeric_only(mFunc.get(0)->get_bool());

    return mFunc.on_return();
}

/** @function set_password
*/
int lua_edit_box::_set_password(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:set_password", pLua);
    mFunc.add(0, "enable", lua::type::NUMBER);
    if (mFunc.check())
        get_object()->enable_password_mode(mFunc.get(0)->get_bool());

    return mFunc.on_return();
}

/** @function set_text
*/
int lua_edit_box::_set_text(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:set_text", pLua);
    mFunc.add(0, "text", lua::type::STRING);
    if (mFunc.check())
        get_object()->set_text(mFunc.get(0)->get_string());

    return mFunc.on_return();
}

/** @function set_text_insets
*/
int lua_edit_box::_set_text_insets(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("EditBox:set_text_insets", pLua);
    mFunc.add(0, "left", lua::type::NUMBER);
    mFunc.add(1, "right", lua::type::NUMBER);
    mFunc.add(2, "top", lua::type::NUMBER);
    mFunc.add(3, "bottom", lua::type::NUMBER);
    if (mFunc.check())
    {
        get_object()->set_text_insets(
            int(mFunc.get(0)->get_number()),
            int(mFunc.get(1)->get_number()),
            int(mFunc.get(2)->get_number()),
            int(mFunc.get(3)->get_number())
        );
    }

    return mFunc.on_return();
}
}
}
