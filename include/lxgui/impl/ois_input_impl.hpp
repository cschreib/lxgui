#ifndef OIS_INPUT_IMPL_HPP
#define OIS_INPUT_IMPL_HPP

#include <lxgui/utils.hpp>
#include <lxgui/input.hpp>

namespace OIS {
    class InputManager;
    class Mouse;
    class Keyboard;
}

namespace input
{
    class ois_key_listener;

    class ois_handler : public handler_impl
    {
    public :

        /// Initializes this handler.
        /** \param sWindowHandle A string containing a formatted window handle
        *   \param fScreenWidth  The width of the window
        *   \param fScreenHeight The height of the window
        *   \param bMouseGrab    'true' to take full control of the mouse and hide it
        *   \note For more infos regarding the window handle, see ois documentation.
        */
        ois_handler(const std::string& sWindowHandle, float fScreenWidth, float fScreenHeight, bool bMouseGrab = false);

        /// Destructor.
        ~ois_handler();

        #ifndef NO_CPP11_DELETE_FUNCTION
        ois_handler(const ois_handler&) = delete;
        ois_handler& operator = (const ois_handler&) = delete;
        #else
    private :
        ois_handler(const ois_handler&);
        ois_handler& operator = (const ois_handler&);

    public :
        #endif

        void toggle_mouse_grab();

        void update();

    private :

        void create_ois_();
        void delete_ois_();

        float fScreenWidth_;
        float fScreenHeight_;

        std::string sWindowHandle_;
        bool        bMouseGrab_;

        float fOldMouseX_, fOldMouseY_;

        OIS::InputManager* pOISInputMgr_;
        OIS::Keyboard*     pKeyboard_;
        OIS::Mouse*        pMouse_;

        utils::refptr<ois_key_listener> pKeyListener_;
    };
}

#endif
