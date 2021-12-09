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

bool region::is_in_region(const vector2f& mPosition) const
{
    return ((lBorderList_.left <= mPosition.x && mPosition.x <= lBorderList_.right  - 1) &&
            (lBorderList_.top  <= mPosition.y && mPosition.y <= lBorderList_.bottom - 1));
}
}
}
