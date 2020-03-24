#ifndef SFML_INPUT_IMPL_HPP
#define SFML_INPUT_IMPL_HPP

#include <lxgui/utils.hpp>
#include <lxgui/input.hpp>

namespace sf {
    class Window;
    class Event;
}

namespace input
{
    class sfml_manager : public manager_impl
    {
    public :

        /// Initializes this handler.
        /** \param pWindow The window from which to receive input
        */
        explicit sfml_manager(const sf::Window& pWindow, bool bMouseGrab = false);

        sfml_manager(const sfml_manager&) = delete;
        sfml_manager& operator = (const sfml_manager&) = delete;

        void toggle_mouse_grab();
        std::string get_key_name(key::code mKey) const;

        void on_sfml_event(const sf::Event& mEvent);

    protected :

        void update_() override;

    private :

        int to_sfml_(key::code mKey) const;

        static const int lKeyToSFML[100][2];

        const sf::Window& mWindow_;

        bool bMouseGrab_;
        bool bFirst_;

        float fOldMouseX_, fOldMouseY_;
        float fWheelCache_;
    };
}

#endif
