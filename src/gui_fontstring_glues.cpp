#include "lxgui/gui_fontstring.hpp"

#include "lxgui/gui_uiobject_tpl.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_string.hpp>

#include <sol/state.hpp>

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
void font_string::register_on_lua(sol::state& mLua)
{
    auto mClass = mLua.new_usertype<font_string>("FontString",
        sol::base_classes, sol::bases<uiobject, layered_region>(),
        sol::meta_function::index,
        member_function<&font_string::get_lua_member_>(),
        sol::meta_function::new_index,
        member_function<&font_string::set_lua_member_>());

    /** @function get_font
    */
    mClass.set_function("get_font", member_function<&font_string::get_font_name>());

    /** @function get_justify_h
    */
    mClass.set_function("get_justify_h", [](const font_string& mSelf)
    {
        text::alignment mAlignment = mSelf.get_justify_h();
        switch (mAlignment)
        {
            case text::alignment::LEFT :   return "LEFT";
            case text::alignment::CENTER : return "CENTER";
            case text::alignment::RIGHT :  return "RIGHT";
            default:                       return "UNKNOWN";
        }
    });

    /** @function get_justify_v
    */
    mClass.set_function("get_justify_v", [](const font_string& mSelf)
    {
        text::vertical_alignment mAlignment = mSelf.get_justify_v();
        switch (mAlignment)
        {
            case text::vertical_alignment::TOP :    return "TOP";
            case text::vertical_alignment::MIDDLE : return "MIDDLE";
            case text::vertical_alignment::BOTTOM : return "BOTTOM";
            default:                                return "UNKNOWN";
        }
    });

    /** @function get_shadow_color
    */
    mClass.set_function("get_shadow_color", [](const font_string& mSelf)
    {
        const color& mShadowColor = mSelf.get_shadow_color();
        return std::make_tuple(mShadowColor.r, mShadowColor.g, mShadowColor.b, mShadowColor.a);
    });

    /** @function get_shadow_offset
    */
    mClass.set_function("get_shadow_offset", [](const font_string& mSelf)
    {
        const vector2f& mShadowOffsets = mSelf.get_shadow_offset();
        return std::make_pair(mShadowOffsets.x, mShadowOffsets.y);
    });

    /** @function get_spacing
    */
    mClass.set_function("get_spacing", member_function<&font_string::get_spacing>());

    /** @function get_line_spacing
    */
    mClass.set_function("get_line_spacing", member_function<&font_string::get_line_spacing>());

    /** @function get_text_color
    */
    mClass.set_function("get_text_color", [](const font_string& mSelf)
    {
        const color& mTextColor = mSelf.get_text_color();
        return std::make_tuple(mTextColor.r, mTextColor.g, mTextColor.b, mTextColor.a);
    });

    /** @function set_font
    */
    mClass.set_function("set_font", [](font_string& mSelf, const std::string& sFile,
        float fHeight, sol::optional<std::string> sFlags)
    {
        mSelf.set_font(sFile, fHeight);

        if (sFlags.has_value())
        {
            if (sFlags.value().find("OUTLINE") != std::string::npos ||
                sFlags.value().find("THICKOUTLINE") != std::string::npos)
                mSelf.set_outlined(true);
            else if (sFlags.value().empty())
                mSelf.set_outlined(false);
            else
            {
                gui::out << gui::warning << "EditBox:set_font : "
                    << "Unknown flags : \"" << sFlags.value() <<"\"." << std::endl;
            }
        }
        else
            mSelf.set_outlined(false);
    });

    /** @function set_justify_h
    */
    mClass.set_function("set_justify_h", [](font_string& mSelf, const std::string& sJustifyH)
    {
        if (sJustifyH == "LEFT")
            mSelf.set_justify_h(text::alignment::LEFT);
        else if (sJustifyH == "CENTER")
            mSelf.set_justify_h(text::alignment::CENTER);
        else if (sJustifyH == "RIGHT")
            mSelf.set_justify_h(text::alignment::RIGHT);
        else
        {
            gui::out << gui::warning << "font_string:set_justify_h : "
                << "Unknown justify behavior : \"" << sJustifyH << "\"." << std::endl;
        }
    });

    /** @function set_justify_v
    */
    mClass.set_function("set_justify_v", [](font_string& mSelf, const std::string& sJustifyV)
    {
        if (sJustifyV == "TOP")
            mSelf.set_justify_v(text::vertical_alignment::TOP);
        else if (sJustifyV == "MIDDLE")
            mSelf.set_justify_v(text::vertical_alignment::MIDDLE);
        else if (sJustifyV == "BOTTOM")
            mSelf.set_justify_v(text::vertical_alignment::BOTTOM);
        else
        {
            gui::out << gui::warning << "font_string:set_justify_v : "
                << "Unknown justify behavior : \"" << sJustifyV << "\"." << std::endl;
        }
    });

    /** @function set_shadow_color
    */
    mClass.set_function("set_shadow_color", sol::overload(
    [](font_string& mSelf, float fR, float fG, float fB, sol::optional<float> fA)
    {
        mSelf.set_shadow_color(color(fR, fG, fB, fA.value_or(1.0f)));
    },
    [](font_string& mSelf, const std::string& sColor)
    {
        mSelf.set_shadow_color(color(sColor));
    }));

    /** @function set_shadow_offset
    */
    mClass.set_function("set_shadow_offset", [](font_string& mSelf, float fXOffset, float fYOffset)
    {
        mSelf.set_shadow_offset(vector2f(fXOffset, fYOffset));
    });

    /** @function set_spacing
    */
    mClass.set_function("set_spacing", member_function<&font_string::set_spacing>());

    /** @function set_line_spacing
    */
    mClass.set_function("set_line_spacing", member_function<&font_string::set_line_spacing>());

    /** @function set_text_color
    */
    mClass.set_function("set_text_color", sol::overload(
    [](font_string& mSelf, float fR, float fG, float fB, sol::optional<float> fA)
    {
        mSelf.set_text_color(color(fR, fG, fB, fA.value_or(1.0f)));
    },
    [](font_string& mSelf, const std::string& sColor)
    {
        mSelf.set_text_color(color(sColor));
    }));

    /** @function can_non_space_wrap
    */
    mClass.set_function("can_non_space_wrap", member_function<&font_string::can_non_space_wrap>());

    /** @function can_word_wrap
    */
    mClass.set_function("can_word_wrap", member_function<&font_string::can_word_wrap>());

    /** @function enable_formatting
    */
    mClass.set_function("enable_formatting", member_function<&font_string::enable_formatting>());

    /** @function get_string_height
    */
    mClass.set_function("get_string_height", member_function<&font_string::get_string_height>());

    /** @function get_string_width
    */
    mClass.set_function("get_string_width", [](const font_string& mSelf, sol::optional<std::string> sText)
    {
        if (sText.has_value())
            return mSelf.get_string_width(utils::utf8_to_unicode(sText.value()));
        else
            return mSelf.get_string_width();
    });

    /** @function get_text
    */
    mClass.set_function("get_text", [](const font_string& mSelf)
    {
        return utils::unicode_to_utf8(mSelf.get_text());
    });

    /** @function is_formatting_enabled
    */
    mClass.set_function("is_formatting_enabled", member_function<&font_string::is_formatting_enabled>());

    /** @function set_non_space_wrap
    */
    mClass.set_function("set_non_space_wrap", member_function<&font_string::set_non_space_wrap>());

    /** @function set_word_wrap
    */
    mClass.set_function("set_word_wrap", [](font_string& mSelf,
        bool bWrap, sol::optional<bool> bEllipsis)
    {
        mSelf.set_word_wrap(bWrap, bEllipsis.value_or(false));
    });

    /** @function set_text
    */
    mClass.set_function("set_text", sol::overload(
    [](font_string& mSelf, bool bValue)
    {
        mSelf.set_text(utils::to_ustring(bValue));
    },
    [](font_string& mSelf, int iValue)
    {
        mSelf.set_text(utils::to_ustring(iValue));
    },
    [](font_string& mSelf, double dValue)
    {
        mSelf.set_text(utils::to_ustring(dValue));
    },
    [](font_string& mSelf, const std::string& sText)
    {
        mSelf.set_text(utils::utf8_to_unicode(sText));
    }));
}

}
}