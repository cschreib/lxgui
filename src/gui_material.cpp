#include "lxgui/gui_material.hpp"

namespace lxgui::gui {

material::material(bool is_atlas) : is_atlas_(is_atlas) {}

vector2f material::get_canvas_uv(const vector2f& texture_uv, bool from_normalized) const {
    const bounds2f quad = get_rect();

    vector2f pixel_uv = texture_uv;
    if (from_normalized)
        pixel_uv *= quad.dimensions();

    pixel_uv += quad.top_left();
    pixel_uv /= vector2f(get_canvas_dimensions());

    return pixel_uv;
}

vector2f material::get_local_uv(const vector2f& canvas_uv, bool as_normalized) const {
    const bounds2f quad = get_rect();

    vector2f pixel_uv = canvas_uv;
    pixel_uv *= vector2f(get_canvas_dimensions());
    pixel_uv -= quad.top_left();

    if (as_normalized)
        pixel_uv /= quad.dimensions();

    return pixel_uv;
}

bool material::is_in_atlas() const {
    return is_atlas_;
}

} // namespace lxgui::gui
