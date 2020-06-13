#include "lxgui/gui_scrollframe.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/luapp_function.hpp>

namespace lxgui {
namespace gui
{
void scroll_frame::register_glue(lua::state* pLua)
{
    pLua->reg<lua_scroll_frame>();
}

lua_scroll_frame::lua_scroll_frame(lua_State* pLua) : lua_frame(pLua)
{
    pScrollFrameParent_ = dynamic_cast<scroll_frame*>(pParent_);
    if (pParent_ && !pScrollFrameParent_)
        throw exception("lua_scroll_frame", "Dynamic cast failed !");
}

int lua_scroll_frame::_get_horizontal_scroll(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("ScrollFrame:get_horizontal_scroll", pLua, 1);

    mFunc.push(pScrollFrameParent_->get_horizontal_scroll());

    return mFunc.on_return();
}

int lua_scroll_frame::_get_horizontal_scroll_range(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("ScrollFrame:get_horizontal_scroll_range", pLua, 1);

    mFunc.push(pScrollFrameParent_->get_horizontal_scroll_range());

    return mFunc.on_return();
}

int lua_scroll_frame::_get_scroll_child(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("ScrollFrame:get_scroll_child", pLua, 1);

    if (pScrollFrameParent_->get_scroll_child())
    {
        pScrollFrameParent_->get_scroll_child()->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_scroll_frame::_get_vertical_scroll(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("ScrollFrame:get_vertical_scroll", pLua, 1);

    mFunc.push(pScrollFrameParent_->get_vertical_scroll());

    return mFunc.on_return();
}

int lua_scroll_frame::_get_vertical_scroll_range(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("ScrollFrame:get_vertical_scroll_range", pLua, 1);

    mFunc.push(pScrollFrameParent_->get_vertical_scroll_range());

    return mFunc.on_return();
}

int lua_scroll_frame::_set_horizontal_scroll(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("ScrollFrame:set_horizontal_scroll", pLua);
    mFunc.add(0, "horizontal scroll", lua::type::NUMBER);
    if (mFunc.check())
    {
        pScrollFrameParent_->set_horizontal_scroll(int(mFunc.get(0)->get_number()));
    }

    return mFunc.on_return();
}

int lua_scroll_frame::_set_scroll_child(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("ScrollFrame:set_scroll_child", pLua);
    mFunc.add(0, "child name", lua::type::STRING, true);
    mFunc.add(0, "child", lua::type::USERDATA, true);
    if (mFunc.check())
    {
        lua::argument* pArg = mFunc.get(0);
        frame* pChild = nullptr;

        if (pArg->is_provided())
        {
            if (pArg->get_type() == lua::type::STRING)
            {
                uiobject* pObj = pParent_->get_manager()->get_uiobject_by_name(pArg->get_string());
                pChild = dynamic_cast<frame*>(pObj);

                if (!pChild && pObj)
                {
                    gui::out << gui::warning << mFunc.get_name() << " : "
                        "\""+pObj->get_name()+"\" is not a frame uiobject." << std::endl;
                }
            }
            else
            {
                lua_uiobject* pObj = pArg->get<lua_uiobject>();
                lua_frame* pFrame = dynamic_cast<lua_frame*>(pObj);

                if (pFrame)
                    pChild = dynamic_cast<frame*>(pFrame->get_parent());
                else if (pObj)
                {
                    gui::out << gui::warning << mFunc.get_name() << " : "
                        "\""+pObj->get_parent()->get_name()+"\" is not a frame uiobject." << std::endl;
                }
            }
        }

        std::unique_ptr<frame> pScrollChild(dynamic_cast<frame*>(pChild->release_from_parent().release()));
        pScrollChild->set_parent(pScrollFrameParent_);
        pScrollFrameParent_->set_scroll_child(std::move(pScrollChild));
    }

    return mFunc.on_return();
}

int lua_scroll_frame::_set_vertical_scroll(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("ScrollFrame:set_vertical_scroll", pLua);
    mFunc.add(0, "vertical scroll", lua::type::NUMBER);
    if (mFunc.check())
    {
        pScrollFrameParent_->set_vertical_scroll(int(mFunc.get(0)->get_number()));
    }

    return mFunc.on_return();
}
}
}
