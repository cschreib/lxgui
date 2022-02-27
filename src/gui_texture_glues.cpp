#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

#include <lxgui/extern_sol2_state.hpp>

/** A @{LayeredRegion} that can draw images and colored rectangles.
 * This object contains either a texture taken from a file,
 * or a plain color (possibly with a different color on each corner).
 *
 * Inherits all methods from: @{Region}, @{LayeredRegion}.
 *
 * Child classes: none.
 * @classmod Texture
 */

namespace lxgui::gui {

std::optional<orientation> get_gradient_orientation(const std::string& orientation_name) {
    auto orient = utils::from_string<orientation>(orientation_name);
    if (!orient.has_value()) {
        gui::out << gui::warning << "Texture:set_gradient: Unknown gradient orientation: \""
                 << orientation_name << "\"." << std::endl;
    }

    return orient;
}

void texture::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<texture>(
        "Texture", sol::base_classes, sol::bases<region, layered_region>(),
        sol::meta_function::index, member_function<&texture::get_lua_member_>(),
        sol::meta_function::new_index, member_function<&texture::set_lua_member_>());

    /** @function get_blend_mode
     */
    type.set_function("get_blend_mode", [](const texture& self) {
        return utils::to_string(self.get_blend_mode());
    });

    /** @function get_filter_mode
     */
    type.set_function("get_filter_mode", [](const texture& self) {
        return utils::to_string(self.get_filter_mode());
    });

    /** @function get_tex_coord
     */
    type.set_function("get_tex_coord", [](const texture& self) {
        const auto& coords = self.get_tex_coord();
        return std::make_tuple(
            coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], coords[6], coords[7]);
    });

    /** @function get_texture_stretching
     */
    type.set_function(
        "get_texture_stretching", member_function<&texture::get_texture_stretching>());

    /** @function get_texture
     */
    type.set_function("get_texture", [](const texture& self) -> sol::optional<std::string> {
        if (self.has_texture_file())
            return self.get_texture_file();
        else
            return {};
    });

    /** @function get_vertex_color
     */
    type.set_function("get_vertex_color", [](const texture& self, std::size_t index) {
        color color = self.get_vertex_color(index);
        return std::make_tuple(color.r, color.g, color.b, color.a);
    });

    /** @function is_desaturated
     */
    type.set_function("is_desaturated", member_function<&texture::is_desaturated>());

    /** @function set_blend_mode
     */
    type.set_function("set_blend_mode", [](texture& self, const std::string& blend_mode_name) {
        if (auto converted = utils::from_string<texture::blend_mode>(blend_mode_name);
            converted.has_value()) {
            self.set_blend_mode(converted.value());
        } else {
            gui::out << gui::warning << "Texture:set_blend_mode: "
                     << "Unknown blending mode: \"" + blend_mode_name + "\"." << std::endl;
        }
    });

    /** @function set_filter_mode
     */
    type.set_function("set_filter_mode", [](texture& self, const std::string& filter_name) {
        if (auto converted = utils::from_string<material::filter>(filter_name);
            converted.has_value()) {
            self.set_filter_mode(converted.value());
        } else {
            gui::out << gui::warning << "Texture:set_filter_mode: "
                     << "Unknown filtering mode: \"" + filter_name + "\"." << std::endl;
        }
    });

    /** @function set_desaturated
     */
    type.set_function("set_desaturated", member_function<&texture::set_desaturated>());

    /** @function set_gradient
     */
    type.set_function(
        "set_gradient",
        sol::overload(
            [](texture& self, const std::string& orientation_name, float min_r, float min_g,
               float min_b, float max_r, float max_g, float max_b) {
                auto orientation = get_gradient_orientation(orientation_name);
                if (!orientation.has_value())
                    return;

                self.set_gradient(gradient{
                    orientation.value(), color(min_r, min_g, min_b), color(max_r, max_g, max_b)});
            },
            [](texture& self, const std::string& orientation_name, const std::string& min_color,
               const std::string& max_color) {
                auto orientation = get_gradient_orientation(orientation_name);
                if (!orientation.has_value())
                    return;

                self.set_gradient(
                    gradient{orientation.value(), color(min_color), color(max_color)});
            }));

    /** @function set_gradient_alpha
     */
    type.set_function(
        "set_gradient_alpha",
        sol::overload(
            [](texture& self, const std::string& orientation_name, float min_r, float min_g,
               float min_b, float min_a, float max_r, float max_g, float max_b, float max_a) {
                auto orientation = get_gradient_orientation(orientation_name);
                if (!orientation.has_value())
                    return;

                self.set_gradient(gradient{
                    orientation.value(), color(min_r, min_g, min_b, min_a),
                    color(max_r, max_g, max_b, max_a)});
            },
            [](texture& self, const std::string& orientation_name, const std::string& min_color,
               const std::string& max_color) {
                auto orientation = get_gradient_orientation(orientation_name);
                if (!orientation.has_value())
                    return;

                self.set_gradient(
                    gradient{orientation.value(), color(min_color), color(max_color)});
            }));

    /** @function set_tex_coord
     */
    type.set_function(
        "set_tex_coord",
        sol::overload(
            [](texture& self, float left, float top, float right, float bottom) {
                self.set_tex_rect({left, top, right, bottom});
            },
            [](texture& self, float top_left_x, float top_left_y, float top_right_x,
               float top_right_y, float bottom_right_x, float bottom_right_y, float bottom_left_x,
               float bottom_left_y) {
                self.set_tex_coord(
                    {top_left_x, top_left_y, top_right_x, top_right_y, bottom_right_x,
                     bottom_right_y, bottom_left_x, bottom_left_y});
            }));

    /** @function set_texture_stretching
     */
    type.set_function(
        "set_texture_stretching", member_function<&texture::set_texture_stretching>());

    /** @function set_texture
     */
    type.set_function(
        "set_texture", sol::overload(
                           [](texture& self, const std::string& texture) {
                               if (!texture.empty() && texture[0] == '#') {
                                   // This is actually a color hash
                                   self.set_solid_color(color(texture));
                               } else {
                                   // Normal texture file
                                   self.set_texture(texture);
                               }
                           },
                           [](texture& self, float r, float g, float b, sol::optional<float> a) {
                               self.set_solid_color(color(r, g, b, a.value_or(1.0f)));
                           }));

    /** @function set_vertex_color
     */
    type.set_function(
        "set_vertex_color",
        sol::overload(
            [](texture& self, const std::string& s) {
                self.set_vertex_color(color(s), std::numeric_limits<std::size_t>::max());
            },
            [](texture& self, float r, float g, float b, sol::optional<float> a) {
                self.set_vertex_color(
                    color(r, g, b, a.value_or(1.0f)), std::numeric_limits<std::size_t>::max());
            },
            [](texture& self, std::size_t index, const std::string& s) {
                self.set_vertex_color(color(s), index);
            },
            [](texture& self, std::size_t index, float r, float g, float b,
               sol::optional<float> a) {
                self.set_vertex_color(color(r, g, b, a.value_or(1.0f)), index);
            }));
}

} // namespace lxgui::gui
