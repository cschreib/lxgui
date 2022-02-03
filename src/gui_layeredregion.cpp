#include "lxgui/gui_layeredregion.hpp"

#include "lxgui/gui_region.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"

#include <sstream>

namespace lxgui {
namespace gui
{

layered_region::layered_region(utils::control_block& mBlock, manager& mManager) :
    base(mBlock, mManager)
{
    lType_.push_back(CLASS_NAME);
}

std::string layered_region::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;
    sStr << base::serialize(sTab);

    sStr << sTab << "  # Layer       : ";
    switch (mLayer_)
    {
        case layer::BACKGROUND : sStr << "BACKGROUND\n"; break;
        case layer::BORDER : sStr << "BORDER\n"; break;
        case layer::ARTWORK : sStr << "ARTWORK\n"; break;
        case layer::OVERLAY : sStr << "OVERLAY\n"; break;
        case layer::HIGHLIGHT : sStr << "HIGHLIGHT\n"; break;
        case layer::SPECIALHIGH : sStr << "SPECIALHIGH\n"; break;
        default : sStr << "<error>\n"; break;
    }

    return sStr.str();
}

void layered_region::create_glue()
{
    create_glue_(this);
}

utils::owner_ptr<region> layered_region::release_from_parent()
{
    if (!pParent_)
        return nullptr;

    return pParent_->remove_region(utils::static_pointer_cast<layered_region>(observer_from_this()));
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

layer layered_region::get_draw_layer() const
{
    return mLayer_;
}

void layered_region::set_draw_layer(layer mLayer)
{
    if (mLayer_ != mLayer)
    {
        mLayer_ = mLayer;
        notify_renderer_need_redraw();
        pParent_->notify_layers_need_update();
    }
}

void layered_region::set_draw_layer(const std::string& sLayer)
{
    set_draw_layer(parse_layer_type(sLayer));
}

void layered_region::notify_renderer_need_redraw()
{
    if (bVirtual_)
        return;

    if (pParent_)
        pParent_->notify_renderer_need_redraw();
}

layer parse_layer_type(const std::string& sLayer)
{
    layer mLayer;
    if (sLayer == "ARTWORK")
        mLayer = layer::ARTWORK;
    else if (sLayer == "BACKGROUND")
        mLayer = layer::BACKGROUND;
    else if (sLayer == "BORDER")
        mLayer = layer::BORDER;
    else if (sLayer == "HIGHLIGHT")
        mLayer = layer::HIGHLIGHT;
    else if (sLayer == "OVERLAY")
        mLayer = layer::OVERLAY;
    else
    {
        gui::out << gui::warning << "gui::parse_layer_type : "
            << "Unknown layer type : \"" << sLayer << "\". Using \"ARTWORK\"." << std::endl;

        mLayer = layer::ARTWORK;
    }

    return mLayer;
}

}
}
