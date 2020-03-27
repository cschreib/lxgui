#include "lxgui/impl/gui_sfml.hpp"

#include <lxgui/impl/sfml_input_impl.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

namespace gui {
namespace sfml
{
std::unique_ptr<gui::manager> create_manager(sf::RenderWindow& mWindow, const std::string& sLocale)
{
    return std::unique_ptr<gui::manager>(new gui::manager(
        std::unique_ptr<input::sfml_manager>(new input::sfml_manager(mWindow)),
        sLocale, mWindow.getSize().x, mWindow.getSize().y,
        std::unique_ptr<gui::renderer_impl>(new gui::sfml::renderer(mWindow))
    ));
}
}
}
