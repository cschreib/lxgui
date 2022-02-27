#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_region_tpl.hpp"

#include <lxgui/extern_sol2_state.hpp>

/** A @{Region} that can be rendered in a layer.
 * LayeredRegions can display content on the screen (texture,
 * texts, 3D models, ...) and must be contained inside a layer,
 * within a @{Frame} object. The frame will then render all
 * its layered regions, sorted by layers.
 *
 * Layered regions cannot themselves react to events; this
 * must be taken care of by the parent @{Frame}.
 *
 * Inherits all methods from: @{Region}.
 *
 * Child classes: @{FontString}, @{Texture}.
 * @classmod LayeredRegion
 */

namespace lxgui::gui {

void layered_region::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<layered_region>(
        "LayeredRegion", sol::base_classes, sol::bases<region>(), sol::meta_function::index,
        member_function<&layered_region::get_lua_member_>(), sol::meta_function::new_index,
        member_function<&layered_region::set_lua_member_>());

    /** @function set_draw_layer
     */
    type.set_function("set_draw_layer", [](layered_region& self, std::string layer_name) {
        if (auto converted = utils::from_string<layer>(layer_name); converted.has_value()) {
            self.set_draw_layer(converted.value());
        } else {
            gui::out << gui::warning << "LayeredRegion.set_draw_layer: "
                     << "Unknown layer type: \"" << layer_name << "\"." << std::endl;
        }
    });

    /** @function get_draw_layer
     */
    type.set_function("get_draw_layer", [](const layered_region& self) {
        return utils::to_string(self.get_draw_layer());
    });
}

} // namespace lxgui::gui
