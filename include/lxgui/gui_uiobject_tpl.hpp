#ifndef LXGUI_GUI_UIOBJECT_TPL_HPP

#include <lxgui/luapp_state.hpp>

namespace lxgui {
namespace gui
{
    template<typename T>
    void uiobject::create_glue_()
    {
        if (lGlue_) return;

        lua::state* pLua = get_lua_();

        if (bVirtual_)
        {
            pLua->push_number(uiID_);
            lGlue_ = pLua->push_new<lua_virtual_glue>();
        }
        else
        {
            pLua->push_string(sLuaName_);
            lGlue_ = pLua->push_new<T>();
        }

        pLua->set_global(sLuaName_);
        pLua->pop();
    }
}
}

#endif
