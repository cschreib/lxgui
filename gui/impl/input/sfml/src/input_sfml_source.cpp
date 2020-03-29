#include "lxgui/impl/input_sfml_source.hpp"
#include <lxgui/utils_string.hpp>

#include <SFML/Window/Window.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

using sf::Keyboard;
using sf::Mouse;

namespace input {
namespace sfml
{
const source::key_mapping source::lKeyToSFML[100] =
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

int source::to_sfml_(key mKey) const
{
    for (size_t i = 0; i < 100; ++i)
    {
        if (lKeyToSFML[i].mKey == mKey)
            return lKeyToSFML[i].mSFKey;
    }

    return Keyboard::Unknown;
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

void source::update_()
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

void source::on_sfml_event(const sf::Event& mEvent)
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
}
