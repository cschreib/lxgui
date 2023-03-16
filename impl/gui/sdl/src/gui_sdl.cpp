#include "lxgui/impl/gui_sdl.hpp"

#include "lxgui/gui_manager.hpp"
#include "lxgui/impl/input_sdl_source.hpp"

namespace lxgui::gui::sdl {

utils::owner_ptr<gui::manager>
create_manager(SDL_Window* win, SDL_Renderer* renderer, bool initialise_sdl_image) {
    return utils::make_owned<gui::manager>(
        std::unique_ptr<input::source>(new input::sdl::source(win, renderer, initialise_sdl_image)),
        std::unique_ptr<gui::renderer>(new gui::sdl::renderer(renderer, false)));
}

} // namespace lxgui::gui::sdl
