#include "lxgui/gui_layeredregion.hpp"

#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

#include <sstream>

namespace lxgui {
namespace gui
{
layered_region::layered_region(manager* pManager) : region(pManager)
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
        case layer_type::BACKGROUND : sStr << "BACKGROUND\n"; break;
        case layer_type::BORDER : sStr << "BORDER\n"; break;
        case layer_type::ARTWORK : sStr << "ARTWORK\n"; break;
        case layer_type::OVERLAY : sStr << "OVERLAY\n"; break;
        case layer_type::HIGHLIGHT : sStr << "HIGHLIGHT\n"; break;
        case layer_type::SPECIALHIGH : sStr << "SPECIALHIGH\n"; break;
        default : sStr << "<error>\n"; break;
    };

    return sStr.str();
}

void layered_region::create_glue()
{
    create_glue_<lua_layered_region>();
}

void layered_region::set_parent(uiobject* pParent)
{
    pFrameParent_ = dynamic_cast<frame*>(pParent);
    uiobject::set_parent(pParent);
}

std::unique_ptr<uiobject> layered_region::release_from_parent()
{
    if (pFrameParent_)
        return pFrameParent_->remove_region(this);
    else
        return pManager_->remove_root_uiobject(this);
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
        mLayer = layer_type::ARTWORK;
    else if (sLayer == "BACKGROUND")
        mLayer = layer_type::BACKGROUND;
    else if (sLayer == "BORDER")
        mLayer = layer_type::BORDER;
    else if (sLayer == "HIGHLIGHT")
        mLayer = layer_type::HIGHLIGHT;
    else if (sLayer == "OVERLAY")
        mLayer = layer_type::OVERLAY;
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Uknown layer type : \"" << sLayer << "\". Using \"ARTWORK\"." << std::endl;

        mLayer = layer_type::ARTWORK;
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
}
