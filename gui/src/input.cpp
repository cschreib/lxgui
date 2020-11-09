#include "lxgui/input.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_exception.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_std.hpp>

#include <iostream>

namespace lxgui {
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

std::vector<gui::event> source_impl::poll_events()
{
    std::vector<gui::event> lTemp;
    std::swap(lTemp, lEvents_);
    return lTemp;
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

void source_impl::set_doubleclick_time(double dDoubleClickTime)
{
    dDoubleClickTime_ = dDoubleClickTime;
}

double source_impl::get_doubleclick_time() const
{
    return dDoubleClickTime_;
}

manager::manager(std::unique_ptr<source_impl> pSource) : pSource_(std::move(pSource))
{
    lKeyDelay_.fill(false);
    lKeyLong_.fill(false);

    lMouseDelay_.fill(0.0);
    lMouseLong_.fill(false);
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
    switch (mKey)
    {
    case key::K_ESCAPE       : return "Escape";
    case key::K_1            : return "1";
    case key::K_2            : return "2";
    case key::K_3            : return "3";
    case key::K_4            : return "4";
    case key::K_5            : return "5";
    case key::K_6            : return "6";
    case key::K_7            : return "7";
    case key::K_8            : return "8";
    case key::K_9            : return "9";
    case key::K_0            : return "0";
    case key::K_MINUS        : return "-";
    case key::K_EQUALS       : return "=";
    case key::K_BACK         : return "Backspace";
    case key::K_TAB          : return "Tab";
    case key::K_Q            : return "Q";
    case key::K_W            : return "W";
    case key::K_E            : return "E";
    case key::K_R            : return "R";
    case key::K_T            : return "T";
    case key::K_Y            : return "Y";
    case key::K_U            : return "U";
    case key::K_I            : return "I";
    case key::K_O            : return "O";
    case key::K_P            : return "P";
    case key::K_LBRACKET     : return "<";
    case key::K_RBRACKET     : return ">";
    case key::K_RETURN       : return "Enter";
    case key::K_LCONTROL     : return "Ctrl";
    case key::K_A            : return "A";
    case key::K_S            : return "S";
    case key::K_D            : return "D";
    case key::K_F            : return "F";
    case key::K_G            : return "G";
    case key::K_H            : return "H";
    case key::K_J            : return "J";
    case key::K_K            : return "K";
    case key::K_L            : return "L";
    case key::K_SEMICOLON    : return ";";
    case key::K_APOSTROPHE   : return "'";
    case key::K_GRAVE        : return "`";
    case key::K_LSHIFT       : return "Shift";
    case key::K_BACKSLASH    : return "\\";
    case key::K_Z            : return "Z";
    case key::K_X            : return "X";
    case key::K_C            : return "C";
    case key::K_V            : return "V";
    case key::K_B            : return "B";
    case key::K_N            : return "N";
    case key::K_M            : return "M";
    case key::K_COMMA        : return ",";
    case key::K_PERIOD       : return ".";
    case key::K_SLASH        : return "/";
    case key::K_RSHIFT       : return "Shift (R)";
    case key::K_MULTIPLY     : return "*";
    case key::K_LMENU        : return "Alt";
    case key::K_SPACE        : return "Space";
    case key::K_CAPITAL      : return "";
    case key::K_F1           : return "F1";
    case key::K_F2           : return "F2";
    case key::K_F3           : return "F3";
    case key::K_F4           : return "F4";
    case key::K_F5           : return "F5";
    case key::K_F6           : return "F6";
    case key::K_F7           : return "F7";
    case key::K_F8           : return "F8";
    case key::K_F9           : return "F9";
    case key::K_F10          : return "F10";
    case key::K_NUMLOCK      : return "Num. Lock";
    case key::K_SCROLL       : return "Scr. Lock";
    case key::K_NUMPAD7      : return "7 (Num.)";
    case key::K_NUMPAD8      : return "8 (Num.)";
    case key::K_NUMPAD9      : return "9 (Num.)";
    case key::K_SUBTRACT     : return "- (Num.)";
    case key::K_NUMPAD4      : return "4 (Num.)";
    case key::K_NUMPAD5      : return "5 (Num.)";
    case key::K_NUMPAD6      : return "6 (Num.)";
    case key::K_ADD          : return "+ (Num.)";
    case key::K_NUMPAD1      : return "1 (Num.)";
    case key::K_NUMPAD2      : return "2 (Num.)";
    case key::K_NUMPAD3      : return "3 (Num.)";
    case key::K_NUMPAD0      : return "0 (Num.)";
    case key::K_DECIMAL      : return ". (Num.)";
    case key::K_F11          : return "F11";
    case key::K_F12          : return "F12";
    case key::K_F13          : return "F13";
    case key::K_F14          : return "F14";
    case key::K_F15          : return "F15";
    case key::K_NUMPADEQUALS : return "= (Num.)";
    case key::K_PREVTRACK    : return "Prev. Track";
    case key::K_NEXTTRACK    : return "Next Track";
    case key::K_NUMPADENTER  : return "Enter (Num.)";
    case key::K_RCONTROL     : return "Ctrl (R)";
    case key::K_MUTE         : return "Mute";
    case key::K_CALCULATOR   : return "Calculator";
    case key::K_PLAYPAUSE    : return "Play";
    case key::K_MEDIASTOP    : return "Stop";
    case key::K_VOLUMEDOWN   : return "Vol. Down";
    case key::K_VOLUMEUP     : return "Vol. Up";
    case key::K_WEBHOME      : return "Web Home";
    case key::K_DIVIDE       : return "/ (Num.)";
    case key::K_SYSRQ        : return "Prt. Scn.";
    case key::K_RMENU        : return "Alt (R)";
    case key::K_PAUSE        : return "Pause";
    case key::K_HOME         : return "Home";
    case key::K_UP           : return "Up";
    case key::K_PGUP         : return "Page Up";
    case key::K_LEFT         : return "Left";
    case key::K_RIGHT        : return "Right";
    case key::K_END          : return "End";
    case key::K_DOWN         : return "Down";
    case key::K_PGDOWN       : return "Page Down";
    case key::K_INSERT       : return "Insert";
    case key::K_DELETE       : return "Delete";
    case key::K_LWIN         : return "Win (L)";
    case key::K_RWIN         : return "Win (R)";
    case key::K_APPS         : return "Apps";
    case key::K_POWER        : return "Power";
    case key::K_SLEEP        : return "Sleep";
    case key::K_WAKE         : return "Wake";
    case key::K_WEBSEARCH    : return "Web Search";
    case key::K_WEBFAVORITES : return "Web Favorites";
    case key::K_WEBREFRESH   : return "Web Refresh";
    case key::K_WEBSTOP      : return "Web Stop";
    case key::K_WEBFORWARD   : return "Web Forward";
    case key::K_WEBBACK      : return "Web Back";
    case key::K_MYCOMPUTER   : return "My Computer";
    case key::K_MAIL         : return "Mail";
    case key::K_MEDIASELECT  : return "Media Select";
    default                  : return "";
    }
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

bool manager::key_is_down(key mKey, bool bForce) const
{
    if (!bForce && bFocus_)
        return false;
    else
        return pSource_->get_key_state().lKeyState[(uint)mKey];
}

bool manager::key_is_down_long(key mKey, bool bForce) const
{
    if (!bForce && bFocus_)
        return false;
    else
        return lKeyLong_[(uint)mKey];
}

double manager::get_key_down_duration(key mKey) const
{
    return lKeyDelay_[(uint)mKey];
}

std::vector<char32_t> manager::get_chars() const
{
    return lChars_;
}

bool manager::mouse_is_down(mouse_button mID) const
{
    return pSource_->get_mouse_state().lButtonState[(uint)mID];
}

bool manager::mouse_is_down_long(mouse_button mID) const
{
    return lMouseLong_[(uint)mID];
}

double manager::get_mouse_down_duration(mouse_button mID) const
{
    return lMouseDelay_[(uint)mID];
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

    // Control extreme delta time after loading/at startup etc
    double dDelta = fTempDelta;
    if ((dDelta < 0.0) || (dDelta > 1.0))
        dDelta = 0.05;

    // Update keys
    const source_impl::key_state& mKeyState = pSource_->get_key_state();
    bKey_ = false;
    for (uint i = 0; i < KEY_NUMBER; ++i)
    {
        // Update delays
        if (mKeyState.lKeyState[i])
        {
            bKey_ = true;
            lKeyDelay_[i] += dDelta;
            if (lKeyDelay_[i] >= dLongPressDelay_)
                lKeyLong_[i] = true;
        }
        else
        {
            lKeyDelay_[i] = 0.0;
            lKeyLong_[i] = false;
        }
    }

    // Handle modifier keys
    bCtrlPressed_  = key_is_down(key::K_LCONTROL, true) || key_is_down(key::K_RCONTROL, true);
    bShiftPressed_ = key_is_down(key::K_LSHIFT, true) || key_is_down(key::K_RSHIFT, true);
    bAltPressed_   = key_is_down(key::K_LMENU, true) || key_is_down(key::K_RMENU, true);

    // Update mouse state
    const source_impl::mouse_state& mMouseState = pSource_->get_mouse_state();
    for (uint i = 0; i < MOUSE_BUTTON_NUMBER; ++i)
    {
        // Update delays
        if (mMouseState.lButtonState[i])
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
    }

    // Send events
    for (auto& mEvent : pSource_->poll_events())
    {
        if (mEvent.get_name() == "KEY_PRESSED" || mEvent.get_name() == "KEY_RELEASED")
        {
            // Add key name to the event
            mEvent.add(get_key_name(utils::get<key>(mEvent.get(0))));
        }
        else if (mEvent.get_name() == "MOUSE_PRESSED" ||
                 mEvent.get_name() == "MOUSE_RELEASED" ||
                 mEvent.get_name() == "MOUSE_DOUBLE_CLICKED")
        {
            // Add button name to the event
            mEvent.add(get_mouse_button_string(mEvent.get<mouse_button>(0)));
        }

        fire_event_(mEvent);
    }

    lChars_ = pSource_->get_chars();
    if (!lChars_.empty())
    {
        gui::event mCharEvent("TEXT_ENTERED");
        mCharEvent.add(std::uint32_t{});
        for (auto cChar : lChars_)
        {
            mCharEvent.get(0) = cChar;
            fire_event_(mCharEvent);
        }
    }

    // Update mouse position
    if (mMouseState.bHasDelta)
    {
        fDMX_    = mMouseState.fDX;
        fDMY_    = mMouseState.fDY;
        fRelDMX_ = mMouseState.fRelDX;
        fRelDMY_ = mMouseState.fRelDY;
    }
    else
    {
        fDMX_    = mMouseState.fAbsX - fMX_;
        fDMY_    = mMouseState.fAbsY - fMY_;
        fRelDMX_ = mMouseState.fRelX - fRelMX_;
        fRelDMY_ = mMouseState.fRelY - fRelMY_;
    }

    fMX_    = mMouseState.fAbsX;
    fMY_    = mMouseState.fAbsY;
    fRelMX_ = mMouseState.fRelX;
    fRelMY_ = mMouseState.fRelY;

    fMWheel_ = mMouseState.fRelWheel;
    if (fMWheel_ == 0.0f)
        bWheelRolled_ = false;
    else
        bWheelRolled_ = true;

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

    if (bWheelRolled_)
    {
        gui::event mMouseWheelEvent("MOUSE_WHEEL", true);
        mMouseWheelEvent.add(fMWheel_);
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
    pSource_->set_doubleclick_time(dDoubleClickTime);
}

double manager::get_doubleclick_time() const
{
    return pSource_->get_doubleclick_time();
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

float manager::get_mouse_wheel() const
{
    return fMWheel_;
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

utils::ustring manager::get_clipboard_content()
{
    return pSource_->get_clipboard_content();
}

void manager::set_clipboard_content(const utils::ustring& sContent)
{
    return pSource_->set_clipboard_content(sContent);
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
}
