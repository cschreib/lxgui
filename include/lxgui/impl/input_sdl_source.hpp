#ifndef LXGUI_INPUT_SFML_SOURCE_HPP
#define LXGUI_INPUT_SFML_SOURCE_HPP

#include <lxgui/utils.hpp>
#include <lxgui/input.hpp>
#include <SFML/System/Clock.hpp>

namespace sf {
    class Window;
    class Event;
}

namespace lxgui {
namespace input {
namespace sfml
{
    class source : public source_impl
    {
    public :

        /// Initializes this input source.
        /** \param pWindow The window from which to receive input
        */
        explicit source(const sf::Window& pWindow, bool bMouseGrab = false);

        source(const source&) = delete;
        source& operator = (const source&) = delete;

        void toggle_mouse_grab() override;
        std::string get_key_name(key mKey) const;

        utils::ustring get_clipboard_content() override;
        void set_clipboard_content(const utils::ustring& sContent) override;

        void on_sfml_event(const sf::Event& mEvent);

    protected :

        void update_() override;

    private :

        input::key from_sfml_(int uiSFKey) const;

        const sf::Window& mWindow_;

        bool bMouseGrab_ = false;
        bool bFirst_ = true;

        float fOldMouseX_ = 0.0f, fOldMouseY_ = 0.0f;
        float fWheelCache_ = 0.0f;

        std::array<sf::Clock, MOUSE_BUTTON_NUMBER> lLastClickClock_;
    };
}
}
}

#endif
