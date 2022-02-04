#ifndef LXGUI_GUI_SDL_HPP
#define LXGUI_GUI_SDL_HPP

#include "lxgui/gui_manager.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"

struct SDL_Window;
struct SDL_Renderer;

namespace lxgui::gui::sdl {

/// Create a new gui::manager using a full SDL implementation.
/** \param pWindow The SDL render window
 *   \param pRenderer The SDL renderer
 *   \param bInitialiseSDLImage Set to 'false' if SDL Image has already been initialised elsewhere
 *   \return The new gui::manager instance
 */
utils::owner_ptr<gui::manager>
create_manager(SDL_Window* pWindow, SDL_Renderer* pRenderer, bool bInitialiseSDLImage = true);

} // namespace lxgui::gui::sdl

#endif
