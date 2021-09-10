#include "lxgui/impl/gui_sfml.hpp"

#include <lxgui/impl/input_sfml_source.hpp>
#include <lxgui/gui_manager.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

namespace lxgui {
namespace gui {
namespace sfml
{
std::unique_ptr<gui::manager> create_manager(sf::RenderWindow& mWindow)
{
    return std::unique_ptr<gui::manager>(new gui::manager(
        std::unique_ptr<input::source>(new input::sfml::source(mWindow)),
        std::unique_ptr<gui::renderer>(new gui::sfml::renderer(mWindow))
    ));
}
}
}
}
