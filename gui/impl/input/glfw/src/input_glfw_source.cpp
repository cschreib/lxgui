#include "lxgui/impl/glfw_input_impl.hpp"
#include <lxgui/utils_string.hpp>
#include <GL/glfw.h>

#ifndef MSVC
#warning "Note : GLFW input handler is not fully functional, since \
GLFW doesn't provide keyboard layout independent key codes."
#endif

namespace input {
namespace glfw
{
const int source::lKeyToGLFW[106][2] =
{
    //{key::K_UNKNOWN, GLFW_KEY_UNKNOWN},
    {key::K_SPACE, GLFW_KEY_SPACE},
    {key::K_Q, 'Q'},
    {key::K_W, 'W'},
    {key::K_E, 'E'},
    {key::K_R, 'R'},
    {key::K_T, 'T'},
    {key::K_Y, 'Y'},
    {key::K_U, 'U'},
    {key::K_I, 'I'},
    {key::K_O, 'O'},
    {key::K_P, 'P'},
    {key::K_A, 'A'},
    {key::K_S, 'S'},
    {key::K_D, 'D'},
    {key::K_F, 'F'},
    {key::K_G, 'G'},
    {key::K_H, 'H'},
    {key::K_J, 'J'},
    {key::K_K, 'K'},
    {key::K_L, 'L'},
    {key::K_Z, 'Z'},
    {key::K_X, 'X'},
    {key::K_C, 'C'},
    {key::K_V, 'V'},
    {key::K_B, 'B'},
    {key::K_N, 'N'},
    {key::K_M, 'M'},
    {key::K_0, '0'},
    {key::K_1, '1'},
    {key::K_2, '2'},
    {key::K_3, '3'},
    {key::K_4, '4'},
    {key::K_5, '5'},
    {key::K_6, '6'},
    {key::K_7, '7'},
    {key::K_8, '8'},
    {key::K_9, '9'},
    {key::K_MINUS, '-'},
    {key::K_EQUALS, '='},
    {key::K_LBRACKET, '['},
    {key::K_RBRACKET, ']'},
    {key::K_SEMICOLON, ';'},
    {key::K_APOSTROPHE, '\''},
    {key::K_GRAVE, '`'},
    {key::K_BACKSLASH, '\\'},
    {key::K_COMMA, ','},
    {key::K_PERIOD, '.'},
    {key::K_SLASH, '/'},
    {key::K_ESCAPE, GLFW_KEY_ESC},
    {key::K_F1, GLFW_KEY_F1},
    {key::K_F2, GLFW_KEY_F2},
    {key::K_F3, GLFW_KEY_F3},
    {key::K_F4, GLFW_KEY_F4},
    {key::K_F5, GLFW_KEY_F5},
    {key::K_F6, GLFW_KEY_F6},
    {key::K_F7, GLFW_KEY_F7},
    {key::K_F8, GLFW_KEY_F8},
    {key::K_F9, GLFW_KEY_F9},
    {key::K_F10, GLFW_KEY_F10},
    {key::K_F11, GLFW_KEY_F11},
    {key::K_F12, GLFW_KEY_F12},
    {key::K_F13, GLFW_KEY_F13},
    {key::K_F14, GLFW_KEY_F14},
    {key::K_F15, GLFW_KEY_F15},
    {key::K_UP, GLFW_KEY_UP},
    {key::K_DOWN, GLFW_KEY_DOWN},
    {key::K_LEFT, GLFW_KEY_LEFT},
    {key::K_RIGHT, GLFW_KEY_RIGHT},
    {key::K_LSHIFT, GLFW_KEY_LSHIFT},
    {key::K_RSHIFT, GLFW_KEY_RSHIFT},
    {key::K_LCONTROL, GLFW_KEY_LCTRL},
    {key::K_RCONTROL, GLFW_KEY_RCTRL},
    {key::K_LMENU, GLFW_KEY_LALT},
    {key::K_RMENU, GLFW_KEY_RALT},
    {key::K_TAB, GLFW_KEY_TAB},
    {key::K_RETURN, GLFW_KEY_ENTER},
    {key::K_BACK, GLFW_KEY_BACKSPACE},
    {key::K_INSERT, GLFW_KEY_INSERT},
    {key::K_DELETE, GLFW_KEY_DEL},
    {key::K_PGUP, GLFW_KEY_PAGEUP},
    {key::K_PGDOWN, GLFW_KEY_PAGEDOWN},
    {key::K_HOME, GLFW_KEY_HOME},
    {key::K_END, GLFW_KEY_END},
    {key::K_NUMPAD0, GLFW_KEY_KP_0},
    {key::K_NUMPAD1, GLFW_KEY_KP_1},
    {key::K_NUMPAD2, GLFW_KEY_KP_2},
    {key::K_NUMPAD3, GLFW_KEY_KP_3},
    {key::K_NUMPAD4, GLFW_KEY_KP_4},
    {key::K_NUMPAD5, GLFW_KEY_KP_5},
    {key::K_NUMPAD6, GLFW_KEY_KP_6},
    {key::K_NUMPAD7, GLFW_KEY_KP_7},
    {key::K_NUMPAD8, GLFW_KEY_KP_8},
    {key::K_NUMPAD9, GLFW_KEY_KP_9},
    {key::K_DIVIDE, GLFW_KEY_KP_DIVIDE},
    {key::K_MULTIPLY, GLFW_KEY_KP_MULTIPLY},
    {key::K_SUBTRACT, GLFW_KEY_KP_SUBTRACT},
    {key::K_ADD, GLFW_KEY_KP_ADD},
    {key::K_DECIMAL, GLFW_KEY_KP_DECIMAL},
    {key::K_NUMPADENTER, GLFW_KEY_KP_ENTER},
    {key::K_NUMLOCK, GLFW_KEY_KP_NUM_LOCK},
    {key::K_CAPITAL, GLFW_KEY_CAPS_LOCK},
    {key::K_SCROLL, GLFW_KEY_SCROLL_LOCK},
    {key::K_PAUSE, GLFW_KEY_PAUSE},
    {key::K_LWIN, GLFW_KEY_LSUPER},
    {key::K_RWIN, GLFW_KEY_RSUPER},
    {key::K_APPS, GLFW_KEY_MENU}
};

source::source(bool bMouseGrab) :
    bMouseGrab_(bMouseGrab), bFirst_(true)
{
    mMouse.bHasDelta = true;
}

int source::to_glfw_(key mKey) const
{
    for (size_t i = 0; i < 106; ++i)
    {
        if (lKeyToGLFW[i][0] == mKey)
            return lKeyToGLFW[i][1];
    }

    return GLFW_KEY_UNKNOWN;
}

void source::toggle_mouse_grab()
{
    bMouseGrab_ = !bMouseGrab_;
#ifndef MSVC
#warning "Note : GLFW 2.x does not support mouse grab switching. \
The mouse is always grabbed if using fullscreen mode, and \
never grabbed if using windowed mode."
#endif
}

void source::update()
{
    for (int i = 0; i < 106; ++i)
        mKeyboard.lKeyState[lKeyToGLFW[i][0]] = glfwGetKey(lKeyToGLFW[i][1]);

    int width, height;
    glfwGetWindowSize(&width, &height);

    if (bFirst_)
    {
        int mx, my;
        glfwGetMousePos(&mx, &my);
        mMouse.fAbsX = mx;
        mMouse.fAbsY = my;
        mMouse.fRelX = mx/float(width);
        mMouse.fRelY = my/float(height);

        mMouse.fDX = mMouse.fDY = mMouse.fRelDX = mMouse.fRelDY = 0.0f;
        bFirst_ = false;

        fOldMouseX_ = mMouse.fAbsX;
        fOldMouseY_ = mMouse.fAbsY;
    }
    else
    {
        int mx, my;
        glfwGetMousePos(&mx, &my);
        mMouse.fDX = mx - fOldMouseX_;
        mMouse.fDY = my - fOldMouseY_;
        mMouse.fRelDX = mMouse.fDX/width;
        mMouse.fRelDY = mMouse.fDY/height;

        mMouse.fAbsX += mMouse.fDX;
        mMouse.fAbsY += mMouse.fDY;
        mMouse.fRelX = mMouse.fAbsX/width;
        mMouse.fRelY = mMouse.fAbsY/height;

        fOldMouseX_ = mMouse.fAbsX;
        fOldMouseY_ = mMouse.fAbsY;
    }

    int wheel = glfwGetMouseWheel();
    mMouse.fRelWheel = wheel - fOldWheel_;
    fOldWheel_ = wheel;

    static const int lMouseToGLFW[3] = {GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE};
    for (int i = 0; i < INPUT_MOUSE_BUTTON_NUMBER; ++i)
        mMouse.lButtonState[i] = glfwGetMouseButton(lMouseToGLFW[i]) == GLFW_PRESS;
}
}
}
