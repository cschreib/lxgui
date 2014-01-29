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
    class sfml_handler : public handler_impl
    {
    public :

        /// Initializes this handler.
        /** \param pWindow The window from which to receive input
        */
        explicit sfml_handler(const sf::Window& pWindow, bool bMouseGrab = false);

        #ifndef NO_CPP11_DELETE_FUNCTION
        sfml_handler(const sfml_handler&) = delete;
        sfml_handler& operator = (const sfml_handler&) = delete;
        #else
    private :
        sfml_handler(const sfml_handler&);
        sfml_handler& operator = (const sfml_handler&);

    public :
        #endif

        void toggle_mouse_grab();
        std::string get_key_name(key::code mKey) const;

        void update();
        void on_sfml_event(const sf::Event& mEvent);

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
