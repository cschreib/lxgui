#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_texture.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

/** A @{Frame} with a movable texture.
*   This frame contains a special texture, the "slider thumb".
*   It can be moved along a single axis (X or Y) and its position
*   can be used to represent a value (for configuration menus, or
*   scroll bars).
*
*   __Events.__ Hard-coded events available to all @{Slider}s,
*   in addition to those from @{Frame}:
*
*   - `OnValueChanged`: Triggered whenever the value controlled by
*   the slider changes. This is triggered whenever the user moves
*   the slider thumb, and by @{Slider:set_value}. This can also be
*   triggered by @{Slider:set_min_value}, @{Slider:set_max_value},
*   @{Slider:set_min_max_values}, and @{Slider:set_value_step} if the
*   previous value would not satisfy the new constraints.
*
*   Inherits all methods from: @{UIObject}, @{Frame}.
*
*   Child classes: none.
*   @classmod Slider
*/

namespace lxgui {
namespace gui
{
void slider::register_on_lua(sol::state& mLua)
{
    mLua.reg<lua_slider>();
}

lua_slider::lua_slider(lua_State* pLua) : lua_frame(pLua)
{
}

/** @function allow_clicks_outside_thumb
*/
int lua_slider::_allow_clicks_outside_thumb(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:allow_clicks_outside_thumb", pLua);
    mFunc.add(0, "allow", lua::type::BOOLEAN);
    if (mFunc.check())
        get_object()->set_allow_clicks_outside_thumb(mFunc.get(0)->get_bool());

    return mFunc.on_return();
}

/** @function get_max_value
*/
int lua_slider::_get_max_value(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:get_max_value", pLua, 1);

    mFunc.push(get_object()->get_max_value());

    return mFunc.on_return();
}

/** @function get_min_value
*/
int lua_slider::_get_min_value(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:get_min_value", pLua, 1);

    mFunc.push(get_object()->get_min_value());

    return mFunc.on_return();
}

/** @function get_min_max_values
*/
int lua_slider::_get_min_max_values(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:get_min_max_values", pLua, 2);

    mFunc.push(get_object()->get_min_value());
    mFunc.push(get_object()->get_max_value());

    return mFunc.on_return();
}

/** @function get_orientation
*/
int lua_slider::_get_orientation(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:get_orientation", pLua, 1);

    switch (get_object()->get_orientation())
    {
        case slider::orientation::VERTICAL   : mFunc.push(std::string("VERTICAL"));   break;
        case slider::orientation::HORIZONTAL : mFunc.push(std::string("HORIZONTAL")); break;
    }

    return mFunc.on_return();
}

/** @function get_thumb_texture
*/
int lua_slider::_get_thumb_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:get_thumb_texture", pLua, 1);

    texture* pTexture = get_object()->get_thumb_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }

    return mFunc.on_return();
}

/** @function get_value
*/
int lua_slider::_get_value(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:get_value", pLua, 1);

    mFunc.push(get_object()->get_value());

    return mFunc.on_return();
}

/** @function get_value_step
*/
int lua_slider::_get_value_step(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:get_value_step", pLua, 1);

    mFunc.push(get_object()->get_value_step());

    return mFunc.on_return();
}

/** @function set_max_value
*/
int lua_slider::_set_max_value(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:set_max_value", pLua);
    mFunc.add(0, "max", lua::type::NUMBER);
    if (mFunc.check())
    {
        get_object()->set_max_value(mFunc.get(0)->get_number());
    }

    return mFunc.on_return();
}

/** @function set_min_value
*/
int lua_slider::_set_min_value(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:set_min_value", pLua);
    mFunc.add(0, "min", lua::type::NUMBER);
    if (mFunc.check())
    {
        get_object()->set_min_value(mFunc.get(0)->get_number());
    }

    return mFunc.on_return();
}

/** @function set_min_max_values
*/
int lua_slider::_set_min_max_values(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:set_min_max_values", pLua);
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
int lua_slider::_set_orientation(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:set_orientation", pLua);
    mFunc.add(0, "value", lua::type::NUMBER);
    if (mFunc.check())
    {
        get_object()->set_value(mFunc.get(0)->get_number());
    }

    return mFunc.on_return();
}

/** @function set_thumb_texture
*/
int lua_slider::_set_thumb_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:set_thumb_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state().get<lua_texture>();
        if (pLuaTexture)
        {
            get_object()->set_thumb_texture(pLuaTexture->get_object());
        }
    }

    return mFunc.on_return();
}

/** @function set_value_step
*/
int lua_slider::_set_value_step(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:set_value_step", pLua);
    mFunc.add(0, "value step", lua::type::NUMBER);
    if (mFunc.check())
        get_object()->set_value_step(mFunc.get(0)->get_number());

    return mFunc.on_return();
}

/** @function set_value
*/
int lua_slider::_set_value(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Slider:set_value", pLua);
    mFunc.add(0, "value", lua::type::NUMBER);
    mFunc.add(1, "silent", lua::type::BOOLEAN, true);
    if (mFunc.check())
    {
        if (mFunc.is_provided(1))
            get_object()->set_value(mFunc.get(0)->get_number(), mFunc.get(1)->get_bool());
        else
            get_object()->set_value(mFunc.get(0)->get_number());
    }

    return mFunc.on_return();
}
}
}
