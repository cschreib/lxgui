#ifndef LXGUI_GUI_SFML_HPP
#define LXGUI_GUI_SFML_HPP

#include "lxgui/gui_manager.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"

/** \cond INCLUDE_INTERNALS_IN_DOC
 */
namespace sf {

class RenderWindow;

}
/** \endcond
 */

namespace lxgui::gui::sfml {

/**
 * \brief Create a new gui::manager using a full SFML implementation.
 * \param win The SFML render window
 * \return The new gui::manager instance
 */
utils::owner_ptr<gui::manager> create_manager(sf::RenderWindow& win);

} // namespace lxgui::gui::sfml

#endif
