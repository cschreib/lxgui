#include "lxgui/input_source.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_exception.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_std.hpp>

#include <iostream>

namespace lxgui {
namespace input
{

void source::update()
{
    update_();
    lChars_.clear();
    std::swap(lChars_, lCharsCache_);
}

bool source::is_manually_updated() const
{
    return bManuallyUpdated_;
}

void source::set_manually_updated(bool bManuallyUpdated)
{
    bManuallyUpdated_ = bManuallyUpdated;
}

const source::key_state& source::get_key_state() const
{
    return mKeyboard_;
}

const std::vector<char32_t>& source::get_chars() const
{
    return lChars_;
}

std::vector<gui::event> source::poll_events()
{
    std::vector<gui::event> lTemp;
    std::swap(lTemp, lEvents_);
    return lTemp;
}

const source::mouse_state& source::get_mouse_state() const
{
    return mMouse_;
}

bool source::has_window_resized() const
{
    return bWindowResized_;
}

void source::reset_window_resized()
{
    bWindowResized_ = false;
}

uint source::get_window_width() const
{
    return uiWindowWidth_;
}

uint source::get_window_height() const
{
    return uiWindowHeight_;
}

void source::set_doubleclick_time(double dDoubleClickTime)
{
    dDoubleClickTime_ = dDoubleClickTime;
}

double source::get_doubleclick_time() const
{
    return dDoubleClickTime_;
}

float source::get_interface_scaling_factor_hint() const
{
    return 1.0f;
}

}
}
