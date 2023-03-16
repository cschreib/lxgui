#ifndef LXGUI_GUI_SDL_HPP
#define LXGUI_GUI_SDL_HPP

#include "lxgui/gui_manager.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"

struct SDL_Window;
struct SDL_Renderer;

namespace lxgui::gui::sdl {

/**
 * \brief Create a new gui::manager using a full SDL implementation.
 * \param win The SDL render window
 * \param rdr The SDL renderer
 * \param initialise_sdl_image Set to 'false' if SDL Image has already been initialised elsewhere
 * \return The new gui::manager instance
 */
utils::owner_ptr<gui::manager>
create_manager(SDL_Window* win, SDL_Renderer* rdr, bool initialise_sdl_image = true);

} // namespace lxgui::gui::sdl

#endif
