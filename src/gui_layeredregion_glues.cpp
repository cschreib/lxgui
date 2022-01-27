#include "lxgui/gui_layeredregion.hpp"

#include "lxgui/gui_region.hpp"
#include "lxgui/gui_region_tpl.hpp"

#include <sol/state.hpp>

/** A @{Region} that can be rendered in a layer.
*   LayeredRegions can display content on the screen (texture,
*   texts, 3D models, ...) and must be contained inside a layer,
*   within a @{Frame} object. The frame will then render all
*   its layered regions, sorted by layers.
*
*   Layered regions cannot themselves react to events; this
*   must be taken care of by the parent @{Frame}.
*
*   Inherits all methods from: @{Region}.
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
        sol::base_classes, sol::bases<region>(),
        sol::meta_function::index,
        member_function<&layered_region::get_lua_member_>(),
        sol::meta_function::new_index,
        member_function<&layered_region::set_lua_member_>());

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
            case layer::BACKGROUND :  return std::string("BACKGROUND");
            case layer::BORDER :      return std::string("BORDER");
            case layer::ARTWORK :     return std::string("ARTWORK");
            case layer::OVERLAY :     return std::string("OVERLAY");
            case layer::HIGHLIGHT :   return std::string("HIGHLIGHT");
            case layer::SPECIALHIGH : return std::string("SPECIALHIGH");
            default:                       throw sol::error("unreachable");
        }
    });
}

}
}
