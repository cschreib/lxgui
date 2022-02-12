#include "lxgui/impl/gui_sfml.hpp"

#include "lxgui/gui_manager.hpp"
#include "lxgui/impl/input_sfml_source.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

namespace lxgui::gui::sfml {

utils::owner_ptr<gui::manager> create_manager(sf::RenderWindow& m_window) {
    return utils::make_owned<gui::manager>(
        std::unique_ptr<input::source>(new input::sfml::source(m_window)),
        std::unique_ptr<gui::renderer>(new gui::sfml::renderer(m_window)));
}

} // namespace lxgui::gui::sfml
