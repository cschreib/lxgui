#include "lxgui/gui_animated_texture.hpp"
#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"

#include <lxgui/extern_sol2_state.hpp>

/** A @{LayeredRegion} that can draw images and colored rectangles.
 * This object contains either an animated texture taken from a file.
 *
 * Inherits all methods from: @{Region}, @{LayeredRegion}.
 *
 * Child classes: none.
 * @classmod AnimatedTexture
 */

namespace lxgui::gui {

void animated_texture::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<animated_texture>(
        "AnimatedTexture", sol::base_classes, sol::bases<region, layered_region>(),
        sol::meta_function::index, member_function<&animated_texture::get_lua_member_>(),
        sol::meta_function::new_index, member_function<&animated_texture::set_lua_member_>());

    /** @function get_texture
     */
    type.set_function("get_texture", member_function<&animated_texture::get_texture_file>());

    /** @function get_vertex_color
     */
    type.set_function("get_vertex_color", [](const animated_texture& self, std::size_t index) {
        color color = self.get_vertex_color(index);
        return std::make_tuple(color.r, color.g, color.b, color.a);
    });

    /** @function set_texture
     */
    type.set_function("set_texture", member_function<&animated_texture::set_texture>());

    /** @function set_vertex_color
     */
    type.set_function(
        "set_vertex_color",
        sol::overload(
            [](animated_texture& self, const std::string& s) {
                self.set_vertex_color(color(s), std::numeric_limits<std::size_t>::max());
            },
            [](animated_texture& self, float r, float g, float b, sol::optional<float> a) {
                self.set_vertex_color(
                    color(r, g, b, a.value_or(1.0f)), std::numeric_limits<std::size_t>::max());
            },
            [](animated_texture& self, std::size_t index, const std::string& s) {
                self.set_vertex_color(color(s), index);
            },
            [](animated_texture& self, std::size_t index, float r, float g, float b,
               sol::optional<float> a) {
                self.set_vertex_color(color(r, g, b, a.value_or(1.0f)), index);
            }));
}

} // namespace lxgui::gui
