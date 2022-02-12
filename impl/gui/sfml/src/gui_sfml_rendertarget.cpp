#include "lxgui/impl/gui_sfml_rendertarget.hpp"

#include "lxgui/impl/gui_sfml_renderer.hpp"

#include <iostream>

namespace lxgui::gui::sfml {

render_target::render_target(const vector2ui& m_dimensions, material::filter m_filter) {
    p_texture_ =
        std::make_shared<sfml::material>(m_dimensions, true, material::wrap::repeat, m_filter);

    p_render_texture_ = p_texture_->get_render_texture();
}

void render_target::begin() {}

void render_target::end() {
    p_render_texture_->display();
}

void render_target::clear(const color& m_color) {
    p_render_texture_->clear(
        sf::Color(m_color.r * 255, m_color.g * 255, m_color.b * 255, m_color.a * 255));
}

bounds2f render_target::get_rect() const {
    return p_texture_->get_rect();
}

vector2ui render_target::get_canvas_dimensions() const {
    return p_texture_->get_canvas_dimensions();
}

bool render_target::set_dimensions(const vector2ui& m_dimensions) {
    return p_texture_->set_dimensions(m_dimensions);
}

std::weak_ptr<sfml::material> render_target::get_material() {
    return p_texture_;
}

sf::RenderTexture* render_target::get_render_texture() {
    return p_render_texture_;
}

} // namespace lxgui::gui::sfml
