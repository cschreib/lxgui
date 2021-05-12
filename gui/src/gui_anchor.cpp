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
anchor::anchor(uiobject* pObj, anchor_point mPoint, const std::string& sParent, anchor_point mParentPoint) :
    pObj_(pObj), mParentPoint_(mParentPoint), mPoint_(mPoint), sParent_(sParent)
{
}

void anchor::update_parent() const
{
    if (bParentUpdated_) return;

    pParent_ = nullptr;
    bParentUpdated_ = true;

    if (sParent_.empty()) return;

    const uiobject* pObjParent = pObj_->get_parent();

    if (pObjParent)
    {
        utils::replace(sParent_, "$parent", pObjParent->get_lua_name());
    }
    else if (sParent_.find("$parent") != sParent_.npos)
    {
        gui::out << gui::warning << "gui::" << pObj_->get_object_type() << " : "
            << "uiobject \"" << pObj_->get_name() << "\" tries to anchor to \""
            << sParent_ << "\", but '$parent' does not exist." << std::endl;
        return;
    }

    const uiobject* pNewParent = pObj_->get_manager()->get_uiobject_by_name(sParent_);
    if (!pNewParent)
    {
        gui::out << gui::warning << "gui::" << pObj_->get_object_type() << " : "
            << "uiobject \"" << pObj_->get_name() << "\" tries to anchor to \""
            << sParent_ << "\" but this widget does not (yet?) exist." << std::endl;
        return;
    }

    if (pObj_->get_top_level_renderer() != pNewParent->get_top_level_renderer())
    {
        gui::out << gui::warning << "gui::" << pObj_->get_object_type() << " : "
            << "uiobject \"" << pObj_->get_name() << "\" tries to anchor to \""
            << sParent_ << "\" which is in another renderer." << std::endl;
        return;
    }

    pParent_ = pNewParent;
}

float anchor::get_abs_x() const
{
    if (pObj_)
    {
        update_parent();

        float fParentX;
        if (pParent_)
        {
            fParentX = pParent_->get_left();
            fParentWidth_ = pParent_->get_apparent_width();
        }
        else
        {
            fParentX = 0;
            fParentWidth_ = pObj_->get_top_level_renderer()->get_target_width();
        }

        float fOffset;
        if (mType_ == anchor_type::ABS)
            fOffset = fAbsOffX_;
        else
            fOffset = fRelOffX_*fParentWidth_;

        float fParentOffset = 0.0f;
        switch (mParentPoint_)
        {
            case anchor_point::TOPLEFT: [[fallthrough]];
            case anchor_point::LEFT: [[fallthrough]];
            case anchor_point::BOTTOMLEFT:
                fParentOffset = 0.0f;
                break;
            case anchor_point::TOP: [[fallthrough]];
            case anchor_point::CENTER: [[fallthrough]];
            case anchor_point::BOTTOM:
                fParentOffset = fParentWidth_/2.0f;
                break;
            case anchor_point::TOPRIGHT: [[fallthrough]];
            case anchor_point::RIGHT: [[fallthrough]];
            case anchor_point::BOTTOMRIGHT:
                fParentOffset = fParentWidth_;
                break;
        }

        return fOffset + fParentOffset + fParentX;
    }

    return 0;
}

float anchor::get_abs_y() const
{
    if (pObj_)
    {
        update_parent();

        float fParentY;
        if (pParent_)
        {
            fParentY = pParent_->get_top();
            fParentHeight_ = pParent_->get_apparent_height();
        }
        else
        {
            fParentY = 0;
            fParentHeight_ = pObj_->get_top_level_renderer()->get_target_height();
        }

        float fOffset;
        if (mType_ == anchor_type::ABS)
            fOffset = fAbsOffY_;
        else
            fOffset = fRelOffY_*fParentHeight_;

        float fParentOffset = 0.0f;
        switch (mParentPoint_)
        {
            case anchor_point::TOPLEFT: [[fallthrough]];
            case anchor_point::TOP: [[fallthrough]];
            case anchor_point::TOPRIGHT:
                fParentOffset = 0.0f;
                break;
            case anchor_point::LEFT: [[fallthrough]];
            case anchor_point::CENTER: [[fallthrough]];
            case anchor_point::RIGHT:
                fParentOffset = fParentHeight_/2.0f;
                break;
            case anchor_point::BOTTOMLEFT: [[fallthrough]];
            case anchor_point::BOTTOM: [[fallthrough]];
            case anchor_point::BOTTOMRIGHT:
                fParentOffset = fParentHeight_;
                break;
        }

        return fOffset + fParentOffset + fParentY;
    }

    return 0;
}

const uiobject* anchor::get_object() const
{
    return pObj_;
}

const uiobject* anchor::get_parent() const
{
    update_parent();
    return pParent_;
}

const std::string& anchor::get_parent_raw_name() const
{
    return sParent_;
}

anchor_point anchor::get_point() const
{
    return mPoint_;
}

anchor_point anchor::get_parent_point() const
{
    return mParentPoint_;
}

anchor_type anchor::get_type() const
{
    return mType_;
}

float anchor::get_abs_offset_x() const
{
    return fAbsOffX_;
}

float anchor::get_abs_offset_y() const
{
    return fAbsOffY_;
}

vector2f anchor::get_abs_offset() const
{
    return vector2f(fAbsOffX_, fAbsOffY_);
}

float anchor::get_rel_offset_x() const
{
    return fRelOffX_;
}

float anchor::get_rel_offset_y() const
{
    return fRelOffY_;
}

vector2f anchor::get_rel_offset() const
{
    return vector2f(fRelOffX_, fRelOffY_);
}

void anchor::set_object(uiobject* pObj)
{
    pObj_ = pObj;
}

void anchor::set_parent_raw_name(const std::string& sName)
{
    sParent_ = sName;
    bParentUpdated_ = false;
}

void anchor::set_point(anchor_point mPoint)
{
    mPoint_ = mPoint;
}

void anchor::set_parent_point(anchor_point mParentPoint)
{
    mParentPoint_ = mParentPoint;
}

void anchor::set_abs_offset(float fX, float fY)
{
    fAbsOffX_ = fX;
    fAbsOffY_ = fY;
    mType_ = anchor_type::ABS;
}

void anchor::set_abs_offset(const vector2f& mOffset)
{
    fAbsOffX_ = mOffset.x;
    fAbsOffY_ = mOffset.y;
    mType_ = anchor_type::ABS;
}

void anchor::set_rel_offset(float fX, float fY)
{
    fRelOffX_ = fX;
    fRelOffY_ = fY;
    mType_ = anchor_type::REL;
}

void anchor::set_rel_offset(const vector2f& mOffset)
{
    fRelOffX_ = mOffset.x;
    fRelOffY_ = mOffset.y;
    mType_ = anchor_type::REL;
}

std::string anchor::serialize(const std::string& sTab) const
{
    std::stringstream sStr;

    sStr << sTab << "  |   # Point      : " << get_string_point(mPoint_) << "\n";
    if (pParent_)
    sStr << sTab << "  |   # Parent     : " << pParent_->get_name();
    else
    sStr << sTab << "  |   # Parent     : none";
    if (!sParent_.empty())
    sStr << " (raw name : " << sParent_ << ")\n";
    else
    sStr << "\n";
    sStr << sTab << "  |   # Rel. point : " << get_string_point(mParentPoint_) << "\n";
    if (mType_ == anchor_type::ABS)
    {
    sStr << sTab << "  |   # Offset X   : " << fAbsOffX_ << "\n";
    sStr << sTab << "  |   # Offset Y   : " << fAbsOffY_ << "\n";
    }
    else
    {
    sStr << sTab << "  |   # Offset X   : " << fRelOffX_ << " (" << fRelOffX_*fParentWidth_ << ")\n";
    sStr << sTab << "  |   # Offset Y   : " << fRelOffY_ << " (" << fRelOffY_*fParentHeight_ << ")\n";
    }

    return sStr.str();
}

std::string anchor::get_string_point(anchor_point mPoint)
{
    switch (mPoint)
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
