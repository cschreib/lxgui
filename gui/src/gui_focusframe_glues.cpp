#include "lxgui/gui_focusframe.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

namespace lxgui {
namespace gui
{
lua_focus_frame::lua_focus_frame(lua_State* pLua) : lua_frame(pLua)
{
    if (pParent_)
    {
        pFocusFrameParent_ = pParent_->down_cast<focus_frame>();
        if (!pFocusFrameParent_)
            throw exception("lua_focus_frame", "Dynamic cast failed !");
    }
}

focus_frame* lua_focus_frame::get_parent()
{
    return pFocusFrameParent_;
}

int lua_focus_frame::_clear_focus(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("FocusFrame:clear_focus", pLua);

    pFocusFrameParent_->set_focus(false);

    return mFunc.on_return();
}

int lua_focus_frame::_is_auto_focus(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("FocusFrame:is_auto_focus", pLua);

    mFunc.push(pFocusFrameParent_->is_auto_focus_enabled());

    return mFunc.on_return();
}

int lua_focus_frame::_set_auto_focus(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("FocusFrame:set_auto_focus", pLua);
    mFunc.add(0, "enabled", lua::type::BOOLEAN);
    if (mFunc.check())
        pFocusFrameParent_->enable_auto_focus(mFunc.get(0)->get_bool());

    return mFunc.on_return();
}

int lua_focus_frame::_set_focus(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("FocusFrame:set_focus", pLua);

    pFocusFrameParent_->set_focus(true);

    return mFunc.on_return();
}
}
}
