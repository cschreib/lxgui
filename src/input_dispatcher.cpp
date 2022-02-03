#include "lxgui/input_dispatcher.hpp"
#include "lxgui/input_source.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"

#include "lxgui/utils_exception.hpp"
#include "lxgui/utils_string.hpp"
#include "lxgui/utils_std.hpp"

#include <iostream>

namespace lxgui {
namespace input
{

dispatcher::dispatcher(source& mSource) : mSource_(mSource)
{
    lConnections_.push_back(mSource.on_key_pressed.connect([&](input::key mKey)
    {
        // Record press time
        lKeyPressedTime_[static_cast<std::size_t>(mKey)] = timer::now();
        // Forward
        on_key_pressed(mKey);
    }));

    lConnections_.push_back(mSource.on_key_released.connect([&](input::key mKey)
    {
        // Forward
        on_key_released(mKey);
    }));

    lConnections_.push_back(mSource.on_text_entered.connect([&](std::uint32_t uiChar)
    {
        // Forward
        on_text_entered(uiChar);
    }));

    lConnections_.push_back(mSource.on_mouse_pressed.connect(
        [&](input::mouse_button mButton, gui::vector2f mMousePos)
        {
            // Apply scaling factor to mouse coordinates
            mMousePos /= fScalingFactor_;

            // Record press time
            auto mTimeLast = lMousePressedTime_[static_cast<std::size_t>(mButton)];
            auto mTimeNow = timer::now();
            lMousePressedTime_[static_cast<std::size_t>(mButton)] = mTimeNow;
            double dClickTime = std::chrono::duration<double>(mTimeNow - mTimeLast).count();

            // Forward
            on_mouse_pressed(mButton, mMousePos);

            if (dClickTime < dDoubleClickTime_)
                on_mouse_double_clicked(mButton, mMousePos);
        }
    ));

    lConnections_.push_back(mSource.on_mouse_released.connect(
        [&](input::mouse_button mButton, gui::vector2f mMousePos)
        {
            // Apply scaling factor to mouse coordinates
            mMousePos /= fScalingFactor_;

            // Forward
            on_mouse_released(mButton, mMousePos);

            if (bMouseDragged_ && mButton == mMouseDragButton_)
            {
                bMouseDragged_ = false;
                on_mouse_drag_stop(mButton, mMousePos);
            }
        }
    ));

    lConnections_.push_back(mSource.on_mouse_wheel.connect(
        [&](float fWheel, gui::vector2f mMousePos)
        {
            // Apply scaling factor to mouse coordinates
            mMousePos /= fScalingFactor_;
            // Forward
            on_mouse_wheel(fWheel, mMousePos);
        }
    ));

    lConnections_.push_back(mSource.on_mouse_moved.connect(
        [&](gui::vector2f mMovement, gui::vector2f mMousePos)
        {
            // Apply scaling factor to mouse coordinates
            mMovement /= fScalingFactor_;
            mMousePos /= fScalingFactor_;

            // Forward
            on_mouse_moved(mMovement, mMousePos);

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
                    on_mouse_drag_start(mMouseDragButton_, mMousePos);
                }
            }
        }
    ));
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
    dDoubleClickTime_ = dDoubleClickTime;
}

double dispatcher::get_doubleclick_time() const
{
    return dDoubleClickTime_;
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

}
}
