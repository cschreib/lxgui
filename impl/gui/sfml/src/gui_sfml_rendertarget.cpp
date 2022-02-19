#include "lxgui/impl/gui_sfml_rendertarget.hpp"

#include "lxgui/impl/gui_sfml_renderer.hpp"

#include <iostream>

namespace lxgui::gui::sfml {

render_target::render_target(const vector2ui& dimensions, material::filter filt) {
    p_texture_ = std::make_shared<sfml::material>(dimensions, true, material::wrap::repeat, filt);

    p_render_texture_ = p_texture_->get_render_texture();
}

void render_target::begin() {}

void render_target::end() {
    p_render_texture_->display();
}

void render_target::clear(const color& c) {
    p_render_texture_->clear(sf::Color(c.r * 255, c.g * 255, c.b * 255, c.a * 255));
}

bounds2f render_target::get_rect() const {
    return p_texture_->get_rect();
}

vector2ui render_target::get_canvas_dimensions() const {
    return p_texture_->get_canvas_dimensions();
}

bool render_target::set_dimensions(const vector2ui& dimensions) {
    return p_texture_->set_dimensions(dimensions);
}

std::weak_ptr<sfml::material> render_target::get_material() {
    return p_texture_;
}

sf::RenderTexture* render_target::get_render_texture() {
    return p_render_texture_;
}

} // namespace lxgui::gui::sfml
