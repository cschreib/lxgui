#include "lxgui/gui_scrollframe.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

namespace lxgui {
namespace gui
{
void scroll_frame::register_glue(lua::state& mLua)
{
    mLua.reg<lua_scroll_frame>();
}

lua_scroll_frame::lua_scroll_frame(lua_State* pLua) : lua_frame(pLua)
{
}

int lua_scroll_frame::_get_horizontal_scroll(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:get_horizontal_scroll", pLua, 1);

    mFunc.push(get_object()->get_horizontal_scroll());

    return mFunc.on_return();
}

int lua_scroll_frame::_get_horizontal_scroll_range(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:get_horizontal_scroll_range", pLua, 1);

    mFunc.push(get_object()->get_horizontal_scroll_range());

    return mFunc.on_return();
}

int lua_scroll_frame::_get_scroll_child(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:get_scroll_child", pLua, 1);

    if (get_object()->get_scroll_child())
    {
        get_object()->get_scroll_child()->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_scroll_frame::_get_vertical_scroll(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:get_vertical_scroll", pLua, 1);

    mFunc.push(get_object()->get_vertical_scroll());

    return mFunc.on_return();
}

int lua_scroll_frame::_get_vertical_scroll_range(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:get_vertical_scroll_range", pLua, 1);

    mFunc.push(get_object()->get_vertical_scroll_range());

    return mFunc.on_return();
}

int lua_scroll_frame::_set_horizontal_scroll(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:set_horizontal_scroll", pLua);
    mFunc.add(0, "horizontal scroll", lua::type::NUMBER);
    if (mFunc.check())
    {
        get_object()->set_horizontal_scroll(int(mFunc.get(0)->get_number()));
    }

    return mFunc.on_return();
}

int lua_scroll_frame::_set_scroll_child(lua_State* pLua)
{
    if (!check_object_())
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
                uiobject* pObj = get_object()->get_manager()->get_uiobject_by_name(pArg->get_string());
                if (!pObj)
                {
                    gui::out << gui::error << mFunc.get_name() << " : "
                        "\""+pObj->get_name()+"\" does not exist." << std::endl;

                    return mFunc.on_return();
                }

                pChild = pObj->down_cast<frame>();
                if (!pChild)
                {
                    gui::out << gui::error << mFunc.get_name() << " : "
                        "\""+pObj->get_name()+"\" is not a frame." << std::endl;

                    return mFunc.on_return();
                }
            }
            else
            {
                lua_frame* pFrame = pArg->get<lua_frame>();
                if (pFrame)
                {
                    pChild = pFrame->get_object();
                }
                else
                {
                    lua_uiobject* pObj = pArg->get<lua_uiobject>();
                    if (pObj)
                    {
                        gui::out << gui::error << mFunc.get_name() << " : "
                            "\""+pObj->get_name()+"\" is not a frame." << std::endl;
                    }
                    else
                    {
                        gui::out << gui::error << mFunc.get_name() << " : "
                            "first argument is not a frame." << std::endl;
                    }

                    return mFunc.on_return();
                }
            }
        }

        std::unique_ptr<frame> pScrollChild = down_cast<frame>(pChild->release_from_parent());
        pScrollChild->set_parent(get_object());
        get_object()->set_scroll_child(std::move(pScrollChild));
    }

    return mFunc.on_return();
}

int lua_scroll_frame::_set_vertical_scroll(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:set_vertical_scroll", pLua);
    mFunc.add(0, "vertical scroll", lua::type::NUMBER);
    if (mFunc.check())
    {
        get_object()->set_vertical_scroll(int(mFunc.get(0)->get_number()));
    }

    return mFunc.on_return();
}
}
}
