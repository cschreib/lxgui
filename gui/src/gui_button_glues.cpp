#include "lxgui/gui_button.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/luapp_function.hpp>

namespace lxgui {
namespace gui
{
void button::register_glue(lua::state* pLua)
{
    pLua->reg<lua_button>();
}

lua_button::lua_button(lua_State* pLua) : lua_frame(pLua)
{
    if (pParent_)
    {
        pButtonParent_ = pParent_->down_cast<button>();
        if (!pButtonParent_)
            throw exception("lua_button", "Dynamic cast failed !");
    }
}

int lua_button::_click(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:click", pLua);

    pButtonParent_->on("Click");

    return mFunc.on_return();
}

int lua_button::_disable(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:disable", pLua);

    pButtonParent_->disable();

    return mFunc.on_return();
}

int lua_button::_enable(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:enable", pLua);

    pButtonParent_->enable();

    return mFunc.on_return();
}

int lua_button::_get_button_state(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_button_state", pLua, 1);

    switch (pButtonParent_->get_button_state())
    {
        case button::state::UP :       mFunc.push(std::string("NORMAL"));   break;
        case button::state::DOWN :     mFunc.push(std::string("PUSHED"));   break;
        case button::state::DISABLED : mFunc.push(std::string("DISABLED")); break;
        default : mFunc.push_nil(); break;
    }

    return mFunc.on_return();
}

int lua_button::_get_disabled_font_object(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_disabled_font_object", pLua, 1);

    font_string* pFontString = pButtonParent_->get_disabled_text();
    if (pFontString)
    {
        pFontString->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_button::_get_disabled_text_color(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_disabled_text_color", pLua, 4);

    font_string* pFontString = pButtonParent_->get_disabled_text();
    if (pFontString)
    {
        const color& mColor = pFontString->get_text_color();
        mFunc.push(mColor.r);
        mFunc.push(mColor.g);
        mFunc.push(mColor.b);
        mFunc.push(mColor.a);
    }
    else
        mFunc.push_nil(4);

    return mFunc.on_return();
}

int lua_button::_get_disabled_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_disabled_texture", pLua, 1);

    texture* pTexture = pButtonParent_->get_disabled_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_button::_get_highlight_font_object(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_highlight_font_object", pLua, 1);

    font_string* pFontString = pButtonParent_->get_highlight_text();
    if (pFontString)
    {
        pFontString->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_button::_get_highlight_text_color(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_highlight_text_color", pLua, 4);

    font_string* pFontString = pButtonParent_->get_highlight_text();
    if (pFontString)
    {
        const color& mColor = pFontString->get_text_color();
        mFunc.push(mColor.r);
        mFunc.push(mColor.g);
        mFunc.push(mColor.b);
        mFunc.push(mColor.a);
    }
    else
        mFunc.push_nil(4);

    return mFunc.on_return();
}

int lua_button::_get_highlight_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_highlight_texture", pLua, 1);

    texture* pTexture = pButtonParent_->get_highlight_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_button::_get_normal_font_object(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_normal_font_object", pLua, 1);

    font_string* pFontString = pButtonParent_->get_normal_text();
    if (pFontString)
    {
        pFontString->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_button::_get_normal_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_normal_texture", pLua, 1);

    texture* pTexture = pButtonParent_->get_normal_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_button::_get_pushed_text_offset(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_pushed_text_offset", pLua, 2);

    vector2i lOffset = pButtonParent_->get_pushed_text_offset();

    mFunc.push(lOffset.x);
    mFunc.push(lOffset.y);

    return mFunc.on_return();
}

int lua_button::_get_pushed_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_pushed_texture", pLua, 1);

    texture* pTexture = pButtonParent_->get_pushed_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_button::_get_text(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_text", pLua, 1);

    mFunc.push(pButtonParent_->get_text());

    return mFunc.on_return();
}

int lua_button::_get_text_height(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_text_height", pLua, 1);

    font_string* pCurrentFont = pButtonParent_->get_current_font_string();
    if (pCurrentFont)
        mFunc.push(pCurrentFont->get_string_height());
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_button::_get_text_width(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:get_text_width", pLua, 1);

    font_string* pCurrentFont = pButtonParent_->get_current_font_string();
    if (pCurrentFont)
        mFunc.push(pCurrentFont->get_string_width());
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_button::_is_enabled(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:is_enabled", pLua, 1);

    mFunc.push(pButtonParent_->is_enabled());

    return mFunc.on_return();
}

int lua_button::_lock_highlight(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:lock_highlight", pLua);

    pButtonParent_->lock_highlight();

    return mFunc.on_return();
}

int lua_button::_set_button_state(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_button_state", pLua);
    mFunc.add(0, "button state", lua::type::STRING);
    if (mFunc.check())
    {
        std::string sState = mFunc.get(0)->get_string();
        if (sState == "NORMAL")
        {
            pButtonParent_->enable();
            pButtonParent_->release();
        }
        else if (sState == "PUSHED")
        {
            pButtonParent_->enable();
            pButtonParent_->push();
        }
        else if (sState == "DISABLED")
        {
            pButtonParent_->disable();
            pButtonParent_->release();
        }
        else
            gui::out << gui::warning << mFunc.get_name() << " : Unknown button state : \""+sState+"\"." << std::endl;
    }

    return mFunc.on_return();
}

int lua_button::_set_disabled_font_object(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_disabled_font_object", pLua);
    mFunc.add(0, "font object", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_font_string* plua_font_string = mFunc.get_state()->get<lua_font_string>();
        if (plua_font_string)
        {
            font_string* pFontString = plua_font_string->get_parent()->down_cast<font_string>();
            pButtonParent_->set_disabled_text(pFontString);
        }
    }

    return mFunc.on_return();
}

int lua_button::_set_disabled_text_color(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_disabled_text_color", pLua);
    mFunc.add(0, "r", lua::type::NUMBER);
    mFunc.add(1, "g", lua::type::NUMBER);
    mFunc.add(2, "b", lua::type::NUMBER);
    mFunc.add(3, "a", lua::type::NUMBER, true);
    if (mFunc.check())
    {
        font_string* pFontString = pButtonParent_->get_disabled_text();
        if (pFontString)
        {
            if (mFunc.is_provided(3))
            {
                pFontString->set_text_color(color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number(),
                    mFunc.get(3)->get_number()
                ));
            }
            else
            {
                pFontString->set_text_color(color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number()
                ));
            }
        }
    }

    return mFunc.on_return();
}

int lua_button::_set_disabled_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_disabled_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state()->get<lua_texture>();
        if (pLuaTexture)
        {
            texture* pTexture = pLuaTexture->get_parent()->down_cast<texture>();
            pButtonParent_->set_disabled_texture(pTexture);
        }
    }

    return mFunc.on_return();
}

int lua_button::_set_highlight_font_object(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_highlight_font_object", pLua);
    mFunc.add(0, "font object", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_font_string* plua_font_string = mFunc.get_state()->get<lua_font_string>();
        if (plua_font_string)
        {
            font_string* pFontString = plua_font_string->get_parent()->down_cast<font_string>();
            pButtonParent_->set_highlight_text(pFontString);
        }
    }

    return mFunc.on_return();
}

int lua_button::_set_highlight_text_color(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_highlight_text_color", pLua);
    mFunc.add(0, "r", lua::type::NUMBER);
    mFunc.add(1, "g", lua::type::NUMBER);
    mFunc.add(2, "b", lua::type::NUMBER);
    mFunc.add(3, "a", lua::type::NUMBER, true);
    if (mFunc.check())
    {
        font_string* pFontString = pButtonParent_->get_highlight_text();
        if (pFontString)
        {
            if (mFunc.is_provided(3))
            {
                pFontString->set_text_color(color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number(),
                    mFunc.get(3)->get_number()
                ));
            }
            else
            {
                pFontString->set_text_color(color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number()
                ));
            }
        }
    }

    return mFunc.on_return();
}

int lua_button::_set_highlight_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_highlight_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state()->get<lua_texture>();
        if (pLuaTexture)
        {
            texture* pTexture = pLuaTexture->get_parent()->down_cast<texture>();
            pButtonParent_->set_highlight_texture(pTexture);
        }
    }

    return mFunc.on_return();
}


int lua_button::_set_normal_font_object(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_normal_font_object", pLua);
    mFunc.add(0, "font object", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_font_string* plua_font_string = mFunc.get_state()->get<lua_font_string>();
        if (plua_font_string)
        {
            font_string* pFontString = plua_font_string->get_parent()->down_cast<font_string>();
            pButtonParent_->set_normal_text(pFontString);
        }
    }

    return mFunc.on_return();
}

int lua_button::_set_normal_text_color(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_normal_text_color", pLua);
    mFunc.add(0, "r", lua::type::NUMBER);
    mFunc.add(1, "g", lua::type::NUMBER);
    mFunc.add(2, "b", lua::type::NUMBER);
    mFunc.add(3, "a", lua::type::NUMBER, true);
    if (mFunc.check())
    {
        font_string* pFontString = pButtonParent_->get_normal_text();
        if (pFontString)
        {
            if (mFunc.is_provided(3))
            {
                pFontString->set_text_color(color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number(),
                    mFunc.get(3)->get_number()
                ));
            }
            else
            {
                pFontString->set_text_color(color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number()
                ));
            }
        }
    }

    return mFunc.on_return();
}

int lua_button::_set_normal_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_normal_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state()->get<lua_texture>();
        if (pLuaTexture)
        {
            texture* pTexture = pLuaTexture->get_parent()->down_cast<texture>();
            pButtonParent_->set_normal_texture(pTexture);
        }
    }

    return mFunc.on_return();
}

int lua_button::_set_pushed_text_offset(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_pushed_text_offset", pLua);
    mFunc.add(0, "x offset", lua::type::NUMBER);
    mFunc.add(1, "y offset", lua::type::NUMBER);
    if (mFunc.check())
    {
        pButtonParent_->set_pushed_text_offset(vector2i(
            (int)mFunc.get(0)->get_number(), (int)mFunc.get(1)->get_number()
        ));
    }

    return mFunc.on_return();
}

int lua_button::_set_pushed_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_pushed_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state()->get<lua_texture>();
        if (pLuaTexture)
        {
            texture* pTexture = pLuaTexture->get_parent()->down_cast<texture>();
            pButtonParent_->set_pushed_texture(pTexture);
        }
    }

    return mFunc.on_return();
}

int lua_button::_set_text(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:set_text", pLua);
    mFunc.add(0, "text", lua::type::STRING);
    if (mFunc.check())
        pButtonParent_->set_text(mFunc.get(0)->get_string());

    return mFunc.on_return();
}

int lua_button::_unlock_highlight(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Button:unlock_highlight", pLua);

    pButtonParent_->unlock_highlight();

    return mFunc.on_return();
}
}
}
