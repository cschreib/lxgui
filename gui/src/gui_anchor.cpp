#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_string.hpp>
#include <sstream>

namespace lxgui {
namespace gui
{

anchor::anchor(const uiobject& mObject, const anchor_data& mAnchor) : anchor_data(mAnchor)
{
    if (!mObject.is_virtual())
    {
        if (sParent == "$default")
            sParent = mObject.get_parent() ? "$parent" : "";

        update_parent_(mObject);
    }
}

void anchor::update_parent_(const uiobject& mObject)
{
    pParent_ = nullptr;

    if (sParent.empty()) return;

    utils::observer_ptr<const frame> pObjParent = mObject.get_parent();

    std::string sParentFullName = sParent;
    if (pObjParent)
    {
        utils::replace(sParentFullName, "$parent", pObjParent->get_lua_name());
    }
    else if (sParentFullName.find("$parent") != sParentFullName.npos)
    {
        gui::out << gui::error << "gui::" << mObject.get_object_type() << " : "
            << "uiobject \"" << mObject.get_name() << "\" tries to anchor to \""
            << sParentFullName << "\", but '$parent' does not exist." << std::endl;
        return;
    }

    utils::observer_ptr<const uiobject> pNewParent =
        mObject.get_manager().get_uiobject_by_name(sParentFullName);

    if (!pNewParent)
    {
        gui::out << gui::error << "gui::" << mObject.get_object_type() << " : "
            << "uiobject \"" << mObject.get_name() << "\" tries to anchor to \""
            << sParentFullName << "\" but this widget does not (yet?) exist." << std::endl;
        return;
    }

    if (mObject.get_top_level_renderer() != pNewParent->get_top_level_renderer())
    {
        gui::out << gui::error << "gui::" << mObject.get_object_type() << " : "
            << "uiobject \"" << mObject.get_name() << "\" tries to anchor to \""
            << sParentFullName << "\" which is in another renderer." << std::endl;
        return;
    }

    pParent_ = pNewParent;
}

float anchor::get_abs_x(const uiobject& mObject) const
{
    float fParentX = 0, fParentWidth = 0;
    if (const uiobject* pRawParent = pParent_.get())
    {
        fParentX = pRawParent->get_left();
        fParentWidth = pRawParent->get_apparent_width();
    }
    else
    {
        fParentWidth = mObject.get_top_level_renderer()->get_target_width();
    }

    float fOffset;
    if (mType == anchor_type::ABS)
        fOffset = mOffset.x;
    else
        fOffset = mOffset.x*fParentWidth;

    fOffset = mObject.round_to_pixel(fOffset, utils::rounding_method::NEAREST_NOT_ZERO);

    float fParentOffset = 0.0f;
    switch (mParentPoint)
    {
        case anchor_point::TOPLEFT: [[fallthrough]];
        case anchor_point::LEFT: [[fallthrough]];
        case anchor_point::BOTTOMLEFT:
            fParentOffset = 0.0f;
            break;
        case anchor_point::TOP: [[fallthrough]];
        case anchor_point::CENTER: [[fallthrough]];
        case anchor_point::BOTTOM:
            fParentOffset = fParentWidth/2.0f;
            break;
        case anchor_point::TOPRIGHT: [[fallthrough]];
        case anchor_point::RIGHT: [[fallthrough]];
        case anchor_point::BOTTOMRIGHT:
            fParentOffset = fParentWidth;
            break;
    }

    return fOffset + fParentOffset + fParentX;
}

float anchor::get_abs_y(const uiobject& mObject) const
{
    float fParentY = 0, fParentHeight = 0;
    if (const uiobject* pRawParent = pParent_.get())
    {
        fParentY = pRawParent->get_top();
        fParentHeight = pRawParent->get_apparent_height();
    }
    else
    {
        fParentHeight = mObject.get_top_level_renderer()->get_target_height();
    }

    float fOffset;
    if (mType == anchor_type::ABS)
        fOffset = mOffset.y;
    else
        fOffset = mOffset.y*fParentHeight;

    fOffset = mObject.round_to_pixel(fOffset, utils::rounding_method::NEAREST_NOT_ZERO);

    float fParentOffset = 0.0f;
    switch (mParentPoint)
    {
        case anchor_point::TOPLEFT: [[fallthrough]];
        case anchor_point::TOP: [[fallthrough]];
        case anchor_point::TOPRIGHT:
            fParentOffset = 0.0f;
            break;
        case anchor_point::LEFT: [[fallthrough]];
        case anchor_point::CENTER: [[fallthrough]];
        case anchor_point::RIGHT:
            fParentOffset = fParentHeight/2.0f;
            break;
        case anchor_point::BOTTOMLEFT: [[fallthrough]];
        case anchor_point::BOTTOM: [[fallthrough]];
        case anchor_point::BOTTOMRIGHT:
            fParentOffset = fParentHeight;
            break;
    }

    return fOffset + fParentOffset + fParentY;
}

std::string anchor::serialize(const std::string& sTab) const
{
    std::stringstream sStr;

    sStr << sTab << "  |   # Point      : " << get_string_point(mPoint) << "\n";
    if (pParent_)
    sStr << sTab << "  |   # Parent     : " << pParent_->get_name();
    else
    sStr << sTab << "  |   # Parent     : none";
    if (!sParent.empty())
    sStr << " (raw name : " << sParent << ")\n";
    else
    sStr << "\n";
    sStr << sTab << "  |   # Rel. point : " << get_string_point(mParentPoint) << "\n";
    if (mType == anchor_type::ABS)
    {
    sStr << sTab << "  |   # Offset X   : " << mOffset.x << "\n";
    sStr << sTab << "  |   # Offset Y   : " << mOffset.y << "\n";
    }
    else
    {
    sStr << sTab << "  |   # Offset X   : " << mOffset.x << " (rel)\n";
    sStr << sTab << "  |   # Offset Y   : " << mOffset.y << " (rel)\n";
    }

    return sStr.str();
}

std::string anchor::get_string_point(anchor_point mP)
{
    switch (mP)
    {
        case anchor_point::TOPLEFT :     return "TOPLEFT";
        case anchor_point::TOP :         return "TOP";
        case anchor_point::TOPRIGHT :    return "TOPRIGHT";
        case anchor_point::RIGHT :       return "RIGHT";
        case anchor_point::BOTTOMRIGHT : return "BOTTOMRIGHT";
        case anchor_point::BOTTOM :      return "BOTTOM";
        case anchor_point::BOTTOMLEFT :  return "BOTTOMLEFT";
        case anchor_point::LEFT :        return "LEFT";
        case anchor_point::CENTER :      return "CENTER";
    }
    return "";
}

anchor_point anchor::get_anchor_point(const std::string& sPoint)
{
    if (sPoint == "TOPLEFT")          return anchor_point::TOPLEFT;
    else if (sPoint == "TOP")         return anchor_point::TOP;
    else if (sPoint == "TOPRIGHT")    return anchor_point::TOPRIGHT;
    else if (sPoint == "RIGHT")       return anchor_point::RIGHT;
    else if (sPoint == "BOTTOMRIGHT") return anchor_point::BOTTOMRIGHT;
    else if (sPoint == "BOTTOM")      return anchor_point::BOTTOM;
    else if (sPoint == "BOTTOMLEFT")  return anchor_point::BOTTOMLEFT;
    else if (sPoint == "LEFT")        return anchor_point::LEFT;
    else if (sPoint == "CENTER")      return anchor_point::CENTER;
    return anchor_point::TOPLEFT;
}

}
}
