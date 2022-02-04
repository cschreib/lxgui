#include "lxgui/gui_keybinder.hpp"

#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input_dispatcher.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <sol/state.hpp>

namespace lxgui::gui {

utils::connection
keybinder::register_key_binding(std::string_view sName, sol::protected_function mLuaFunction) {
    auto mFunction = [mLuaFunction = std::move(mLuaFunction)]() {
        // Call function
        auto mResult = mLuaFunction();

        // Handle errors
        if (!mResult.valid()) {
            sol::error mError = mResult;
            throw gui::exception(mError.what());
        }
    };

    return register_key_binding(sName, std::move(mFunction));
}

utils::connection keybinder::register_key_binding(std::string_view sName, function_type mFunction) {
    auto mIter = utils::find_if(
        lKeyBindings_, [&](const auto& mBinding) { return mBinding.sName == sName; });

    if (mIter == lKeyBindings_.end()) {
        gui::out << gui::error << "keybinder: a binding already exists with name '" << sName << "'."
                 << std::endl;
        return {};
    }

    key_binding mBinding;
    mBinding.sName   = std::string(sName);
    auto mConnection = mBinding.mSignal.connect(std::move(mFunction));
    lKeyBindings_.push_back(std::move(mBinding));

    return mConnection;
}

void keybinder::set_key_binding(
    std::string_view sName,
    input::key       mKey,
    bool             bShiftIsPressed,
    bool             bCtrlIsPressed,
    bool             bAltIsPressed) {
    auto mIter = utils::find_if(
        lKeyBindings_, [&](const auto& mBinding) { return mBinding.sName == sName; });

    if (mIter == lKeyBindings_.end()) {
        gui::out << gui::error << "keybinder: no binding with name '" << sName << "'." << std::endl;
        return;
    }

    mIter->mKey = mKey;
    mIter->bShiftIsPressed =
        bShiftIsPressed || mKey == input::key::K_LSHIFT || mKey == input::key::K_RSHIFT;
    mIter->bCtrlIsPressed =
        bCtrlIsPressed || mKey == input::key::K_LCONTROL || mKey == input::key::K_RCONTROL;
    mIter->bAltIsPressed =
        bAltIsPressed || mKey == input::key::K_LMENU || mKey == input::key::K_RMENU;
}

void keybinder::set_key_binding(std::string_view sName, std::string_view sKey) {
    bool bShiftIsPressed = false;
    bool bCtrlIsPressed  = false;
    bool bAltIsPressed   = false;

    const auto lTokens = utils::cut(sKey, "-");
    for (auto sToken : lTokens) {
        if (sToken == "Shift")
            bShiftIsPressed = true;
        else if (sToken == "Ctrl")
            bCtrlIsPressed = true;
        else if (sToken == "Alt")
            bAltIsPressed = true;
    }

    set_key_binding(
        sName, input::get_key_from_codename(lTokens.back()), bShiftIsPressed, bCtrlIsPressed,
        bAltIsPressed);
}

void keybinder::remove_key_binding(std::string_view sName) {
    auto mIter = utils::find_if(
        lKeyBindings_, [&](const auto& mBinding) { return mBinding.sName == sName; });

    if (mIter == lKeyBindings_.end()) {
        gui::out << gui::error << "keybinder: no binding with name '" << sName << "'." << std::endl;
        return;
    }

    lKeyBindings_.erase(mIter);
}

keybinder::key_binding* keybinder::find_binding_(
    input::key mKey, bool bShiftIsPressed, bool bCtrlIsPressed, bool bAltIsPressed) {
    auto mIter = utils::find_if(lKeyBindings_, [&](const auto& mBinding) {
        return mBinding.mKey == mKey && mBinding.bShiftIsPressed == bShiftIsPressed &&
               mBinding.bCtrlIsPressed == bCtrlIsPressed && mBinding.bAltIsPressed == bAltIsPressed;
    });

    if (mIter == lKeyBindings_.end())
        return nullptr;

    return &*mIter;
}

bool keybinder::on_key_down(
    input::key mKey, bool bShiftIsPressed, bool bCtrlIsPressed, bool bAltIsPressed) {
    auto* pKeyBinding = find_binding_(mKey, bShiftIsPressed, bCtrlIsPressed, bAltIsPressed);
    if (!pKeyBinding)
        return false;

    try {
        pKeyBinding->mSignal();
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Bound action: " + pKeyBinding->sName + ": " + std::string(e.what()));
    }

    return true;
}

} // namespace lxgui::gui
