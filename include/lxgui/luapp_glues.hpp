#ifndef LXGUI_LUAPP_GLUES_HPP
#define LXGUI_LUAPP_GLUES_HPP

#include "lxgui/luapp_state.hpp"

namespace lxgui {
namespace lua
{
    int glue_send_string(lua_State* pLua);
    int glue_empty_string(lua_State* pLua);
    int glue_table_to_string(lua_State* pLua);
}
}

#endif
