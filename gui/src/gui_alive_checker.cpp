#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_uiobject.hpp"

namespace lxgui {
namespace gui
{
alive_checker::alive_checker(uiobject& mObject) : pObject_(mObject.observer_from_this())
{
}

bool alive_checker::is_alive() const
{
    return !pObject_.expired();
}

}
}
