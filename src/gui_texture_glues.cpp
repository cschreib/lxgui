#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

#include <sol/state.hpp>

/** A @{LayeredRegion} that can draw images and colored rectangles.
 *   This object contains either a texture taken from a file,
 *   or a plain color (possibly with a different color on each corner).
 *
 *   Inherits all methods from: @{Region}, @{LayeredRegion}.
 *
 *   Child classes: none.
 *   @classmod Texture
 */

namespace lxgui::gui {

sol::optional<gradient::orientation> get_gradient_orientation(const std::string& orientation) {
    sol::optional<gradient::orientation> m_orientation;
    if (orientation == "HORIZONTAL")
        m_orientation = gradient::orientation::horizontal;
    else if (orientation == "VERTICAL")
        m_orientation = gradient::orientation::vertical;
    else {
        gui::out << gui::warning
                 << "Texture:set_gradient : "
                    "Unknown gradient orientation : \"" +
                        orientation + "\"."
                 << std::endl;
    }

    return m_orientation;
}

void texture::register_on_lua(sol::state& m_lua) {
    auto m_class = m_lua.new_usertype<texture>(
        "Texture", sol::base_classes, sol::bases<region, layered_region>(),
        sol::meta_function::index, member_function<&texture::get_lua_member_>(),
        sol::meta_function::new_index, member_function<&texture::set_lua_member_>());

    /** @function get_blend_mode
     */
    m_class.set_function("get_blend_mode", [](const texture& m_self) {
        texture::blend_mode m_blend = m_self.get_blend_mode();
        switch (m_blend) {
        case texture::blend_mode::none: return "NONE";
        case texture::blend_mode::blend: return "BLEND";
        case texture::blend_mode::key: return "KEY";
        case texture::blend_mode::add: return "ADD";
        case texture::blend_mode::mod: return "MOD";
        default: return "UNKNOWN";
        }
    });

    /** @function get_filter_mode
     */
    m_class.set_function("get_filter_mode", [](const texture& m_self) {
        material::filter m_filter = m_self.get_filter_mode();
        switch (m_filter) {
        case material::filter::none: return "NONE";
        case material::filter::linear: return "LINEAR";
        default: return "UNKNOWN";
        }
    });

    /** @function get_tex_coord
     */
    m_class.set_function("get_tex_coord", [](const texture& m_self) {
        const auto& coords = m_self.get_tex_coord();
        return std::make_tuple(
            coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], coords[6], coords[7]);
    });

    /** @function get_texture_stretching
     */
    m_class.set_function(
        "get_texture_stretching", member_function<&texture::get_texture_stretching>());

    /** @function get_texture
     */
    m_class.set_function("get_texture", [](const texture& m_self) {
        sol::optional<std::string> m_return;
        if (m_self.has_texture_file())
            m_return = m_self.get_texture_file();
    });

    /** @function get_vertex_color
     */
    m_class.set_function("get_vertex_color", [](const texture& m_self, std::size_t ui_index) {
        color m_color = m_self.get_vertex_color(ui_index);
        return std::make_tuple(m_color.r, m_color.g, m_color.b, m_color.a);
    });

    /** @function is_desaturated
     */
    m_class.set_function("is_desaturated", member_function<&texture::is_desaturated>());

    /** @function set_blend_mode
     */
    m_class.set_function("set_blend_mode", [](texture& m_self, const std::string& blend) {
        texture::blend_mode m_blend;
        if (blend == "NONE")
            m_blend = texture::blend_mode::none;
        else if (blend == "BLEND")
            m_blend = texture::blend_mode::blend;
        else if (blend == "KEY")
            m_blend = texture::blend_mode::key;
        else if (blend == "ADD")
            m_blend = texture::blend_mode::add;
        else if (blend == "MOD")
            m_blend = texture::blend_mode::mod;
        else {
            gui::out << gui::warning << "Texture:set_blend_mode : "
                     << "Unknown blending mode : \"" + blend + "\"." << std::endl;
            return;
        }

        m_self.set_blend_mode(m_blend);
    });

    /** @function set_filter_mode
     */
    m_class.set_function("set_filter_mode", [](texture& m_self, const std::string& filter) {
        material::filter m_filter;
        if (filter == "NONE")
            m_filter = material::filter::none;
        else if (filter == "LINEAR")
            m_filter = material::filter::linear;
        else {
            gui::out << gui::warning << "Texture:set_filter_mode : "
                     << "Unknown filtering mode : \"" + filter + "\"." << std::endl;
            return;
        }

        m_self.set_filter_mode(m_filter);
    });

    /** @function set_desaturated
     */
    m_class.set_function("set_desaturated", member_function<&texture::set_desaturated>());

    /** @function set_gradient
     */
    m_class.set_function(
        "set_gradient",
        sol::overload(
            [](texture& m_self, const std::string& orientation, float f_min_r, float f_min_g,
               float f_min_b, float f_max_r, float f_max_g, float f_max_b) {
                sol::optional<gradient::orientation> m_orientation =
                    get_gradient_orientation(orientation);
                if (!m_orientation.has_value())
                    return;

                m_self.set_gradient(gradient(
                    m_orientation.value(), color(f_min_r, f_min_g, f_min_b),
                    color(f_max_r, f_max_g, f_max_b)));
            },
            [](texture& m_self, const std::string& orientation, const std::string& min_color,
               const std::string& max_color) {
                sol::optional<gradient::orientation> m_orientation =
                    get_gradient_orientation(orientation);
                if (!m_orientation.has_value())
                    return;

                m_self.set_gradient(
                    gradient(m_orientation.value(), color(min_color), color(max_color)));
            }));

    /** @function set_gradient_alpha
     */
    m_class.set_function(
        "set_gradient_alpha",
        sol::overload(
            [](texture& m_self, const std::string& orientation, float f_min_r, float f_min_g,
               float f_min_b, float f_min_a, float f_max_r, float f_max_g, float f_max_b,
               float f_max_a) {
                sol::optional<gradient::orientation> m_orientation =
                    get_gradient_orientation(orientation);
                if (!m_orientation.has_value())
                    return;

                m_self.set_gradient(gradient(
                    m_orientation.value(), color(f_min_r, f_min_g, f_min_b, f_min_a),
                    color(f_max_r, f_max_g, f_max_b, f_max_a)));
            },
            [](texture& m_self, const std::string& orientation, const std::string& min_color,
               const std::string& max_color) {
                sol::optional<gradient::orientation> m_orientation =
                    get_gradient_orientation(orientation);
                if (!m_orientation.has_value())
                    return;

                m_self.set_gradient(
                    gradient(m_orientation.value(), color(min_color), color(max_color)));
            }));

    /** @function set_tex_coord
     */
    m_class.set_function(
        "set_tex_coord",
        sol::overload(
            [](texture& m_self, float f_left, float f_top, float f_right, float f_bottom) {
                m_self.set_tex_rect({f_left, f_top, f_right, f_bottom});
            },
            [](texture& m_self, float f_top_left_x, float f_top_left_y, float f_top_right_x,
               float f_top_right_y, float f_bottom_right_x, float f_bottom_right_y,
               float f_bottom_left_x, float f_bottom_left_y) {
                m_self.set_tex_coord(
                    {f_top_left_x, f_top_left_y, f_top_right_x, f_top_right_y, f_bottom_right_x,
                     f_bottom_right_y, f_bottom_left_x, f_bottom_left_y});
            }));

    /** @function set_texture_stretching
     */
    m_class.set_function(
        "set_texture_stretching", member_function<&texture::set_texture_stretching>());

    /** @function set_texture
     */
    m_class.set_function(
        "set_texture",
        sol::overload(
            [](texture& m_self, const std::string& texture) {
                if (!texture.empty() && texture[0] == '#') {
                    // This is actually a color hash
                    m_self.set_solid_color(color(texture));
                } else {
                    // Normal texture file
                    m_self.set_texture(texture);
                }
            },
            [](texture& m_self, float f_r, float f_g, float f_b, sol::optional<float> f_a) {
                m_self.set_solid_color(color(f_r, f_g, f_b, f_a.value_or(1.0f)));
            }));

    /** @function set_vertex_color
     */
    m_class.set_function(
        "set_vertex_color",
        sol::overload(
            [](texture& m_self, const std::string& s) {
                m_self.set_vertex_color(color(s), std::numeric_limits<std::size_t>::max());
            },
            [](texture& m_self, float f_r, float f_g, float f_b, sol::optional<float> f_a) {
                m_self.set_vertex_color(
                    color(f_r, f_g, f_b, f_a.value_or(1.0f)),
                    std::numeric_limits<std::size_t>::max());
            },
            [](texture& m_self, std::size_t ui_index, const std::string& s) {
                m_self.set_vertex_color(color(s), ui_index);
            },
            [](texture& m_self, std::size_t ui_index, float f_r, float f_g, float f_b,
               sol::optional<float> f_a) {
                m_self.set_vertex_color(color(f_r, f_g, f_b, f_a.value_or(1.0f)), ui_index);
            }));
}

} // namespace lxgui::gui
