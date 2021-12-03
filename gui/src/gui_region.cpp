#include "lxgui/gui_region.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

namespace lxgui {
namespace gui
{
region::region(manager& mManager) : uiobject(mManager)
{
    lType_.push_back(CLASS_NAME);
}

void region::render()
{
}

void region::create_glue()
{
    // The "region" type is not exposed to Lua, just use uiobject.
    create_glue_(static_cast<uiobject*>(this));
}

bool region::is_in_region(float fX, float fY) const
{
    return ((lBorderList_.left <= fX && fX <= lBorderList_.right  - 1) &&
            (lBorderList_.top  <= fY && fY <= lBorderList_.bottom - 1));
}
}
}
