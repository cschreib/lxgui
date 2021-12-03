#include "lxgui/gui_layeredregion.hpp"

#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

#include <sstream>

namespace lxgui {
namespace gui
{
layered_region::layered_region(manager& mManager) : region(mManager)
{
    lType_.push_back(CLASS_NAME);
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
    }

    return sStr.str();
}

void layered_region::create_glue()
{
    create_glue_(this);
}

utils::owner_ptr<uiobject> layered_region::release_from_parent()
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

layer_type layered_region::get_draw_layer() const
{
    return mLayer_;
}

void layered_region::set_draw_layer(layer_type mLayer)
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
            << "Unknown layer type : \"" << sLayer << "\". Using \"ARTWORK\"." << std::endl;

        mLayer = layer_type::ARTWORK;
    }

    if (mLayer_ != mLayer)
    {
        mLayer_ = mLayer;
        notify_renderer_need_redraw();
        pParent_->notify_layers_need_update();
    }
}

void layered_region::notify_renderer_need_redraw() const
{
    if (bVirtual_)
        return;

    if (pParent_)
        pParent_->notify_renderer_need_redraw();
}
}
}
