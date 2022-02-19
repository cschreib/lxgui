#include "lxgui/impl/gui_sfml_atlas.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui::sfml {

atlas_page::atlas_page(gui::renderer& rdr, material::filter filt) : gui::atlas_page(filt) {
    const std::size_t ui_size = rdr.get_texture_atlas_page_size();

    if (!texture_.create(ui_size, ui_size)) {
        throw gui::exception(
            "gui::sfml::atlas_page", "Could not create texture with dimensions " +
                                         utils::to_string(ui_size) + " x " +
                                         utils::to_string(ui_size) + ".");
    }

    texture_.setSmooth(filt == material::filter::linear);
}

std::shared_ptr<gui::material>
atlas_page::add_material_(const gui::material& mat, const bounds2f& location) {
    const sfml::material& sf_mat = static_cast<const sfml::material&>(mat);

    const sf::Image image = sf_mat.get_texture()->copyToImage();
    texture_.update(image, location.left, location.top);

    return std::make_shared<sfml::material>(texture_, location, filter_);
}

float atlas_page::get_width_() const {
    return texture_.getSize().x;
}

float atlas_page::get_height_() const {
    return texture_.getSize().y;
}

atlas::atlas(renderer& rdr, material::filter filt) : gui::atlas(rdr, filt) {}

std::unique_ptr<gui::atlas_page> atlas::create_page_() {
    return std::make_unique<sfml::atlas_page>(renderer_, filter_);
}

} // namespace lxgui::gui::sfml
