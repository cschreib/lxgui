#ifndef LXGUI_GUI_SFML_HPP
#define LXGUI_GUI_SFML_HPP

#include "lxgui/impl/gui_sfml_renderer.hpp"
#include "lxgui/gui_manager.hpp"

namespace sf
{
    class RenderWindow;
}

namespace lxgui {
namespace gui {
namespace sfml
{
    /// Create a new gui::manager using a full SFML implementation.
    /** \param mWindow The SFML render window
    *   \param sLocale The name of the game locale ("enGB", ...)
    *   \return The new gui::manager instance
    */
    std::unique_ptr<gui::manager> create_manager(sf::RenderWindow& mWindow, const std::string& sLocale);
}
}
}

#endif
