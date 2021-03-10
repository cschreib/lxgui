#include "lxgui/gui_scrollframe.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

/** A @{Frame} with scrollable content.
*   This frame has a special child frame, the "scroll child".
*   The scroll child is rendered on a separate render target,
*   which is then rendered on the screen. This allows clipping
*   the content of the scroll child and only display a portion
*   of it (as if scrolling on a page). The displayed portion is
*   controlled by the scroll value, which can be changed in both
*   the vertical and horizontal directions.
*
*   __Events.__ Hard-coded events available to all @{ScrollFrame}s,
*   in addition to those from @{Frame}:
*
*   - `OnHorizontalScroll`: TODO.
*   - `OnScrollRangeChanged`: TODO.
*   - `OnVerticalScroll`: TODO.
*
*   Inherits all methods from: @{UIObject}, @{Frame}.
*
*   Child classes: none.
*   @classmod ScrollFrame
*/

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

/** @function get_horizontal_scroll
*/
int lua_scroll_frame::_get_horizontal_scroll(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:get_horizontal_scroll", pLua, 1);

    mFunc.push(get_object()->get_horizontal_scroll());

    return mFunc.on_return();
}

/** @function get_horizontal_scroll_range
*/
int lua_scroll_frame::_get_horizontal_scroll_range(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:get_horizontal_scroll_range", pLua, 1);

    mFunc.push(get_object()->get_horizontal_scroll_range());

    return mFunc.on_return();
}

/** @function get_scroll_child
*/
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

/** @function get_vertical_scroll
*/
int lua_scroll_frame::_get_vertical_scroll(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:get_vertical_scroll", pLua, 1);

    mFunc.push(get_object()->get_vertical_scroll());

    return mFunc.on_return();
}

/** @function get_vertical_scroll_range
*/
int lua_scroll_frame::_get_vertical_scroll_range(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("ScrollFrame:get_vertical_scroll_range", pLua, 1);

    mFunc.push(get_object()->get_vertical_scroll_range());

    return mFunc.on_return();
}

/** @function set_horizontal_scroll
*/
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

/** @function set_scroll_child
*/
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

/** @function set_vertical_scroll
*/
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
