#include "lxgui/input.hpp"
#include "lxgui/input_source.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_exception.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_std.hpp>
#include <lxgui/utils_range.hpp>

#include <iostream>

namespace lxgui {
namespace input
{

manager::manager(utils::control_block& mBlock, source& mSource) :
    event_receiver(mBlock, mSource), mSource_(mSource)
{
    register_event("KEY_PRESSED");
    register_event("KEY_RELEASED");
    register_event("MOUSE_PRESSED");
    register_event("MOUSE_RELEASED");
    register_event("MOUSE_DOUBLE_CLICKED");
    register_event("MOUSE_WHEEL");
    register_event("MOUSE_MOVED");
    register_event("TEXT_ENTERED");
    register_event("WINDOW_RESIZED");
}

void manager::on_event(const gui::event& mOrigEvent)
{
    bool bMouseEvent = mOrigEvent.get_name().find("MOUSE_") == 0u;
    if (bMouseEvent && is_mouse_blocked())
        return;

    gui::event mEvent = mOrigEvent;

    if (mEvent.get_name() == "KEY_PRESSED")
    {
        const auto mKey = mEvent.get<key>(0);
        lKeyPressedTime_[static_cast<std::size_t>(mKey)] = timer::now();
    }
    else if (mEvent.get_name() == "MOUSE_PRESSED" ||
             mEvent.get_name() == "MOUSE_RELEASED" ||
             mEvent.get_name() == "MOUSE_DOUBLE_CLICKED")
    {
        const auto mButton = mEvent.get<mouse_button>(0);

        // Apply scaling factor to mouse coordinates
        for (std::size_t i = 1; i <= 2; ++i)
            mEvent.get(i) = mEvent.get<float>(i)/fScalingFactor_;

        // Record press time
        if (mEvent.get_name() == "MOUSE_PRESSED")
            lMousePressedTime_[static_cast<std::size_t>(mButton)] = timer::now();
    }
    else if (mEvent.get_name() == "MOUSE_MOVED")
    {
        // Apply scaling factor to mouse coordinates
        for (std::size_t i = 0; i < 4; ++i)
            mEvent.get(i) = mEvent.get<float>(i)/fScalingFactor_;
    }

    // Forward event to all registered emitters.
    fire_event_(mEvent);

    if (mEvent.get_name() == "MOUSE_MOVED")
    {
        if (!bMouseDragged_)
        {
            std::size_t uiMouseButtonPressed = std::numeric_limits<std::size_t>::max();
            for (std::size_t i = 0; i < MOUSE_BUTTON_NUMBER; ++i)
            {
                if (mouse_is_down(static_cast<mouse_button>(i)))
                {
                    uiMouseButtonPressed = i;
                    break;
                }
            }

            if (uiMouseButtonPressed != std::numeric_limits<std::size_t>::max())
            {
                bMouseDragged_ = true;
                mMouseDragButton_ = static_cast<mouse_button>(uiMouseButtonPressed);

                gui::event mMouseDragEvent("MOUSE_DRAG_START");
                mMouseDragEvent.add(static_cast<std::underlying_type_t<mouse_button>>(mMouseDragButton_));
                mMouseDragEvent.add(mEvent.get<float>(2));
                mMouseDragEvent.add(mEvent.get<float>(3));
                fire_event_(mMouseDragEvent);
            }
        }
    }
    else if (mEvent.get_name() == "MOUSE_RELEASED")
    {
        if (bMouseDragged_ && mEvent.get<mouse_button>(0) == mMouseDragButton_)
        {
            bMouseDragged_ = false;

            gui::event mMouseDragEvent("MOUSE_DRAG_STOP");
            mMouseDragEvent.add(static_cast<std::underlying_type_t<mouse_button>>(mMouseDragButton_));
            mMouseDragEvent.add(mEvent.get<float>(1));
            mMouseDragEvent.add(mEvent.get<float>(2));
            fire_event_(mMouseDragEvent);
        }
    }
}

void manager::block_mouse_events(bool bBlock)
{
    bMouseBlocked_ = bBlock;
}

bool manager::is_mouse_blocked() const
{
    return bMouseBlocked_;
}

bool manager::any_key_is_down() const
{
    const auto& lKeyState = mSource_.get_key_state().lKeyState;
    for (std::size_t i = 1; i < KEY_NUMBER; ++i)
    {
        if (lKeyState[i])
            return true;
    }

    return false;
}

bool manager::key_is_down(key mKey) const
{
    return mSource_.get_key_state().lKeyState[static_cast<std::size_t>(mKey)];
}

double manager::get_key_down_duration(key mKey) const
{
    if (!key_is_down(mKey))
        return 0.0;

    return std::chrono::duration<double>(
        timer::now() - lKeyPressedTime_[static_cast<std::size_t>(mKey)]).count();
}

bool manager::mouse_is_down(mouse_button mID) const
{
    return mSource_.get_mouse_state().lButtonState[static_cast<std::size_t>(mID)];
}

double manager::get_mouse_down_duration(mouse_button mID) const
{
    if (!mouse_is_down(mID))
        return 0.0;

    return std::chrono::duration<double>(
        timer::now() - lMousePressedTime_[static_cast<std::size_t>(mID)]).count();
}

void manager::set_doubleclick_time(double dDoubleClickTime)
{
    mSource_.set_doubleclick_time(dDoubleClickTime);
}

double manager::get_doubleclick_time() const
{
    return mSource_.get_doubleclick_time();
}

void release_focus_to_list(const gui::event_receiver& mReceiver,
    std::vector<utils::observer_ptr<gui::event_receiver>>& lList)
{
    if (lList.empty())
        return;

    // Find receiver in the list
    auto mIter = utils::find_if(lList,
        [&](const auto& pPtr) {
            return pPtr.get() == &mReceiver;
        }
    );

    if (mIter == lList.end())
        return;

    // Set it to null
    *mIter = nullptr;

    // Clean up null entries
    auto mEndIter = std::remove_if(lList.begin(), lList.end(),
        [](const auto& pPtr)
        {
            return pPtr == nullptr;
        }
    );

    lList.erase(mEndIter, lList.end());
}

void request_focus_to_list(utils::observer_ptr<gui::event_receiver> pReceiver,
    std::vector<utils::observer_ptr<gui::event_receiver>>& lList)
{
    auto* pRawPointer = pReceiver.get();
    if (!pRawPointer)
        return;

    // Check if this receiver was already in the focus stack and remove it
    release_focus_to_list(*pRawPointer, lList);

    // Add receiver at the top of the stack
    lList.push_back(std::move(pReceiver));
}

void manager::request_keyboard_focus(utils::observer_ptr<gui::event_receiver> pReceiver)
{
    auto* pOldFocus = get_keyboard_focus_();
    request_focus_to_list(std::move(pReceiver), lKeyboardFocusStack_);
    auto* pNewFocus = get_keyboard_focus_();

    if (pOldFocus != pNewFocus)
    {
        if (pOldFocus)
            pOldFocus->on_event(gui::event("KEYBOARD_FOCUS_LOST"));

        if (pNewFocus)
            pNewFocus->on_event(gui::event("KEYBOARD_FOCUS_GAINED"));
    }
}

void manager::request_mouse_focus(utils::observer_ptr<gui::event_receiver> pReceiver)
{
    auto* pOldFocus = get_mouse_focus_();
    request_focus_to_list(std::move(pReceiver), lMouseFocusStack_);
    auto* pNewFocus = get_mouse_focus_();

    if (pOldFocus != pNewFocus)
    {
        if (pOldFocus)
            pOldFocus->on_event(gui::event("MOUSE_FOCUS_LOST"));

        if (pNewFocus)
            pNewFocus->on_event(gui::event("MOUSE_FOCUS_GAINED"));
    }
}

void manager::release_keyboard_focus(const gui::event_receiver& mReceiver)
{
    auto* pOldFocus = get_keyboard_focus_();
    release_focus_to_list(mReceiver, lKeyboardFocusStack_);
    auto* pNewFocus = get_keyboard_focus_();

    if (pOldFocus != pNewFocus)
    {
        if (pOldFocus)
            pOldFocus->on_event(gui::event("KEYBOARD_FOCUS_LOST"));

        if (pNewFocus)
            pNewFocus->on_event(gui::event("KEYBOARD_FOCUS_GAINED"));
    }
}

void manager::release_mouse_focus(const gui::event_receiver& mReceiver)
{
    auto* pOldFocus = get_mouse_focus_();
    release_focus_to_list(mReceiver, lMouseFocusStack_);
    auto* pNewFocus = get_mouse_focus_();

    if (pOldFocus != pNewFocus)
    {
        if (pOldFocus)
            pOldFocus->on_event(gui::event("MOUSE_FOCUS_LOST"));

        if (pNewFocus)
            pNewFocus->on_event(gui::event("MOUSE_FOCUS_GAINED"));
    }
}

bool manager::is_keyboard_focused() const
{
    return get_keyboard_focus_() != nullptr;
}

bool manager::is_mouse_focused() const
{
    return get_mouse_focus_() != nullptr;
}

bool manager::alt_is_pressed() const
{
    return key_is_down(key::K_LMENU) || key_is_down(key::K_RMENU);
}

bool manager::shift_is_pressed() const
{
    return key_is_down(key::K_LSHIFT) || key_is_down(key::K_RSHIFT);
}

bool manager::ctrl_is_pressed() const
{
    return key_is_down(key::K_LCONTROL) || key_is_down(key::K_RCONTROL);
}

gui::vector2f manager::get_mouse_position() const
{
    return mSource_.get_mouse_state().mPosition/fScalingFactor_;
}

float manager::get_mouse_wheel() const
{
    return mSource_.get_mouse_state().fWheel;
}

const source& manager::get_source() const
{
    return mSource_;
}

source& manager::get_source()
{
    return mSource_;
}

void manager::register_event_emitter(utils::observer_ptr<gui::event_emitter> pEmitter)
{
    gui::event_emitter* pEmitterRaw = pEmitter.get();
    auto mIter = utils::find_if(lEventEmitterList_,
        [&](const auto& pPtr)
        {
            return pPtr.get() == pEmitterRaw;
        }
    );

    if (mIter != lEventEmitterList_.end())
    {
        gui::out << gui::warning << "event emitter " << pEmitterRaw
            << " has already been registered" << std::endl;
        return;
    }

    lEventEmitterList_.push_back(std::move(pEmitter));
}

void manager::unregister_event_emitter(gui::event_emitter& mEmitter)
{
    auto mIter = utils::find_if(lEventEmitterList_,
        [&](const auto& pPtr)
        {
            return pPtr.get() == &mEmitter;
        }
    );

    if (mIter != lEventEmitterList_.end())
        lEventEmitterList_.erase(mIter);
}

void manager::set_interface_scaling_factor(float fScalingFactor)
{
    fScalingFactor_ = fScalingFactor;
}

float manager::get_interface_scaling_factor() const
{
    return fScalingFactor_;
}

gui::event_receiver* manager::get_keyboard_focus_() const
{
    for (const auto& pPtr : utils::range::reverse(lKeyboardFocusStack_))
    {
        if (auto* pRawPointer = pPtr.get())
            return pRawPointer;
    }

    return nullptr;
}

gui::event_receiver* manager::get_mouse_focus_() const
{
    for (const auto& pPtr : utils::range::reverse(lMouseFocusStack_))
    {
        if (auto* pRawPointer = pPtr.get())
            return pRawPointer;
    }

    return nullptr;
}

void manager::fire_event_(const gui::event& mEvent)
{
    bool bMouseEvent = mEvent.get_name().find("MOUSE_") == 0u;
    bool bKeyboardEvent = mEvent.get_name().find("KEY_") == 0u || mEvent.get_name() == "TEXT_ENTERED";

    if (bMouseEvent)
    {
        if (auto* pMouseFocusReceiver = get_mouse_focus_())
        {
            pMouseFocusReceiver->on_event(mEvent);
            return;
        }
    }
    else if (bKeyboardEvent)
    {
        if (auto* pKeyboardFocusReceiver = get_keyboard_focus_())
        {
            pKeyboardFocusReceiver->on_event(mEvent);
            return;
        }
    }

    for (const auto& pManager : lEventEmitterList_)
    {
        if (auto* pManagerRaw = pManager.get())
            pManagerRaw->fire_event(mEvent);
    }
}

}
}
