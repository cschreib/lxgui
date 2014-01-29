#ifndef LUAPP_GLUES_HPP
#define LUAPP_GLUES_HPP

#include "lxgui/luapp_state.hpp"

namespace lua
{
    int glue_send_string(lua_State* pLua);
    int glue_empty_string(lua_State* pLua);
    int glue_table_to_string(lua_State* pLua);
}

#endif
