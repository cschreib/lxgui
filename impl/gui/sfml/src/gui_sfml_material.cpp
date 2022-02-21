#include "lxgui/impl/gui_sfml_material.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui::sfml {

material::material(const vector2ui& dimensions, bool is_render_target, wrap wrp, filter filt) :
    gui::material(false),
    dimensions_(dimensions),
    canvas_dimensions_(dimensions),
    wrap_(wrp),
    filter_(filt) {
    if (dimensions_.x > sf::Texture::getMaximumSize() ||
        dimensions_.y > sf::Texture::getMaximumSize()) {
        throw gui::exception(
            "gui::sfml::material", "Texture dimensions not supported by graphics card: (" +
                                       utils::to_string(dimensions_.x) + " x " +
                                       utils::to_string(dimensions_.y) + ").");
    }

    is_render_target_ = is_render_target;

    if (is_render_target_) {
        if (!render_texture_.create(dimensions_.x, dimensions_.y)) {
            throw gui::exception(
                "gui::sfml::material", "Could not create render target with dimensions " +
                                           utils::to_string(dimensions_.x) + " x " +
                                           utils::to_string(dimensions_.y) + ".");
        }
        render_texture_.setSmooth(filt == filter::linear);
        render_texture_.setRepeated(wrp == wrap::repeat);
    } else {
        if (!texture_.create(dimensions_.x, dimensions_.y)) {
            throw gui::exception(
                "gui::sfml::material", "Could not create texture with dimensions " +
                                           utils::to_string(dimensions_.x) + " x " +
                                           utils::to_string(dimensions_.y) + ".");
        }
        texture_.setSmooth(filt == filter::linear);
        texture_.setRepeated(wrp == wrap::repeat);
    }

    rect_ = bounds2f(0, dimensions_.x, 0, dimensions_.y);
}

material::material(const sf::Image& data, wrap wrp, filter filt) : gui::material(false) {
    is_render_target_ = false;
    texture_.loadFromImage(data);
    texture_.setSmooth(filt == filter::linear);
    texture_.setRepeated(wrp == wrap::repeat);

    dimensions_        = vector2ui(texture_.getSize().x, texture_.getSize().y);
    canvas_dimensions_ = dimensions_;
    wrap_              = wrp;
    filter_            = filt;

    rect_ = bounds2f(0, dimensions_.x, 0, dimensions_.y);
}

material::material(const std::string& file_name, wrap wrp, filter filt) : gui::material(false) {
    is_render_target_ = false;
    sf::Image data;
    if (!data.loadFromFile(file_name))
        throw utils::exception("gui::sfml::material", "loading failed: '" + file_name + "'.");

    premultiply_alpha(data);
    texture_.loadFromImage(data);
    texture_.setSmooth(filt == filter::linear);
    texture_.setRepeated(wrp == wrap::repeat);

    dimensions_        = vector2ui(texture_.getSize().x, texture_.getSize().y);
    canvas_dimensions_ = dimensions_;
    wrap_              = wrp;
    filter_            = filt;

    rect_ = bounds2f(0, dimensions_.x, 0, dimensions_.y);
}

material::material(const sf::Texture& texture, const bounds2f& location, filter filt) :
    gui::material(true) {
    rect_          = location;
    filter_        = filt;
    atlas_texture_ = &texture;
}

void material::set_wrap(wrap wrp) {
    if (atlas_texture_) {
        throw gui::exception(
            "gui::sfml::material", "A material in an atlas cannot change its wrapping mode.");
    }

    wrap_ = wrp;

    if (is_render_target_)
        render_texture_.setRepeated(wrp == wrap::repeat);
    else
        texture_.setRepeated(wrp == wrap::repeat);
}

void material::set_filter(filter filt) {
    if (atlas_texture_) {
        throw gui::exception(
            "gui::sfml::material", "A material in an atlas cannot change its filtering.");
    }

    filter_ = filt;

    if (is_render_target_)
        render_texture_.setSmooth(filt == filter::linear);
    else
        texture_.setSmooth(filt == filter::linear);
}

material::filter material::get_filter() const {
    return filter_;
}

void material::update_texture(const ub32color* data) {
    if (is_render_target_)
        throw gui::exception("gui::sfml::material", "A render texture cannot be updated.");

    if (atlas_texture_)
        throw gui::exception("gui::sfml::material", "A material in an atlas cannot be updated.");

    texture_.update(
        reinterpret_cast<const sf::Uint8*>(data), rect_.width(), rect_.height(), rect_.left,
        rect_.top);
}

void material::premultiply_alpha(sf::Image& data) {
    const std::size_t width  = data.getSize().x;
    const std::size_t height = data.getSize().y;
    for (std::size_t x = 0; x < width; ++x)
        for (std::size_t y = 0; y < height; ++y) {
            sf::Color c = data.getPixel(x, y);
            float     a = c.a / 255.0f;
            c.r *= a;
            c.g *= a;
            c.b *= a;
            data.setPixel(x, y, c);
        }
}

bounds2f material::get_rect() const {
    return rect_;
}

vector2ui material::get_canvas_dimensions() const {
    if (atlas_texture_)
        return vector2ui(atlas_texture_->getSize().x, atlas_texture_->getSize().y);
    else
        return canvas_dimensions_;
}

bool material::uses_same_texture(const gui::material& other) const {
    return atlas_texture_ &&
           atlas_texture_ == static_cast<const sfml::material&>(other).atlas_texture_;
}

bool material::set_dimensions(const vector2ui& dimensions) {
    if (atlas_texture_) {
        throw gui::exception("gui::sfml::material", "A material in an atlas cannot be resized.");
    }

    if (!is_render_target_)
        return false;

    if (dimensions.x > sf::Texture::getMaximumSize() ||
        dimensions.y > sf::Texture::getMaximumSize())
        return false;

    dimensions_ = dimensions;
    rect_       = bounds2f(0, dimensions_.x, 0, dimensions_.y);

    if (dimensions_.x > canvas_dimensions_.x || dimensions_.y > canvas_dimensions_.y) {
        // SFML is not efficient at resizing render texture, so use an exponential growth pattern
        // to avoid re-allocating a new render texture on every resize operation.
        if (dimensions_.x > canvas_dimensions_.x)
            canvas_dimensions_.x = dimensions_.x + dimensions_.x / 2;
        if (dimensions_.y > canvas_dimensions_.y)
            canvas_dimensions_.y = dimensions_.y + dimensions_.y / 2;

        if (!render_texture_.create(canvas_dimensions_.x, canvas_dimensions_.y)) {
            throw gui::exception(
                "gui::sfml::material", "Could not create render target with dimensions " +
                                           utils::to_string(canvas_dimensions_.x) + " x " +
                                           utils::to_string(canvas_dimensions_.y) + ".");
        }

        render_texture_.setSmooth(filter_ == filter::linear);
        render_texture_.setRepeated(wrap_ == wrap::repeat);

        return true;
    } else {
        return false;
    }
}

sf::RenderTexture* material::get_render_texture() {
    if (!is_render_target_)
        return nullptr;

    return &render_texture_;
}

const sf::Texture* material::get_texture() const {
    if (is_render_target_)
        return &render_texture_.getTexture();
    else if (atlas_texture_)
        return atlas_texture_;
    else
        return &texture_;
}

} // namespace lxgui::gui::sfml
