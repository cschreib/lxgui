#include "lxgui/impl/input_sfml_source.hpp"
#include <lxgui/gui_event.hpp>
#include <lxgui/utils_string.hpp>

#include <SFML/Window/Window.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Clipboard.hpp>

using sf::Keyboard;
using sf::Mouse;

namespace lxgui {
namespace input {
namespace sfml
{
source::source(const sf::Window& mWindow, bool bMouseGrab) :
    mWindow_(mWindow), bMouseGrab_(bMouseGrab), bFirst_(true), fWheelCache_(0.0f)
{
    if (bMouseGrab_)
    {
        fOldMouseX_ = mWindow_.getSize().x/2;
        fOldMouseY_ = mWindow_.getSize().y/2;
    }

    mMouse_.bHasDelta = true;
}

void source::toggle_mouse_grab()
{
    bMouseGrab_ = !bMouseGrab_;
    if (bMouseGrab_)
    {
        fOldMouseX_ = mWindow_.getSize().x/2;
        fOldMouseY_ = mWindow_.getSize().y/2;
    }
}

utils::ustring source::get_clipboard_content()
{
    auto sUtfString = sf::Clipboard::getString().toUtf32();
    return utils::ustring(sUtfString.begin(), sUtfString.end());
}

void source::set_clipboard_content(const utils::ustring& sContent)
{
    sf::Clipboard::setString(sf::String::fromUtf32(sContent.begin(), sContent.end()));
}

key source::from_sfml_(int uiSFKey) const
{
    switch ((sf::Keyboard::Key)uiSFKey)
    {
    case Keyboard::Escape:    return key::K_ESCAPE;
    case Keyboard::Num0:      return key::K_0;
    case Keyboard::Num1:      return key::K_1;
    case Keyboard::Num2:      return key::K_2;
    case Keyboard::Num3:      return key::K_3;
    case Keyboard::Num4:      return key::K_4;
    case Keyboard::Num5:      return key::K_5;
    case Keyboard::Num6:      return key::K_6;
    case Keyboard::Num7:      return key::K_7;
    case Keyboard::Num8:      return key::K_8;
    case Keyboard::Num9:      return key::K_9;
    case Keyboard::Dash:      return key::K_MINUS;
    case Keyboard::Equal:     return key::K_EQUALS;
    case Keyboard::BackSpace: return key::K_BACK;
    case Keyboard::Tab:       return key::K_TAB;
    case Keyboard::Q:         return key::K_Q;
    case Keyboard::W:         return key::K_W;
    case Keyboard::E:         return key::K_E;
    case Keyboard::R:         return key::K_R;
    case Keyboard::T:         return key::K_T;
    case Keyboard::Y:         return key::K_Y;
    case Keyboard::U:         return key::K_U;
    case Keyboard::I:         return key::K_I;
    case Keyboard::O:         return key::K_O;
    case Keyboard::P:         return key::K_P;
    case Keyboard::LBracket:  return key::K_LBRACKET;
    case Keyboard::RBracket:  return key::K_RBRACKET;
    case Keyboard::Return:    return key::K_RETURN;
    case Keyboard::LControl:  return key::K_LCONTROL;
    case Keyboard::A:         return key::K_A;
    case Keyboard::S:         return key::K_S;
    case Keyboard::D:         return key::K_D;
    case Keyboard::F:         return key::K_F;
    case Keyboard::G:         return key::K_G;
    case Keyboard::H:         return key::K_H;
    case Keyboard::J:         return key::K_J;
    case Keyboard::K:         return key::K_K;
    case Keyboard::L:         return key::K_L;
    case Keyboard::SemiColon: return key::K_SEMICOLON;
    case Keyboard::Quote:     return key::K_APOSTROPHE;
    case Keyboard::LShift:    return key::K_LSHIFT;
    case Keyboard::BackSlash: return key::K_BACKSLASH;
    case Keyboard::Z:         return key::K_Z;
    case Keyboard::X:         return key::K_X;
    case Keyboard::C:         return key::K_C;
    case Keyboard::V:         return key::K_V;
    case Keyboard::B:         return key::K_B;
    case Keyboard::N:         return key::K_N;
    case Keyboard::M:         return key::K_M;
    case Keyboard::Comma:     return key::K_COMMA;
    case Keyboard::Period:    return key::K_PERIOD;
    case Keyboard::Slash:     return key::K_SLASH;
    case Keyboard::RShift:    return key::K_RSHIFT;
    case Keyboard::Multiply:  return key::K_MULTIPLY;
    case Keyboard::LAlt:      return key::K_LMENU;
    case Keyboard::Space:     return key::K_SPACE;
    case Keyboard::F1:        return key::K_F1;
    case Keyboard::F2:        return key::K_F2;
    case Keyboard::F3:        return key::K_F3;
    case Keyboard::F4:        return key::K_F4;
    case Keyboard::F5:        return key::K_F5;
    case Keyboard::F6:        return key::K_F6;
    case Keyboard::F7:        return key::K_F7;
    case Keyboard::F8:        return key::K_F8;
    case Keyboard::F9:        return key::K_F9;
    case Keyboard::F10:       return key::K_F10;
    case Keyboard::Numpad7:   return key::K_NUMPAD7;
    case Keyboard::Numpad8:   return key::K_NUMPAD8;
    case Keyboard::Numpad9:   return key::K_NUMPAD9;
    case Keyboard::Subtract:  return key::K_SUBTRACT;
    case Keyboard::Numpad4:   return key::K_NUMPAD4;
    case Keyboard::Numpad5:   return key::K_NUMPAD5;
    case Keyboard::Numpad6:   return key::K_NUMPAD6;
    case Keyboard::Add:       return key::K_ADD;
    case Keyboard::Numpad1:   return key::K_NUMPAD1;
    case Keyboard::Numpad2:   return key::K_NUMPAD2;
    case Keyboard::Numpad3:   return key::K_NUMPAD3;
    case Keyboard::Numpad0:   return key::K_NUMPAD0;
    case Keyboard::F11:       return key::K_F11;
    case Keyboard::F12:       return key::K_F12;
    case Keyboard::F13:       return key::K_F13;
    case Keyboard::F14:       return key::K_F14;
    case Keyboard::F15:       return key::K_F15;
    case Keyboard::RControl:  return key::K_RCONTROL;
    case Keyboard::Divide:    return key::K_DIVIDE;
    case Keyboard::RAlt:      return key::K_RMENU;
    case Keyboard::Pause:     return key::K_PAUSE;
    case Keyboard::Home:      return key::K_HOME;
    case Keyboard::Up:        return key::K_UP;
    case Keyboard::PageUp:    return key::K_PGUP;
    case Keyboard::Left:      return key::K_LEFT;
    case Keyboard::Right:     return key::K_RIGHT;
    case Keyboard::End:       return key::K_END;
    case Keyboard::Down:      return key::K_DOWN;
    case Keyboard::PageDown:  return key::K_PGDOWN;
    case Keyboard::Insert:    return key::K_INSERT;
    case Keyboard::Delete:    return key::K_DELETE;
    case Keyboard::LSystem:   return key::K_LWIN;
    case Keyboard::RSystem:   return key::K_RWIN;
    case Keyboard::Menu:      return key::K_APPS;
    default:                  return key::K_UNASSIGNED;
    }
}

void source::update_()
{
    const float width  = mWindow_.getSize().x;
    const float height = mWindow_.getSize().y;

    const sf::Vector2i mMousePos = Mouse::getPosition(mWindow_);

    if (bFirst_)
    {
        mMouse_.fAbsX = mMousePos.x;
        mMouse_.fAbsY = mMousePos.y;
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
        mMouse_.fDX = mMousePos.x - fOldMouseX_;
        mMouse_.fDY = mMousePos.y - fOldMouseY_;
        mMouse_.fRelDX = mMouse_.fDX/width;
        mMouse_.fRelDY = mMouse_.fDY/height;

        mMouse_.fAbsX += mMouse_.fDX;
        mMouse_.fAbsY += mMouse_.fDY;
        mMouse_.fRelX = mMouse_.fAbsX/width;
        mMouse_.fRelY = mMouse_.fAbsY/height;

        if (bMouseGrab_)
        {
            Mouse::setPosition(sf::Vector2i(fOldMouseX_, fOldMouseY_), mWindow_);
        }
        else
        {
            fOldMouseX_ = mMouse_.fAbsX;
            fOldMouseY_ = mMouse_.fAbsY;
        }
    }

    mMouse_.fRelWheel = 0.0f;
    std::swap(mMouse_.fRelWheel, fWheelCache_);
}

void source::on_sfml_event(const sf::Event& mEvent)
{
    static const mouse_button lMouseFromSFML[3] = {mouse_button::LEFT, mouse_button::RIGHT, mouse_button::MIDDLE};

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
    else if (mEvent.type == sf::Event::KeyPressed)
    {
        key mKey = from_sfml_(mEvent.key.code);
        mKeyboard_.lKeyState[(uint)mKey] = true;

        gui::event mKeyboardEvent("KEY_PRESSED");
        mKeyboardEvent.add(static_cast<std::underlying_type_t<key>>(mKey));
        lEvents_.push_back(mKeyboardEvent);
    }
    else if (mEvent.type == sf::Event::KeyReleased)
    {
        key mKey = from_sfml_(mEvent.key.code);
        mKeyboard_.lKeyState[(uint)mKey] = false;

        gui::event mKeyboardEvent("KEY_RELEASED");
        mKeyboardEvent.add(static_cast<std::underlying_type_t<key>>(mKey));
        lEvents_.push_back(mKeyboardEvent);
    }
    else if (mEvent.type == sf::Event::MouseButtonPressed)
    {
        mouse_button mButton = lMouseFromSFML[mEvent.mouseButton.button];
        mMouse_.lButtonState[(uint)mButton] = true;

        const sf::Vector2i mMousePos = Mouse::getPosition(mWindow_);

        gui::event mMouseEvent("MOUSE_PRESSED");
        mMouseEvent.add(static_cast<std::underlying_type_t<mouse_button>>(mButton));
        mMouseEvent.add((float)mMousePos.x);
        mMouseEvent.add((float)mMousePos.y);
        lEvents_.push_back(mMouseEvent);

        if ((double)lLastClickClock_[(uint)mButton].getElapsedTime().asSeconds() < dDoubleClickTime_)
        {
            mMouseEvent.set_name("MOUSE_DOUBLE_CLICKED");
            lEvents_.push_back(mMouseEvent);
        }

        lLastClickClock_[(uint)mButton].restart();
    }
    else if (mEvent.type == sf::Event::MouseButtonReleased)
    {
        mouse_button mButton = lMouseFromSFML[mEvent.mouseButton.button];
        mMouse_.lButtonState[(uint)mButton] = false;

        const sf::Vector2i mMousePos = Mouse::getPosition(mWindow_);

        gui::event mMouseEvent("MOUSE_RELEASED");
        mMouseEvent.add(static_cast<std::underlying_type_t<mouse_button>>(mButton));
        mMouseEvent.add((float)mMousePos.x);
        mMouseEvent.add((float)mMousePos.y);
        lEvents_.push_back(mMouseEvent);
    }
}
}
}
}
