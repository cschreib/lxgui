#include "lxgui/impl/gui_sdl.hpp"

#include "lxgui/gui_manager.hpp"
#include "lxgui/impl/input_sdl_source.hpp"

namespace lxgui::gui::sdl {

utils::owner_ptr<gui::manager>
create_manager(SDL_Window* p_window, SDL_Renderer* p_renderer, bool initialise_sdl_image) {
    return utils::make_owned<gui::manager>(
        std::unique_ptr<input::source>(
            new input::sdl::source(p_window, p_renderer, initialise_sdl_image)),
        std::unique_ptr<gui::renderer>(new gui::sdl::renderer(p_renderer, false)));
}

} // namespace lxgui::gui::sdl
