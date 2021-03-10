#include "lxgui/gui_focusframe.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

/** A @{Frame} that can receive and loose focus.
*   A typical usage example is the @{EditBox}.
*
*   Inherits all methods from: @{UIObject}, @{Frame}.
*
*   Child classes: @{EditBox}.
*   @classmod FocusFrame
*/

namespace lxgui {
namespace gui
{
lua_focus_frame::lua_focus_frame(lua_State* pLua) : lua_frame(pLua)
{
}

/** @function clear_focus
*/
int lua_focus_frame::_clear_focus(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FocusFrame:clear_focus", pLua);

    get_object()->set_focus(false);

    return mFunc.on_return();
}

/** @function is_auto_focus
*/
int lua_focus_frame::_is_auto_focus(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FocusFrame:is_auto_focus", pLua);

    mFunc.push(get_object()->is_auto_focus_enabled());

    return mFunc.on_return();
}

/** @function set_auto_focus
*/
int lua_focus_frame::_set_auto_focus(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FocusFrame:set_auto_focus", pLua);
    mFunc.add(0, "enabled", lua::type::BOOLEAN);
    if (mFunc.check())
        get_object()->enable_auto_focus(mFunc.get(0)->get_bool());

    return mFunc.on_return();
}

/** @function set_focus
*/
int lua_focus_frame::_set_focus(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FocusFrame:set_focus", pLua);

    get_object()->set_focus(true);

    return mFunc.on_return();
}
}
}
