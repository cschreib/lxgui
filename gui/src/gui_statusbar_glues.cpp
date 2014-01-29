#include "lxgui/gui_statusbar.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/luapp_function.hpp>

namespace gui
{
void status_bar::register_glue(utils::wptr<lua::state> pLua)
{
    pLua->reg<lua_status_bar>();
}

lua_status_bar::lua_status_bar(lua_State* pLua) : lua_frame(pLua)
{
    pStatusBarParent_ = dynamic_cast<status_bar*>(pParent_);
    if (pParent_ && !pStatusBarParent_)
        throw exception("lua_status_bar", "Dynamic cast failed !");
}

int lua_status_bar::_get_min_max_values(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:get_min_max_values", pLua, 2);

    mFunc.push(pStatusBarParent_->get_min_value());
    mFunc.push(pStatusBarParent_->get_max_value());

    return mFunc.on_return();
}

int lua_status_bar::_get_orientation(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:get_orientation", pLua, 1);

    switch (pStatusBarParent_->get_orientation())
    {
        case status_bar::ORIENT_HORIZONTAL : mFunc.push(std::string("HORIZONTAL")); break;
        case status_bar::ORIENT_VERTICAL   : mFunc.push(std::string("VERTICAL")); break;
    }

    return mFunc.on_return();
}

int lua_status_bar::_get_status_bar_color(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:get_status_bar_color", pLua, 4);

    const color& mColor = pStatusBarParent_->get_bar_color();

    mFunc.push(mColor.r);
    mFunc.push(mColor.g);
    mFunc.push(mColor.b);
    mFunc.push(mColor.a);

    return mFunc.on_return();
}

int lua_status_bar::_get_status_bar_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:get_status_bar_texture", pLua, 1);

    texture* pTexture = pStatusBarParent_->get_bar_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }

    return mFunc.on_return();
}

int lua_status_bar::_get_value(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:get_value", pLua, 1);

    mFunc.push(pStatusBarParent_->get_value());

    return mFunc.on_return();
}

int lua_status_bar::_is_reversed(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:is_reversed", pLua, 1);

    mFunc.push(pStatusBarParent_->is_reversed());

    return mFunc.on_return();
}

int lua_status_bar::_set_min_max_values(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:set_min_max_values", pLua);
    mFunc.add(0, "min", lua::TYPE_NUMBER);
    mFunc.add(1, "max", lua::TYPE_NUMBER);
    if (mFunc.check())
    {
        pStatusBarParent_->set_min_max_values(
            mFunc.get(0)->get_number(),
            mFunc.get(1)->get_number()
        );
    }

    return mFunc.on_return();
}

int lua_status_bar::_set_orientation(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:set_orientation", pLua);
    mFunc.add(0, "orientation", lua::TYPE_STRING);

    if (mFunc.check())
    {
        std::string sOrient = mFunc.get(0)->get_string();
        if (sOrient == "HORIZONTAL")
            pStatusBarParent_->set_orientation(status_bar::ORIENT_HORIZONTAL);
        else if (sOrient == "VERTICAL")
            pStatusBarParent_->set_orientation(status_bar::ORIENT_VERTICAL);
        else
        {
            gui::out << gui::warning << mFunc.get_name()
                << " : Unkonwn status bar orientation : \""+sOrient+"\"." << std::endl;
        }
    }

    return mFunc.on_return();
}

int lua_status_bar::_set_status_bar_color(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:set_status_bar_color", pLua);
    mFunc.add(0, "red", lua::TYPE_NUMBER);
    mFunc.add(1, "green", lua::TYPE_NUMBER);
    mFunc.add(2, "blue", lua::TYPE_NUMBER);
    mFunc.add(3, "alpha", lua::TYPE_NUMBER, true);
    mFunc.new_param_set();
    mFunc.add(0, "color", lua::TYPE_STRING);

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

        pStatusBarParent_->set_bar_color(mColor);
    }

    return mFunc.on_return();
}

int lua_status_bar::_set_status_bar_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:set_status_bar_texture", pLua);
    mFunc.add(0, "texture", lua::TYPE_USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state()->get<lua_texture>();
        if (pLuaTexture)
        {
            texture* pTexture = dynamic_cast<texture*>(pLuaTexture->get_parent());
            pStatusBarParent_->set_bar_texture(pTexture);
        }
    }

    return mFunc.on_return();
}

int lua_status_bar::_set_value(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:set_value", pLua);
    mFunc.add(0, "value", lua::TYPE_NUMBER);
    if (mFunc.check())
        pStatusBarParent_->set_value(mFunc.get(0)->get_number());

    return mFunc.on_return();
}

int lua_status_bar::_set_reversed(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("StatusBar:set_reversed", pLua);
    mFunc.add(0, "reversed", lua::TYPE_BOOLEAN);
    if (mFunc.check())
        pStatusBarParent_->set_reversed(mFunc.get(0)->get_bool());

    return mFunc.on_return();
}
}
