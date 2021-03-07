#ifndef LXGUI_INPUT_SDL_SOURCE_HPP
#define LXGUI_INPUT_SDL_SOURCE_HPP

#include <lxgui/utils.hpp>
#include <lxgui/input.hpp>

#include <SDL_events.h>
#include <chrono>

struct SDL_Window;

namespace lxgui {
namespace input {
namespace sdl
{
    class source : public source_impl
    {
    public :

        /// Initializes this input source.
        /** \param pWindow The window from which to receive input
        */
        explicit source(SDL_Window* pWindow, bool bMouseGrab = false);

        source(const source&) = delete;
        source& operator = (const source&) = delete;

        void toggle_mouse_grab() override;
        std::string get_key_name(key mKey) const;

        utils::ustring get_clipboard_content() override;
        void set_clipboard_content(const utils::ustring& sContent) override;

        void on_sdl_event(const SDL_Event& mEvent);

    protected :

        void update_() override;

    private :

        input::key from_sdl_(int iSDLKey) const;

        SDL_Window* pWindow_ = nullptr;

        bool bFirst_ = true;
        bool bMouseGrab_ = false;

        float fOldMouseX_ = 0.0f, fOldMouseY_ = 0.0f;
        float fWheelCache_ = 0.0f;

        using clock = std::chrono::high_resolution_clock;
        std::array<clock::time_point, MOUSE_BUTTON_NUMBER> lLastClickClock_;
    };
}
}
}

#endif
