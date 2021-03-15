#ifndef LXGUI_GUI_SDL_HPP
#define LXGUI_GUI_SDL_HPP

#include "lxgui/impl/gui_sdl_renderer.hpp"

struct SDL_Window;

namespace lxgui {
namespace gui {
namespace sdl
{
/// Create a new gui::manager using a full SDL implementation.
/** \param mWindow The SDL render window
*   \param sLocale The name of the game locale ("enGB", ...)
*   \return The new gui::manager instance
*/
std::unique_ptr<gui::manager> create_manager(SDL_Window* pWindow, const std::string& sLocale);
}
}
}

#endif
