#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>
#include <lxgui/utils_string.hpp>

/** A @{LayeredRegion} that can draw text on the screen.
*   This class holds a string and a reference to a font, which
*   is used to draw the string on the screen. The appearance of
*   the string can be changed (font, size, color, alignment, wrapping).
*   In addition, it is possible to change the color of a portion of
*   the string, for example to highlight a particular name.
*
*   __Sizing.__ The @{FontString} class has a special property when it
*   comes to determining the size of its region on the screen, hence
*   how other object anchor to it, and how it anchors to other objects.
*   See the documentation for @{UIObject} for more information on anchors.
*   While other @{UIObject}s must either have a fixed size or more than two
*   anchors constraining their size, the @{FontString} does not. If only
*   one anchor is specified, the width and height of the @{FontString} will
*   be determined by the area occupied by the displayed text, however long and
*   tall this may be. If the width is already constrained by the fixed size
*   or anchors, then the text will word wrap (if allowed) and the
*   @{FontString}'s height will be as tall as the height of the wrapped text.
*   Finally, if both the width and height are constrained by fixed sizes or
*   anchors, the text will simply word wrap (if allowed) and be cut to fit
*   in the specified area.
*
*   __Formatting.__ If @{FontString:is_formatting_enabled} is true (default),
*   the string displayed by the @{FontString} class can have special
*   formatting. This is done by entering the pipe `|` character, followed by
*   a command:
*
*   - `|caarrggbb` sets the color of the text. `aa`, `rr`, `gg`, and `bb`
*   must be two-digit hexadecimal values representing each color
*   component. For example, an opaque red color would be `|cffff0000`,
*   while a semi-transparent blue would be `|c880000ff`.
*
*   - `|r` cancels the change of color from the previous `|c[...]`
*   command. Color change commands stack up, and can be nested. In the
*   formatted string
*   `|cffff0000 hello |cff00ff00 world |r example |r formatting`,
*   the word `hello` would be displayed in red, `world` in blue,
*   `example` in red again, and `formatting` would be of whatever color
*   is the default for the @{FontString} object (see
*   @{FontString:get_text_color}).
*
*   - `|Tpath/to/texture.png:w:h|t` displays a texture. The command
*   parameters must be contained within an opening `|T` and a closing
*   `|t`. The parameters are separated by colons `:`. The first parameter
*   must be the path to the texture file, and it is mandatory.
*   The other two parameters are optional, and define the size of the
*   displayed texture. If omitted, the texture will be displayed as a
*   square matching the size of the current font. If only `w` is supplied,
*   the texture will be displayed as a square of size `w` pixels.
*   If both `w` and `h` are supplied, they set the width and height of the
*   displayed texture, in pixels.
*
*   - `||` displays a single `|` character.
*
*   Inherits all methods from: @{UIObject}, @{LayeredRegion}.
*
*   Child classes: none.
*   @classmod FontString
*/

namespace lxgui {
namespace gui
{
void font_string::register_glue(lua::state& mLua)
{
    mLua.reg<lua_font_string>();
}

lua_font_string::lua_font_string(lua_State* pLua) : lua_layered_region(pLua)
{
}

/** @function get_font
*/
int lua_font_string::_get_font(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:get_font", pLua, 1);

    mFunc.push(get_object()->get_font_name());

    return mFunc.on_return();
}

/** @function get_justify_h
*/
int lua_font_string::_get_justify_h(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:get_justify_h", pLua, 1);

    text::alignment mAlignment = get_object()->get_justify_h();
    std::string sAligment;

    switch (mAlignment)
    {
        case text::alignment::LEFT :   sAligment = "LEFT"; break;
        case text::alignment::CENTER : sAligment = "CENTER"; break;
        case text::alignment::RIGHT :  sAligment = "RIGHT"; break;
    }

    mFunc.push(sAligment);

    return mFunc.on_return();
}

/** @function get_justify_v
*/
int lua_font_string::_get_justify_v(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:get_justify_v", pLua, 1);

    text::vertical_alignment mAlignment = get_object()->get_justify_v();
    std::string sAligment;

    switch (mAlignment)
    {
        case text::vertical_alignment::TOP : sAligment = "TOP"; break;
        case text::vertical_alignment::MIDDLE : sAligment = "MIDDLE"; break;
        case text::vertical_alignment::BOTTOM : sAligment = "BOTTOM"; break;
    }

    mFunc.push(sAligment);

    return mFunc.on_return();
}

/** @function get_shadow_color
*/
int lua_font_string::_get_shadow_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:get_shadow_color", pLua, 4);

    const color& mShadowColor = get_object()->get_shadow_color();

    mFunc.push(mShadowColor.r);
    mFunc.push(mShadowColor.g);
    mFunc.push(mShadowColor.b);
    mFunc.push(mShadowColor.a);

    return mFunc.on_return();
}

/** @function get_shadow_offset
*/
int lua_font_string::_get_shadow_offset(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:get_shadow_offset", pLua, 2);

    mFunc.push(get_object()->get_shadow_x_offset());
    mFunc.push(get_object()->get_shadow_y_offset());

    return mFunc.on_return();
}

/** @function get_spacing
*/
int lua_font_string::_get_spacing(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:get_spacing", pLua, 1);

    mFunc.push(get_object()->get_spacing());

    return mFunc.on_return();
}

/** @function get_text_color
*/
int lua_font_string::_get_text_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:get_text_color", pLua, 4);

    const color& mTextColor = get_object()->get_text_color();

    mFunc.push(mTextColor.r);
    mFunc.push(mTextColor.g);
    mFunc.push(mTextColor.b);
    mFunc.push(mTextColor.a);

    return mFunc.on_return();
}

/** @function set_font
*/
int lua_font_string::_set_font(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:set_font", pLua);
    mFunc.add(0, "file", lua::type::STRING);
    mFunc.add(1, "height", lua::type::NUMBER);
    mFunc.add(3, "flags", lua::type::STRING, true);

    if (mFunc.check())
    {
        get_object()->set_font(mFunc.get(0)->get_string(), mFunc.get(1)->get_number());
        if (mFunc.is_provided(2))
        {
            std::string sFlags = mFunc.get(2)->get_string();
            if (sFlags.find("OUTLINE") != std::string::npos ||
                sFlags.find("THICKOUTLINE") != std::string::npos)
                get_object()->set_outlined(true);
            else if (sFlags.empty())
                get_object()->set_outlined(false);
            else
            {
                gui::out << gui::warning << mFunc.get_name() << " : "
                    << "Unknown flag list : \"" << sFlags <<"\"." << std::endl;
            }
        }
        else
            get_object()->set_outlined(false);
    }

    return mFunc.on_return();
}

/** @function set_justify_h
*/
int lua_font_string::_set_justify_h(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:set_justify_h", pLua);
    mFunc.add(0, "justify horizontal", lua::type::STRING);

    if (mFunc.check())
    {
        std::string sJustifyH = mFunc.get(0)->get_string();
        if (sJustifyH == "LEFT")
            get_object()->set_justify_h(text::alignment::LEFT);
        else if (sJustifyH == "CENTER")
            get_object()->set_justify_h(text::alignment::CENTER);
        else if (sJustifyH == "RIGHT")
            get_object()->set_justify_h(text::alignment::RIGHT);
        else
        {
            gui::out << gui::warning << mFunc.get_name() << " : "
                << "Unknown justify behavior : \"" << sJustifyH << "\"." << std::endl;
        }
    }

    return mFunc.on_return();
}

/** @function set_justify_v
*/
int lua_font_string::_set_justify_v(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:set_justify_v", pLua);
    mFunc.add(0, "justify vertical", lua::type::STRING);

    if (mFunc.check())
    {
        std::string sJustifyV = mFunc.get(0)->get_string();
        if (sJustifyV == "TOP")
            get_object()->set_justify_v(text::vertical_alignment::TOP);
        else if (sJustifyV == "MIDDLE")
            get_object()->set_justify_v(text::vertical_alignment::MIDDLE);
        else if (sJustifyV == "BOTTOM")
            get_object()->set_justify_v(text::vertical_alignment::BOTTOM);
        else
        {
            gui::out << gui::warning << mFunc.get_name() << " : "
                << "Unknown justify behavior : \"" << sJustifyV << "\"." << std::endl;
        }
    }

    return mFunc.on_return();
}

/** @function set_shadow_color
*/
int lua_font_string::_set_shadow_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:set_shadow_color", pLua);
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

        get_object()->set_shadow_color(mColor);
    }

    return mFunc.on_return();
}

/** @function set_shadow_offset
*/
int lua_font_string::_set_shadow_offset(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:set_shadow_offset", pLua);
    mFunc.add(0, "x offset", lua::type::NUMBER);
    mFunc.add(1, "y offset", lua::type::NUMBER);

    if (mFunc.check())
    {
        get_object()->set_shadow_offsets(
            mFunc.get(0)->get_number(),
            mFunc.get(1)->get_number()
        );
    }

    return mFunc.on_return();
}

/** @function set_spacing
*/
int lua_font_string::_set_spacing(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:set_spacing", pLua);
    mFunc.add(0, "spacing", lua::type::NUMBER);

    if (mFunc.check())
    {
        get_object()->set_spacing(mFunc.get(0)->get_number());
    }

    return mFunc.on_return();
}

/** @function set_text_color
*/
int lua_font_string::_set_text_color(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:set_text_color", pLua);
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

        get_object()->set_text_color(mColor);
    }

    return mFunc.on_return();
}

/** @function can_non_space_wrap
*/
int lua_font_string::_can_non_space_wrap(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:can_non_space_wrap", pLua, 1);

    mFunc.push(get_object()->can_non_space_wrap());

    return mFunc.on_return();
}

/** @function can_word_wrap
*/
int lua_font_string::_can_word_wrap(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:can_word_wrap", pLua, 1);

    mFunc.push(get_object()->can_word_wrap());

    return mFunc.on_return();
}

/** @function enable_formatting
*/
int lua_font_string::_enable_formatting(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:enable_formatting", pLua);
    mFunc.add(0, "formatting", lua::type::BOOLEAN);

    if (mFunc.check())
    {
        get_object()->enable_formatting(mFunc.get(0)->get_bool());
    }

    return mFunc.on_return();
}

/** @function get_string_height
*/
int lua_font_string::_get_string_height(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:get_string_height", pLua, 1);

    mFunc.push(get_object()->get_string_height());

    return mFunc.on_return();
}

/** @function get_string_width
*/
int lua_font_string::_get_string_width(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:get_string_width", pLua, 1);
    mFunc.add(0, "string", lua::type::STRING, true);

    if (mFunc.check())
    {
        if (mFunc.is_provided(0))
        {
            mFunc.push(get_object()->get_string_width(
                utils::utf8_to_unicode(mFunc.get(0)->get_string())));
        }
        else
            mFunc.push(get_object()->get_string_width());
    }

    return mFunc.on_return();
}

/** @function get_text
*/
int lua_font_string::_get_text(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:get_text", pLua, 1);

    mFunc.push(utils::unicode_to_utf8(get_object()->get_text()));

    return mFunc.on_return();
}

/** @function is_formatting_enabled
*/
int lua_font_string::_is_formatting_enabled(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:is_formatting_enabled", pLua, 1);

    mFunc.push(get_object()->is_formatting_enabled());

    return mFunc.on_return();
}

/** @function set_non_space_wrap
*/
int lua_font_string::_set_non_space_wrap(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:set_non_space_wrap", pLua);
    mFunc.add(0, "can non space wrap", lua::type::BOOLEAN);

    if (mFunc.check())
    {
        get_object()->set_non_space_wrap(mFunc.get(0)->get_bool());
    }

    return mFunc.on_return();
}

/** @function set_word_wrap
*/
int lua_font_string::_set_word_wrap(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:set_word_wrap", pLua);
    mFunc.add(0, "can word wrap", lua::type::BOOLEAN);
    mFunc.add(1, "add ellipsis", lua::type::BOOLEAN, true);

    if (mFunc.check())
    {
        bool bEllipsis = true;
        if (mFunc.is_provided(1))
            bEllipsis = mFunc.get(1)->get_bool();

        get_object()->set_word_wrap(mFunc.get(0)->get_bool(), bEllipsis);
    }

    return mFunc.on_return();
}

/** @function set_text
*/
int lua_font_string::_set_text(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("FontString:set_text", pLua);
    mFunc.add(0, "text", lua::type::STRING);
    mFunc.add(0, "number", lua::type::NUMBER);
    mFunc.add(0, "bool", lua::type::BOOLEAN);

    if (mFunc.check())
    {
        std::string sText;
        if (mFunc.get(0)->get_type() == lua::type::STRING)
            sText = mFunc.get(0)->get_string();
        else if (mFunc.get(0)->get_type() == lua::type::NUMBER)
            sText = utils::to_string(mFunc.get(0)->get_number());
        else if (mFunc.get(0)->get_type() == lua::type::BOOLEAN)
            sText = utils::to_string(mFunc.get(0)->get_bool());

        get_object()->set_text(utils::utf8_to_unicode(sText));
    }

    return mFunc.on_return();
}
}
}
