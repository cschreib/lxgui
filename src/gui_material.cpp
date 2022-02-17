#include "lxgui/gui_material.hpp"

namespace lxgui::gui {

material::material(bool is_atlas) : is_atlas_(is_atlas) {}

vector2f material::get_canvas_uv(const vector2f& m_texture_uv, bool from_normalized) const {
    const bounds2f m_quad = get_rect();

    vector2f m_pixel_uv = m_texture_uv;
    if (from_normalized)
        m_pixel_uv *= m_quad.dimensions();

    m_pixel_uv += m_quad.top_left();
    m_pixel_uv /= vector2f(get_canvas_dimensions());

    return m_pixel_uv;
}

vector2f material::get_local_uv(const vector2f& m_canvas_uv, bool as_normalized) const {
    const bounds2f m_quad = get_rect();

    vector2f m_pixel_uv = m_canvas_uv;
    m_pixel_uv *= vector2f(get_canvas_dimensions());
    m_pixel_uv -= m_quad.top_left();

    if (as_normalized)
        m_pixel_uv /= m_quad.dimensions();

    return m_pixel_uv;
}

bool material::is_in_atlas() const {
    return is_atlas_;
}

} // namespace lxgui::gui
