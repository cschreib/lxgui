#include "lxgui/impl/gui_sdl.hpp"

#include <lxgui/impl/input_sdl_source.hpp>
#include <lxgui/gui_manager.hpp>

namespace lxgui {
namespace gui {
namespace sdl
{
std::unique_ptr<gui::manager> create_manager(SDL_Window* pWindow, SDL_Renderer* pRenderer,
    const std::string& sLocale, bool bInitialiseSDLImage)
{
    int iWidth = 0, iHeight = 0;
    SDL_GetWindowSize(pWindow, &iWidth, &iHeight);

    return std::unique_ptr<gui::manager>(new gui::manager(
        std::unique_ptr<input::source_impl>(new input::sdl::source(pWindow, bInitialiseSDLImage)),
        sLocale, iWidth, iHeight,
        std::unique_ptr<gui::renderer_impl>(new gui::sdl::renderer(pRenderer, false))
    ));
}
}
}
}
