#ifndef LXGUI_INPUT_SFML_SOURCE_HPP
#define LXGUI_INPUT_SFML_SOURCE_HPP

#include <lxgui/utils.hpp>
#include <lxgui/input_source.hpp>
#include <lxgui/gui_vector2.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Cursor.hpp>

#include <unordered_map>
#include <memory>

namespace sf {
    class Window;
    class Event;
}

namespace lxgui {
namespace input {
namespace sfml
{
    class source final : public input::source
    {
    public :

        /// Initializes this input source.
        /** \param pWindow The window from which to receive input
        *   \param bMouseGrab Set to 'true' to turn on mouse grab (locked on screen center)
        */
        explicit source(sf::Window& pWindow, bool bMouseGrab = false);

        source(const source&) = delete;
        source& operator = (const source&) = delete;

        void toggle_mouse_grab() override;
        std::string get_key_name(key mKey) const;

        utils::ustring get_clipboard_content() override;
        void set_clipboard_content(const utils::ustring& sContent) override;

        void on_sfml_event(const sf::Event& mEvent);

        void set_mouse_cursor(const std::string& sFileName, const gui::vector2i& mHotSpot) override;
        void reset_mouse_cursor() override;

    protected :

        void update_() override;

    private :

        input::key from_sfml_(int uiSFKey) const;

        sf::Window& mWindow_;

        bool bMouseGrab_ = false;
        bool bFirst_ = true;

        gui::vector2f mOldMousePos_;
        float fWheelCache_ = 0.0f;

        std::array<sf::Clock, MOUSE_BUTTON_NUMBER> lLastClickClock_;

        std::unordered_map<std::string,std::unique_ptr<sf::Cursor>> lCursorMap_;
    };
}
}
}

#endif
