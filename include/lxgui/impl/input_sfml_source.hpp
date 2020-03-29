#ifndef INPUT_SFML_SOURCE_HPP
#define INPUT_SFML_SOURCE_HPP

#include <lxgui/utils.hpp>
#include <lxgui/input.hpp>

namespace sf {
    class Window;
    class Event;
}

namespace input
{
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

            void toggle_mouse_grab();
            std::string get_key_name(key mKey) const;

            void on_sfml_event(const sf::Event& mEvent);

        protected :

            void update_() override;

        private :

            int to_sfml_(key mKey) const;

            struct key_mapping
            {
                input::key mKey;
                int        mSFKey;
            };

            static const key_mapping lKeyToSFML[100];

            const sf::Window& mWindow_;

            bool bMouseGrab_;
            bool bFirst_;

            float fOldMouseX_, fOldMouseY_;
            float fWheelCache_;
        };
    }
}

#endif
