#include "lxgui/gui_button.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

/** A @{Frame} with a button that can be clicked.
*   This class can handle three different states: "normal", "pushed"
*   and "disabled". You can provide a different texture for each of
*   these states, and two different fontstrings for "normal" and
*   "disabled". Note that a @{Button} has @{Frame:enable_mouse} set
*   to `true` by default.
*
*   In addition, you can provide another texture/fontstring for the
*   "highlight" state (when the mouse is over the button widget).
*
*   Note that there is no fontstring for the "pushed" state: in this
*   case, the "normal" font is rendered with a slight offset that you
*   are free to define.
*
*   __Events.__ Hard-coded events available to all @{Button}s, in
*   addition to those from @{Frame}:
*
*   - `OnClick`: Triggered when the button is clicked, either when
*   @{Button:click} is called, or after the mouse is released after a
*   click over the button.
*   - `OnDoubleClick`: Triggered when the button is double-clicked.
*   - `OnEnable`: Triggered by @{Button:enable}.
*   - `OnDisable`: Triggered by @{Button:disable}.
*
*   Inherits all methods from: @{UIObject}, @{Frame}.
*
*   Child classes: @{CheckButton}.
*   @classmod Button
*/

namespace lxgui {
namespace gui
{
void button::register_glue(lua::state& mLua)
{
    mLua.reg<lua_button>();
}

lua_button::lua_button(lua_State* pLua) : lua_frame(pLua)
{
}

/** @function click
*/
int lua_button::_click(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:click", pLua);

    get_object()->on("Click");

    return mFunc.on_return();
}

/** @function disable
*/
int lua_button::_disable(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:disable", pLua);

    get_object()->disable();

    return mFunc.on_return();
}

/** @function enable
*/
int lua_button::_enable(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:enable", pLua);

    get_object()->enable();

    return mFunc.on_return();
}

/** @function get_button_state
*/
int lua_button::_get_button_state(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_button_state", pLua, 1);

    switch (get_object()->get_button_state())
    {
        case button::state::UP :       mFunc.push(std::string("NORMAL"));   break;
        case button::state::DOWN :     mFunc.push(std::string("PUSHED"));   break;
        case button::state::DISABLED : mFunc.push(std::string("DISABLED")); break;
        default : mFunc.push_nil(); break;
    }

    return mFunc.on_return();
}

/** @function get_disabled_font_object
*/
int lua_button::_get_disabled_font_object(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_disabled_font_object", pLua, 1);

    font_string* pFontString = get_object()->get_disabled_text();
    if (pFontString)
    {
        pFontString->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function get_disabled_text_color
*/
int lua_button::_get_disabled_text_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_disabled_text_color", pLua, 4);

    font_string* pFontString = get_object()->get_disabled_text();
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

/** @function get_disabled_texture
*/
int lua_button::_get_disabled_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_disabled_texture", pLua, 1);

    texture* pTexture = get_object()->get_disabled_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function get_highlight_font_object
*/
int lua_button::_get_highlight_font_object(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_highlight_font_object", pLua, 1);

    font_string* pFontString = get_object()->get_highlight_text();
    if (pFontString)
    {
        pFontString->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function get_highlight_text_color
*/
int lua_button::_get_highlight_text_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_highlight_text_color", pLua, 4);

    font_string* pFontString = get_object()->get_highlight_text();
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

/** @function get_highlight_texture
*/
int lua_button::_get_highlight_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_highlight_texture", pLua, 1);

    texture* pTexture = get_object()->get_highlight_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function get_normal_font_object
*/
int lua_button::_get_normal_font_object(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_normal_font_object", pLua, 1);

    font_string* pFontString = get_object()->get_normal_text();
    if (pFontString)
    {
        pFontString->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function get_normal_texture
*/
int lua_button::_get_normal_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_normal_texture", pLua, 1);

    texture* pTexture = get_object()->get_normal_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function get_pushed_text_offset
*/
int lua_button::_get_pushed_text_offset(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_pushed_text_offset", pLua, 2);

    vector2i lOffset = get_object()->get_pushed_text_offset();

    mFunc.push(lOffset.x);
    mFunc.push(lOffset.y);

    return mFunc.on_return();
}

/** @function get_pushed_texture
*/
int lua_button::_get_pushed_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_pushed_texture", pLua, 1);

    texture* pTexture = get_object()->get_pushed_texture();
    if (pTexture)
    {
        pTexture->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function get_text
*/
int lua_button::_get_text(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_text", pLua, 1);

    mFunc.push(get_object()->get_text());

    return mFunc.on_return();
}

/** @function get_text_height
*/
int lua_button::_get_text_height(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_text_height", pLua, 1);

    font_string* pCurrentFont = get_object()->get_current_font_string();
    if (pCurrentFont)
        mFunc.push(pCurrentFont->get_string_height());
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function get_text_width
*/
int lua_button::_get_text_width(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:get_text_width", pLua, 1);

    font_string* pCurrentFont = get_object()->get_current_font_string();
    if (pCurrentFont)
        mFunc.push(pCurrentFont->get_string_width());
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function is_enabled
*/
int lua_button::_is_enabled(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:is_enabled", pLua, 1);

    mFunc.push(get_object()->is_enabled());

    return mFunc.on_return();
}

/** @function lock_highlight
*/
int lua_button::_lock_highlight(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:lock_highlight", pLua);

    get_object()->lock_highlight();

    return mFunc.on_return();
}

/** @function set_button_state
*/
int lua_button::_set_button_state(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_button_state", pLua);
    mFunc.add(0, "button state", lua::type::STRING);
    if (mFunc.check())
    {
        std::string sState = mFunc.get(0)->get_string();
        if (sState == "NORMAL")
        {
            get_object()->enable();
            get_object()->release();
        }
        else if (sState == "PUSHED")
        {
            get_object()->enable();
            get_object()->push();
        }
        else if (sState == "DISABLED")
        {
            get_object()->disable();
            get_object()->release();
        }
        else
            gui::out << gui::warning << mFunc.get_name() << " : Unknown button state : \""+sState+"\"." << std::endl;
    }

    return mFunc.on_return();
}

/** @function set_disabled_font_object
*/
int lua_button::_set_disabled_font_object(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_disabled_font_object", pLua);
    mFunc.add(0, "font object", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_font_string* pLuaFontString = mFunc.get_state().get<lua_font_string>();
        if (pLuaFontString)
        {
            get_object()->set_disabled_text(pLuaFontString->get_object());
        }
    }

    return mFunc.on_return();
}

/** @function set_disabled_text_color
*/
int lua_button::_set_disabled_text_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_disabled_text_color", pLua);
    mFunc.add(0, "r", lua::type::NUMBER);
    mFunc.add(1, "g", lua::type::NUMBER);
    mFunc.add(2, "b", lua::type::NUMBER);
    mFunc.add(3, "a", lua::type::NUMBER, true);
    if (mFunc.check())
    {
        font_string* pFontString = get_object()->get_disabled_text();
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

/** @function set_disabled_texture
*/
int lua_button::_set_disabled_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_disabled_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state().get<lua_texture>();
        if (pLuaTexture)
        {
            get_object()->set_disabled_texture(pLuaTexture->get_object());
        }
    }

    return mFunc.on_return();
}

/** @function set_highlight_font_object
*/
int lua_button::_set_highlight_font_object(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_highlight_font_object", pLua);
    mFunc.add(0, "font object", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_font_string* pLuaFontString = mFunc.get_state().get<lua_font_string>();
        if (pLuaFontString)
        {
            get_object()->set_highlight_text(pLuaFontString->get_object());
        }
    }

    return mFunc.on_return();
}

/** @function set_highlight_text_color
*/
int lua_button::_set_highlight_text_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_highlight_text_color", pLua);
    mFunc.add(0, "r", lua::type::NUMBER);
    mFunc.add(1, "g", lua::type::NUMBER);
    mFunc.add(2, "b", lua::type::NUMBER);
    mFunc.add(3, "a", lua::type::NUMBER, true);
    if (mFunc.check())
    {
        font_string* pFontString = get_object()->get_highlight_text();
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

/** @function set_highlight_texture
*/
int lua_button::_set_highlight_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_highlight_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state().get<lua_texture>();
        if (pLuaTexture)
        {
            get_object()->set_highlight_texture(pLuaTexture->get_object());
        }
    }

    return mFunc.on_return();
}


/** @function set_normal_font_object
*/
int lua_button::_set_normal_font_object(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_normal_font_object", pLua);
    mFunc.add(0, "font object", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_font_string* pLuaFontString = mFunc.get_state().get<lua_font_string>();
        if (pLuaFontString)
        {
            get_object()->set_normal_text(pLuaFontString->get_object());
        }
    }

    return mFunc.on_return();
}

/** @function set_normal_text_color
*/
int lua_button::_set_normal_text_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_normal_text_color", pLua);
    mFunc.add(0, "r", lua::type::NUMBER);
    mFunc.add(1, "g", lua::type::NUMBER);
    mFunc.add(2, "b", lua::type::NUMBER);
    mFunc.add(3, "a", lua::type::NUMBER, true);
    if (mFunc.check())
    {
        font_string* pFontString = get_object()->get_normal_text();
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

/** @function set_normal_texture
*/
int lua_button::_set_normal_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_normal_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state().get<lua_texture>();
        if (pLuaTexture)
        {
            get_object()->set_normal_texture(pLuaTexture->get_object());
        }
    }

    return mFunc.on_return();
}

/** @function set_pushed_text_offset
*/
int lua_button::_set_pushed_text_offset(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_pushed_text_offset", pLua);
    mFunc.add(0, "x offset", lua::type::NUMBER);
    mFunc.add(1, "y offset", lua::type::NUMBER);
    if (mFunc.check())
    {
        get_object()->set_pushed_text_offset(vector2i(
            (int)mFunc.get(0)->get_number(), (int)mFunc.get(1)->get_number()
        ));
    }

    return mFunc.on_return();
}

/** @function set_pushed_texture
*/
int lua_button::_set_pushed_texture(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_pushed_texture", pLua);
    mFunc.add(0, "texture", lua::type::USERDATA);
    if (mFunc.check())
    {
        lua_texture* pLuaTexture = mFunc.get_state().get<lua_texture>();
        if (pLuaTexture)
        {
            get_object()->set_pushed_texture(pLuaTexture->get_object());
        }
    }

    return mFunc.on_return();
}

/** @function set_text
*/
int lua_button::_set_text(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:set_text", pLua);
    mFunc.add(0, "text", lua::type::STRING);
    if (mFunc.check())
        get_object()->set_text(mFunc.get(0)->get_string());

    return mFunc.on_return();
}

/** @function unlock_highlight
*/
int lua_button::_unlock_highlight(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("Button:unlock_highlight", pLua);

    get_object()->unlock_highlight();

    return mFunc.on_return();
}
}
}
