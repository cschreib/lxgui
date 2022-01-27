#include "lxgui/gui_keybinder.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input_dispatcher.hpp"
#include "lxgui/utils_string.hpp"
#include "lxgui/utils_std.hpp"

#include <sol/state.hpp>

namespace lxgui {
namespace gui
{

void keybinder::register_key_binding(std::string_view sName, sol::protected_function mHandler)
{
    auto mFunction = [mHandler = std::move(mHandler)]()
    {
        // Call function
        auto mResult = mHandler();

        // Handle errors
        if (!mResult.valid())
        {
            sol::error mError = mResult;
            throw gui::exception(mError.what());
        }
    };

    register_key_binding(sName, std::move(mFunction));
}

void keybinder::register_key_binding(std::string_view sName, function_type mFunction)
{
    auto mIter = utils::find_if(lKeyBindings_,
        [&](const auto& mBinding)
        {
            return mBinding.sName == sName;
        }
    );

    if (mIter == lKeyBindings_.end())
    {
        gui::out << gui::error << "keybinder: a binding already exists with name '" <<
            sName << "'." << std::endl;
        return;
    }

    key_binding mBinding;
    mBinding.sName = std::string(sName);
    mBinding.mCallback = std::move(mFunction);
    lKeyBindings_.push_back(std::move(mBinding));
}

void keybinder::set_key_binding(std::string_view sName, input::key mKey,
    bool bShiftIsPressed, bool bCtrlIsPressed, bool bAltIsPressed)
{
    auto mIter = utils::find_if(lKeyBindings_,
        [&](const auto& mBinding)
        {
            return mBinding.sName == sName;
        }
    );

    if (mIter == lKeyBindings_.end())
    {
        gui::out << gui::error << "keybinder: no binding with name '" << sName << "'." << std::endl;
        return;
    }

    mIter->mKey = mKey;
    mIter->bShiftIsPressed = bShiftIsPressed || mKey == input::key::K_LSHIFT || mKey == input::key::K_RSHIFT;
    mIter->bCtrlIsPressed = bCtrlIsPressed || mKey == input::key::K_LCONTROL || mKey == input::key::K_RCONTROL;
    mIter->bAltIsPressed = bAltIsPressed || mKey == input::key::K_LMENU || mKey == input::key::K_RMENU;
}

void keybinder::set_key_binding(std::string_view sName, std::string_view sKey)
{
    bool bShiftIsPressed = false;
    bool bCtrlIsPressed = false;
    bool bAltIsPressed = false;

    const auto lTokens = utils::cut(sKey, "-");
    for (auto sToken : lTokens)
    {
        if (sToken == "Shift")
            bShiftIsPressed = true;
        else if (sToken == "Ctrl")
            bCtrlIsPressed = true;
        else if (sToken == "Alt")
            bAltIsPressed = true;
    }

    set_key_binding(sName, input::get_key_from_codename(lTokens.back()),
        bShiftIsPressed, bCtrlIsPressed, bAltIsPressed);
}

void keybinder::remove_key_binding(std::string_view sName)
{
    auto mIter = utils::find_if(lKeyBindings_,
        [&](const auto& mBinding)
        {
            return mBinding.sName == sName;
        }
    );

    if (mIter == lKeyBindings_.end())
    {
        gui::out << gui::error << "keybinder: no binding with name '" << sName << "'." << std::endl;
        return;
    }

    lKeyBindings_.erase(mIter);
}

const keybinder::key_binding* keybinder::find_binding_(input::key mKey,
    bool bShiftIsPressed, bool bCtrlIsPressed, bool bAltIsPressed) const
{
    auto mIter = utils::find_if(lKeyBindings_,
        [&](const auto& mBinding)
        {
            return mBinding.mKey == mKey &&
                mBinding.bShiftIsPressed == bShiftIsPressed &&
                mBinding.bCtrlIsPressed == bCtrlIsPressed &&
                mBinding.bAltIsPressed == bAltIsPressed;
        }
    );

    if (mIter == lKeyBindings_.end())
        return nullptr;

    return &*mIter;
}

bool keybinder::on_key_down(input::key mKey,
    bool bShiftIsPressed, bool bCtrlIsPressed, bool bAltIsPressed) const
{
    const auto* pKeyBinding = find_binding_(mKey, bShiftIsPressed, bCtrlIsPressed, bAltIsPressed);
    if (!pKeyBinding)
        return false;

    try
    {
        pKeyBinding->mCallback();
    }
    catch (const std::exception& e)
    {
        std::string sError = "Bound action: " + pKeyBinding->sName + ": " + std::string(e.what());
        throw std::runtime_error(std::move(sError));
    }

    return true;
}

}
}
