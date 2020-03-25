#include "lxgui/impl/glfw_input_impl.hpp"
#include <lxgui/utils_string.hpp>
#include <GL/glfw.h>

#ifndef MSVC
#warning "Note : GLFW input handler is not fully functional, since \
GLFW doesn't provide keyboard layout independent key codes."
#endif

namespace input
{
const int glfw_handler::lKeyToGLFW[106][2] =
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

glfw_handler::glfw_handler(bool bMouseGrab) :
    bMouseGrab_(bMouseGrab), bFirst_(true)
{
    mMouse.bHasDelta = true;
}

int glfw_handler::to_glfw_(key mKey) const
{
    for (size_t i = 0; i < 106; ++i)
    {
        if (lKeyToGLFW[i][0] == mKey)
            return lKeyToGLFW[i][1];
    }

    return GLFW_KEY_UNKNOWN;
}

void glfw_handler::toggle_mouse_grab()
{
    bMouseGrab_ = !bMouseGrab_;
#ifndef MSVC
#warning "Note : GLFW 2.x does not support mouse grab switching. \
The mouse is always grabbed if using fullscreen mode, and \
never grabbed if using windowed mode."
#endif
}

#ifdef WIN32
#include <windows.h>

#ifndef VK_OEM_COMMA
    #define VK_OEM_PLUS     0xBB
    #define VK_OEM_COMMA    0xBC
    #define VK_OEM_MINUS    0xBD
    #define VK_OEM_PERIOD   0xBE
#endif

int to_vkey_(int key)
{
    switch (key)
    {
        default:                   return 0;
        case 'A':                  return 'A';
        case 'B':                  return 'B';
        case 'C':                  return 'C';
        case 'D':                  return 'D';
        case 'E':                  return 'E';
        case 'F':                  return 'F';
        case 'G':                  return 'G';
        case 'H':                  return 'H';
        case 'I':                  return 'I';
        case 'J':                  return 'J';
        case 'K':                  return 'K';
        case 'L':                  return 'L';
        case 'M':                  return 'M';
        case 'N':                  return 'N';
        case 'O':                  return 'O';
        case 'P':                  return 'P';
        case 'Q':                  return 'Q';
        case 'R':                  return 'R';
        case 'S':                  return 'S';
        case 'T':                  return 'T';
        case 'U':                  return 'U';
        case 'V':                  return 'V';
        case 'W':                  return 'W';
        case 'X':                  return 'X';
        case 'Y':                  return 'Y';
        case 'Z':                  return 'Z';
        case '0':                  return '0';
        case '1':                  return '1';
        case '2':                  return '2';
        case '3':                  return '3';
        case '4':                  return '4';
        case '5':                  return '5';
        case '6':                  return '6';
        case '7':                  return '7';
        case '8':                  return '8';
        case '9':                  return '9';
        case GLFW_KEY_ESC:         return VK_ESCAPE;
        case GLFW_KEY_LCTRL:       return VK_LCONTROL;
        case GLFW_KEY_LSHIFT:      return VK_LSHIFT;
        case GLFW_KEY_LALT:        return VK_LMENU;
        case GLFW_KEY_LSUPER:      return VK_LWIN;
        case GLFW_KEY_RCTRL:       return VK_RCONTROL;
        case GLFW_KEY_RSHIFT:      return VK_RSHIFT;
        case GLFW_KEY_RALT:        return VK_RMENU;
        case GLFW_KEY_RSUPER:      return VK_RWIN;
        case GLFW_KEY_MENU:        return VK_APPS;
        case '[':                  return VK_OEM_4;
        case ']':                  return VK_OEM_6;
        case ';':                  return VK_OEM_1;
        case ',':                  return VK_OEM_COMMA;
        case '.':                  return VK_OEM_PERIOD;
        case '\'':                 return VK_OEM_7;
        case '/':                  return VK_OEM_2;
        case '\\':                 return VK_OEM_5;
        case '~':                  return VK_OEM_3;
        case '=':                  return VK_OEM_PLUS;
        case '-':                  return VK_OEM_MINUS;
        case GLFW_KEY_SPACE:       return VK_SPACE;
        case GLFW_KEY_ENTER:       return VK_RETURN;
        case GLFW_KEY_BACKSPACE:   return VK_BACK;
        case GLFW_KEY_TAB:         return VK_TAB;
        case GLFW_KEY_PAGEUP:      return VK_PRIOR;
        case GLFW_KEY_PAGEDOWN:    return VK_NEXT;
        case GLFW_KEY_END:         return VK_END;
        case GLFW_KEY_HOME:        return VK_HOME;
        case GLFW_KEY_INSERT:      return VK_INSERT;
        case GLFW_KEY_DEL:         return VK_DELETE;
        case GLFW_KEY_KP_ADD:      return VK_ADD;
        case GLFW_KEY_KP_SUBTRACT: return VK_SUBTRACT;
        case GLFW_KEY_KP_MULTIPLY: return VK_MULTIPLY;
        case GLFW_KEY_KP_DIVIDE:   return VK_DIVIDE;
        case GLFW_KEY_LEFT:        return VK_LEFT;
        case GLFW_KEY_RIGHT:       return VK_RIGHT;
        case GLFW_KEY_UP:          return VK_UP;
        case GLFW_KEY_DOWN:        return VK_DOWN;
        case GLFW_KEY_KP_0:        return VK_NUMPAD0;
        case GLFW_KEY_KP_1:        return VK_NUMPAD1;
        case GLFW_KEY_KP_2:        return VK_NUMPAD2;
        case GLFW_KEY_KP_3:        return VK_NUMPAD3;
        case GLFW_KEY_KP_4:        return VK_NUMPAD4;
        case GLFW_KEY_KP_5:        return VK_NUMPAD5;
        case GLFW_KEY_KP_6:        return VK_NUMPAD6;
        case GLFW_KEY_KP_7:        return VK_NUMPAD7;
        case GLFW_KEY_KP_8:        return VK_NUMPAD8;
        case GLFW_KEY_KP_9:        return VK_NUMPAD9;
        case GLFW_KEY_F1:          return VK_F1;
        case GLFW_KEY_F2:          return VK_F2;
        case GLFW_KEY_F3:          return VK_F3;
        case GLFW_KEY_F4:          return VK_F4;
        case GLFW_KEY_F5:          return VK_F5;
        case GLFW_KEY_F6:          return VK_F6;
        case GLFW_KEY_F7:          return VK_F7;
        case GLFW_KEY_F8:          return VK_F8;
        case GLFW_KEY_F9:          return VK_F9;
        case GLFW_KEY_F10:         return VK_F10;
        case GLFW_KEY_F11:         return VK_F11;
        case GLFW_KEY_F12:         return VK_F12;
        case GLFW_KEY_F13:         return VK_F13;
        case GLFW_KEY_F14:         return VK_F14;
        case GLFW_KEY_F15:         return VK_F16;
        case GLFW_KEY_PAUSE:       return VK_PAUSE;
    }
}

#else
#include <X11/Xlib.h>
#include <X11/keysym.h>

KeySym to_xkey_(int key)
{
    switch (key)
    {
        case 'A':                  return XK_A;
        case 'B':                  return XK_B;
        case 'C':                  return XK_C;
        case 'D':                  return XK_D;
        case 'E':                  return XK_E;
        case 'F':                  return XK_F;
        case 'G':                  return XK_G;
        case 'H':                  return XK_H;
        case 'I':                  return XK_I;
        case 'J':                  return XK_J;
        case 'K':                  return XK_K;
        case 'L':                  return XK_L;
        case 'M':                  return XK_M;
        case 'N':                  return XK_N;
        case 'O':                  return XK_O;
        case 'P':                  return XK_P;
        case 'Q':                  return XK_Q;
        case 'R':                  return XK_R;
        case 'S':                  return XK_S;
        case 'T':                  return XK_T;
        case 'U':                  return XK_U;
        case 'V':                  return XK_V;
        case 'W':                  return XK_W;
        case 'X':                  return XK_X;
        case 'Y':                  return XK_Y;
        case 'Z':                  return XK_Z;
        case '0':                  return XK_0;
        case '1':                  return XK_1;
        case '2':                  return XK_2;
        case '3':                  return XK_3;
        case '4':                  return XK_4;
        case '5':                  return XK_5;
        case '6':                  return XK_6;
        case '7':                  return XK_7;
        case '8':                  return XK_8;
        case '9':                  return XK_9;
        case GLFW_KEY_ESC:         return XK_Escape;
        case GLFW_KEY_LCTRL:       return XK_Control_L;
        case GLFW_KEY_LSHIFT:      return XK_Shift_L;
        case GLFW_KEY_LALT:        return XK_Alt_L;
        case GLFW_KEY_LSUPER:      return XK_Super_L;
        case GLFW_KEY_RCTRL:       return XK_Control_R;
        case GLFW_KEY_RSHIFT:      return XK_Shift_R;
        case GLFW_KEY_RALT:        return XK_Alt_R;
        case GLFW_KEY_RSUPER:      return XK_Super_R;
        case GLFW_KEY_MENU:        return XK_Menu;
        case '[':                  return XK_bracketleft;
        case ']':                  return XK_bracketright;
        case ';':                  return XK_semicolon;
        case ',':                  return XK_comma;
        case '.':                  return XK_period;
        case '\'':                 return XK_dead_acute;
        case '/':                  return XK_slash;
        case '\\':                 return XK_backslash;
        case '~':                  return XK_dead_grave;
        case '=':                  return XK_equal;
        case '-':                  return XK_minus;
        case GLFW_KEY_SPACE:       return XK_space;
        case GLFW_KEY_ENTER:       return XK_Return;
        case GLFW_KEY_BACKSPACE:   return XK_BackSpace;
        case GLFW_KEY_TAB:         return XK_Tab;
        case GLFW_KEY_PAGEUP:      return XK_Prior;
        case GLFW_KEY_PAGEDOWN:    return XK_Next;
        case GLFW_KEY_END:         return XK_End;
        case GLFW_KEY_HOME:        return XK_Home;
        case GLFW_KEY_INSERT:      return XK_Insert;
        case GLFW_KEY_DEL:         return XK_Delete;
        case GLFW_KEY_KP_ADD:      return XK_KP_Add;
        case GLFW_KEY_KP_SUBTRACT: return XK_KP_Subtract;
        case GLFW_KEY_KP_MULTIPLY: return XK_KP_Multiply;
        case GLFW_KEY_KP_DIVIDE:   return XK_KP_Divide;
        case GLFW_KEY_LEFT:        return XK_Left;
        case GLFW_KEY_RIGHT:       return XK_Right;
        case GLFW_KEY_UP:          return XK_Up;
        case GLFW_KEY_DOWN:        return XK_Down;
        case GLFW_KEY_KP_0:        return XK_KP_0;
        case GLFW_KEY_KP_1:        return XK_KP_1;
        case GLFW_KEY_KP_2:        return XK_KP_2;
        case GLFW_KEY_KP_3:        return XK_KP_3;
        case GLFW_KEY_KP_4:        return XK_KP_4;
        case GLFW_KEY_KP_5:        return XK_KP_5;
        case GLFW_KEY_KP_6:        return XK_KP_6;
        case GLFW_KEY_KP_7:        return XK_KP_7;
        case GLFW_KEY_KP_8:        return XK_KP_8;
        case GLFW_KEY_KP_9:        return XK_KP_9;
        case GLFW_KEY_F1:          return XK_F1;
        case GLFW_KEY_F2:          return XK_F2;
        case GLFW_KEY_F3:          return XK_F3;
        case GLFW_KEY_F4:          return XK_F4;
        case GLFW_KEY_F5:          return XK_F5;
        case GLFW_KEY_F6:          return XK_F6;
        case GLFW_KEY_F7:          return XK_F7;
        case GLFW_KEY_F8:          return XK_F8;
        case GLFW_KEY_F9:          return XK_F9;
        case GLFW_KEY_F10:         return XK_F10;
        case GLFW_KEY_F11:         return XK_F11;
        case GLFW_KEY_F12:         return XK_F12;
        case GLFW_KEY_F13:         return XK_F13;
        case GLFW_KEY_F14:         return XK_F14;
        case GLFW_KEY_F15:         return XK_F15;
        case GLFW_KEY_PAUSE:       return XK_Pause;
        default:                   return 0;
    }
}

#endif

std::string glfw_handler::get_key_name(key mKey) const
{
#ifdef WIN32
    int vkey = to_vkey_(to_glfw_(mKey));
    unsigned int code = MapVirtualKey(vkey, 0 /*MAPVK_VK_TO_VSC*/);

    char name[50];
    if (GetKeyNameText(code << 16, name, sizeof(name)) != 0)
        return name;
    else
        return "Unknown";

#else
    // Get the corresponding X11 keysym
    KeySym keysym = to_xkey_(to_glfw_(mKey));
    if (keysym == 0)
        return "Unknown";

    // Get the key name
    char* name = XKeysymToString(keysym);
    if (name != 0)
    {
        std::string s(name);

        if (s.substr(0, 3) == "KP_")
        {
            s.erase(0, 3);
            s.append(" (Num.)");
        }

        s[0] = toupper(s[0]);
        bool bPrevUnderscore = false;
        for (auto& c : s)
        {
            if (c == '_')
            {
                c = ' ';
                bPrevUnderscore = true;
            }
            else if (bPrevUnderscore)
            {
                c = toupper(c);
                bPrevUnderscore = false;
            }
        }

        return s;
    }
    else
        return "Unknown";
#endif
}

void glfw_handler::update()
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
