#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_uiobject.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>

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
lua_layered_region::lua_layered_region(lua_State* pLua) : lua_uiobject(pLua)
{
}

/** @function set_draw_layer
*/
int lua_layered_region::_set_draw_layer(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("LayeredRegion:set_draw_layer", pLua);
    mFunc.add(0, "layer", lua::type::STRING);
    if (mFunc.check())
        get_object()->set_draw_layer(mFunc.get(0)->get_string());

    return mFunc.on_return();
}

/** @function get_draw_layer
*/
int lua_layered_region::_get_draw_layer(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("LayeredRegion:get_draw_layer", pLua, 1);

    switch (get_object()->get_draw_layer())
    {
        case layer_type::BACKGROUND :  mFunc.push(std::string("BACKGROUND")); break;
        case layer_type::BORDER :      mFunc.push(std::string("BORDER")); break;
        case layer_type::ARTWORK :     mFunc.push(std::string("ARTWORK")); break;
        case layer_type::OVERLAY :     mFunc.push(std::string("OVERLAY")); break;
        case layer_type::HIGHLIGHT :   mFunc.push(std::string("HIGHLIGHT")); break;
        case layer_type::SPECIALHIGH : mFunc.push(std::string("SPECIALHIGH")); break;
        case layer_type::ENUM_SIZE :   break;
    }

    return mFunc.on_return();
}
}
}
