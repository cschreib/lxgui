#include "lxgui/impl/sfml_input_impl.hpp"
#include <lxgui/utils_string.hpp>

#include <SFML/Window/Window.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#ifndef MSVC
#warning "Note : SFML input handler is not fully functional, since \
SFML doesn't provide keyboard layout independent key codes."
#endif

using sf::Keyboard;
using sf::Mouse;

namespace input
{
const sfml_manager::key_mapping sfml_manager::lKeyToSFML[100] =
{
    {key::K_ESCAPE,Keyboard::Escape},
    {key::K_0,Keyboard::Num0},
    {key::K_1,Keyboard::Num1},
    {key::K_2,Keyboard::Num2},
    {key::K_3,Keyboard::Num3},
    {key::K_4,Keyboard::Num4},
    {key::K_5,Keyboard::Num5},
    {key::K_6,Keyboard::Num6},
    {key::K_7,Keyboard::Num7},
    {key::K_8,Keyboard::Num8},
    {key::K_9,Keyboard::Num9},
    {key::K_MINUS,Keyboard::Dash},
    {key::K_EQUALS,Keyboard::Equal},
    {key::K_BACK,Keyboard::BackSpace},
    {key::K_TAB,Keyboard::Tab},
    {key::K_Q,Keyboard::Q},
    {key::K_W,Keyboard::W},
    {key::K_E,Keyboard::E},
    {key::K_R,Keyboard::R},
    {key::K_T,Keyboard::T},
    {key::K_Y,Keyboard::Y},
    {key::K_U,Keyboard::U},
    {key::K_I,Keyboard::I},
    {key::K_O,Keyboard::O},
    {key::K_P,Keyboard::P},
    {key::K_LBRACKET,Keyboard::LBracket},
    {key::K_RBRACKET,Keyboard::RBracket},
    {key::K_RETURN,Keyboard::Return},
    {key::K_LCONTROL,Keyboard::LControl},
    {key::K_A,Keyboard::A},
    {key::K_S,Keyboard::S},
    {key::K_D,Keyboard::D},
    {key::K_F,Keyboard::F},
    {key::K_G,Keyboard::G},
    {key::K_H,Keyboard::H},
    {key::K_J,Keyboard::J},
    {key::K_K,Keyboard::K},
    {key::K_L,Keyboard::L},
    {key::K_SEMICOLON,Keyboard::SemiColon},
    {key::K_APOSTROPHE,Keyboard::Quote},
    //{key::K_GRAVE,Keyboard:: },
    {key::K_LSHIFT,Keyboard::LShift},
    {key::K_BACKSLASH,Keyboard::BackSlash},
    {key::K_Z,Keyboard::Z},
    {key::K_X,Keyboard::X},
    {key::K_C,Keyboard::C},
    {key::K_V,Keyboard::V},
    {key::K_B,Keyboard::B},
    {key::K_N,Keyboard::N},
    {key::K_M,Keyboard::M},
    {key::K_COMMA,Keyboard::Comma},
    {key::K_PERIOD,Keyboard::Period},
    {key::K_SLASH,Keyboard::Slash},
    {key::K_RSHIFT,Keyboard::RShift},
    {key::K_MULTIPLY,Keyboard::Multiply},
    {key::K_LMENU,Keyboard::LAlt},
    {key::K_SPACE,Keyboard::Space},
    //{key::K_CAPITAL,Keyboard:: }, // CapsLock
    {key::K_F1,Keyboard::F1},
    {key::K_F2,Keyboard::F2},
    {key::K_F3,Keyboard::F3},
    {key::K_F4,Keyboard::F4},
    {key::K_F5,Keyboard::F5},
    {key::K_F6,Keyboard::F6},
    {key::K_F7,Keyboard::F7},
    {key::K_F8,Keyboard::F8},
    {key::K_F9,Keyboard::F9},
    {key::K_F10,Keyboard::F10},
    {key::K_NUMPAD7,Keyboard::Numpad7},
    {key::K_NUMPAD8,Keyboard::Numpad8},
    {key::K_NUMPAD9,Keyboard::Numpad9},
    {key::K_SUBTRACT,Keyboard::Subtract},
    {key::K_NUMPAD4,Keyboard::Numpad4},
    {key::K_NUMPAD5,Keyboard::Numpad5},
    {key::K_NUMPAD6,Keyboard::Numpad6},
    {key::K_ADD,Keyboard::Add},
    {key::K_NUMPAD1,Keyboard::Numpad1},
    {key::K_NUMPAD2,Keyboard::Numpad2},
    {key::K_NUMPAD3,Keyboard::Numpad3},
    {key::K_NUMPAD0,Keyboard::Numpad0},
    //{key::K_DECIMAL,Keyboard::"Decimal"},
    //{key::K_OEM_102,Keyboard::"<>"},
    {key::K_F11,Keyboard::F11},
    {key::K_F12,Keyboard::F12},
    {key::K_F13,Keyboard::F13},
    {key::K_F14,Keyboard::F14},
    {key::K_F15,Keyboard::F15},
    {key::K_RCONTROL,Keyboard::RControl},
    {key::K_DIVIDE,Keyboard::Divide},
    {key::K_RMENU,Keyboard::RAlt},
    {key::K_PAUSE,Keyboard::Pause},
    {key::K_HOME,Keyboard::Home},
    {key::K_UP,Keyboard::Up},
    {key::K_PGUP,Keyboard::PageUp},
    {key::K_LEFT,Keyboard::Left},
    {key::K_RIGHT,Keyboard::Right},
    {key::K_END,Keyboard::End},
    {key::K_DOWN,Keyboard::Down},
    {key::K_PGDOWN,Keyboard::PageDown},
    {key::K_INSERT,Keyboard::Insert},
    {key::K_DELETE,Keyboard::Delete},
    {key::K_LWIN,Keyboard::LSystem},
    {key::K_RWIN,Keyboard::RSystem},
    {key::K_APPS,Keyboard::Menu}
};

sfml_manager::sfml_manager(const sf::Window& mWindow, bool bMouseGrab) :
    mWindow_(mWindow), bMouseGrab_(bMouseGrab), bFirst_(true), fWheelCache_(0.0f)
{
    if (bMouseGrab_)
    {
        fOldMouseX_ = mWindow_.getSize().x/2;
        fOldMouseY_ = mWindow_.getSize().y/2;
    }

    mMouse_.bHasDelta = true;
}

int sfml_manager::to_sfml_(key mKey) const
{
    for (size_t i = 0; i < 100; ++i)
    {
        if (lKeyToSFML[i].mKey == mKey)
            return lKeyToSFML[i].mSFKey;
    }

    return Keyboard::Unknown;
}

void sfml_manager::toggle_mouse_grab()
{
    bMouseGrab_ = !bMouseGrab_;
    if (bMouseGrab_)
    {
        fOldMouseX_ = mWindow_.getSize().x/2;
        fOldMouseY_ = mWindow_.getSize().y/2;
    }
}

#ifdef WIN32
#include <windows.h>

#ifndef VK_OEM_COMMA
    #define VK_OEM_PLUS     0xBB
    #define VK_OEM_COMMA    0xBC
    #define VK_OEM_MINUS    0xBD
    #define VK_OEM_PERIOD   0xBE
#endif

int to_vkey_(Keyboard::Key key)
{
    switch (key)
    {
        default:                   return 0;
        case Keyboard::A:          return 'A';
        case Keyboard::B:          return 'B';
        case Keyboard::C:          return 'C';
        case Keyboard::D:          return 'D';
        case Keyboard::E:          return 'E';
        case Keyboard::F:          return 'F';
        case Keyboard::G:          return 'G';
        case Keyboard::H:          return 'H';
        case Keyboard::I:          return 'I';
        case Keyboard::J:          return 'J';
        case Keyboard::K:          return 'K';
        case Keyboard::L:          return 'L';
        case Keyboard::M:          return 'M';
        case Keyboard::N:          return 'N';
        case Keyboard::O:          return 'O';
        case Keyboard::P:          return 'P';
        case Keyboard::Q:          return 'Q';
        case Keyboard::R:          return 'R';
        case Keyboard::S:          return 'S';
        case Keyboard::T:          return 'T';
        case Keyboard::U:          return 'U';
        case Keyboard::V:          return 'V';
        case Keyboard::W:          return 'W';
        case Keyboard::X:          return 'X';
        case Keyboard::Y:          return 'Y';
        case Keyboard::Z:          return 'Z';
        case Keyboard::Num0:       return '0';
        case Keyboard::Num1:       return '1';
        case Keyboard::Num2:       return '2';
        case Keyboard::Num3:       return '3';
        case Keyboard::Num4:       return '4';
        case Keyboard::Num5:       return '5';
        case Keyboard::Num6:       return '6';
        case Keyboard::Num7:       return '7';
        case Keyboard::Num8:       return '8';
        case Keyboard::Num9:       return '9';
        case Keyboard::Escape:     return VK_ESCAPE;
        case Keyboard::LControl:   return VK_LCONTROL;
        case Keyboard::LShift:     return VK_LSHIFT;
        case Keyboard::LAlt:       return VK_LMENU;
        case Keyboard::LSystem:    return VK_LWIN;
        case Keyboard::RControl:   return VK_RCONTROL;
        case Keyboard::RShift:     return VK_RSHIFT;
        case Keyboard::RAlt:       return VK_RMENU;
        case Keyboard::RSystem:    return VK_RWIN;
        case Keyboard::Menu:       return VK_APPS;
        case Keyboard::LBracket:   return VK_OEM_4;
        case Keyboard::RBracket:   return VK_OEM_6;
        case Keyboard::SemiColon:  return VK_OEM_1;
        case Keyboard::Comma:      return VK_OEM_COMMA;
        case Keyboard::Period:     return VK_OEM_PERIOD;
        case Keyboard::Quote:      return VK_OEM_7;
        case Keyboard::Slash:      return VK_OEM_2;
        case Keyboard::BackSlash:  return VK_OEM_5;
        case Keyboard::Tilde:      return VK_OEM_3;
        case Keyboard::Equal:      return VK_OEM_PLUS;
        case Keyboard::Dash:       return VK_OEM_MINUS;
        case Keyboard::Space:      return VK_SPACE;
        case Keyboard::Return:     return VK_RETURN;
        case Keyboard::BackSpace:  return VK_BACK;
        case Keyboard::Tab:        return VK_TAB;
        case Keyboard::PageUp:     return VK_PRIOR;
        case Keyboard::PageDown:   return VK_NEXT;
        case Keyboard::End:        return VK_END;
        case Keyboard::Home:       return VK_HOME;
        case Keyboard::Insert:     return VK_INSERT;
        case Keyboard::Delete:     return VK_DELETE;
        case Keyboard::Add:        return VK_ADD;
        case Keyboard::Subtract:   return VK_SUBTRACT;
        case Keyboard::Multiply:   return VK_MULTIPLY;
        case Keyboard::Divide:     return VK_DIVIDE;
        case Keyboard::Left:       return VK_LEFT;
        case Keyboard::Right:      return VK_RIGHT;
        case Keyboard::Up:         return VK_UP;
        case Keyboard::Down:       return VK_DOWN;
        case Keyboard::Numpad0:    return VK_NUMPAD0;
        case Keyboard::Numpad1:    return VK_NUMPAD1;
        case Keyboard::Numpad2:    return VK_NUMPAD2;
        case Keyboard::Numpad3:    return VK_NUMPAD3;
        case Keyboard::Numpad4:    return VK_NUMPAD4;
        case Keyboard::Numpad5:    return VK_NUMPAD5;
        case Keyboard::Numpad6:    return VK_NUMPAD6;
        case Keyboard::Numpad7:    return VK_NUMPAD7;
        case Keyboard::Numpad8:    return VK_NUMPAD8;
        case Keyboard::Numpad9:    return VK_NUMPAD9;
        case Keyboard::F1:         return VK_F1;
        case Keyboard::F2:         return VK_F2;
        case Keyboard::F3:         return VK_F3;
        case Keyboard::F4:         return VK_F4;
        case Keyboard::F5:         return VK_F5;
        case Keyboard::F6:         return VK_F6;
        case Keyboard::F7:         return VK_F7;
        case Keyboard::F8:         return VK_F8;
        case Keyboard::F9:         return VK_F9;
        case Keyboard::F10:        return VK_F10;
        case Keyboard::F11:        return VK_F11;
        case Keyboard::F12:        return VK_F12;
        case Keyboard::F13:        return VK_F13;
        case Keyboard::F14:        return VK_F14;
        case Keyboard::F15:        return VK_F16;
        case Keyboard::Pause:      return VK_PAUSE;
    }
}

#else
#include <X11/Xlib.h>
#include <X11/keysym.h>

KeySym to_xkey_(Keyboard::Key key)
{
    switch (key)
    {
        case Keyboard::A:          return XK_A;
        case Keyboard::B:          return XK_B;
        case Keyboard::C:          return XK_C;
        case Keyboard::D:          return XK_D;
        case Keyboard::E:          return XK_E;
        case Keyboard::F:          return XK_F;
        case Keyboard::G:          return XK_G;
        case Keyboard::H:          return XK_H;
        case Keyboard::I:          return XK_I;
        case Keyboard::J:          return XK_J;
        case Keyboard::K:          return XK_K;
        case Keyboard::L:          return XK_L;
        case Keyboard::M:          return XK_M;
        case Keyboard::N:          return XK_N;
        case Keyboard::O:          return XK_O;
        case Keyboard::P:          return XK_P;
        case Keyboard::Q:          return XK_Q;
        case Keyboard::R:          return XK_R;
        case Keyboard::S:          return XK_S;
        case Keyboard::T:          return XK_T;
        case Keyboard::U:          return XK_U;
        case Keyboard::V:          return XK_V;
        case Keyboard::W:          return XK_W;
        case Keyboard::X:          return XK_X;
        case Keyboard::Y:          return XK_Y;
        case Keyboard::Z:          return XK_Z;
        case Keyboard::Num0:       return XK_0;
        case Keyboard::Num1:       return XK_1;
        case Keyboard::Num2:       return XK_2;
        case Keyboard::Num3:       return XK_3;
        case Keyboard::Num4:       return XK_4;
        case Keyboard::Num5:       return XK_5;
        case Keyboard::Num6:       return XK_6;
        case Keyboard::Num7:       return XK_7;
        case Keyboard::Num8:       return XK_8;
        case Keyboard::Num9:       return XK_9;
        case Keyboard::Escape:     return XK_Escape;
        case Keyboard::LControl:   return XK_Control_L;
        case Keyboard::LShift:     return XK_Shift_L;
        case Keyboard::LAlt:       return XK_Alt_L;
        case Keyboard::LSystem:    return XK_Super_L;
        case Keyboard::RControl:   return XK_Control_R;
        case Keyboard::RShift:     return XK_Shift_R;
        case Keyboard::RAlt:       return XK_Alt_R;
        case Keyboard::RSystem:    return XK_Super_R;
        case Keyboard::Menu:       return XK_Menu;
        case Keyboard::LBracket:   return XK_bracketleft;
        case Keyboard::RBracket:   return XK_bracketright;
        case Keyboard::SemiColon:  return XK_semicolon;
        case Keyboard::Comma:      return XK_comma;
        case Keyboard::Period:     return XK_period;
        case Keyboard::Quote:      return XK_dead_acute;
        case Keyboard::Slash:      return XK_slash;
        case Keyboard::BackSlash:  return XK_backslash;
        case Keyboard::Tilde:      return XK_dead_grave;
        case Keyboard::Equal:      return XK_equal;
        case Keyboard::Dash:       return XK_minus;
        case Keyboard::Space:      return XK_space;
        case Keyboard::Return:     return XK_Return;
        case Keyboard::BackSpace:  return XK_BackSpace;
        case Keyboard::Tab:        return XK_Tab;
        case Keyboard::PageUp:     return XK_Prior;
        case Keyboard::PageDown:   return XK_Next;
        case Keyboard::End:        return XK_End;
        case Keyboard::Home:       return XK_Home;
        case Keyboard::Insert:     return XK_Insert;
        case Keyboard::Delete:     return XK_Delete;
        case Keyboard::Add:        return XK_KP_Add;
        case Keyboard::Subtract:   return XK_KP_Subtract;
        case Keyboard::Multiply:   return XK_KP_Multiply;
        case Keyboard::Divide:     return XK_KP_Divide;
        case Keyboard::Left:       return XK_Left;
        case Keyboard::Right:      return XK_Right;
        case Keyboard::Up:         return XK_Up;
        case Keyboard::Down:       return XK_Down;
        case Keyboard::Numpad0:    return XK_KP_0;
        case Keyboard::Numpad1:    return XK_KP_1;
        case Keyboard::Numpad2:    return XK_KP_2;
        case Keyboard::Numpad3:    return XK_KP_3;
        case Keyboard::Numpad4:    return XK_KP_4;
        case Keyboard::Numpad5:    return XK_KP_5;
        case Keyboard::Numpad6:    return XK_KP_6;
        case Keyboard::Numpad7:    return XK_KP_7;
        case Keyboard::Numpad8:    return XK_KP_8;
        case Keyboard::Numpad9:    return XK_KP_9;
        case Keyboard::F1:         return XK_F1;
        case Keyboard::F2:         return XK_F2;
        case Keyboard::F3:         return XK_F3;
        case Keyboard::F4:         return XK_F4;
        case Keyboard::F5:         return XK_F5;
        case Keyboard::F6:         return XK_F6;
        case Keyboard::F7:         return XK_F7;
        case Keyboard::F8:         return XK_F8;
        case Keyboard::F9:         return XK_F9;
        case Keyboard::F10:        return XK_F10;
        case Keyboard::F11:        return XK_F11;
        case Keyboard::F12:        return XK_F12;
        case Keyboard::F13:        return XK_F13;
        case Keyboard::F14:        return XK_F14;
        case Keyboard::F15:        return XK_F15;
        case Keyboard::Pause:      return XK_Pause;
        default:                   return 0;
    }
}

#endif

std::string sfml_manager::get_key_name(key mKey) const
{
#ifdef WIN32
    int vkey = to_vkey_((sf::Keyboard::Key)to_sfml_(mKey));
    unsigned int code = MapVirtualKey(vkey, 0 /*MAPVK_VK_TO_VSC*/);

    char name[50];
    if (GetKeyNameText(code << 16, name, sizeof(name)) != 0)
        return name;
    else
        return "Unknown";

#else
    // Get the corresponding X11 keysym
    KeySym keysym = to_xkey_((sf::Keyboard::Key)to_sfml_(mKey));
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

void sfml_manager::update_()
{
    for (int i = 0; i < 100; ++i)
        mKeyboard_.lKeyState[(uint)lKeyToSFML[i].mKey] = Keyboard::isKeyPressed((Keyboard::Key)lKeyToSFML[i].mSFKey);

    const float width  = mWindow_.getSize().x;
    const float height = mWindow_.getSize().y;

    if (bFirst_)
    {
        mMouse_.fAbsX = Mouse::getPosition(mWindow_).x;
        mMouse_.fAbsY = Mouse::getPosition(mWindow_).y;
        mMouse_.fRelX = mMouse_.fAbsX/width;
        mMouse_.fRelY = mMouse_.fAbsY/height;

        mMouse_.fDX = mMouse_.fDY = mMouse_.fRelDX = mMouse_.fRelDY = 0.0f;
        bFirst_ = false;

        if (!bMouseGrab_)
        {
            fOldMouseX_ = mMouse_.fAbsX;
            fOldMouseY_ = mMouse_.fAbsY;
        }
    }
    else
    {
        mMouse_.fDX = Mouse::getPosition(mWindow_).x - fOldMouseX_;
        mMouse_.fDY = Mouse::getPosition(mWindow_).y - fOldMouseY_;
        mMouse_.fRelDX = mMouse_.fDX/width;
        mMouse_.fRelDY = mMouse_.fDY/height;

        mMouse_.fAbsX += mMouse_.fDX;
        mMouse_.fAbsY += mMouse_.fDY;
        mMouse_.fRelX = mMouse_.fAbsX/width;
        mMouse_.fRelY = mMouse_.fAbsY/height;

        if (bMouseGrab_)
            Mouse::setPosition(sf::Vector2i(fOldMouseX_, fOldMouseY_), mWindow_);
        else
        {
            fOldMouseX_ = mMouse_.fAbsX;
            fOldMouseY_ = mMouse_.fAbsY;
        }
    }

    mMouse_.fRelWheel = 0.0f;
    std::swap(mMouse_.fRelWheel, fWheelCache_);

    static const Mouse::Button lMouseToSFML[3] = {Mouse::Left, Mouse::Right, Mouse::Middle};

    for (std::size_t i = 0; i < MOUSE_BUTTON_NUMBER; ++i)
        mMouse_.lButtonState[i] = Mouse::isButtonPressed(lMouseToSFML[i]);
}

void sfml_manager::on_sfml_event(const sf::Event& mEvent)
{
    if (mEvent.type == sf::Event::TextEntered)
    {
        auto c = mEvent.text.unicode;
        // Remove non printable characters (< 32) and Del. (127)
        if (c >= 32 && c != 127)
            lCharsCache_.push_back(c);
    }
    else if (mEvent.type == sf::Event::MouseWheelMoved)
    {
        fWheelCache_ += mEvent.mouseWheel.delta;
    }
    else if (mEvent.type == sf::Event::Resized)
    {
        bWindowResized_ = true;
        uiNewWindowWidth_ = mEvent.size.width;
        uiNewWindowHeight_ = mEvent.size.height;
    }
}
}
