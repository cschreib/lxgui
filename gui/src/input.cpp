#include "lxgui/input.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_out.hpp"
#include <lxgui/utils_exception.hpp>
#include <lxgui/utils_string.hpp>
#include <iostream>

namespace input
{
void source_impl::update()
{
    update_();
    lChars_.clear();
    std::swap(lChars_, lCharsCache_);
}

bool source_impl::is_manually_updated() const
{
    return bManuallyUpdated_;
}

void source_impl::set_manually_updated(bool bManuallyUpdated)
{
    bManuallyUpdated_ = bManuallyUpdated;
}

const source_impl::key_state& source_impl::get_key_state() const
{
    return mKeyboard_;
}

const std::vector<char32_t>& source_impl::get_chars() const
{
    return lChars_;
}

const source_impl::mouse_state& source_impl::get_mouse_state() const
{
    return mMouse_;
}

bool source_impl::has_window_resized() const
{
    return bWindowResized_;
}

void source_impl::reset_window_resized()
{
    bWindowResized_ = false;
}

uint source_impl::get_window_new_width() const
{
    return uiNewWindowWidth_;
}

uint source_impl::get_window_new_height() const
{
    return uiNewWindowHeight_;
}

manager::manager(std::unique_ptr<source_impl> pSource) :
    bRemoveFocus_(false), bFocus_(false), pFocusReceiver_(nullptr), bCtrlPressed_(false),
    bShiftPressed_(false), bAltPressed_ (false), bKey_(false),
    dDoubleClickTime_(0.25), fMX_(0.0f), fMY_(0.0f), fRelMX_(0.0f), fRelMY_(0.0f),
    fDMX_(0.0f), fDMY_(0.0f), fRelDMX_(0.0f), fRelDMY_(0.0f), fRawDMX_(0.0f), fRawDMY_(0.0f),
    fMouseSensibility_(1.0f), dMouseHistoryMaxLength_(0.1), dLongPressDelay_(0.7),
    fSmoothDMX_(0.0f), fSmoothDMY_(0.0f), fSmoothMWheel_(0.0f), fMWheel_(0.0f),
    bWheelRolled_(false), bLastDragged_(false), dTime_(0.0), pSource_(std::move(pSource))
{
    lKeyDelay_.fill(false);
    lKeyLong_.fill(false);
    lKeyBuf_.fill(false);
    lKeyBufOld_.fill(false);

    lDoubleClickDelay_.fill(0.0);
    lMouseDelay_.fill(0.0);
    lMouseLong_.fill(false);
    lMouseBuf_.fill(false);
    lMouseBufOld_.fill(false);
    lMouseState_.fill(mouse_state::UP);
}

void manager::allow_input(const std::string& sGroupName)
{
    lClickGroupList_[sGroupName] = true;
}

void manager::block_input(const std::string& sGroupName)
{
    lClickGroupList_[sGroupName] = false;
}

bool manager::can_receive_input(const std::string& sGroupName) const
{
    std::map<std::string, bool>::const_iterator iter = lClickGroupList_.find(sGroupName);
    if (iter != lClickGroupList_.end() && !iter->second)
    {
        iter = lForcedClickGroupList_.find(sGroupName);
        if (iter == lForcedClickGroupList_.end() || !iter->second)
            return false;
    }

    return true;
}

void manager::force_input_allowed(const std::string& sGroupName, bool bForce)
{
    lForcedClickGroupList_[sGroupName] = bForce;
}

bool manager::get_key(bool bForce) const
{
    if (!bForce && bFocus_)
        return false;
    else
        return bKey_;
}

std::string manager::get_key_name(key mKey) const
{
    return pSource_->get_key_name(mKey);
}

std::string manager::get_key_name(key mKey, key mModifier) const
{
    std::string sString;
    switch (mModifier)
    {
        case (key::K_LSHIFT) :
        case (key::K_RSHIFT) :
            sString = "Shift + ";
            break;

        case (key::K_LCONTROL) :
        case (key::K_RCONTROL) :
            sString = "Ctrl + ";
            break;

        case (key::K_LMENU) :
        case (key::K_RMENU) :
            sString = "Alt + ";
            break;

        default :
            sString = get_key_name(mModifier) + " + ";
            break;
    }

    return sString + get_key_name(mKey);
}

std::string manager::get_key_name(key mKey, key mModifier1, key mModifier2) const
{
    std::string sString;
    switch (mModifier1)
    {
        case (key::K_LSHIFT) :
        case (key::K_RSHIFT) :
            sString = "Shift + ";
            break;

        case (key::K_LCONTROL) :
        case (key::K_RCONTROL) :
            sString = "Ctrl + ";
            break;

        case (key::K_LMENU) :
        case (key::K_RMENU) :
            sString = "Alt + ";
            break;

        default :
            sString = get_key_name(mModifier1) + " + ";
            break;
    }

    switch (mModifier2)
    {
        case (key::K_LSHIFT) :
        case (key::K_RSHIFT) :
            sString += "Shift + ";
            break;

        case (key::K_LCONTROL) :
        case (key::K_RCONTROL) :
            sString += "Ctrl + ";
            break;

        case (key::K_LMENU) :
        case (key::K_RMENU) :
            sString += "Alt + ";
            break;

        default :
            sString += get_key_name(mModifier2) + " + ";
            break;
    }

    return sString + get_key_name(mKey);
}

const std::deque<key>& manager::get_key_press_stack() const
{
    return lDownStack_;
}

const std::deque<key>& manager::get_key_release_stack() const
{
    return lUpStack_;
}

bool manager::key_is_down(key mKey, bool bForce) const
{
    if (!bForce && bFocus_)
        return false;
    else
        return lKeyBuf_[(uint)mKey];
}

bool manager::key_is_down_long(key mKey, bool bForce) const
{
    if (!bForce && bFocus_)
        return false;
    else
        return (lKeyBuf_[(uint)mKey] && lKeyLong_[(uint)mKey]);
}

double manager::get_key_press_duration(key mKey) const
{
    return lKeyDelay_[(uint)mKey];
}

bool manager::key_is_pressed(key mKey, bool bForce) const
{
    if (!bForce && bFocus_)
        return false;
    else
        return (lKeyBuf_[(uint)mKey] && !lKeyBufOld_[(uint)mKey]);
}

bool manager::key_is_released(key mKey, bool bForce) const
{
    if (!bForce && bFocus_)
        return false;
    else
        return (!lKeyBuf_[(uint)mKey] && lKeyBufOld_[(uint)mKey]);
}

std::vector<char32_t> manager::get_chars() const
{
    return lChars_;
}

bool manager::mouse_is_down(mouse_button mID) const
{
    return lMouseBuf_[(uint)mID];
}

bool manager::mouse_is_down_long(mouse_button mID) const
{
    return (lMouseBuf_[(uint)mID] && lMouseLong_[(uint)mID]);
}

double manager::get_mouse_press_duration(mouse_button mID) const
{
    return lMouseDelay_[(uint)mID];
}

bool manager::mouse_is_pressed(mouse_button mID) const
{
    return (lMouseBuf_[(uint)mID] && !lMouseBufOld_[(uint)mID]);
}

bool manager::mouse_is_released(mouse_button mID) const
{
    return (!lMouseBuf_[(uint)mID] && lMouseBufOld_[(uint)mID]);
}

bool manager::mouse_is_doubleclicked(mouse_button mID) const
{
    return (mouse_is_pressed(mID) && lDoubleClickDelay_[(uint)mID] > 0.0);
}

bool manager::wheel_is_rolled() const
{
    return bWheelRolled_;
}

void manager::update(float fTempDelta)
{
    if (bRemoveFocus_)
    {
        bFocus_ = false;
        pFocusReceiver_ = nullptr;
        bRemoveFocus_ = false;
    }

    if (!pSource_->is_manually_updated())
        pSource_->update();

    lChars_ = pSource_->get_chars();

    lDownStack_.clear();
    lUpStack_.clear();

    // Control extreme delta time after loading/at startup etc
    double dDelta = fTempDelta;
    if ((dDelta < 0.0) || (dDelta > 1.0))
        dDelta = 0.05;

    gui::event mKeyboardEvent;
    mKeyboardEvent.add(key::K_UNASSIGNED);
    mKeyboardEvent.add(std::string());

    // Update keys
    bKey_ = false;
    for (uint i = 0; i < KEY_NUMBER; ++i)
    {
        lKeyBufOld_[i] = lKeyBuf_[i];

        // Update delays
        if (lKeyBufOld_[i])
        {
            lKeyDelay_[i] += dDelta;
            if (lKeyDelay_[i] >= dLongPressDelay_)
                lKeyLong_[i] = true;
        }
        else
        {
            lKeyDelay_[i] = 0.0;
            lKeyLong_[i] = false;
        }

        // Update state
        lKeyBuf_[i] = pSource_->get_key_state().lKeyState[i];

        if (lKeyBuf_[i])
        {
            bKey_ = true;
            if (!lKeyBufOld_[i])
            {
                // Key is pressed
                lDownStack_.push_back((key)i);
            }
        }
        else if (lKeyBufOld_[i])
        {
            // Key is released
            lUpStack_.push_back((key)i);
        }

        // Send events
        if (lKeyBuf_[i])
        {
            if (!lKeyBufOld_[i])
            {
                mKeyboardEvent.set_name("KEY_PRESSED");
                mKeyboardEvent[0] = (key)i;
                mKeyboardEvent[1] = get_key_name((key)i);
                fire_event_(mKeyboardEvent);
            }
        }
        else if (lKeyBufOld_[i])
        {
            mKeyboardEvent.set_name("KEY_RELEASED");
            mKeyboardEvent[0] = (key)i;
            mKeyboardEvent[1] = get_key_name((key)i);
            fire_event_(mKeyboardEvent);
        }
    }

    if (!lChars_.empty())
    {
        gui::event mCharEvent("TEXT_ENTERED");
        mCharEvent.add(char32_t(0));
        for (auto cChar : lChars_)
        {
            mCharEvent[0] = cChar;
            fire_event_(mCharEvent);
        }
    }

    // Handle modifier keys
    bCtrlPressed_  = key_is_down(key::K_LCONTROL, true) || key_is_down(key::K_RCONTROL, true);
    bShiftPressed_ = key_is_down(key::K_LSHIFT, true) || key_is_down(key::K_RSHIFT, true);
    bAltPressed_   = key_is_down(key::K_LMENU, true) || key_is_down(key::K_RMENU, true);

    const source_impl::mouse_state& mMouseState = pSource_->get_mouse_state();
    gui::event mMouseEvent;
    mMouseEvent.add(mouse_button::LEFT);
    mMouseEvent.add(mMouseState.fAbsX);
    mMouseEvent.add(mMouseState.fAbsY);
    mMouseEvent.add(std::string());

    // Update mouse state
    bLastDragged_ = false;
    for (uint i = 0; i < MOUSE_BUTTON_NUMBER; ++i)
    {
        bool bOldMouseState = lMouseBufOld_[i] = lMouseBuf_[i];

        // Handle double clicking
        lDoubleClickDelay_[i] -= dDelta;

        if (bOldMouseState)
            lDoubleClickDelay_[i] = dDoubleClickTime_;

        // Update delays
        if (bOldMouseState)
        {
            lMouseDelay_[i] += dDelta;
            if (lMouseDelay_[i] >= dLongPressDelay_)
                lMouseLong_[i] = true;
        }
        else
        {
            lMouseDelay_[i] = 0.0;
            lMouseLong_[i] = false;
        }

        // Update state
        bool bMouseState = lMouseBuf_[i] = mMouseState.lButtonState[i];

        // Handle dragging
        if (bMouseState)
        {
            if (!bOldMouseState)
            {
                lMouseState_[i] = mouse_state::CLICKED; // single pressed

                if (lDoubleClickDelay_[i] > 0.0)
                    lMouseState_[i] = mouse_state::DOUBLE; // double clicked
            }
            else
            {
                bLastDragged_ = true;
                lMouseState_[i] = mouse_state::DRAGGED; // dragged
            }
        }
        else if (bOldMouseState)
            lMouseState_[i] = mouse_state::RELEASED; // released
        else
            lMouseState_[i] = mouse_state::UP; // no input

        // Send events
        mMouseEvent[0] = (mouse_button)i;
        mMouseEvent[3] = get_mouse_button_string((mouse_button)i);
        if (bMouseState)
        {
            if (!bOldMouseState)
            {
                mMouseEvent.set_name("MOUSE_PRESSED");
                fire_event_(mMouseEvent, true);

                if (lDoubleClickDelay_[i] > 0.0)
                {
                    mMouseEvent.set_name("MOUSE_DOUBLE_CLICKED");
                    fire_event_(mMouseEvent, true);
                }
            }
        }
        else if (bOldMouseState)
        {
            mMouseEvent.set_name("MOUSE_RELEASED");
            fire_event_(mMouseEvent, true);
        }
    }

    // Update mouse position
    if (mMouseState.bHasDelta)
    {
        fRawDMX_ = fDMX_ = mMouseState.fDX;
        fRawDMY_ = fDMY_ = mMouseState.fDY;
        fRelDMX_ = mMouseState.fRelDX;
        fRelDMY_ = mMouseState.fRelDY;
    }
    else
    {
        fRawDMX_ = fDMX_ = mMouseState.fAbsX - fMX_;
        fRawDMY_ = fDMY_ = mMouseState.fAbsY - fMY_;
        fRelDMX_ = mMouseState.fRelX - fRelMX_;
        fRelDMY_ = mMouseState.fRelY - fRelMY_;
    }

    fMX_    = mMouseState.fAbsX;
    fMY_    = mMouseState.fAbsY;
    fRelMX_ = mMouseState.fRelX;
    fRelMY_ = mMouseState.fRelY;

    fDMX_ *= fMouseSensibility_;
    fDMY_ *= fMouseSensibility_;
    fRelDMX_ *= fMouseSensibility_;
    fRelDMY_ *= fMouseSensibility_;

    fMWheel_ = mMouseState.fRelWheel;
    if (fMWheel_ == 0.0f)
        bWheelRolled_ = false;
    else
        bWheelRolled_ = true;

    if (dMouseHistoryMaxLength_ == 0.0)
    {
        fSmoothDMX_    = fDMX_;
        fSmoothDMY_    = fDMY_;
        fSmoothMWheel_ = fMWheel_;
    }
    else
    {
        std::array<float,3> data;
        data[0] = fDMX_; data[1] = fDMY_; data[2] = fMWheel_;
        lMouseHistory_.push_front(std::make_pair(dTime_, data));

        double dHistoryLength = lMouseHistory_.front().first - lMouseHistory_.back().first;
        while (dHistoryLength > dMouseHistoryMaxLength_ && (lMouseHistory_.size() > 1))
        {
            lMouseHistory_.pop_back();
            dHistoryLength = lMouseHistory_.front().first - lMouseHistory_.back().first;
        }

        fSmoothDMX_ = fSmoothDMY_ = fSmoothMWheel_ = 0.0f;
        float fHistoryWeight = 0.0f;
        float fWeight = 1.0f/lMouseHistory_.size();
        for (auto mHistory : lMouseHistory_)
        {
            fSmoothDMX_    += mHistory.second[0]*fWeight;
            fSmoothDMY_    += mHistory.second[1]*fWeight;
            fSmoothMWheel_ += mHistory.second[2]*fWeight;

            fHistoryWeight += fWeight;
        }

        fSmoothDMX_    /= fHistoryWeight;
        fSmoothDMY_    /= fHistoryWeight;
        fSmoothMWheel_ /= fHistoryWeight;
    }

    // Send movement event
    if (fDMX_ != 0.0 || fDMY_ != 0.0)
    {
        gui::event mMouseMovedEvent("MOUSE_MOVED", true);
        mMouseMovedEvent.add(fDMX_);
        mMouseMovedEvent.add(fDMY_);
        mMouseMovedEvent.add(fDMX_*mMouseState.fRelX/mMouseState.fAbsX);
        mMouseMovedEvent.add(fDMY_*mMouseState.fRelY/mMouseState.fAbsY);
        fire_event_(mMouseMovedEvent, true);
    }

    if (fDMX_ != 0.0 || fDMY_ != 0.0)
    {
        gui::event mMouseMovedEvent("MOUSE_MOVED_RAW", true);
        mMouseMovedEvent.add(fRawDMX_);
        mMouseMovedEvent.add(fRawDMY_);
        mMouseMovedEvent.add(fRawDMX_*mMouseState.fRelX/mMouseState.fAbsX);
        mMouseMovedEvent.add(fRawDMY_*mMouseState.fRelY/mMouseState.fAbsY);
        fire_event_(mMouseMovedEvent, true);
    }

    if (fSmoothDMX_ != 0.0 || fSmoothDMY_ != 0.0)
    {
        gui::event mMouseMovedEvent("MOUSE_MOVED_SMOOTH", true);
        mMouseMovedEvent.add(fSmoothDMX_);
        mMouseMovedEvent.add(fSmoothDMY_);
        mMouseMovedEvent.add(fSmoothDMX_*mMouseState.fRelX/mMouseState.fAbsX);
        mMouseMovedEvent.add(fSmoothDMY_*mMouseState.fRelY/mMouseState.fAbsY);
        fire_event_(mMouseMovedEvent, true);
    }

    if (bWheelRolled_)
    {
        gui::event mMouseWheelEvent("MOUSE_WHEEL", true);
        mMouseWheelEvent.add(fMWheel_);
        fire_event_(mMouseWheelEvent, true);
    }

    if (fSmoothMWheel_ != 0.0)
    {
        gui::event mMouseWheelEvent("MOUSE_WHEEL_SMOOTH", true);
        mMouseWheelEvent.add(fSmoothMWheel_);
        fire_event_(mMouseWheelEvent, true);
    }

    if (pSource_->has_window_resized())
    {
        gui::event mWindowResizedEvent("WINDOW_RESIZED", true);
        mWindowResizedEvent.add(pSource_->get_window_new_width());
        mWindowResizedEvent.add(pSource_->get_window_new_height());
        fire_event_(mWindowResizedEvent, true);
        pSource_->reset_window_resized();
    }

    dTime_ += dDelta;
}

void manager::set_doubleclick_time(double dDoubleClickTime)
{
    dDoubleClickTime_ = dDoubleClickTime;
}

double manager::get_doubleclick_time() const
{
    return dDoubleClickTime_;
}

void manager::set_mouse_buffer_duration(double dMouseHistoryMaxLength)
{
    dMouseHistoryMaxLength_ = dMouseHistoryMaxLength;
}

double manager::get_mouse_buffer_duration() const
{
    return dMouseHistoryMaxLength_;
}

void manager::set_focus(bool bFocus, gui::event_receiver* pReceiver)
{
    if (bFocus_ && !bFocus)
        bRemoveFocus_ = true;
    else
    {
        bRemoveFocus_ = false;
        bFocus_ = bFocus;
        pFocusReceiver_ = pReceiver;
    }
}

bool manager::is_focused() const
{
    return bFocus_;
}

bool manager::alt_is_pressed() const
{
    return bAltPressed_;
}

bool manager::shift_is_pressed() const
{
    return bShiftPressed_;
}

bool manager::ctrl_is_pressed() const
{
    return bCtrlPressed_;
}

bool manager::mouse_last_dragged() const
{
    return bLastDragged_;
}

mouse_state manager::get_mouse_state(mouse_button mID) const
{
    return lMouseState_[(uint)mID];
}

float manager::get_mouse_x() const
{
    return fMX_;
}

float manager::get_mouse_y() const
{
    return fMY_;
}

float manager::get_mouse_rel_x() const
{
    return fRelMX_;
}

float manager::get_mouse_rel_y() const
{
    return fRelMY_;
}

float manager::get_mouse_raw_dx() const
{
    return fRawDMX_;
}

float manager::get_mouse_raw_dy() const
{
    return fRawDMY_;
}

float manager::get_mouse_dx() const
{
    return fDMX_;
}

float manager::get_mouse_dy() const
{
    return fDMY_;
}

float manager::get_mouse_rel_dx() const
{
    return fRelDMX_;
}

float manager::get_mouse_rel_dy() const
{
    return fRelDMY_;
}

float manager::get_mouse_smooth_dx() const
{
    return fSmoothDMX_;
}

float manager::get_mouse_smooth_dy() const
{
    return fSmoothDMY_;
}

float manager::get_mouse_wheel() const
{
    return fMWheel_;
}

void manager::set_mouse_sensibility(float fMouseSensibility)
{
    fMouseSensibility_ = fMouseSensibility;
}

float manager::get_mouse_sensibility() const
{
    return fMouseSensibility_;
}

void manager::set_long_press_delay(double dLongPressDelay)
{
    dLongPressDelay_ = dLongPressDelay;
}

double manager::get_long_press_delay() const
{
    return dLongPressDelay_;
}

std::string manager::get_mouse_button_string(mouse_button mID) const
{
    switch (mID)
    {
        case mouse_button::LEFT :   return "LeftButton";
        case mouse_button::RIGHT :  return "RightButton";
        case mouse_button::MIDDLE : return "MiddleButton";
        default :                   return "";
    }
}

const source_impl* manager::get_source() const
{
    return pSource_.get();
}

source_impl* manager::get_source()
{
    return pSource_.get();
}

void manager::register_event_manager(gui::event_manager* pManager)
{
    if (utils::find(lEventManagerList_, pManager) == lEventManagerList_.end())
        lEventManagerList_.push_back(pManager);
}

void manager::unregister_event_manager(gui::event_manager* pManager)
{
    auto iter = utils::find(lEventManagerList_, pManager);
    if (iter != lEventManagerList_.end())
        lEventManagerList_.erase(iter);
}

void manager::fire_event_(const gui::event& mEvent, bool bForce)
{
    if (pFocusReceiver_ && !bForce)
        pFocusReceiver_->on_event(mEvent);
    else
    {
        for (auto* pManager : lEventManagerList_)
            pManager->fire_event(mEvent);
    }
}
}
