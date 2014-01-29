/* ###################################### */
/* ###     Frost Engine, by Kalith    ### */
/* ###################################### */
/*               Lua source               */
/*                                        */

#include "lxgui/luapp_glues.hpp"
#include "lxgui/luapp_function.hpp"
#include "lxgui/luapp_state.hpp"

/** \cond NOT_REMOVE_FROM_DOC
*/

namespace lua
{
int glue_send_string( lua_State* pLua )
{
    state::get_state(pLua)->sComString += lua_tostring(pLua, 1);

    return 0;
}

int glue_empty_string( lua_State* pLua )
{
    state::get_state(pLua)->sComString = "";

    return 0;
}

int glue_table_to_string( lua_State* pLua )
{
    std::string t = lua_tostring(pLua, 1);
    std::string s = state::get_state(pLua)->table_to_string(lua_tostring(pLua, 2));
    s = t+s;
    lua_pushstring(pLua, s.c_str());

    return 1;
}
}

/** \endcond
*/
