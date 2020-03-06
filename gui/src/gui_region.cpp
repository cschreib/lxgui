#include "lxgui/gui_region.hpp"

namespace gui
{
#ifdef NO_CPP11_CONSTEXPR
const char* region::CLASS_NAME = "Region";
#endif

region::region(manager* pManager) : uiobject(pManager)
{
    lType_.push_back(CLASS_NAME);
}

region::~region()
{
}

void region::render()
{
}

void region::create_glue()
{
    if (lGlue_) return;

    utils::wptr<lua::state> pLua = pManager_->get_lua();
    pLua->push_string(sName_);
    lGlue_ = pLua->push_new<lua_uiobject>();
    pLua->set_global(sName_);
    pLua->pop();
}

bool region::is_in_region(int iX, int iY) const
{
    return ((lBorderList_.left <= iX && iX <= lBorderList_.right  - 1) &&
            (lBorderList_.top  <= iY && iY <= lBorderList_.bottom - 1));
}
}
