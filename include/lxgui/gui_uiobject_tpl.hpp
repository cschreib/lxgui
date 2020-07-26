#ifndef LXGUI_GUI_UIOBJECT_TPL_HPP

#include <lxgui/luapp_state.hpp>

namespace lxgui {
namespace gui
{
    template<typename T>
    void uiobject::create_glue_()
    {
        if (lGlue_) return;

        lua::state& mLua = get_luapp_();

        if (bVirtual_)
        {
            mLua.push_number(uiID_);
            lGlue_ = mLua.push_new<lua_virtual_glue>();
        }
        else
        {
            mLua.push_string(sLuaName_);
            lGlue_ = mLua.push_new<T>();
        }

        mLua.set_global(sLuaName_);
        mLua.pop();
    }
}
}

#endif
