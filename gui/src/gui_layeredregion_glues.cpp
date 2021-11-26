#include "lxgui/gui_layeredregion.hpp"

#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

#include <sol/state.hpp>

/** A @{UIObject} that can be rendered in a layer.
*   LayeredRegions can display content on the screen (texture,
*   texts, 3D models, ...) and must be contained inside a layer,
*   within a @{Frame} object. The frame will then render all
*   its layered regions, sorted by layers.
*
*   Layered regions cannot themselves react to events; this
*   must be taken care of by the parent @{Frame}.
*
*   Inherits all methods from: @{UIObject}.
*
*   Child classes: @{FontString}, @{Texture}.
*   @classmod LayeredRegion
*/

namespace lxgui {
namespace gui
{

void layered_region::register_on_lua(sol::state& mLua)
{
    auto mClass = mLua.new_usertype<layered_region>("LayeredRegion",
        sol::base_classes, sol::bases<uiobject>(),
        sol::meta_function::index,
        &layered_region::set_lua_member_,
        sol::meta_function::new_index,
        &layered_region::get_lua_member_);

    /** @function set_draw_layer
    */
    mClass.set_function("set_draw_layer", member_function< // select the right overload for Lua
        static_cast<void (layered_region::*)(const std::string&)>(&layered_region::set_draw_layer)>());

    /** @function get_draw_layer
    */
    mClass.set_function("get_draw_layer", [](const layered_region& mSelf)
    {
        switch (mSelf.get_draw_layer())
        {
            case layer_type::BACKGROUND :  return std::string("BACKGROUND");
            case layer_type::BORDER :      return std::string("BORDER");
            case layer_type::ARTWORK :     return std::string("ARTWORK");
            case layer_type::OVERLAY :     return std::string("OVERLAY");
            case layer_type::HIGHLIGHT :   return std::string("HIGHLIGHT");
            case layer_type::SPECIALHIGH : return std::string("SPECIALHIGH");
            default:                       throw sol::error("unreachable");
        }
    });
}

}
}
