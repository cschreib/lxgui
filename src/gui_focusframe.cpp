#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

using namespace lxgui::input;

namespace lxgui {
namespace gui
{
focus_frame::focus_frame(manager& mManager) : frame(mManager)
{
    lType_.push_back(CLASS_NAME);
}

focus_frame::~focus_frame()
{
    if (has_focus())
        get_manager().request_focus(nullptr);
}

void focus_frame::copy_from(const uiobject& mObj)
{
    base::copy_from(mObj);

    const focus_frame* pFocusFrame = down_cast<focus_frame>(&mObj);
    if (!pFocusFrame)
        return;

    this->enable_auto_focus(pFocusFrame->is_auto_focus_enabled());
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
    if (bFocus == bFocus_)
        return;

    if (bFocus)
    {
        notify_focus(true);
        get_manager().request_focus(observer_from(this));
    }
    else if (bFocus_)
    {
        notify_focus(false);
        get_manager().request_focus(nullptr);
    }
}

bool focus_frame::has_focus() const
{
    return bFocus_;
}

void focus_frame::notify_focus(bool bFocus)
{
    bFocus_ = bFocus;
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
