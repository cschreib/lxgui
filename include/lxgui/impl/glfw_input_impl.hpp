#ifndef GLFW_INPUT_IMPL_HPP
#define GLFW_INPUT_IMPL_HPP

#include <lxgui/utils.hpp>
#include <lxgui/input.hpp>

namespace input
{
    class glfw_handler : public handler_impl
    {
    public :

        /// Initializes this handler.
        /** \param pWindow The window from which to receive input
        */
        explicit glfw_handler(bool bMouseGrab = false);

        void toggle_mouse_grab();
        std::string get_key_name(key::code mKey) const;

        void update();

    private :

        int to_glfw_(key::code mKey) const;

        static const int lKeyToGLFW[106][2];

        bool bMouseGrab_;
        bool bFirst_;
        float fOldMouseX_, fOldMouseY_, fOldWheel_;
    };
}

#endif
