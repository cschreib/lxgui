#include "lxgui/gui_checkbutton.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_texture.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

/** A @{Button} with two additional states: checked and unchecked.
*   This widget works exactly like a classic @{Button}, but is has two
*   additional special textures for the check sign.
*
*   Inherits all methods from: @{UIObject}, @{Frame}, @{Button}.
*
*   Child classes: none.
*   @classmod CheckButton
*/

namespace lxgui {
namespace gui
{
void check_button::register_glue(lua::state& mLua)
{
    mLua.reg<lua_check_button>();
}

lua_check_button::lua_check_button(lua_State* pLua) : lua_button(pLua)
{
}

/** Checks if this CheckButton is checked.
*   @function is_checked
*   @treturn boolean 'true' if checked, 'false' otherwise
*/
int lua_check_button::_is_checked(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("CheckButton:is_checked", pLua, 1);

    mFunc.push(get_object()->is_checked());

    return mFunc.on_return();
}

/** Returns this button's checked texture.
*   @function get_checked_texture
*   @treturn Texture This button's checked texture
*/
int lua_check_button::_get_checked_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("CheckButton:get_checked_texture", pLua, 1);

    utils::observer_ptr<texture> pTexture = get_object()->get_checked_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** Returns this button's disabled checked texture.
*   @function get_disabled_checked_texture
*   @treturn Texture This button's disabled checked texture
*/
int lua_check_button::_get_disabled_checked_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("CheckButton:get_disabled_checked_texture", pLua, 1);

    utils::observer_ptr<texture> pTexture = get_object()->get_disabled_checked_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** Check or uncheck the button.
*   @function set_checked
*   @tparam boolean checked Value convertible to a boolean; 'true' to check, 'false' to uncheck
*/
/** Checks the button.
*   @function set_checked
*/
int lua_check_button::_set_checked(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    // This function is a bit trickier, as it doesn't require any particular
    // type for its argument. Whatever it is, Lua can convert it to a boolean
    // value, and we use this value to check/uncheck the button.

    if (lua_gettop(pLua) > 0)
    {
        if (lua_toboolean(pLua, 1) == 0)
            get_object()->uncheck();
        else
            get_object()->check();
    }
    else
        get_object()->check();

    return 0;
}

/** Sets this button's checked texture.
*   @function set_checked_texture
*   @tparam Texture tex The new texture
*/
int lua_check_button::_set_checked_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("CheckButton:set_checked_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state().get<lua_texture>();
        if (pLuaTexture)
        {
            get_object()->set_checked_texture(pLuaTexture->get_object());
        }
    }

    return mFunc.on_return();
}

/** Sets this button's disabled checked texture.
*   @function set_disabled_checked_texture
*   @tparam Texture tex The new texture
*/
int lua_check_button::_set_disabled_checked_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("CheckButton:set_disabled_checked_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state().get<lua_texture>();
        if (pLuaTexture)
        {
            get_object()->set_disabled_checked_texture(pLuaTexture->get_object());
        }
    }

    return mFunc.on_return();
}
}
}
