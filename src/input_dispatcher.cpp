#include "lxgui/input_dispatcher.hpp"
#include "lxgui/input_source.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_exception.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_std.hpp>

#include <iostream>

namespace lxgui {
namespace input
{

dispatcher::dispatcher(utils::control_block& mBlock, source& mSource, gui::event_emitter& mEventEmitter) :
    event_receiver(mBlock, mSource), mEventEmitter_(mEventEmitter), mSource_(mSource)
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

void dispatcher::on_event(const gui::event& mOrigEvent)
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

void dispatcher::block_mouse_events(bool bBlock)
{
    bMouseBlocked_ = bBlock;
}

bool dispatcher::is_mouse_blocked() const
{
    return bMouseBlocked_;
}

void dispatcher::block_keyboard_events(bool bBlock)
{
    bKeyboardBlocked_ = bBlock;
}

bool dispatcher::is_keyboard_blocked() const
{
    return bKeyboardBlocked_;
}

bool dispatcher::any_key_is_down() const
{
    const auto& lKeyState = mSource_.get_key_state().lKeyState;
    for (std::size_t i = 1; i < KEY_NUMBER; ++i)
    {
        if (lKeyState[i])
            return true;
    }

    return false;
}

bool dispatcher::key_is_down(key mKey) const
{
    return mSource_.get_key_state().lKeyState[static_cast<std::size_t>(mKey)];
}

double dispatcher::get_key_down_duration(key mKey) const
{
    if (!key_is_down(mKey))
        return 0.0;

    return std::chrono::duration<double>(
        timer::now() - lKeyPressedTime_[static_cast<std::size_t>(mKey)]).count();
}

bool dispatcher::mouse_is_down(mouse_button mID) const
{
    return mSource_.get_mouse_state().lButtonState[static_cast<std::size_t>(mID)];
}

double dispatcher::get_mouse_down_duration(mouse_button mID) const
{
    if (!mouse_is_down(mID))
        return 0.0;

    return std::chrono::duration<double>(
        timer::now() - lMousePressedTime_[static_cast<std::size_t>(mID)]).count();
}

void dispatcher::set_doubleclick_time(double dDoubleClickTime)
{
    mSource_.set_doubleclick_time(dDoubleClickTime);
}

double dispatcher::get_doubleclick_time() const
{
    return mSource_.get_doubleclick_time();
}

bool dispatcher::alt_is_pressed() const
{
    return key_is_down(key::K_LMENU) || key_is_down(key::K_RMENU);
}

bool dispatcher::shift_is_pressed() const
{
    return key_is_down(key::K_LSHIFT) || key_is_down(key::K_RSHIFT);
}

bool dispatcher::ctrl_is_pressed() const
{
    return key_is_down(key::K_LCONTROL) || key_is_down(key::K_RCONTROL);
}

gui::vector2f dispatcher::get_mouse_position() const
{
    return mSource_.get_mouse_state().mPosition/fScalingFactor_;
}

float dispatcher::get_mouse_wheel() const
{
    return mSource_.get_mouse_state().fWheel;
}

const source& dispatcher::get_source() const
{
    return mSource_;
}

source& dispatcher::get_source()
{
    return mSource_;
}

void dispatcher::set_interface_scaling_factor(float fScalingFactor)
{
    fScalingFactor_ = fScalingFactor;
}

float dispatcher::get_interface_scaling_factor() const
{
    return fScalingFactor_;
}

void dispatcher::fire_event_(const gui::event& mEvent)
{
    bool bMouseEvent = mEvent.get_name().find("MOUSE_") == 0u;
    if (bMouseEvent && bMouseBlocked_)
        return;

    bool bKeyboardEvent = mEvent.get_name().find("KEY_") == 0u || mEvent.get_name() == "TEXT_ENTERED";
    if (bKeyboardEvent && bKeyboardBlocked_)
        return;

    mEventEmitter_.fire_event(mEvent);
}

}
}
