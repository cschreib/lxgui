#include "lxgui/impl/gui_sfml_render_target.hpp"

#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"

#include <iostream>

namespace lxgui::gui::sfml {

render_target::render_target(const vector2ui& dimensions, material::filter filt) {
    texture_ = std::make_shared<sfml::material>(dimensions, true, material::wrap::repeat, filt);

    render_texture_ = texture_->get_render_texture();
}

void render_target::begin() {}

void render_target::end() {
    render_texture_->display();
}

void render_target::clear(const color& c) {
    render_texture_->clear(sf::Color(c.r * 255, c.g * 255, c.b * 255, c.a * 255));
}

bounds2f render_target::get_rect() const {
    return texture_->get_rect();
}

vector2ui render_target::get_canvas_dimensions() const {
    return texture_->get_canvas_dimensions();
}

bool render_target::set_dimensions(const vector2ui& dimensions) {
    return texture_->set_dimensions(dimensions);
}

void render_target::save_to_file(std::string filename) const {
    auto image = render_texture_->getTexture().copyToImage();

    // De-multiply alpha
    const std::size_t width  = image.getSize().x;
    const std::size_t height = image.getSize().y;
    for (std::size_t x = 0; x < width; ++x) {
        for (std::size_t y = 0; y < height; ++y) {
            sf::Color c = image.getPixel(sf::Vector2u(x, y));
            float     a = c.a / 255.0f;
            if (a > 0.0f) {
                c.r /= a;
                c.g /= a;
                c.b /= a;
            }
            image.setPixel(sf::Vector2u(x, y), c);
        }
    }

    if (!image.saveToFile(filename)) {
        gui::out << gui::warning << "sfml::render_target: "
                 << "Could not save render target to '" << filename << "'." << std::endl;
    }
}

std::weak_ptr<sfml::material> render_target::get_material() {
    return texture_;
}

sf::RenderTexture* render_target::get_render_texture() {
    return render_texture_;
}

} // namespace lxgui::gui::sfml
