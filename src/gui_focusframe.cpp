#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"
#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/input_dispatcher.hpp"

namespace lxgui {
namespace gui
{
focus_frame::focus_frame(utils::control_block& mBlock, manager& mManager) : frame(mBlock, mManager)
{
    lType_.push_back(CLASS_NAME);

    register_event("KEYBOARD_FOCUS_LOST");
    register_event("KEYBOARD_FOCUS_GAINED");
}

void focus_frame::copy_from(const uiobject& mObj)
{
    base::copy_from(mObj);

    const focus_frame* pFocusFrame = down_cast<focus_frame>(&mObj);
    if (!pFocusFrame)
        return;

    this->enable_auto_focus(pFocusFrame->is_auto_focus_enabled());
}

void focus_frame::on_event(const event& mEvent)
{
    alive_checker mChecker(*this);

    base::on_event(mEvent);
    if (!mChecker.is_alive())
        return;

    if (mEvent.get_name() == "KEYBOARD_FOCUS_LOST")
        notify_focus_(false);
    else if (mEvent.get_name() == "KEYBOARD_FOCUS_GAINED")
        notify_focus_(true);
}

void focus_frame::create_glue()
{
    create_glue_(this);
}

void focus_frame::enable_auto_focus(bool bEnable)
{
    bAutoFocus_ = bEnable;
}

bool focus_frame::is_auto_focus_enabled() const
{
    return bAutoFocus_;
}

void focus_frame::set_focus(bool bFocus)
{
    auto& mInput = get_manager().get_input_dispatcher();
    if (bFocus)
        mInput.request_keyboard_focus(observer_from(this));
    else
        mInput.release_keyboard_focus(*this);
}

bool focus_frame::has_focus() const
{
    return bFocus_;
}

void focus_frame::notify_focus_(bool bFocus)
{
    if (bFocus_ == bFocus)
        return;

    bFocus_ = bFocus;

    if (bFocus_)
        on_script("OnFocusGained");
    else
        on_script("OnFocusLost");
}

void focus_frame::notify_visible()
{
    if (bAutoFocus_)
        set_focus(true);

    base::notify_visible();
}

void focus_frame::notify_invisible()
{
    set_focus(false);

    base::notify_invisible();
}

}
}
