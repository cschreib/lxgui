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
    if (!bParentUpdated_)
    {
        if (!sParent_.empty())
        {
            const uiobject* pObjParent = pObj_->get_parent();

            if (pObjParent)
            {
                utils::replace(sParent_, "$parent", pObjParent->get_lua_name());
            }
            else if (sParent_.find("$parent") != sParent_.npos)
            {
                pParent_ = nullptr;
                bParentUpdated_ = true;
                return;
            }

            pParent_ = pObj_->get_manager()->get_uiobject_by_name(sParent_);
            if (!pParent_)
            {
                gui::out << gui::warning << "gui::" << pObj_->get_object_type() << " : "
                    << "uiobject \"" << pObj_->get_name() << "\" tries to anchor to \""
                    << sParent_ << "\" but this widget does not exist." << std::endl;
            }
        }
        else
            pParent_ = nullptr;

        bParentUpdated_ = true;
    }
}

int anchor::get_abs_x() const
{
    if (pObj_)
    {
        update_parent();

        int iParentX;
        if (pParent_)
        {
            iParentX = pParent_->get_left();
            iParentWidth_ = pParent_->get_apparent_width();
        }
        else
        {
            iParentX = 0;
            iParentWidth_ = pObj_->get_manager()->get_screen_width();
        }

        int iOffset;
        if (mType_ == anchor_type::ABS)
            iOffset = iAbsOffX_;
        else
            iOffset = fRelOffX_*iParentWidth_;

        int iParentOffset;
        if ((mParentPoint_ == anchor_point::TOPLEFT) || (mParentPoint_ == anchor_point::LEFT) || (mParentPoint_ == anchor_point::BOTTOMLEFT))
            iParentOffset = 0;
        else if ((mParentPoint_ == anchor_point::TOP) || (mParentPoint_ == anchor_point::CENTER) || (mParentPoint_ == anchor_point::BOTTOM))
            iParentOffset = iParentWidth_/2;
        else if ((mParentPoint_ == anchor_point::TOPRIGHT) || (mParentPoint_ == anchor_point::RIGHT) || (mParentPoint_ == anchor_point::BOTTOMRIGHT))
            iParentOffset = iParentWidth_;
        else iParentOffset = 0;

        return iOffset + iParentOffset + iParentX;
    }

    return 0;
}

int anchor::get_abs_y() const
{
    if (pObj_)
    {
        update_parent();

        int iParentY;
        if (pParent_)
        {
            iParentY = pParent_->get_top();
            iParentHeight_ = pParent_->get_apparent_height();
        }
        else
        {
            iParentY = 0;
            iParentHeight_ = pObj_->get_manager()->get_screen_height();
        }

        int iOffset;
        if (mType_ == anchor_type::ABS)
            iOffset = iAbsOffY_;
        else
            iOffset = fRelOffY_*iParentHeight_;

        int iParentOffset;
        if ((mParentPoint_ == anchor_point::TOPLEFT) || (mParentPoint_ == anchor_point::TOP) || (mParentPoint_ == anchor_point::TOPRIGHT))
            iParentOffset = 0;
        else if ((mParentPoint_ == anchor_point::LEFT) || (mParentPoint_ == anchor_point::CENTER) || (mParentPoint_ == anchor_point::RIGHT))
            iParentOffset = iParentHeight_/2;
        else if ((mParentPoint_ == anchor_point::BOTTOMLEFT) || (mParentPoint_ == anchor_point::BOTTOM) || (mParentPoint_ == anchor_point::BOTTOMRIGHT))
            iParentOffset = iParentHeight_;
        else iParentOffset = 0;

        return iOffset + iParentOffset + iParentY;
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

int anchor::get_abs_offset_x() const
{
    return iAbsOffX_;
}

int anchor::get_abs_offset_y() const
{
    return iAbsOffY_;
}

vector2i anchor::get_abs_offset() const
{
    return vector2i(iAbsOffX_, iAbsOffY_);
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

void anchor::set_abs_offset(int iX, int iY)
{
    iAbsOffX_ = iX;
    iAbsOffY_ = iY;
    mType_ = anchor_type::ABS;
}

void anchor::set_abs_offset(const vector2i& mOffset)
{
    iAbsOffX_ = mOffset.x;
    iAbsOffY_ = mOffset.y;
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
    sStr << sTab << "  |   # Offset X   : " << iAbsOffX_ << "\n";
    sStr << sTab << "  |   # Offset Y   : " << iAbsOffY_ << "\n";
    }
    else
    {
    sStr << sTab << "  |   # Offset X   : " << fRelOffX_ << " (" << fRelOffX_*iParentWidth_ << ")\n";
    sStr << sTab << "  |   # Offset Y   : " << fRelOffY_ << " (" << fRelOffY_*iParentHeight_ << ")\n";
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
    };
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
