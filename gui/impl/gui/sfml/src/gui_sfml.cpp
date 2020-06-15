#include "lxgui/impl/gui_sfml.hpp"

#include <lxgui/impl/input_sfml_source.hpp>
#include <lxgui/gui_manager.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

namespace lxgui {
namespace gui {
namespace sfml
{
std::unique_ptr<gui::manager> create_manager(sf::RenderWindow& mWindow, const std::string& sLocale)
{
    return std::unique_ptr<gui::manager>(new gui::manager(
        std::unique_ptr<input::source_impl>(new input::sfml::source(mWindow)),
        sLocale, mWindow.getSize().x, mWindow.getSize().y,
        std::unique_ptr<gui::renderer_impl>(new gui::sfml::renderer(mWindow))
    ));
}
}
}
}
