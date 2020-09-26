#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_manager.hpp"

namespace lxgui {
namespace gui
{
alive_checker::alive_checker(uiobject* pObject) : pObject_(pObject)
{
    if (pObject_)
    {
        uiID_ = pObject_->get_id();
        pManager_ = pObject_->get_manager();
    }
}

bool alive_checker::is_alive() const
{
    if (!pObject_)
        return false;

    return pManager_->get_uiobject(uiID_) == pObject_;
}

}
}
