#include "lxgui/gui_button.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"
#include "lxgui/gui_out.hpp"

#include <sol/state.hpp>

/** A @{Frame} with a button that can be clicked.
*   This class can handle three different states: "normal", "pushed"
*   and "disabled". You can provide a different texture for each of
*   these states, and two different fontstrings for "normal" and
*   "disabled".
*
*   In addition, you can provide another texture/fontstring for the
*   "highlight" state (when the mouse is over the button widget).
*
*   Note that there is no fontstring for the "pushed" state: in this
*   case, the "normal" font is rendered with a slight offset that you
*   are free to define.
*
*   Note that a @{Button} has @{Frame:enable_mouse} set to `true` by
*   default.
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

void button::register_on_lua(sol::state& mLua)
{
    auto mClass = mLua.new_usertype<button>("Button",
        sol::base_classes, sol::bases<uiobject, frame>(),
        sol::meta_function::index,
        &button::set_lua_member_,
        sol::meta_function::new_index,
        &button::get_lua_member_);

    /** @function click
    */
    mClass.set_function("click", [](button& mSelf)
    {
        mSelf.on_script("OnClick");
    });

    /** @function disable
    */
    mClass.set_function("disable", member_function<&button::disable>());

    /** @function enable
    */
    mClass.set_function("enable", member_function<&button::enable>());

    /** @function get_button_state
    */
    mClass.set_function("get_button_state", [](const button& mSelf)
    {
        switch (mSelf.get_button_state())
        {
            case button::state::UP :       return "NORMAL";
            case button::state::DOWN :     return "PUSHED";
            case button::state::DISABLED : return "DISABLED";
            default :                      return "UNKNOWN";
        }
    });

    /** @function get_disabled_font_object
    */
    mClass.set_function("get_disabled_font_object", member_function< // select the right overload for Lua
        static_cast<const utils::observer_ptr<font_string>& (button::*)()>(&button::get_disabled_text)>());

    /** @function get_disabled_text_color
    */
    mClass.set_function("get_disabled_text_color", [](const button& mSelf)
    {
        sol::optional<std::tuple<float,float,float,float>> mReturn;
        auto pFontString = mSelf.get_disabled_text();
        if (pFontString)
        {
            const color& mColor = pFontString->get_text_color();
            mReturn = std::make_tuple(mColor.r, mColor.g, mColor.b, mColor.a);
        }

        return mReturn;
    });

    /** @function get_disabled_texture
    */
    mClass.set_function("get_disabled_texture", member_function< // select the right overload for Lua
        static_cast<const utils::observer_ptr<texture>& (button::*)()>(&button::get_disabled_texture)>());

    /** @function get_highlight_font_object
    */
    mClass.set_function("get_highlight_font_object", member_function< // select the right overload for Lua
        static_cast<const utils::observer_ptr<font_string>& (button::*)()>(&button::get_highlight_text)>());

    /** @function get_highlight_text_color
    */
    mClass.set_function("get_highlight_text_color", [](const button& mSelf)
    {
        sol::optional<std::tuple<float,float,float,float>> mReturn;
        auto pFontString = mSelf.get_highlight_text();
        if (pFontString)
        {
            const color& mColor = pFontString->get_text_color();
            mReturn = std::make_tuple(mColor.r, mColor.g, mColor.b, mColor.a);
        }

        return mReturn;
    });

    /** @function get_highlight_texture
    */
    mClass.set_function("get_highlight_texture", member_function< // select the right overload for Lua
        static_cast<const utils::observer_ptr<texture>& (button::*)()>(&button::get_highlight_texture)>());

    /** @function get_normal_font_object
    */
    mClass.set_function("get_normal_font_object", member_function< // select the right overload for Lua
        static_cast<const utils::observer_ptr<font_string>& (button::*)()>(&button::get_normal_text)>());

    /** @function get_normal_texture
    */
    mClass.set_function("get_normal_texture", member_function< // select the right overload for Lua
        static_cast<const utils::observer_ptr<texture>& (button::*)()>(&button::get_normal_texture)>());

    /** @function get_pushed_text_offset
    */
    mClass.set_function("get_pushed_text_offset", [](const button& mSelf)
    {
        vector2f lOffset = mSelf.get_pushed_text_offset();
        return std::make_pair(lOffset.x, lOffset.y);
    });

    /** @function get_pushed_texture
    */
    mClass.set_function("get_pushed_texture", member_function< // select the right overload for Lua
        static_cast<const utils::observer_ptr<texture>& (button::*)()>(&button::get_pushed_texture)>());

    /** @function get_text
    */
    mClass.set_function("get_text", [](const button& mSelf)
    {
        return utils::unicode_to_utf8(mSelf.get_text());
    });

    /** @function get_text_height
    */
    mClass.set_function("get_text_height", [](const button& mSelf)
    {
        sol::optional<float> mReturn;
        auto pCurrentFont = mSelf.get_current_font_string();
        if (pCurrentFont)
            mReturn = pCurrentFont->get_string_height();

        return mReturn;
    });

    /** @function get_text_width
    */
    mClass.set_function("get_text_width", [](const button& mSelf)
    {
        sol::optional<float> mReturn;
        auto pCurrentFont = mSelf.get_current_font_string();
        if (pCurrentFont)
            mReturn = pCurrentFont->get_string_width();

        return mReturn;
    });

    /** @function is_enabled
    */
    mClass.set_function("is_enabled", member_function<&button::is_enabled>());

    /** @function lock_highlight
    */
    mClass.set_function("lock_highlight", member_function<&button::lock_highlight>());

    /** @function set_button_state
    */
    mClass.set_function("set_button_state", [](button& mSelf, const std::string& sState)
    {
        if (sState == "NORMAL")
        {
            mSelf.enable();
            mSelf.release();
        }
        else if (sState == "PUSHED")
        {
            mSelf.enable();
            mSelf.push();
        }
        else if (sState == "DISABLED")
        {
            mSelf.disable();
            mSelf.release();
        }
        else
        {
            gui::out << gui::warning << "Button:set_button_state" <<
                " : Unknown button state : \""+sState+"\"." << std::endl;
        }
    });

    /** @function set_disabled_font_object
    */
    mClass.set_function("set_disabled_font_object", member_function<&button::set_disabled_text>());

    /** @function set_disabled_text_color
    */
    mClass.set_function("set_disabled_text_color", sol::overload(
    [](button& mSelf, float fR, float fG, float fB, sol::optional<float> fA)
    {
        auto pFontString = mSelf.get_disabled_text();
        if (pFontString)
            pFontString->set_text_color(color(fR, fG, fB, fA.value_or(1.0f)));
    },
    [](button& mSelf, const std::string& sColor)
    {
        auto pFontString = mSelf.get_disabled_text();
        if (pFontString)
            pFontString->set_text_color(color(sColor));
    }));

    /** @function set_disabled_texture
    */
    mClass.set_function("set_disabled_texture", member_function<&button::set_disabled_texture>());

    /** @function set_highlight_font_object
    */
    mClass.set_function("set_highlight_font_object", member_function<&button::set_highlight_text>());

    /** @function set_highlight_text_color
    */
    mClass.set_function("set_highlight_text_color", sol::overload(
    [](button& mSelf, float fR, float fG, float fB, sol::optional<float> fA)
    {
        auto pFontString = mSelf.get_highlight_text();
        if (pFontString)
            pFontString->set_text_color(color(fR, fG, fB, fA.value_or(1.0f)));
    },
    [](button& mSelf, const std::string& sColor)
    {
        auto pFontString = mSelf.get_highlight_text();
        if (pFontString)
            pFontString->set_text_color(color(sColor));
    }));

    /** @function set_highlight_texture
    */
    mClass.set_function("set_highlight_texture", member_function<&button::set_highlight_texture>());

    /** @function set_normal_font_object
    */
    mClass.set_function("set_normal_font_object", member_function<&button::set_normal_text>());

    /** @function set_normal_text_color
    */
    mClass.set_function("set_normal_text_color", sol::overload(
    [](button& mSelf, float fR, float fG, float fB, sol::optional<float> fA)
    {
        auto pFontString = mSelf.get_normal_text();
        if (pFontString)
            pFontString->set_text_color(color(fR, fG, fB, fA.value_or(1.0f)));
    },
    [](button& mSelf, const std::string& sColor)
    {
        auto pFontString = mSelf.get_normal_text();
        if (pFontString)
            pFontString->set_text_color(color(sColor));
    }));

    /** @function set_normal_texture
    */
    mClass.set_function("set_normal_texture", member_function<&button::set_normal_texture>());

    /** @function set_pushed_text_offset
    */
    mClass.set_function("set_pushed_text_offset", [](button& mSelf, float fXOffset, float fYOffset)
    {
        mSelf.set_pushed_text_offset(vector2f(fXOffset, fYOffset));
    });

    /** @function set_pushed_texture
    */
    mClass.set_function("set_pushed_texture", member_function<&button::set_pushed_texture>());

    /** @function set_text
    */
    mClass.set_function("set_text", [](button& mSelf, const std::string& sText)
    {
        mSelf.set_text(utils::utf8_to_unicode(sText));
    });

    /** @function unlock_highlight
    */
    mClass.set_function("unlock_highlight", member_function<&button::unlock_highlight>());
}

}
}
