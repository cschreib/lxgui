#include "lxgui/impl/input_sfml_source.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/utils_string.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Window.hpp>

using sf::Keyboard;
using sf::Mouse;

namespace lxgui::input { namespace sfml {
source::source(sf::Window& mWindow) : mWindow_(mWindow) {
    mWindowDimensions_ = gui::vector2ui(mWindow_.getSize().x, mWindow_.getSize().y);
}

utils::ustring source::get_clipboard_content() {
    auto sUtfString = sf::Clipboard::getString().toUtf32();
    return utils::ustring(sUtfString.begin(), sUtfString.end());
}

void source::set_clipboard_content(const utils::ustring& sContent) {
    sf::Clipboard::setString(sf::String::fromUtf32(sContent.begin(), sContent.end()));
}

void source::set_mouse_cursor(const std::string& sFileName, const gui::vector2i& mHotSpot) {
    auto mIter = lCursorMap_.find(sFileName);
    if (mIter == lCursorMap_.end()) {
        sf::Image mImage;
        if (!mImage.loadFromFile(sFileName)) {
            throw gui::exception(
                "input::sfml::source", "Could not load cursor file '" + sFileName + "'.");
        }

        auto pCursor = std::make_unique<sf::Cursor>();
        pCursor->loadFromPixels(
            mImage.getPixelsPtr(), mImage.getSize(), sf::Vector2u(mHotSpot.x, mHotSpot.y));
        mIter = lCursorMap_.insert(std::make_pair(sFileName, std::move(pCursor))).first;
    }

    mWindow_.setMouseCursor(*mIter->second);
}

void source::reset_mouse_cursor() {
    const std::string sName = "system_arrow";
    auto              mIter = lCursorMap_.find(sName);
    if (mIter == lCursorMap_.end()) {
        auto pCursor = std::make_unique<sf::Cursor>();
        pCursor->loadFromSystem(sf::Cursor::Arrow);
        mIter = lCursorMap_.insert(std::make_pair(sName, std::move(pCursor))).first;
    }

    mWindow_.setMouseCursor(*mIter->second);
}

key source::from_sfml_(int uiSFKey) const {
    switch ((sf::Keyboard::Key)uiSFKey) {
    case Keyboard::Escape: return key::K_ESCAPE;
    case Keyboard::Num0: return key::K_0;
    case Keyboard::Num1: return key::K_1;
    case Keyboard::Num2: return key::K_2;
    case Keyboard::Num3: return key::K_3;
    case Keyboard::Num4: return key::K_4;
    case Keyboard::Num5: return key::K_5;
    case Keyboard::Num6: return key::K_6;
    case Keyboard::Num7: return key::K_7;
    case Keyboard::Num8: return key::K_8;
    case Keyboard::Num9: return key::K_9;
    case Keyboard::Dash: return key::K_MINUS;
    case Keyboard::Equal: return key::K_EQUALS;
    case Keyboard::BackSpace: return key::K_BACK;
    case Keyboard::Tab: return key::K_TAB;
    case Keyboard::Q: return key::K_Q;
    case Keyboard::W: return key::K_W;
    case Keyboard::E: return key::K_E;
    case Keyboard::R: return key::K_R;
    case Keyboard::T: return key::K_T;
    case Keyboard::Y: return key::K_Y;
    case Keyboard::U: return key::K_U;
    case Keyboard::I: return key::K_I;
    case Keyboard::O: return key::K_O;
    case Keyboard::P: return key::K_P;
    case Keyboard::LBracket: return key::K_LBRACKET;
    case Keyboard::RBracket: return key::K_RBRACKET;
    case Keyboard::Return: return key::K_RETURN;
    case Keyboard::LControl: return key::K_LCONTROL;
    case Keyboard::A: return key::K_A;
    case Keyboard::S: return key::K_S;
    case Keyboard::D: return key::K_D;
    case Keyboard::F: return key::K_F;
    case Keyboard::G: return key::K_G;
    case Keyboard::H: return key::K_H;
    case Keyboard::J: return key::K_J;
    case Keyboard::K: return key::K_K;
    case Keyboard::L: return key::K_L;
    case Keyboard::SemiColon: return key::K_SEMICOLON;
    case Keyboard::Quote: return key::K_APOSTROPHE;
    case Keyboard::LShift: return key::K_LSHIFT;
    case Keyboard::BackSlash: return key::K_BACKSLASH;
    case Keyboard::Z: return key::K_Z;
    case Keyboard::X: return key::K_X;
    case Keyboard::C: return key::K_C;
    case Keyboard::V: return key::K_V;
    case Keyboard::B: return key::K_B;
    case Keyboard::N: return key::K_N;
    case Keyboard::M: return key::K_M;
    case Keyboard::Comma: return key::K_COMMA;
    case Keyboard::Period: return key::K_PERIOD;
    case Keyboard::Slash: return key::K_SLASH;
    case Keyboard::RShift: return key::K_RSHIFT;
    case Keyboard::Multiply: return key::K_MULTIPLY;
    case Keyboard::LAlt: return key::K_LMENU;
    case Keyboard::Space: return key::K_SPACE;
    case Keyboard::F1: return key::K_F1;
    case Keyboard::F2: return key::K_F2;
    case Keyboard::F3: return key::K_F3;
    case Keyboard::F4: return key::K_F4;
    case Keyboard::F5: return key::K_F5;
    case Keyboard::F6: return key::K_F6;
    case Keyboard::F7: return key::K_F7;
    case Keyboard::F8: return key::K_F8;
    case Keyboard::F9: return key::K_F9;
    case Keyboard::F10: return key::K_F10;
    case Keyboard::Numpad7: return key::K_NUMPAD7;
    case Keyboard::Numpad8: return key::K_NUMPAD8;
    case Keyboard::Numpad9: return key::K_NUMPAD9;
    case Keyboard::Subtract: return key::K_SUBTRACT;
    case Keyboard::Numpad4: return key::K_NUMPAD4;
    case Keyboard::Numpad5: return key::K_NUMPAD5;
    case Keyboard::Numpad6: return key::K_NUMPAD6;
    case Keyboard::Add: return key::K_ADD;
    case Keyboard::Numpad1: return key::K_NUMPAD1;
    case Keyboard::Numpad2: return key::K_NUMPAD2;
    case Keyboard::Numpad3: return key::K_NUMPAD3;
    case Keyboard::Numpad0: return key::K_NUMPAD0;
    case Keyboard::F11: return key::K_F11;
    case Keyboard::F12: return key::K_F12;
    case Keyboard::F13: return key::K_F13;
    case Keyboard::F14: return key::K_F14;
    case Keyboard::F15: return key::K_F15;
    case Keyboard::RControl: return key::K_RCONTROL;
    case Keyboard::Divide: return key::K_DIVIDE;
    case Keyboard::RAlt: return key::K_RMENU;
    case Keyboard::Pause: return key::K_PAUSE;
    case Keyboard::Home: return key::K_HOME;
    case Keyboard::Up: return key::K_UP;
    case Keyboard::PageUp: return key::K_PGUP;
    case Keyboard::Left: return key::K_LEFT;
    case Keyboard::Right: return key::K_RIGHT;
    case Keyboard::End: return key::K_END;
    case Keyboard::Down: return key::K_DOWN;
    case Keyboard::PageDown: return key::K_PGDOWN;
    case Keyboard::Insert: return key::K_INSERT;
    case Keyboard::Delete: return key::K_DELETE;
    case Keyboard::LSystem: return key::K_LWIN;
    case Keyboard::RSystem: return key::K_RWIN;
    case Keyboard::Menu: return key::K_APPS;
    default: return key::K_UNASSIGNED;
    }
}

void source::on_sfml_event(const sf::Event& mEvent) {
    static const mouse_button lMouseFromSFML[3] = {
        mouse_button::LEFT, mouse_button::RIGHT, mouse_button::MIDDLE};

    if (mEvent.type == sf::Event::TextEntered) {
        auto c = mEvent.text.unicode;
        // Remove non printable characters (< 32) and Del. (127)
        if (c >= 32 && c != 127)
            on_text_entered(c);
    } else if (mEvent.type == sf::Event::MouseWheelMoved) {
        mMouse_.fWheel += mEvent.mouseWheel.delta;
        const sf::Vector2i mMousePos = Mouse::getPosition(mWindow_);
        on_mouse_wheel(mEvent.mouseWheel.delta, gui::vector2f(mMousePos.x, mMousePos.y));
    } else if (mEvent.type == sf::Event::Resized) {
        mWindowDimensions_ = gui::vector2ui(mEvent.size.width, mEvent.size.height);
        on_window_resized(mWindowDimensions_);
    } else if (mEvent.type == sf::Event::KeyPressed) {
        key mKey                                             = from_sfml_(mEvent.key.code);
        mKeyboard_.lKeyState[static_cast<std::size_t>(mKey)] = true;
        on_key_pressed(mKey);
    } else if (mEvent.type == sf::Event::KeyReleased) {
        key mKey                                             = from_sfml_(mEvent.key.code);
        mKeyboard_.lKeyState[static_cast<std::size_t>(mKey)] = false;
        on_key_released(mKey);
    } else if (mEvent.type == sf::Event::MouseButtonPressed) {
        mouse_button mButton = lMouseFromSFML[mEvent.mouseButton.button];
        mMouse_.lButtonState[static_cast<std::size_t>(mButton)] = true;

        const sf::Vector2i mMousePos = Mouse::getPosition(mWindow_);
        on_mouse_pressed(mButton, gui::vector2f(mMousePos.x, mMousePos.y));
    } else if (mEvent.type == sf::Event::MouseButtonReleased) {
        mouse_button mButton = lMouseFromSFML[mEvent.mouseButton.button];
        mMouse_.lButtonState[static_cast<std::size_t>(mButton)] = false;

        const sf::Vector2i mMousePos = Mouse::getPosition(mWindow_);
        on_mouse_released(mButton, gui::vector2f(mMousePos.x, mMousePos.y));
    } else if (mEvent.type == sf::Event::MouseMoved) {
        gui::vector2i mMousePos(mEvent.mouseMove.x, mEvent.mouseMove.y);
        gui::vector2i mMouseDelta;
        if (!bFirstMouseMove_) {
            mMouseDelta   = mMousePos - mOldMousePos_;
            mOldMousePos_ = mMousePos;
        }

        bFirstMouseMove_ = false;

        mMouse_.mPosition = gui::vector2f(mMousePos.x, mMousePos.y);
        on_mouse_moved(gui::vector2f(mMouseDelta.x, mMouseDelta.y), mMouse_.mPosition);
    }
}
}} // namespace lxgui::input::sfml
