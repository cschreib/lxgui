#ifndef LXGUI_GUI_UIOBJECT_TPL_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/luapp_state.hpp>

namespace lxgui {
namespace gui
{
    template<typename T>
    void uiobject::create_glue_()
    {
        if (lGlue_) return;

        lua::state& mLua = get_luapp_();
        mLua.push_string(sLuaName_);
        lGlue_ = mLua.push_new<T>();

        mLua.set_global(sLuaName_);
        mLua.pop();
    }
}
}

#endif
