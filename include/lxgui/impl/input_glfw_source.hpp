#ifndef LXGUI_INPUT_GLFW_SOURCE_HPP
#define LXGUI_INPUT_GLFW_SOURCE_HPP

#include <lxgui/utils.hpp>
#include <lxgui/input.hpp>

namespace lxgui {
namespace input {
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

        bool bMouseGrab_ = false;
        bool bFirst_ = true;
        float fOldMouseX_ = 0.0f, fOldMouseY_ = 0.0f, fOldWheel_ = 0.0f;
    };
}
}
}

#endif
