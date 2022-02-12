#include "lxgui/impl/gui_sfml_atlas.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui::sfml {

atlas_page::atlas_page(gui::renderer& m_renderer, material::filter m_filter) :
    gui::atlas_page(m_filter) {
    const std::size_t ui_size = m_renderer.get_texture_atlas_page_size();

    if (!m_texture_.create(ui_size, ui_size)) {
        throw gui::exception(
            "gui::sfml::atlas_page", "Could not create texture with dimensions " +
                                         utils::to_string(ui_size) + " x " +
                                         utils::to_string(ui_size) + ".");
    }

    m_texture_.setSmooth(m_filter == material::filter::linear);
}

std::shared_ptr<gui::material>
atlas_page::add_material_(const gui::material& m_mat, const bounds2f& m_location) {
    const sfml::material& m_sf_mat = static_cast<const sfml::material&>(m_mat);

    const sf::Image m_image = m_sf_mat.get_texture()->copyToImage();
    m_texture_.update(m_image, m_location.left, m_location.top);

    return std::make_shared<sfml::material>(m_texture_, m_location, m_filter_);
}

float atlas_page::get_width_() const {
    return m_texture_.getSize().x;
}

float atlas_page::get_height_() const {
    return m_texture_.getSize().y;
}

atlas::atlas(renderer& m_renderer, material::filter m_filter) : gui::atlas(m_renderer, m_filter) {}

std::unique_ptr<gui::atlas_page> atlas::create_page_() {
    return std::make_unique<sfml::atlas_page>(m_renderer_, m_filter_);
}

} // namespace lxgui::gui::sfml
