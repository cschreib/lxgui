#ifndef LXGUI_INPUT_SDL_SOURCE_HPP
#define LXGUI_INPUT_SDL_SOURCE_HPP

#include <lxgui/utils.hpp>
#include <lxgui/gui_vector2.hpp>
#include <lxgui/input.hpp>

#include <SDL_events.h>

#include <chrono>
#include <unordered_map>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Cursor;

namespace lxgui {
namespace input {
namespace sdl
{
    class source : public source_impl
    {
    public :

        /// Initializes this input source.
        /** \param pWindow The window from which to receive input
        *   \param pRenderer The SDL renderer, or null if using raw OpenGL
        *   \param bInitialiseSDLImage Set to 'true' if SDL Image has not been initialised yet
        */
        explicit source(SDL_Window* pWindow, SDL_Renderer* pRenderer, bool bInitialiseSDLImage,
            bool bMouseGrab = false);

        source(const source&) = delete;
        source& operator = (const source&) = delete;

        void toggle_mouse_grab() override;
        std::string get_key_name(key mKey) const;

        utils::ustring get_clipboard_content() override;
        void set_clipboard_content(const utils::ustring& sContent) override;

        void on_sdl_event(const SDL_Event& mEvent);

        void set_mouse_cursor(const std::string& sFileName, const gui::vector2i& mHotSpot) override;
        void reset_mouse_cursor() override;

        float get_interface_scaling_factor_hint() const;

    protected :

        void update_() override;

    private :

        gui::vector2ui get_window_pixel_size_() const;
        void update_pixel_per_unit_();
        input::key from_sdl_(int iSDLKey) const;

        SDL_Window* pWindow_ = nullptr;
        SDL_Renderer* pRenderer_ = nullptr;

        bool bFirst_ = true;
        bool bMouseGrab_ = false;
        float fPixelsPerUnit_ = 1.0f;

        float fOldMouseX_ = 0.0f, fOldMouseY_ = 0.0f;
        float fWheelCache_ = 0.0f;

        using clock = std::chrono::high_resolution_clock;
        std::array<clock::time_point, MOUSE_BUTTON_NUMBER> lLastClickClock_;

        using wrapped_cursor = std::unique_ptr<SDL_Cursor, void(*)(SDL_Cursor*)>;
        std::unordered_map<std::string,wrapped_cursor> lCursorMap_;
    };
}
}
}

#endif
