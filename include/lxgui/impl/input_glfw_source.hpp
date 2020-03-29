#ifndef INPUT_GLFW_SOURCE_HPP
#define INPUT_GLFW_SOURCE_HPP

#include <lxgui/utils.hpp>
#include <lxgui/input.hpp>

namespace input
{
    namespace glfw
    {
        class source : public source_impl
        {
        public :

            /// Initializes this handler.
            /** \param pWindow The window from which to receive input
            */
            explicit source(bool bMouseGrab = false);

            void toggle_mouse_grab();
            std::string get_key_name(key mKey) const;

            void update();

        private :

            int to_glfw_(key mKey) const;

            static const int lKeyToGLFW[106][2];

            bool bMouseGrab_;
            bool bFirst_;
            float fOldMouseX_, fOldMouseY_, fOldWheel_;
        };
    }
}

#endif
