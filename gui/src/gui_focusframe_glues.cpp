#include "lxgui/gui_focusframe.hpp"

#include "lxgui/gui_uiobject_tpl.hpp"

#include <sol/state.hpp>

/** A @{Frame} that can receive and loose focus.
*   A typical usage example is the @{EditBox}.
*
*   Inherits all methods from: @{UIObject}, @{Frame}.
*
*   Child classes: @{EditBox}.
*   @classmod FocusFrame
*/

namespace lxgui {
namespace gui
{

void focus_frame::register_on_lua(sol::state& mLua)
{
    auto mClass = mLua.new_usertype<focus_frame>("focus_frame",
        sol::base_classes, sol::bases<uiobject, frame>(),
        sol::meta_function::index,
        &focus_frame::set_lua_member_,
        sol::meta_function::new_index,
        &focus_frame::get_lua_member_);

    /** @function clear_focus
    */
    mClass.set_function("clear_focus", [](focus_frame& mSelf)
    {
        mSelf.set_focus(false);
    });

    /** @function is_auto_focus
    */
    mClass.set_function("is_auto_focus", member_function<&focus_frame::is_auto_focus_enabled>());

    /** @function set_auto_focus
    */
    mClass.set_function("set_auto_focus", member_function<&focus_frame::enable_auto_focus>());

    /** @function set_focus
    */
    mClass.set_function("set_focus", [](focus_frame& mSelf)
    {
        mSelf.set_focus(true);
    });
}

}
}
