#include "lxgui/gui_layeredregion.hpp"

#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

#include <sstream>

namespace gui
{
#ifdef NO_CPP11_CONSTEXPR
const char* layered_region::CLASS_NAME = "LayeredRegion";
#endif

layered_region::layered_region(manager* pManager) : region(pManager),
    mLayer_(LAYER_ARTWORK), pFrameParent_(nullptr)
{
    lType_.push_back(CLASS_NAME);
}

layered_region::~layered_region()
{
}

std::string layered_region::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;
    sStr << region::serialize(sTab);

    sStr << sTab << "  # Layer       : ";
    switch (mLayer_)
    {
        case LAYER_BACKGROUND : sStr << "BACKGROUND\n"; break;
        case LAYER_BORDER : sStr << "BORDER\n"; break;
        case LAYER_ARTWORK : sStr << "ARTWORK\n"; break;
        case LAYER_OVERLAY : sStr << "OVERLAY\n"; break;
        case LAYER_HIGHLIGHT : sStr << "HIGHLIGHT\n"; break;
        case LAYER_SPECIALHIGH : sStr << "SPECIALHIGH\n"; break;
        default : sStr << "<error>\n"; break;
    };

    return sStr.str();
}

void layered_region::create_glue()
{
    if (lGlue_) return;

    utils::wptr<lua::state> pLua = pManager_->get_lua();
    pLua->push_string(sName_);
    lGlue_ = pLua->push_new<lua_layered_region>();
    pLua->set_global(sName_);
    pLua->pop();
}

void layered_region::set_parent(uiobject* pParent)
{
    pFrameParent_ = dynamic_cast<frame*>(pParent);
    uiobject::set_parent(pParent);
}

void layered_region::show()
{
    if (!bIsShown_)
    {
        bIsShown_ = true;
        notify_renderer_need_redraw();
    }
}

void layered_region::hide()
{
    if (bIsShown_)
    {
        bIsShown_ = false;
        notify_renderer_need_redraw();
    }
}

bool layered_region::is_visible() const
{
    return pParent_->is_visible() && bIsShown_;
}

layer_type layered_region::get_draw_layer()
{
    return mLayer_;
}

void layered_region::set_draw_layer(layer_type mLayer)
{
    if (mLayer_ != mLayer)
    {
        mLayer_ = mLayer;
        notify_renderer_need_redraw();
        pFrameParent_->fire_build_layer_list();
    }
}

void layered_region::set_draw_layer(const std::string& sLayer)
{
    layer_type mLayer;
    if (sLayer == "ARTWORK")
        mLayer = LAYER_ARTWORK;
    else if (sLayer == "BACKGROUND")
        mLayer = LAYER_BACKGROUND;
    else if (sLayer == "BORDER")
        mLayer = LAYER_BORDER;
    else if (sLayer == "HIGHLIGHT")
        mLayer = LAYER_HIGHLIGHT;
    else if (sLayer == "OVERLAY")
        mLayer = LAYER_OVERLAY;
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Uknown layer type : \"" << sLayer << "\". Using \"ARTWORK\"." << std::endl;

        mLayer = LAYER_ARTWORK;
    }

    if (mLayer_ != mLayer)
    {
        mLayer_ = mLayer;
        notify_renderer_need_redraw();
        pFrameParent_->fire_build_layer_list();
    }
}

void layered_region::notify_renderer_need_redraw() const
{
    if (!bVirtual_)
    {
        if (pRenderer_)
            pRenderer_->fire_redraw();
        else if (pFrameParent_)
            pFrameParent_->notify_renderer_need_redraw();
    }
}
}
