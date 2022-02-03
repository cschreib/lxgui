#include "lxgui/impl/gui_sdl.hpp"

#include <lxgui/impl/input_sdl_source.hpp>
#include <lxgui/gui_manager.hpp>

namespace lxgui {
namespace gui {
namespace sdl
{
utils::owner_ptr<gui::manager> create_manager(SDL_Window* pWindow, SDL_Renderer* pRenderer,
    bool bInitialiseSDLImage)
{
    return utils::make_owned<gui::manager>(
        std::unique_ptr<input::source>(new input::sdl::source(pWindow, pRenderer, bInitialiseSDLImage)),
        std::unique_ptr<gui::renderer>(new gui::sdl::renderer(pRenderer, false))
    );
}
}
}
}
