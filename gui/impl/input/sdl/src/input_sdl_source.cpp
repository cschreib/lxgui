#include "lxgui/impl/input_sdl_source.hpp"
#include <lxgui/gui_event.hpp>
#include <lxgui/utils_string.hpp>

#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <SDL_video.h>
#include <SDL_clipboard.h>

namespace lxgui {
namespace input {
namespace sdl
{
source::source(SDL_Window* pWindow, bool bMouseGrab) :
    pWindow_(pWindow), bMouseGrab_(bMouseGrab)
{
    mMouse_.bHasDelta = true;
}

void source::toggle_mouse_grab()
{
    bMouseGrab_ = !bMouseGrab_;
    if (bMouseGrab_)
    {
        int iWidth = 0, iHeight = 0;
        SDL_GetWindowSize(pWindow_, &iWidth, &iHeight);

        fOldMouseX_ = iWidth/2;
        fOldMouseY_ = iHeight/2;
    }
}

utils::ustring source::get_clipboard_content()
{
    if (SDL_HasClipboardText())
    {
        char* sText = SDL_GetClipboardText();
        utils::ustring sUnicodeText = utils::UTF8_to_unicode(sText);
        SDL_free(sText);
        return sUnicodeText;
    }
    else
    {
        return {};
    }
}

void source::set_clipboard_content(const utils::ustring& sContent)
{
    SDL_SetClipboardText(utils::unicode_to_UTF8(sContent).c_str());
}

key source::from_sdl_(int iSDLKey) const
{
    switch (static_cast<SDL_Keycode>(iSDLKey))
    {
    case SDLK_ESCAPE:       return key::K_ESCAPE;
    case SDLK_0:            return key::K_0;
    case SDLK_1:            return key::K_1;
    case SDLK_2:            return key::K_2;
    case SDLK_3:            return key::K_3;
    case SDLK_4:            return key::K_4;
    case SDLK_5:            return key::K_5;
    case SDLK_6:            return key::K_6;
    case SDLK_7:            return key::K_7;
    case SDLK_8:            return key::K_8;
    case SDLK_9:            return key::K_9;
    case SDLK_MINUS:        return key::K_MINUS;
    case SDLK_EQUALS:       return key::K_EQUALS;
    case SDLK_BACKSPACE:    return key::K_BACK;
    case SDLK_TAB:          return key::K_TAB;
    case SDLK_q:            return key::K_Q;
    case SDLK_w:            return key::K_W;
    case SDLK_e:            return key::K_E;
    case SDLK_r:            return key::K_R;
    case SDLK_t:            return key::K_T;
    case SDLK_y:            return key::K_Y;
    case SDLK_u:            return key::K_U;
    case SDLK_i:            return key::K_I;
    case SDLK_o:            return key::K_O;
    case SDLK_p:            return key::K_P;
    case SDLK_LEFTBRACKET:  return key::K_LBRACKET;
    case SDLK_RIGHTBRACKET: return key::K_RBRACKET;
    case SDLK_RETURN:       return key::K_RETURN;
    case SDLK_LCTRL:        return key::K_LCONTROL;
    case SDLK_a:            return key::K_A;
    case SDLK_s:            return key::K_S;
    case SDLK_d:            return key::K_D;
    case SDLK_f:            return key::K_F;
    case SDLK_g:            return key::K_G;
    case SDLK_h:            return key::K_H;
    case SDLK_j:            return key::K_J;
    case SDLK_k:            return key::K_K;
    case SDLK_l:            return key::K_L;
    case SDLK_SEMICOLON:    return key::K_SEMICOLON;
    case SDLK_QUOTE:        return key::K_APOSTROPHE;
    case SDLK_LSHIFT:       return key::K_LSHIFT;
    case SDLK_BACKSLASH:    return key::K_BACKSLASH;
    case SDLK_z:            return key::K_Z;
    case SDLK_x:            return key::K_X;
    case SDLK_c:            return key::K_C;
    case SDLK_v:            return key::K_V;
    case SDLK_b:            return key::K_B;
    case SDLK_n:            return key::K_N;
    case SDLK_m:            return key::K_M;
    case SDLK_COMMA:        return key::K_COMMA;
    case SDLK_PERIOD:       return key::K_PERIOD;
    case SDLK_SLASH:        return key::K_SLASH;
    case SDLK_RSHIFT:       return key::K_RSHIFT;
    case SDLK_ASTERISK:     return key::K_MULTIPLY;
    case SDLK_LALT:         return key::K_LMENU;
    case SDLK_SPACE:        return key::K_SPACE;
    case SDLK_F1:           return key::K_F1;
    case SDLK_F2:           return key::K_F2;
    case SDLK_F3:           return key::K_F3;
    case SDLK_F4:           return key::K_F4;
    case SDLK_F5:           return key::K_F5;
    case SDLK_F6:           return key::K_F6;
    case SDLK_F7:           return key::K_F7;
    case SDLK_F8:           return key::K_F8;
    case SDLK_F9:           return key::K_F9;
    case SDLK_F10:          return key::K_F10;
    case SDLK_KP_7:         return key::K_NUMPAD7;
    case SDLK_KP_8:         return key::K_NUMPAD8;
    case SDLK_KP_9:         return key::K_NUMPAD9;
    case SDLK_KP_MINUS:     return key::K_SUBTRACT;
    case SDLK_KP_4:         return key::K_NUMPAD4;
    case SDLK_KP_5:         return key::K_NUMPAD5;
    case SDLK_KP_6:         return key::K_NUMPAD6;
    case SDLK_KP_PLUS:      return key::K_ADD;
    case SDLK_KP_1:         return key::K_NUMPAD1;
    case SDLK_KP_2:         return key::K_NUMPAD2;
    case SDLK_KP_3:         return key::K_NUMPAD3;
    case SDLK_KP_0:         return key::K_NUMPAD0;
    case SDLK_F11:          return key::K_F11;
    case SDLK_F12:          return key::K_F12;
    case SDLK_F13:          return key::K_F13;
    case SDLK_F14:          return key::K_F14;
    case SDLK_F15:          return key::K_F15;
    case SDLK_RCTRL:        return key::K_RCONTROL;
    case SDLK_KP_DIVIDE:    return key::K_DIVIDE;
    case SDLK_RALT:         return key::K_RMENU;
    case SDLK_PAUSE:        return key::K_PAUSE;
    case SDLK_HOME:         return key::K_HOME;
    case SDLK_UP:           return key::K_UP;
    case SDLK_PAGEUP:       return key::K_PGUP;
    case SDLK_LEFT:         return key::K_LEFT;
    case SDLK_RIGHT:        return key::K_RIGHT;
    case SDLK_END:          return key::K_END;
    case SDLK_DOWN:         return key::K_DOWN;
    case SDLK_PAGEDOWN:     return key::K_PGDOWN;
    case SDLK_INSERT:       return key::K_INSERT;
    case SDLK_DELETE:       return key::K_DELETE;
    case SDLK_APPLICATION:  return key::K_LWIN;
    case SDLK_MENU:         return key::K_APPS;
    default:                return key::K_UNASSIGNED;
    }
}

void source::update_()
{
    int iMouseX, iMouseY;
    SDL_GetMouseState(&iMouseX, &iMouseY);

    int iWidth = 0, iHeight = 0;
    SDL_GetWindowSize(pWindow_, &iWidth, &iHeight);

    if (bFirst_)
    {
        mMouse_.fAbsX = iMouseX;
        mMouse_.fAbsY = iMouseY;
        mMouse_.fRelX = mMouse_.fAbsX/iWidth;
        mMouse_.fRelY = mMouse_.fAbsY/iHeight;

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
        mMouse_.fDX = iMouseX - fOldMouseX_;
        mMouse_.fDY = iMouseY - fOldMouseY_;
        mMouse_.fRelDX = mMouse_.fDX/iWidth;
        mMouse_.fRelDY = mMouse_.fDY/iHeight;

        mMouse_.fAbsX += mMouse_.fDX;
        mMouse_.fAbsY += mMouse_.fDY;
        mMouse_.fRelX = mMouse_.fAbsX/iWidth;
        mMouse_.fRelY = mMouse_.fAbsY/iHeight;

        if (bMouseGrab_)
        {
            SDL_WarpMouseInWindow(pWindow_, fOldMouseX_, fOldMouseY_);
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

void source::on_sdl_event(const SDL_Event& mEvent)
{
    static const mouse_button lMouseFromSDL[3] = {mouse_button::LEFT, mouse_button::MIDDLE, mouse_button::RIGHT};

    switch (mEvent.type)
    {
        case SDL_KEYDOWN:
        {
            key mKey = from_sdl_(mEvent.key.keysym.sym);
            mKeyboard_.lKeyState[(uint)mKey] = true;

            gui::event mKeyboardEvent("KEY_PRESSED");
            mKeyboardEvent.add(static_cast<std::underlying_type_t<key>>(mKey));
            lEvents_.push_back(mKeyboardEvent);
            break;
        }
        case SDL_KEYUP:
        {
            key mKey = from_sdl_(mEvent.key.keysym.sym);
            mKeyboard_.lKeyState[(uint)mKey] = true;

            gui::event mKeyboardEvent("KEY_RELEASED");
            mKeyboardEvent.add(static_cast<std::underlying_type_t<key>>(mKey));
            lEvents_.push_back(mKeyboardEvent);
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            mouse_button mButton = lMouseFromSDL[mEvent.button.button - 1];
            mMouse_.lButtonState[(uint)mButton] = true;

            gui::event mMouseEvent("MOUSE_PRESSED");
            mMouseEvent.add(static_cast<std::underlying_type_t<mouse_button>>(mButton));
            mMouseEvent.add((float)mEvent.button.x);
            mMouseEvent.add((float)mEvent.button.y);
            lEvents_.push_back(mMouseEvent);

            clock::time_point mPrev = lLastClickClock_[(uint)mButton];
            clock::time_point mNow = clock::now();
            double dElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(mNow - mPrev).count()/1000.0;
            if (dElapsed < dDoubleClickTime_)
            {
                mMouseEvent.set_name("MOUSE_DOUBLE_CLICKED");
                lEvents_.push_back(mMouseEvent);
            }

            lLastClickClock_[(uint)mButton] = mNow;
            break;
        }
        case SDL_MOUSEBUTTONUP:
        {
            mouse_button mButton = lMouseFromSDL[mEvent.button.button - 1];
            mMouse_.lButtonState[(uint)mButton] = false;

            gui::event mMouseEvent("MOUSE_RELEASED");
            mMouseEvent.add(static_cast<std::underlying_type_t<mouse_button>>(mButton));
            mMouseEvent.add((float)mEvent.button.x);
            mMouseEvent.add((float)mEvent.button.y);
            lEvents_.push_back(mMouseEvent);
            break;
        }
        case SDL_MOUSEWHEEL:
        {
            fWheelCache_ += (mEvent.wheel.direction == SDL_MOUSEWHEEL_NORMAL ?
                mEvent.wheel.y : -mEvent.wheel.y);
            break;
        }
        case SDL_TEXTINPUT:
        {
            for (auto c : utils::UTF8_to_unicode(mEvent.text.text))
            {
                // Remove non printable characters (< 32) and Del. (127)
                if (c >= 32 && c != 127)
                    lCharsCache_.push_back(c);
            }
            break;
        }
        case SDL_WINDOWEVENT:
        {
            if (mEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED &&
                mEvent.window.windowID == SDL_GetWindowID(pWindow_))
            {
                bWindowResized_ = true;
                int iWidth = 0, iHeight = 0;
                SDL_GetWindowSize(pWindow_, &iWidth, &iHeight);
                uiNewWindowWidth_ = iWidth;
                uiNewWindowHeight_ = iHeight;
            }
            break;
        }
    }
}
}
}
}