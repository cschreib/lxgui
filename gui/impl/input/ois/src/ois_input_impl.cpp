#include "lxgui/impl/ois_input_impl.hpp"
#include <lxgui/utils_exception.hpp>
#include <lxgui/utils_string.hpp>
#include <OIS.h>

namespace input
{
class ois_key_listener : public OIS::KeyListener
{
    ois_manager& mManager;

public :

    ois_key_listener(ois_manager& manager) : mManager(manager) {}

    bool keyPressed(const OIS::KeyEvent& arg) {
        if (arg.text >= 32)
            mManager.lCharsCache_.push_back(arg.text);
        return true;
    }

    bool keyReleased(const OIS::KeyEvent& arg) {
        return true;
    }
};

ois_manager::ois_manager(const std::string& sWindowHandle, float fScreenWidth, float fScreenHeight, bool bMouseGrab) :
    fScreenWidth_(fScreenWidth), fScreenHeight_(fScreenHeight),
    sWindowHandle_(sWindowHandle), bMouseGrab_(bMouseGrab),
    fOldMouseX_(-6666.6f), fOldMouseY_(-6666.6f),
    pKeyListener_(new ois_key_listener(*this))
{
    create_ois_();
}

ois_manager::~ois_manager()
{
    delete_ois_();
}

void ois_manager::create_ois_()
{
    std::multimap<std::string, std::string> mPL;
    mPL.insert(std::make_pair(std::string("WINDOW"), sWindowHandle_));

#ifdef WIN32
    mPL.insert(std::make_pair(std::string("w32_mouse"),     std::string("DISCL_FOREGROUND")));
    if (bMouseGrab_)
        mPL.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_EXCLUSIVE")));
    else
        mPL.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
    mPL.insert(std::make_pair(std::string("w32_keyboard"),  std::string("DISCL_FOREGROUND")));
    mPL.insert(std::make_pair(std::string("w32_keyboard"),  std::string("DISCL_NONEXCLUSIVE")));
#else
    if (bMouseGrab_)
    {
        mPL.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("true")));
        mPL.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("true")));
    }
    else
    {
        mPL.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
        mPL.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
    }

    mPL.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
    mPL.insert(std::make_pair(std::string("XAutoRepeatOn"),     std::string("true")));
#endif

    pOISInputMgr_ = OIS::InputManager::createInputSystem(mPL);

    if (pOISInputMgr_)
    {
        pKeyboard_ = static_cast<OIS::Keyboard*>(pOISInputMgr_->createInputObject(OIS::OISKeyboard, true));
        pKeyboard_->setTextTranslation(OIS::Keyboard::Unicode);
        pKeyboard_->setEventCallback(pKeyListener_.get());

        pMouse_ = static_cast<OIS::Mouse*>(pOISInputMgr_->createInputObject(OIS::OISMouse, false));

        // Oww... these are "mutable" attributes, and can be changed even
        // if the object is declared "const". This is ugly, but it's not
        // my code ;)
        const OIS::MouseState& mMouse_ = pMouse_->getMouseState();
        mMouse_.width = fScreenWidth_;
        mMouse_.height = fScreenHeight_;
    }
    else
        throw utils::exception("ois_input_impl", "Couldn't create OIS input system.");
}

void ois_manager::delete_ois_()
{
    pOISInputMgr_->destroyInputObject(pMouse_);
    pOISInputMgr_->destroyInputObject(pKeyboard_);
    OIS::InputManager::destroyInputSystem(pOISInputMgr_);
}

void ois_manager::toggle_mouse_grab()
{
    bMouseGrab_ = !bMouseGrab_;
    delete_ois_();
    fOldMouseX_ = fOldMouseY_ = -6666.6f;
    create_ois_();
}

void ois_manager::update_()
{
    pKeyboard_->capture();

    char lTempBuff[key::K_MAXKEY];
    pKeyboard_->copyKeyStates(lTempBuff);

    for (int i = 0; i < key::K_MAXKEY; ++i)
        mKeyboard_.lKeyState[i] = (lTempBuff[i] != 0);

    pMouse_->capture();

    OIS::MouseState mMouseState = pMouse_->getMouseState();
    mMouse_.fAbsX = mMouseState.X.abs;
    mMouse_.fAbsY = mMouseState.Y.abs;
    mMouse_.fRelX = mMouseState.X.abs/fScreenWidth_;
    mMouse_.fRelY = mMouseState.Y.abs/fScreenHeight_;

    mMouse_.bHasDelta = true;
    if (fOldMouseX_ != -6666.6f || fOldMouseY_ != -6666.6f)
    {
        mMouse_.fDX    = mMouse_.fAbsX - fOldMouseX_;
        mMouse_.fDY    = mMouse_.fAbsY - fOldMouseY_;
        mMouse_.fRelDX = mMouse_.fRelX - fOldMouseX_/fScreenWidth_;
        mMouse_.fRelDY = mMouse_.fRelY - fOldMouseY_/fScreenHeight_;
    }
    else
        mMouse_.fDX = mMouse_.fDY = mMouse_.fRelDX = mMouse_.fRelDY = 0.0f;

    fOldMouseX_ = mMouse_.fAbsX;
    fOldMouseY_ = mMouse_.fAbsY;

    /*mMouse_.fDX = mMouseState.X.rel;
    mMouse_.fDY = mMouseState.Y.rel;*/

    mMouse_.fRelWheel = mMouseState.Z.rel/120.0f;

    for (int i = 0; i < INPUT_MOUSE_BUTTON_NUMBER; ++i)
        mMouse_.lButtonState[i] = mMouseState.buttonDown((OIS::MouseButtonID)i);
}
}
