#include "lxgui/impl/gui_sfml_material.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui::sfml {

material::material(
    const vector2ui& m_dimensions, bool is_render_target, wrap m_wrap, filter m_filter) :
    gui::material(false),
    m_dimensions_(m_dimensions),
    m_canvas_dimensions_(m_dimensions),
    m_wrap_(m_wrap),
    m_filter_(m_filter) {
    if (m_dimensions_.x > sf::Texture::getMaximumSize() ||
        m_dimensions_.y > sf::Texture::getMaximumSize()) {
        throw gui::exception(
            "gui::sfml::material", "Texture dimensions not supported by graphics card : (" +
                                       utils::to_string(m_dimensions_.x) + " x " +
                                       utils::to_string(m_dimensions_.y) + ").");
    }

    is_render_target_ = is_render_target;

    if (is_render_target_) {
        if (!m_render_texture_.create(m_dimensions_.x, m_dimensions_.y)) {
            throw gui::exception(
                "gui::sfml::material", "Could not create render target with dimensions " +
                                           utils::to_string(m_dimensions_.x) + " x " +
                                           utils::to_string(m_dimensions_.y) + ".");
        }
        m_render_texture_.setSmooth(m_filter == filter::linear);
        m_render_texture_.setRepeated(m_wrap == wrap::repeat);
    } else {
        if (!m_texture_.create(m_dimensions_.x, m_dimensions_.y)) {
            throw gui::exception(
                "gui::sfml::material", "Could not create texture with dimensions " +
                                           utils::to_string(m_dimensions_.x) + " x " +
                                           utils::to_string(m_dimensions_.y) + ".");
        }
        m_texture_.setSmooth(m_filter == filter::linear);
        m_texture_.setRepeated(m_wrap == wrap::repeat);
    }

    m_rect_ = bounds2f(0, m_dimensions_.x, 0, m_dimensions_.y);
}

material::material(const sf::Image& m_data, wrap m_wrap, filter m_filter) : gui::material(false) {
    is_render_target_ = false;
    m_texture_.loadFromImage(m_data);
    m_texture_.setSmooth(m_filter == filter::linear);
    m_texture_.setRepeated(m_wrap == wrap::repeat);

    m_dimensions_        = vector2ui(m_texture_.getSize().x, m_texture_.getSize().y);
    m_canvas_dimensions_ = m_dimensions_;
    m_wrap_              = m_wrap;
    m_filter_            = m_filter;

    m_rect_ = bounds2f(0, m_dimensions_.x, 0, m_dimensions_.y);
}

material::material(const std::string& file_name, wrap m_wrap, filter m_filter) :
    gui::material(false) {
    is_render_target_ = false;
    sf::Image m_data;
    if (!m_data.loadFromFile(file_name))
        throw utils::exception("gui::sfml::material", "loading failed: '" + file_name + "'.");

    premultiply_alpha(m_data);
    m_texture_.loadFromImage(m_data);
    m_texture_.setSmooth(m_filter == filter::linear);
    m_texture_.setRepeated(m_wrap == wrap::repeat);

    m_dimensions_        = vector2ui(m_texture_.getSize().x, m_texture_.getSize().y);
    m_canvas_dimensions_ = m_dimensions_;
    m_wrap_              = m_wrap;
    m_filter_            = m_filter;

    m_rect_ = bounds2f(0, m_dimensions_.x, 0, m_dimensions_.y);
}

material::material(const sf::Texture& m_texture, const bounds2f& m_location, filter m_filter) :
    gui::material(true) {
    m_rect_          = m_location;
    m_filter_        = m_filter;
    p_atlas_texture_ = &m_texture;
}

void material::set_wrap(wrap m_wrap) {
    if (p_atlas_texture_) {
        throw gui::exception(
            "gui::sfml::material", "A material in an atlas cannot change its wrapping mode.");
    }

    m_wrap_ = m_wrap;

    if (is_render_target_)
        m_render_texture_.setRepeated(m_wrap == wrap::repeat);
    else
        m_texture_.setRepeated(m_wrap == wrap::repeat);
}

void material::set_filter(filter m_filter) {
    if (p_atlas_texture_) {
        throw gui::exception(
            "gui::sfml::material", "A material in an atlas cannot change its filtering.");
    }

    m_filter_ = m_filter;

    if (is_render_target_)
        m_render_texture_.setSmooth(m_filter == filter::linear);
    else
        m_texture_.setSmooth(m_filter == filter::linear);
}

material::filter material::get_filter() const {
    return m_filter_;
}

void material::update_texture(const ub32color* p_data) {
    if (is_render_target_)
        throw gui::exception("gui::sfml::material", "A render texture cannot be updated.");

    if (p_atlas_texture_)
        throw gui::exception("gui::sfml::material", "A material in an atlas cannot be updated.");

    m_texture_.update(
        reinterpret_cast<const sf::Uint8*>(p_data), m_rect_.width(), m_rect_.height(), m_rect_.left,
        m_rect_.top);
}

void material::premultiply_alpha(sf::Image& m_data) {
    const std::size_t ui_width  = m_data.getSize().x;
    const std::size_t ui_height = m_data.getSize().y;
    for (std::size_t x = 0; x < ui_width; ++x)
        for (std::size_t y = 0; y < ui_height; ++y) {
            sf::Color c = m_data.getPixel(x, y);
            float     a = c.a / 255.0f;
            c.r *= a;
            c.g *= a;
            c.b *= a;
            m_data.setPixel(x, y, c);
        }
}

bounds2f material::get_rect() const {
    return m_rect_;
}

vector2ui material::get_canvas_dimensions() const {
    if (p_atlas_texture_)
        return vector2ui(p_atlas_texture_->getSize().x, p_atlas_texture_->getSize().y);
    else
        return m_canvas_dimensions_;
}

bool material::uses_same_texture(const gui::material& m_other) const {
    return p_atlas_texture_ &&
           p_atlas_texture_ == static_cast<const sfml::material&>(m_other).p_atlas_texture_;
}

bool material::set_dimensions(const vector2ui& m_dimensions) {
    if (p_atlas_texture_) {
        throw gui::exception("gui::sfml::material", "A material in an atlas cannot be resized.");
    }

    if (!is_render_target_)
        return false;

    if (m_dimensions.x > sf::Texture::getMaximumSize() ||
        m_dimensions.y > sf::Texture::getMaximumSize())
        return false;

    m_dimensions_ = m_dimensions;
    m_rect_       = bounds2f(0, m_dimensions_.x, 0, m_dimensions_.y);

    if (m_dimensions_.x > m_canvas_dimensions_.x || m_dimensions_.y > m_canvas_dimensions_.y) {
        // SFML is not efficient at resizing render texture, so use an exponential growth pattern
        // to avoid re-allocating a new render texture on every resize operation.
        if (m_dimensions_.x > m_canvas_dimensions_.x)
            m_canvas_dimensions_.x = m_dimensions_.x + m_dimensions_.x / 2;
        if (m_dimensions_.y > m_canvas_dimensions_.y)
            m_canvas_dimensions_.y = m_dimensions_.y + m_dimensions_.y / 2;

        if (!m_render_texture_.create(m_canvas_dimensions_.x, m_canvas_dimensions_.y)) {
            throw gui::exception(
                "gui::sfml::material", "Could not create render target with dimensions " +
                                           utils::to_string(m_canvas_dimensions_.x) + " x " +
                                           utils::to_string(m_canvas_dimensions_.y) + ".");
        }

        m_render_texture_.setSmooth(m_filter_ == filter::linear);
        m_render_texture_.setRepeated(m_wrap_ == wrap::repeat);

        return true;
    } else {
        return false;
    }
}

sf::RenderTexture* material::get_render_texture() {
    if (!is_render_target_)
        return nullptr;

    return &m_render_texture_;
}

const sf::Texture* material::get_texture() const {
    if (is_render_target_)
        return &m_render_texture_.getTexture();
    else if (p_atlas_texture_)
        return p_atlas_texture_;
    else
        return &m_texture_;
}

} // namespace lxgui::gui::sfml
