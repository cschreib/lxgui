#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_std.hpp>

#include <sol/state.hpp>

#include <sstream>

namespace lxgui {
namespace gui
{
uiobject::uiobject(manager* pManager) : pManager_(pManager)
{
    lType_.push_back(CLASS_NAME);
}

uiobject::~uiobject()
{
    if (!bVirtual_)
    {
        // Tell this widget's anchor parents that it is no longer anchored to them
        for (auto& mAnchor : lAnchorList_)
        {
            if (mAnchor && mAnchor->get_parent())
                mAnchor->get_parent()->notify_anchored_object(this, false);

            mAnchor.reset();
        }

        // Replace anchors pointing to this widget by absolute anchors
        // (need to copy the anchored object list, because the objects will attempt to modify it when
        // un-anchored, which would invalidate our iteration)
        std::vector<uiobject*> lTempAnchoredObjectList = std::move(lAnchoredObjectList_);
        for (auto* pObj : lTempAnchoredObjectList)
        {
            std::vector<anchor_point> lAnchoredPointList;
            for (const auto& mAnchor : pObj->get_point_list())
            {
                if (mAnchor && mAnchor->get_parent() == this)
                    lAnchoredPointList.push_back(mAnchor->get_point());
            }

            for (const auto& mPoint : lAnchoredPointList)
            {
                const anchor* pAnchor = pObj->get_point(mPoint);
                anchor mNewAnchor = anchor(pObj, mPoint, "", anchor_point::TOPLEFT);
                vector2f mOffset = pAnchor->get_abs_offset();

                switch (pAnchor->get_parent_point())
                {
                    case anchor_point::TOPLEFT :     mOffset   += lBorderList_.top_left();     break;
                    case anchor_point::TOP :         mOffset.y += lBorderList_.top;            break;
                    case anchor_point::TOPRIGHT :    mOffset   += lBorderList_.top_right();    break;
                    case anchor_point::RIGHT :       mOffset.x += lBorderList_.right;          break;
                    case anchor_point::BOTTOMRIGHT : mOffset   += lBorderList_.bottom_right(); break;
                    case anchor_point::BOTTOM :      mOffset.y += lBorderList_.bottom;         break;
                    case anchor_point::BOTTOMLEFT :  mOffset   += lBorderList_.bottom_left();  break;
                    case anchor_point::LEFT :        mOffset.x += lBorderList_.left;           break;
                    case anchor_point::CENTER :      mOffset   += lBorderList_.center();       break;
                }

                mNewAnchor.set_abs_offset(mOffset);
                pObj->set_point(mNewAnchor);
            }

            pObj->update_anchors_();
        }
    }

    // Unregister this object from the GUI manager
    pManager_->remove_uiobject(this);

    if (lGlue_)
    {
        lGlue_->notify_deleted();
        remove_glue();
    }
}

const manager* uiobject::get_manager() const
{
    return pManager_;
}

manager* uiobject::get_manager()
{
    return pManager_;
}

std::string uiobject::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;

    sStr << sTab << "  # Name        : " << sName_ << " ("+std::string(bReady_ ? "ready" : "not ready")+std::string(bSpecial_ ? ", special)\n" : ")\n");
    sStr << sTab << "  # Raw name    : " << sRawName_ << "\n";
    sStr << sTab << "  # Lua name    : " << sLuaName_ << "\n";
    sStr << sTab << "  # ID          : " << uiID_ << "\n";
    sStr << sTab << "  # Type        : " << lType_.back() << "\n";
    if (pParent_)
    sStr << sTab << "  # Parent      : " << pParent_->get_name() << "\n";
    else
    sStr << sTab << "  # Parent      : none\n";
    sStr << sTab << "  # Num anchors : " << get_num_point() << "\n";
    if (!lAnchorList_.empty())
    {
        sStr << sTab << "  |-###\n";
        for (const auto& mAnchor : lAnchorList_)
        {
            if (mAnchor)
            {
                sStr << mAnchor->serialize(sTab);
                sStr << sTab << "  |-###\n";
            }
        }
    }
    sStr << sTab << "  # Borders :\n";
    sStr << sTab << "  |-###\n";
    sStr << sTab << "  |   # left   : " << lBorderList_.left << "\n";
    sStr << sTab << "  |   # top    : " << lBorderList_.top << "\n";
    sStr << sTab << "  |   # right  : " << lBorderList_.right << "\n";
    sStr << sTab << "  |   # bottom : " << lBorderList_.bottom << "\n";
    sStr << sTab << "  |-###\n";
    sStr << sTab << "  # Alpha       : " << fAlpha_ << "\n";
    sStr << sTab << "  # Shown       : " << bIsShown_ << "\n";
    sStr << sTab << "  # Abs width   : " << fAbsWidth_ << "\n";
    sStr << sTab << "  # Abs height  : " << fAbsHeight_ << "\n";

    return sStr.str();
}

void uiobject::copy_from(uiobject* pObj)
{
    if (pObj)
    {
        bInherits_ = true;
        pInheritance_ = pObj;

        // Inherit properties
        this->set_alpha(pObj->get_alpha());
        this->set_shown(pObj->is_shown());
        this->set_abs_width(pObj->get_abs_width());
        this->set_abs_height(pObj->get_abs_height());

        for (const auto& mAnchor : pObj->get_point_list())
        {
            if (!mAnchor)
                continue;

            if (mAnchor->get_type() == anchor_type::ABS)
            {
                this->set_abs_point(
                    mAnchor->get_point(),
                    mAnchor->get_parent_raw_name(),
                    mAnchor->get_parent_point(),
                    mAnchor->get_abs_offset_x(),
                    mAnchor->get_abs_offset_y()
                );
            }
            else
            {
                this->set_rel_point(
                    mAnchor->get_point(),
                    mAnchor->get_parent_raw_name(),
                    mAnchor->get_parent_point(),
                    mAnchor->get_rel_offset_x(),
                    mAnchor->get_rel_offset_y()
                );
            }
        }
    }
}

const std::string& uiobject::get_name() const
{
    return sName_;
}

const std::string& uiobject::get_lua_name() const
{
    return sLuaName_;
}

const std::string& uiobject::get_raw_name() const
{
    return sRawName_;
}

void uiobject::set_name(const std::string& sName)
{
    if (sName_.empty())
    {
        sName_ = sLuaName_ = sRawName_ = sName;
        if (utils::starts_with(sName_, "$parent"))
        {
            if (pParent_)
                utils::replace(sLuaName_, "$parent", pParent_->get_lua_name());
            else
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : \"" << sName_ << "\" has no parent" << std::endl;
                utils::replace(sLuaName_, "$parent", "");
            }
        }

        if (!bVirtual_)
            sName_ = sLuaName_;
    }
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "set_name() can only be called once." << std::endl;
    }
}

const std::string& uiobject::get_object_type() const
{
    return lType_.back();
}

const std::vector<std::string>& uiobject::get_object_type_list() const
{
    return lType_;
}

bool uiobject::is_object_type(const std::string& sType) const
{
    return utils::find(lType_, sType) != lType_.end();
}

float uiobject::get_alpha() const
{
    return fAlpha_;
}

float uiobject::get_effective_alpha() const
{
    if (pParent_)
    {
        return pParent_->get_effective_alpha()*get_alpha();
    }
    else
    {
        return get_alpha();
    }
}

void uiobject::set_alpha(float fAlpha)
{
    if (fAlpha_ != fAlpha)
    {
        fAlpha_ = fAlpha;
        notify_renderer_need_redraw();
    }
}

void uiobject::show()
{
    if (bIsShown_)
        return;

    bIsShown_ = true;

    if (!bIsVisible_ && (!pParent_ || pParent_->is_visible()))
        notify_visible();
}

void uiobject::hide()
{
    if (!bIsShown_)
        return;

    bIsShown_ = false;

    if (bIsVisible_)
        notify_invisible();
}

void uiobject::set_shown(bool bIsShown)
{
    if (bIsShown == bIsShown_) return;

    bIsShown_ = bIsShown;

    bool bNewVisible = bIsShown_ && (!pParent_ || pParent_->is_visible());
    if (bNewVisible != bIsVisible_)
    {
        if (bNewVisible)
            notify_visible(false);
        else
            notify_invisible(false);
    }
}

bool uiobject::is_shown() const
{
    return bIsShown_;
}

bool uiobject::is_visible() const
{
    return bIsVisible_;
}

void uiobject::set_abs_dimensions(float fAbsWidth, float fAbsHeight)
{
    if (fAbsWidth_ != fAbsWidth || fAbsHeight_ != fAbsHeight)
    {
        if (!bVirtual_)
            pManager_->notify_object_moved();

        fAbsWidth_ = fAbsWidth;
        fAbsHeight_ = fAbsHeight;

        if (!bVirtual_)
        {
            notify_borders_need_update();
            notify_renderer_need_redraw();
        }
    }
}

void uiobject::set_abs_width(float fAbsWidth)
{
    if (fAbsWidth_ != fAbsWidth)
    {
        if (!bVirtual_)
            pManager_->notify_object_moved();

        fAbsWidth_ = fAbsWidth;

        if (!bVirtual_)
        {
            notify_borders_need_update();
            notify_renderer_need_redraw();
        }
    }
}

void uiobject::set_abs_height(float fAbsHeight)
{
    if (fAbsHeight_ != fAbsHeight)
    {
        if (!bVirtual_)
            pManager_->notify_object_moved();

        fAbsHeight_ = fAbsHeight;

        if (!bVirtual_)
        {
            notify_borders_need_update();
            notify_renderer_need_redraw();
        }
    }
}

void uiobject::set_rel_dimensions(float fRelWidth, float fRelHeight)
{
    set_rel_width(fRelWidth);
    set_rel_height(fRelHeight);
}

void uiobject::set_rel_width(float fRelWidth)
{
    if (pParent_)
        set_abs_width(fRelWidth*pParent_->get_apparent_width());
    else
        set_abs_width(fRelWidth*get_top_level_renderer()->get_target_width());
}

void uiobject::set_rel_height(float fRelHeight)
{
    if (pParent_)
        set_abs_height(fRelHeight*pParent_->get_apparent_height());
    else
        set_abs_height(fRelHeight*get_top_level_renderer()->get_target_height());
}

float uiobject::get_abs_width() const
{
    return fAbsWidth_;
}

float uiobject::get_apparent_width() const
{
    update_borders_();
    return lBorderList_.width();
}

float uiobject::get_abs_height() const
{
    return fAbsHeight_;
}

float uiobject::get_apparent_height() const
{
    update_borders_();
    return lBorderList_.height();
}

void uiobject::set_parent(frame* pParent)
{
    if (pParent == this)
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : Cannot call set_parent(this)." << std::endl;
        return;
    }

    if (pParent_ != pParent)
    {
        pParent_ = pParent;

        if (!bVirtual_)
            notify_borders_need_update();
    }
}

void uiobject::set_name_and_parent(const std::string& sName, frame* pParent)
{
    if (pParent == this)
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : Cannot call set_parent(this)." << std::endl;
        return;
    }

    if (pParent_ == pParent && sName == sName_)
        return;

    pParent_ = pParent;
    set_name(sName);

    if (!bVirtual_)
        notify_borders_need_update();
}

const frame* uiobject::get_parent() const
{
    return pParent_;
}

frame* uiobject::get_parent()
{
    return pParent_;
}

std::unique_ptr<uiobject> uiobject::release_from_parent()
{
    return nullptr;
}

void uiobject::destroy()
{
    // Gracefully disappear (trigger events, etc).
    hide();

    // Ignoring the return value destroys the object.
    release_from_parent();
}

uiobject* uiobject::get_base()
{
    return pInheritance_;
}

vector2f uiobject::get_center() const
{
    update_borders_();
    return lBorderList_.center();
}

float uiobject::get_left() const
{
    update_borders_();
    return lBorderList_.left;
}

float uiobject::get_right() const
{
    update_borders_();
    return lBorderList_.right;
}

float uiobject::get_top() const
{
    update_borders_();
    return lBorderList_.top;
}

float uiobject::get_bottom() const
{
    update_borders_();
    return lBorderList_.bottom;
}

const quad2f& uiobject::get_borders() const
{
    update_borders_();
    return lBorderList_;
}

void uiobject::clear_all_points()
{
    bool bHadAnchors = false;
    for (auto& mAnchor : lAnchorList_)
    {
        if (mAnchor)
        {
            mAnchor.reset();
            bHadAnchors = true;
        }
    }

    if (bHadAnchors)
    {
        lDefinedBorderList_ = quad2<bool>(false, false, false, false);

        if (!bVirtual_)
        {
            update_anchors_();
            notify_borders_need_update();
            notify_renderer_need_redraw();
            pManager_->notify_object_moved();
        }
    }
}

void uiobject::set_all_points(const std::string& sObjName)
{
    if (sObjName != sName_)
    {
        clear_all_points();

        lAnchorList_[static_cast<int>(anchor_point::TOPLEFT)].emplace(
            this, anchor_point::TOPLEFT,     sObjName, anchor_point::TOPLEFT);
        lAnchorList_[static_cast<int>(anchor_point::BOTTOMRIGHT)].emplace(
            this, anchor_point::BOTTOMRIGHT, sObjName, anchor_point::BOTTOMRIGHT);

        lDefinedBorderList_ = quad2<bool>(true, true, true, true);

        if (!bVirtual_)
        {
            update_anchors_();
            notify_borders_need_update();
            notify_renderer_need_redraw();
            pManager_->notify_object_moved();
        }
    }
    else
        gui::out << gui::error << "gui::" << lType_.back() << " : Cannot call set_all_points(this)." << std::endl;
}

void uiobject::set_all_points(uiobject* pObj)
{
    if (pObj != this)
    {
        set_all_points(pObj ? pObj->get_name() : "");
    }
    else
        gui::out << gui::error << "gui::" << lType_.back() << " : Cannot call set_all_points(this)." << std::endl;
}

void uiobject::set_abs_point(anchor_point mPoint, const std::string& sParentName, anchor_point mRelativePoint, float fX, float fY)
{
    set_abs_point(mPoint, sParentName, mRelativePoint, vector2f(fX, fY));
}

void uiobject::set_abs_point(anchor_point mPoint, const std::string& sParentName, anchor_point mRelativePoint, const vector2f& mOffset)
{
    auto& mAnchor = lAnchorList_[static_cast<int>(mPoint)].emplace(
        this, mPoint, sParentName, mRelativePoint);

    mAnchor.set_abs_offset(mOffset);

    switch (mPoint)
    {
        case anchor_point::TOPLEFT :
            lDefinedBorderList_.top    = true;
            lDefinedBorderList_.left   = true;
            break;
        case anchor_point::TOP :
            lDefinedBorderList_.top    = true;
            break;
        case anchor_point::TOPRIGHT :
            lDefinedBorderList_.top    = true;
            lDefinedBorderList_.right  = true;
            break;
        case anchor_point::RIGHT :
            lDefinedBorderList_.right  = true;
            break;
        case anchor_point::BOTTOMRIGHT :
            lDefinedBorderList_.bottom = true;
            lDefinedBorderList_.right  = true;
            break;
        case anchor_point::BOTTOM :
            lDefinedBorderList_.bottom = true;
            break;
        case anchor_point::BOTTOMLEFT :
            lDefinedBorderList_.bottom = true;
            lDefinedBorderList_.left   = true;
            break;
        case anchor_point::LEFT :
            lDefinedBorderList_.left   = true;
            break;
        default : break;
    }

    if (!bVirtual_)
    {
        update_anchors_();
        notify_borders_need_update();
        notify_renderer_need_redraw();
        pManager_->notify_object_moved();
    }
}

void uiobject::set_rel_point(anchor_point mPoint, const std::string& sParentName, anchor_point mRelativePoint, float fX, float fY)
{
    set_rel_point(mPoint, sParentName, mRelativePoint, vector2f(fX, fY));
}

void uiobject::set_rel_point(anchor_point mPoint, const std::string& sParentName, anchor_point mRelativePoint, const vector2f& mOffset)
{
    auto& mAnchor = lAnchorList_[static_cast<int>(mPoint)].emplace(
        this, mPoint, sParentName, mRelativePoint);

    mAnchor.set_rel_offset(mOffset);

    switch (mPoint)
    {
        case anchor_point::TOPLEFT :
            lDefinedBorderList_.top    = true;
            lDefinedBorderList_.left   = true;
            break;
        case anchor_point::TOP :
            lDefinedBorderList_.top    = true;
            break;
        case anchor_point::TOPRIGHT :
            lDefinedBorderList_.top    = true;
            lDefinedBorderList_.right  = true;
            break;
        case anchor_point::RIGHT :
            lDefinedBorderList_.right  = true;
            break;
        case anchor_point::BOTTOMRIGHT :
            lDefinedBorderList_.bottom = true;
            lDefinedBorderList_.right  = true;
            break;
        case anchor_point::BOTTOM :
            lDefinedBorderList_.bottom = true;
            break;
        case anchor_point::BOTTOMLEFT :
            lDefinedBorderList_.bottom = true;
            lDefinedBorderList_.left   = true;
            break;
        case anchor_point::LEFT :
            lDefinedBorderList_.left   = true;
            break;
        default : break;
    }

    if (!bVirtual_)
    {
        update_anchors_();
        notify_borders_need_update();
        notify_renderer_need_redraw();
        pManager_->notify_object_moved();
    }
}

void uiobject::set_point(const anchor& mAnchor)
{
    lAnchorList_[static_cast<int>(mAnchor.get_point())].emplace(mAnchor);

    switch (mAnchor.get_point())
    {
        case anchor_point::TOPLEFT :
            lDefinedBorderList_.top    = true;
            lDefinedBorderList_.left   = true;
            break;
        case anchor_point::TOP :
            lDefinedBorderList_.top    = true;
            break;
        case anchor_point::TOPRIGHT :
            lDefinedBorderList_.top    = true;
            lDefinedBorderList_.right  = true;
            break;
        case anchor_point::RIGHT :
            lDefinedBorderList_.right  = true;
            break;
        case anchor_point::BOTTOMRIGHT :
            lDefinedBorderList_.bottom = true;
            lDefinedBorderList_.right  = true;
            break;
        case anchor_point::BOTTOM :
            lDefinedBorderList_.bottom = true;
            break;
        case anchor_point::BOTTOMLEFT :
            lDefinedBorderList_.bottom = true;
            lDefinedBorderList_.left   = true;
            break;
        case anchor_point::LEFT :
            lDefinedBorderList_.left   = true;
            break;
        default : break;
    }

    if (!bVirtual_)
    {
        update_anchors_();
        notify_borders_need_update();
        notify_renderer_need_redraw();
        pManager_->notify_object_moved();
    }
}

bool uiobject::depends_on(const uiobject* pObj) const
{
    if (pObj)
    {
        for (const auto& mAnchor : lAnchorList_)
        {
            if (!mAnchor)
                continue;

            const uiobject* pParent = mAnchor->get_parent();
            if (pParent == pObj)
                return true;

            if (pParent)
                return pParent->depends_on(pObj);
        }
    }

    return false;
}

uint uiobject::get_num_point() const
{
    uint uiNumAnchors = 0u;
    for (const auto& mAnchor : lAnchorList_)
    {
        if (mAnchor)
            ++uiNumAnchors;
    }

    return uiNumAnchors;
}

anchor* uiobject::modify_point(anchor_point mPoint)
{
    pManager_->notify_object_moved();

    if (!bVirtual_)
        notify_borders_need_update();

    auto& mAnchor = lAnchorList_[static_cast<int>(mPoint)];
    if (mAnchor)
        return &(*mAnchor);
    else
        return nullptr;
}

const anchor* uiobject::get_point(anchor_point mPoint) const
{
    const auto& mAnchor = lAnchorList_[static_cast<int>(mPoint)];
    if (mAnchor)
        return &(*mAnchor);
    else
        return nullptr;
}

const std::array<std::optional<anchor>,9>& uiobject::get_point_list() const
{
    return lAnchorList_;
}

bool uiobject::is_virtual() const
{
    return bVirtual_;
}

void uiobject::set_virtual()
{
    bVirtual_ = true;
}

uint uiobject::get_id() const
{
    return uiID_;
}

void uiobject::set_id(uint uiID)
{
    if (uiID_ == uint(-1))
        uiID_ = uiID;
    else
        gui::out << gui::warning << "gui::" << lType_.back() << " : set_id() cannot be called more than once." << std::endl;
}

void uiobject::notify_anchored_object(uiobject* pObj, bool bAnchored) const
{
    if (!pObj)
        return;

    auto mIter = utils::find(lAnchoredObjectList_, pObj);
    if (bAnchored)
    {
        if (mIter == lAnchoredObjectList_.end())
            lAnchoredObjectList_.push_back(pObj);
    }
    else
    {
        if (mIter != lAnchoredObjectList_.end())
            lAnchoredObjectList_.erase(mIter);
    }
}

namespace
{
    float round_internal(float fValue, float fUnit, rounding_method mMethod)
    {
        switch (mMethod)
        {
        case rounding_method::NEAREST:
            return std::round(fValue/fUnit)*fUnit;
        case rounding_method::NEAREST_NOT_ZERO:
            if (fValue > 0.0f)
                return std::max(1.0f, std::round(fValue/fUnit)*fUnit);
            else if (fValue < 0.0f)
                return std::min(-1.0f, std::round(fValue/fUnit)*fUnit);
            else
                return 0.0f;
        case rounding_method::UP:
            return std::ceil(fValue/fUnit)*fUnit;
        case rounding_method::DOWN:
            return std::floor(fValue/fUnit)*fUnit;
        default:
            return std::round(fValue/fUnit)*fUnit;
        }
    }
}

float uiobject::round_to_pixel(float fValue, rounding_method mMethod) const
{
    float fScalingFactor = pManager_->get_interface_scaling_factor();
    return round_internal(fValue, 1.0f/fScalingFactor, mMethod);
}

vector2f uiobject::round_to_pixel(const vector2f& mPosition, rounding_method mMethod) const
{
    float fScalingFactor = pManager_->get_interface_scaling_factor();
    return vector2f(round_internal(mPosition.x, 1.0f/fScalingFactor, mMethod),
                    round_internal(mPosition.y, 1.0f/fScalingFactor, mMethod));
}

void uiobject::make_borders_(float& fMin, float& fMax, float fCenter, float fSize) const
{
    if (std::isinf(fMin) && std::isinf(fMax))
    {
        if (!std::isinf(fSize) && fSize > 0.0f && !std::isinf(fCenter))
        {
            fMin = fCenter - fSize/2.0f;
            fMax = fCenter + fSize/2.0f;
        }
        else
            bReady_ = false;
    }
    else if (std::isinf(fMax))
    {
        if (!std::isinf(fSize) && fSize > 0.0f)
            fMax = fMin + fSize;
        else if (!std::isinf(fCenter))
            fMax = fMin + 2.0f*(fCenter - fMin);
        else
            bReady_ = false;
    }
    else if (std::isinf(fMin))
    {
        if (!std::isinf(fSize) && fSize > 0.0f)
            fMin = fMax - fSize;
        else if (!std::isinf(fCenter))
            fMin = fMax - 2.0f*(fMax - fCenter);
        else
            bReady_ = false;
    }
}

void uiobject::read_anchors_(float& fLeft, float& fRight, float& fTop,
    float& fBottom, float& fXCenter, float& fYCenter) const
{
    fLeft   = +std::numeric_limits<float>::infinity();
    fRight  = -std::numeric_limits<float>::infinity();
    fTop    = +std::numeric_limits<float>::infinity();
    fBottom = -std::numeric_limits<float>::infinity();

    for (const auto& mOptAnchor : lAnchorList_)
    {
        if (!mOptAnchor)
            continue;

        const anchor& mAnchor = mOptAnchor.value();

        // Make sure the anchored object has its borders updated
        const uiobject* pObj = mAnchor.get_parent();
        if (pObj)
            pObj->update_borders_();

        switch (mAnchor.get_point())
        {
            case anchor_point::TOPLEFT :
                fTop = std::min<float>(fTop, mAnchor.get_abs_y());
                fLeft = std::min<float>(fLeft, mAnchor.get_abs_x());
                break;
            case anchor_point::TOP :
                fTop = std::min<float>(fTop, mAnchor.get_abs_y());
                fXCenter = mAnchor.get_abs_x();
                break;
            case anchor_point::TOPRIGHT :
                fTop = std::min<float>(fTop, mAnchor.get_abs_y());
                fRight = std::max<float>(fRight, mAnchor.get_abs_x());
                break;
            case anchor_point::RIGHT :
                fRight = std::max<float>(fRight, mAnchor.get_abs_x());
                fYCenter = mAnchor.get_abs_y();
                break;
            case anchor_point::BOTTOMRIGHT :
                fBottom = std::max<float>(fBottom, mAnchor.get_abs_y());
                fRight = std::max<float>(fRight, mAnchor.get_abs_x());
                break;
            case anchor_point::BOTTOM :
                fBottom = std::max<float>(fBottom, mAnchor.get_abs_y());
                fXCenter = mAnchor.get_abs_x();
                break;
            case anchor_point::BOTTOMLEFT :
                fBottom = std::max<float>(fBottom, mAnchor.get_abs_y());
                fLeft = std::min<float>(fLeft, mAnchor.get_abs_x());
                break;
            case anchor_point::LEFT :
                fLeft = std::min<float>(fLeft, mAnchor.get_abs_x());
                fYCenter = mAnchor.get_abs_y();
                break;
            case anchor_point::CENTER :
                fXCenter = mAnchor.get_abs_x();
                fYCenter = mAnchor.get_abs_y();
                break;
        }
    }
}

void uiobject::update_borders_() const
{
    if (!bUpdateBorders_)
        return;

    //#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    bool bOldReady = bReady_;
    bReady_ = true;

    if (!lAnchorList_.empty())
    {
        float fLeft = 0.0f, fRight = 0.0f, fTop = 0.0f, fBottom = 0.0f;
        float fXCenter = 0.0f, fYCenter = 0.0f;

        float fRoundedWidth = round_to_pixel(fAbsWidth_, rounding_method::NEAREST_NOT_ZERO);
        float fRoundedHeight = round_to_pixel(fAbsHeight_, rounding_method::NEAREST_NOT_ZERO);

        DEBUG_LOG("  Read anchors");
        read_anchors_(fLeft, fRight, fTop, fBottom, fXCenter, fYCenter);

        DEBUG_LOG("  Make borders");
        make_borders_(fTop,  fBottom, fYCenter, fRoundedHeight);
        make_borders_(fLeft, fRight,  fXCenter, fRoundedWidth);

        if (bReady_)
        {
            if (fRight < fLeft)
                fRight = fLeft+1;
            if (fBottom < fTop)
                fBottom = fTop+1;

            lBorderList_ = quad2f(fLeft, fRight, fTop, fBottom);
        }
        else
            lBorderList_ = quad2f::ZERO;

        bUpdateBorders_ = false;
    }
    else
    {
        lBorderList_ = quad2f(0.0, 0.0, fAbsWidth_, fAbsHeight_);
        bReady_ = false;
    }

    lBorderList_.left = round_to_pixel(lBorderList_.left);
    lBorderList_.right = round_to_pixel(lBorderList_.right);
    lBorderList_.top = round_to_pixel(lBorderList_.top);
    lBorderList_.bottom = round_to_pixel(lBorderList_.bottom);

    if (bReady_ || (!bReady_ && bOldReady))
    {
        DEBUG_LOG("  Fire redraw");
        notify_renderer_need_redraw();
    }
    DEBUG_LOG("  @");
}

void uiobject::update_anchors_()
{
    std::vector<const uiobject*> lAnchorParentList;
    for (auto& mAnchor : lAnchorList_)
    {
        if (!mAnchor)
            continue;

        const uiobject* pObj = mAnchor->get_parent();
        if (pObj)
        {
            if (pObj->depends_on(this))
            {
                gui::out << gui::error << "gui::" << lType_.back() << " : Cyclic anchor dependency ! "
                    << "\"" << sName_ << "\" and \"" << pObj->get_name() << "\" depend on "
                    "eachothers (directly or indirectly).\n\""
                    << anchor::get_string_point(mAnchor->get_point()) << "\" anchor removed." << std::endl;

                mAnchor.reset();
                continue;
            }

            if (utils::find(lAnchorParentList, pObj) == lAnchorParentList.end())
                lAnchorParentList.push_back(pObj);
        }
    }

    for (const auto* pParent : lPreviousAnchorParentList_)
    {
        if (utils::find(lAnchorParentList, pParent) == lAnchorParentList.end())
            pParent->notify_anchored_object(this, false);
    }

    for (const auto* pParent : lAnchorParentList)
    {
        if (utils::find(lPreviousAnchorParentList_, pParent) == lPreviousAnchorParentList_.end())
            pParent->notify_anchored_object(this, true);
    }

    lPreviousAnchorParentList_ = std::move(lAnchorParentList);
}

void uiobject::notify_borders_need_update() const
{
    if (bUpdateBorders_) return;

    bUpdateBorders_ = true;

    for (auto* pObject : lAnchoredObjectList_)
        pObject->notify_borders_need_update();
}

void uiobject::notify_scaling_factor_updated()
{
    if (!bVirtual_)
        notify_borders_need_update();
}

void uiobject::update(float fDelta)
{
    //#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)
    DEBUG_LOG("  Update " + sName_ + " (" + lType_.back() + ")");
    update_borders_();

    if (bNewlyCreated_)
    {
        bNewlyCreated_ = false;
        DEBUG_LOG("  Fire redraw");
        notify_renderer_need_redraw();
    }
    DEBUG_LOG("  +");
}

sol::state& uiobject::get_lua_()
{
    return pManager_->get_lua();
}

lua::state& uiobject::get_luapp_()
{
    return pManager_->get_luapp();
}

void uiobject::push_on_lua(sol::state& mLua) const
{
    sol::stack::push(mLua.lua_state(), mLua[sLuaName_]);
}

void uiobject::push_on_lua(lua::state& mLua) const
{
    mLua.push_global(sLuaName_);
}

void uiobject::remove_glue()
{
    pManager_->get_lua()[sLuaName_] = sol::lua_nil;
    lGlue_ = nullptr;
}

void uiobject::set_special()
{
    bSpecial_ = true;
}

bool uiobject::is_special() const
{
    return bSpecial_;
}

void uiobject::notify_renderer_need_redraw() const
{
}

void uiobject::set_newly_created()
{
    bNewlyCreated_ = true;
}

bool uiobject::is_newly_created() const
{
    return bNewlyCreated_;
}

const std::vector<uiobject*>& uiobject::get_anchored_objects() const
{
    return lAnchoredObjectList_;
}

void uiobject::notify_loaded()
{
    bLoaded_ = true;
}

frame_renderer* uiobject::get_top_level_renderer()
{
    if (!pParent_) return pManager_;
    return pParent_->get_top_level_renderer();
}

const frame_renderer* uiobject::get_top_level_renderer() const
{
    if (!pParent_) return pManager_;
    return pParent_->get_top_level_renderer();
}

void uiobject::notify_visible(bool)
{
    bIsVisible_ = true;
}

void uiobject::notify_invisible(bool)
{
    bIsVisible_ = false;
}

}
}
