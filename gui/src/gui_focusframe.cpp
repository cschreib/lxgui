#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

#include <lxgui/luapp_state.hpp>

using namespace lxgui::input;

namespace lxgui {
namespace gui
{
focus_frame::focus_frame(manager* pManager) : frame(pManager)
{
    lType_.push_back(CLASS_NAME);
}

focus_frame::~focus_frame()
{
    set_focus(false);
}

void focus_frame::copy_from(uiobject* pObj)
{
    frame::copy_from(pObj);

    focus_frame* pFocusFrame = pObj->down_cast<focus_frame>();
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
        pManager_->request_focus(this);
    else if (bFocus_)
        pManager_->request_focus(nullptr);
}

void focus_frame::notify_focus(bool bFocus)
{
    bFocus_ = bFocus;
}
}
}
