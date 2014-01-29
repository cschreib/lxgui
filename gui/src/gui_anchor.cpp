#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_string.hpp>
#include <sstream>

namespace gui
{
anchor::anchor() :
    pObj_(nullptr), mParentPoint_(ANCHOR_TOPLEFT), mPoint_(ANCHOR_TOPLEFT),
    mType_(ANCHOR_ABS), iAbsOffX_(0), iAbsOffY_(0), fRelOffX_(0.0f), fRelOffY_(0.0f),
    iParentWidth_(0u), iParentHeight_(0u), pParent_(nullptr), bParentUpdated_(false)
{
}

anchor::anchor(uiobject* pObj, anchor_point mPoint, const std::string& sParent, anchor_point mParentPoint) :
    pObj_(pObj), mParentPoint_(mParentPoint), mPoint_(mPoint), mType_(ANCHOR_ABS),
    iAbsOffX_(0), iAbsOffY_(0), fRelOffX_(0.0f), fRelOffY_(0.0f),
    iParentWidth_(0u), iParentHeight_(0u), pParent_(nullptr), sParent_(sParent),
    bParentUpdated_(false)
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
        if (mType_ == ANCHOR_ABS)
            iOffset = iAbsOffX_;
        else
            iOffset = fRelOffX_*iParentWidth_;

        int iParentOffset;
        if ((mParentPoint_ == ANCHOR_TOPLEFT) || (mParentPoint_ == ANCHOR_LEFT) || (mParentPoint_ == ANCHOR_BOTTOMLEFT))
            iParentOffset = 0;
        else if ((mParentPoint_ == ANCHOR_TOP) || (mParentPoint_ == ANCHOR_CENTER) || (mParentPoint_ == ANCHOR_BOTTOM))
            iParentOffset = iParentWidth_/2;
        else if ((mParentPoint_ == ANCHOR_TOPRIGHT) || (mParentPoint_ == ANCHOR_RIGHT) || (mParentPoint_ == ANCHOR_BOTTOMRIGHT))
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
        if (mType_ == ANCHOR_ABS)
            iOffset = iAbsOffY_;
        else
            iOffset = fRelOffY_*iParentHeight_;

        int iParentOffset;
        if ((mParentPoint_ == ANCHOR_TOPLEFT) || (mParentPoint_ == ANCHOR_TOP) || (mParentPoint_ == ANCHOR_TOPRIGHT))
            iParentOffset = 0;
        else if ((mParentPoint_ == ANCHOR_LEFT) || (mParentPoint_ == ANCHOR_CENTER) || (mParentPoint_ == ANCHOR_RIGHT))
            iParentOffset = iParentHeight_/2;
        else if ((mParentPoint_ == ANCHOR_BOTTOMLEFT) || (mParentPoint_ == ANCHOR_BOTTOM) || (mParentPoint_ == ANCHOR_BOTTOMRIGHT))
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
    mType_ = ANCHOR_ABS;
}

void anchor::set_abs_offset(const vector2i& mOffset)
{
    iAbsOffX_ = mOffset.x;
    iAbsOffY_ = mOffset.y;
    mType_ = ANCHOR_ABS;
}

void anchor::set_rel_offset(float fX, float fY)
{
    fRelOffX_ = fX;
    fRelOffY_ = fY;
    mType_ = ANCHOR_REL;
}

void anchor::set_rel_offset(const vector2f& mOffset)
{
    fRelOffX_ = mOffset.x;
    fRelOffY_ = mOffset.y;
    mType_ = ANCHOR_REL;
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
    if (mType_ == ANCHOR_ABS)
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
        case ANCHOR_TOPLEFT :     return "TOPLEFT";
        case ANCHOR_TOP :         return "TOP";
        case ANCHOR_TOPRIGHT :    return "TOPRIGHT";
        case ANCHOR_RIGHT :       return "RIGHT";
        case ANCHOR_BOTTOMRIGHT : return "BOTTOMRIGHT";
        case ANCHOR_BOTTOM :      return "BOTTOM";
        case ANCHOR_BOTTOMLEFT :  return "BOTTOMLEFT";
        case ANCHOR_LEFT :        return "LEFT";
        case ANCHOR_CENTER :      return "CENTER";
    };
    return "";
}

anchor_point anchor::get_anchor_point(const std::string& sPoint)
{
    if (sPoint == "TOPLEFT")          return ANCHOR_TOPLEFT;
    else if (sPoint == "TOP")         return ANCHOR_TOP;
    else if (sPoint == "TOPRIGHT")    return ANCHOR_TOPRIGHT;
    else if (sPoint == "RIGHT")       return ANCHOR_RIGHT;
    else if (sPoint == "BOTTOMRIGHT") return ANCHOR_BOTTOMRIGHT;
    else if (sPoint == "BOTTOM")      return ANCHOR_BOTTOM;
    else if (sPoint == "BOTTOMLEFT")  return ANCHOR_BOTTOMLEFT;
    else if (sPoint == "LEFT")        return ANCHOR_LEFT;
    else if (sPoint == "CENTER")      return ANCHOR_CENTER;
    return ANCHOR_TOPLEFT;
}
}
