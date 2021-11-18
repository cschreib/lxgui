#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

#include <lxgui/luapp_state.hpp>

using namespace lxgui::input;

namespace lxgui {
namespace gui
{
focus_frame::focus_frame(manager& mManager) : frame(mManager)
{
    lType_.push_back(CLASS_NAME);
}

void focus_frame::copy_from(const uiobject& mObj)
{
    frame::copy_from(mObj);

    const focus_frame* pFocusFrame = down_cast<focus_frame>(&mObj);
    if (!pFocusFrame)
        return;

    this->enable_auto_focus(pFocusFrame->is_auto_focus_enabled());
}

void focus_frame::create_glue()
{
    create_glue_<lua_focus_frame>();
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
    if (bFocus)
        get_manager().request_focus(observer_from(this));
    else if (bFocus_)
        get_manager().request_focus(nullptr);
}

void focus_frame::notify_focus(bool bFocus)
{
    bFocus_ = bFocus;
}

void focus_frame::notify_visible(bool bTriggerEvents)
{
    if (bAutoFocus_)
        set_focus(true);

    frame::notify_visible(bTriggerEvents);
}

void focus_frame::notify_invisible(bool bTriggerEvents)
{
    set_focus(false);

    frame::notify_invisible(bTriggerEvents);
}

}
}
