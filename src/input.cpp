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

manager::manager(utils::control_block& mBlock, std::unique_ptr<source> pSource) :
    event_receiver(mBlock, *pSource), pSource_(std::move(pSource))
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

    lKeyDelay_.fill(0.0);
    lMouseDelay_.fill(0.0);
}

void manager::on_event(const gui::event& mOrigEvent)
{
    gui::event mEvent = mOrigEvent;

    if (mEvent.get_name() == "KEY_PRESSED" || mEvent.get_name() == "KEY_RELEASED")
    {
        // Add key name to the event
        mEvent.add(get_key_name(utils::get<key>(mEvent.get(0))));
    }
    else if (mEvent.get_name() == "MOUSE_PRESSED" ||
             mEvent.get_name() == "MOUSE_RELEASED" ||
             mEvent.get_name() == "MOUSE_DOUBLE_CLICKED")
    {
        // Apply scaling factor to mouse coordinates
        for (std::size_t i = 1; i <= 2; ++i)
            mEvent.get(i) = mEvent.get<float>(i)/fScalingFactor_;
        // Add button name to the event
        mEvent.add(get_mouse_button_string(mEvent.get<mouse_button>(0)));
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
                mMouseDragEvent.add(get_mouse_button_string(mMouseDragButton_));
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
            mMouseDragEvent.add(get_mouse_button_string(mMouseDragButton_));
            fire_event_(mMouseDragEvent);
        }
    }
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
    auto iter = lClickGroupList_.find(sGroupName);
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

bool manager::any_key_is_down() const
{
    const auto& lKeyState = pSource_->get_key_state().lKeyState;
    for (std::size_t i = 1; i < KEY_NUMBER; ++i)
    {
        if (lKeyState[i])
            return true;
    }

    return false;
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

bool manager::key_is_down(key mKey) const
{
    return pSource_->get_key_state().lKeyState[static_cast<std::size_t>(mKey)];
}

double manager::get_key_down_duration(key mKey) const
{
    return lKeyDelay_[static_cast<std::size_t>(mKey)];
}

bool manager::mouse_is_down(mouse_button mID) const
{
    return pSource_->get_mouse_state().lButtonState[static_cast<std::size_t>(mID)];
}

double manager::get_mouse_down_duration(mouse_button mID) const
{
    return lMouseDelay_[static_cast<std::size_t>(mID)];
}

void manager::update(float fTempDelta)
{
    // Control extreme delta time after loading/at startup etc
    double dDelta = fTempDelta;
    if ((dDelta < 0.0) || (dDelta > 1.0))
        dDelta = 0.05;

    // Update keys
    const source::key_state& mKeyState = pSource_->get_key_state();
    for (std::size_t i = 0; i < KEY_NUMBER; ++i)
    {
        // Update delays
        if (mKeyState.lKeyState[i])
            lKeyDelay_[i] += dDelta;
        else
            lKeyDelay_[i] = 0.0;
    }

    // Update mouse state
    const source::mouse_state& mMouseState = pSource_->get_mouse_state();
    for (std::size_t i = 0; i < MOUSE_BUTTON_NUMBER; ++i)
    {
        // Update delays
        if (mMouseState.lButtonState[i])
            lMouseDelay_[i] += dDelta;
        else
            lMouseDelay_[i] = 0.0;
    }
}

void manager::set_doubleclick_time(double dDoubleClickTime)
{
    pSource_->set_doubleclick_time(dDoubleClickTime);
}

double manager::get_doubleclick_time() const
{
    return pSource_->get_doubleclick_time();
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
    return pSource_->get_mouse_state().mPosition/fScalingFactor_;
}

float manager::get_mouse_wheel() const
{
    return pSource_->get_mouse_state().fWheel;
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

const source& manager::get_source() const
{
    return *pSource_;
}

source& manager::get_source()
{
    return *pSource_;
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

utils::ustring manager::get_clipboard_content()
{
    return pSource_->get_clipboard_content();
}

void manager::set_clipboard_content(const utils::ustring& sContent)
{
    return pSource_->set_clipboard_content(sContent);
}

void manager::set_mouse_cursor(const std::string& sFileName, const gui::vector2i& mHotSpot)
{
    return pSource_->set_mouse_cursor(sFileName, mHotSpot);
}

void manager::reset_mouse_cursor()
{
    return pSource_->reset_mouse_cursor();
}

const gui::vector2ui& manager::get_window_dimensions() const
{
    return pSource_->get_window_dimensions();
}

void manager::set_interface_scaling_factor(float fScalingFactor)
{
    fScalingFactor_ = fScalingFactor;
}

float manager::get_interface_scaling_factor() const
{
    return fScalingFactor_;
}

float manager::get_interface_scaling_factor_hint() const
{
    return pSource_->get_interface_scaling_factor_hint();
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
