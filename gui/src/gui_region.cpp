#include "lxgui/gui_region.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

namespace lxgui {
namespace gui
{
region::region(manager* pManager) : uiobject(pManager)
{
    lType_.push_back(CLASS_NAME);
}

void region::render()
{
}

void region::create_glue()
{
    create_glue_<lua_uiobject>();
}

bool region::is_in_region(int iX, int iY) const
{
    return ((lBorderList_.left <= iX && iX <= lBorderList_.right  - 1) &&
            (lBorderList_.top  <= iY && iY <= lBorderList_.bottom - 1));
}
}
}
