#include "lxgui/gui_statusbar.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

/** A @{Frame} representing a variable-length bar.
*   This frame has three main properties: a minimum value, a
*   maximum value, and a current value that must be contained
*   between the minimum and maximum values. The frame will
*   render a textured bar that will either be full, empty, or
*   anything in between depending on the current value.
*
*   This can be used to display health bars, or progress bars.
*
*   __Events.__ Hard-coded events available to all @{StatusBar}s,
*   in addition to those from @{Frame}:
*
*   - `OnValueChanged`: Triggered whenever the value represented by
*   the status bar changes. This is triggered by @{StatusBar:set_value}.
*   This can also be triggered by @{StatusBar:set_min_max_values} if
*   the previous value would not satisfy the new constraints.
*
*   Inherits all methods from: @{UIObject}, @{Frame}.
*
*   Child classes: none.
*   @classmod StatusBar
*/

namespace lxgui {
namespace gui
{
void status_bar::register_on_lua(sol::state& mLua)
{
    mLua.reg<lua_status_bar>();
}

lua_status_bar::lua_status_bar(lua_State* pLua) : lua_frame(pLua)
{
}

/** @function get_min_max_values
*/
int lua_status_bar::_get_min_max_values(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:get_min_max_values", pLua, 2);

    mFunc.push(get_object()->get_min_value());
    mFunc.push(get_object()->get_max_value());

    return mFunc.on_return();
}

/** @function get_orientation
*/
int lua_status_bar::_get_orientation(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:get_orientation", pLua, 1);

    switch (get_object()->get_orientation())
    {
        case status_bar::orientation::HORIZONTAL : mFunc.push(std::string("HORIZONTAL")); break;
        case status_bar::orientation::VERTICAL   : mFunc.push(std::string("VERTICAL")); break;
    }

    return mFunc.on_return();
}

/** @function get_status_bar_color
*/
int lua_status_bar::_get_status_bar_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:get_status_bar_color", pLua, 4);

    const color& mColor = get_object()->get_bar_color();

    mFunc.push(mColor.r);
    mFunc.push(mColor.g);
    mFunc.push(mColor.b);
    mFunc.push(mColor.a);

    return mFunc.on_return();
}

/** @function get_status_bar_texture
*/
int lua_status_bar::_get_status_bar_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:get_status_bar_texture", pLua, 1);

    texture* pTexture = get_object()->get_bar_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }

    return mFunc.on_return();
}

/** @function get_value
*/
int lua_status_bar::_get_value(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:get_value", pLua, 1);

    mFunc.push(get_object()->get_value());

    return mFunc.on_return();
}

/** @function is_reversed
*/
int lua_status_bar::_is_reversed(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:is_reversed", pLua, 1);

    mFunc.push(get_object()->is_reversed());

    return mFunc.on_return();
}

/** @function set_min_max_values
*/
int lua_status_bar::_set_min_max_values(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:set_min_max_values", pLua);
    mFunc.add(0, "min", lua::type::NUMBER);
    mFunc.add(1, "max", lua::type::NUMBER);
    if (mFunc.check())
    {
        get_object()->set_min_max_values(
            mFunc.get(0)->get_number(),
            mFunc.get(1)->get_number()
        );
    }

    return mFunc.on_return();
}

/** @function set_orientation
*/
int lua_status_bar::_set_orientation(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:set_orientation", pLua);
    mFunc.add(0, "orientation", lua::type::STRING);

    if (mFunc.check())
    {
        std::string sOrient = mFunc.get(0)->get_string();
        if (sOrient == "HORIZONTAL")
            get_object()->set_orientation(status_bar::orientation::HORIZONTAL);
        else if (sOrient == "VERTICAL")
            get_object()->set_orientation(status_bar::orientation::VERTICAL);
        else
        {
            gui::out << gui::warning << mFunc.get_name()
                << " : Unkonwn status bar orientation : \""+sOrient+"\"." << std::endl;
        }
    }

    return mFunc.on_return();
}

/** @function set_status_bar_color
*/
int lua_status_bar::_set_status_bar_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:set_status_bar_color", pLua);
    mFunc.add(0, "red", lua::type::NUMBER);
    mFunc.add(1, "green", lua::type::NUMBER);
    mFunc.add(2, "blue", lua::type::NUMBER);
    mFunc.add(3, "alpha", lua::type::NUMBER, true);
    mFunc.new_param_set();
    mFunc.add(0, "color", lua::type::STRING);

    if (mFunc.check())
    {
        color mColor;
        if (mFunc.get_param_set_rank() == 0)
        {
            if (mFunc.is_provided(3))
            {
                mColor = color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number(),
                    mFunc.get(3)->get_number()
                );
            }
            else
            {
                mColor = color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number()
                );
            }
        }
        else
            mColor = color(mFunc.get(0)->get_string());

        get_object()->set_bar_color(mColor);
    }

    return mFunc.on_return();
}

/** @function set_status_bar_texture
*/
int lua_status_bar::_set_status_bar_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:set_status_bar_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state().get<lua_texture>();
        if (pLuaTexture)
        {
            get_object()->set_bar_texture(pLuaTexture->get_object());
        }
    }

    return mFunc.on_return();
}

/** @function set_value
*/
int lua_status_bar::_set_value(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:set_value", pLua);
    mFunc.add(0, "value", lua::type::NUMBER);
    if (mFunc.check())
        get_object()->set_value(mFunc.get(0)->get_number());

    return mFunc.on_return();
}

/** @function set_reversed
*/
int lua_status_bar::_set_reversed(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("StatusBar:set_reversed", pLua);
    mFunc.add(0, "reversed", lua::type::BOOLEAN);
    if (mFunc.check())
        get_object()->set_reversed(mFunc.get(0)->get_bool());

    return mFunc.on_return();
}
}
}
