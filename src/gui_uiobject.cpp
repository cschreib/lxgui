#include "lxgui/gui_uiobject.hpp"

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
uiobject::uiobject(manager& mManager) : mManager_(mManager)
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
            if (mAnchor)
            {
                if (auto* pAnchorParent = mAnchor->get_parent().get())
                    pAnchorParent->notify_anchored_object(observer_from_this(), false);
            }

            mAnchor.reset();
        }

        // Replace anchors pointing to this widget by absolute anchors
        // (need to copy the anchored object list, because the objects will attempt to modify it when
        // un-anchored, which would invalidate our iteration)
        std::vector<utils::observer_ptr<uiobject>> lTempAnchoredObjectList = std::move(lAnchoredObjectList_);
        for (const auto& pObj : lTempAnchoredObjectList)
        {
            if (!pObj)
                continue;

            std::vector<anchor_point> lAnchoredPointList;
            for (const auto& mAnchor : pObj->get_point_list())
            {
                if (mAnchor && mAnchor->get_parent().get() == this)
                    lAnchoredPointList.push_back(mAnchor->mPoint);
            }

            for (const auto& mPoint : lAnchoredPointList)
            {
                const anchor& mAnchor = pObj->get_point(mPoint);
                anchor_data mNewAnchor = anchor_data(mPoint, "", anchor_point::TOPLEFT);
                mNewAnchor.mOffset = mAnchor.mOffset;

                switch (mAnchor.mParentPoint)
                {
                    case anchor_point::TOPLEFT :     mNewAnchor.mOffset   += lBorderList_.top_left();     break;
                    case anchor_point::TOP :         mNewAnchor.mOffset.y += lBorderList_.top;            break;
                    case anchor_point::TOPRIGHT :    mNewAnchor.mOffset   += lBorderList_.top_right();    break;
                    case anchor_point::RIGHT :       mNewAnchor.mOffset.x += lBorderList_.right;          break;
                    case anchor_point::BOTTOMRIGHT : mNewAnchor.mOffset   += lBorderList_.bottom_right(); break;
                    case anchor_point::BOTTOM :      mNewAnchor.mOffset.y += lBorderList_.bottom;         break;
                    case anchor_point::BOTTOMLEFT :  mNewAnchor.mOffset   += lBorderList_.bottom_left();  break;
                    case anchor_point::LEFT :        mNewAnchor.mOffset.x += lBorderList_.left;           break;
                    case anchor_point::CENTER :      mNewAnchor.mOffset   += lBorderList_.center();       break;
                }

                pObj->set_point(mNewAnchor);
            }

            pObj->update_anchors_();
        }

        remove_glue();
    }

    // Unregister this object from the GUI manager
    get_manager().remove_uiobject(observer_from_this());
}

std::string uiobject::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;

    sStr << sTab << "  # Name        : " << sName_ << " ("+std::string(bReady_ ? "ready" : "not ready")+std::string(bSpecial_ ? ", special)\n" : ")\n");
    sStr << sTab << "  # Raw name    : " << sRawName_ << "\n";
    sStr << sTab << "  # Lua name    : " << sLuaName_ << "\n";
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
    sStr << sTab << "  # Abs width   : " << mDimensions_.x << "\n";
    sStr << sTab << "  # Abs height  : " << mDimensions_.y << "\n";

    return sStr.str();
}

void uiobject::copy_from(const uiobject& mObj)
{
    bInherits_ = true;

    // Inherit properties
    this->set_alpha(mObj.get_alpha());
    this->set_shown(mObj.is_shown());
    this->set_dimensions(mObj.get_dimensions());

    for (const auto& mAnchor : mObj.get_point_list())
    {
        if (!mAnchor)
            continue;

        this->set_point(mAnchor->get_data());
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

void uiobject::set_dimensions(const vector2f& mDimensions)
{
    if (mDimensions_ == mDimensions)
        return;

    if (!bVirtual_)
        get_manager().notify_object_moved();

    mDimensions_ = mDimensions;

    if (!bVirtual_)
    {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void uiobject::set_width(float fAbsWidth)
{
    if (mDimensions_.x == fAbsWidth)
        return;

    if (!bVirtual_)
        get_manager().notify_object_moved();

    mDimensions_.x = fAbsWidth;

    if (!bVirtual_)
    {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void uiobject::set_height(float fAbsHeight)
{
    if (mDimensions_.y == fAbsHeight)
        return;

    if (!bVirtual_)
        get_manager().notify_object_moved();

    mDimensions_.y = fAbsHeight;

    if (!bVirtual_)
    {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void uiobject::set_relative_dimensions(const vector2f& mDimensions)
{
    if (pParent_)
        set_dimensions(mDimensions*pParent_->get_apparent_dimensions());
    else
        set_dimensions(mDimensions*get_top_level_renderer()->get_target_dimensions());
}

void uiobject::set_relative_width(float fRelWidth)
{
    if (pParent_)
        set_width(fRelWidth*pParent_->get_apparent_dimensions().x);
    else
        set_width(fRelWidth*get_top_level_renderer()->get_target_dimensions().x);
}

void uiobject::set_relative_height(float fRelHeight)
{
    if (pParent_)
        set_height(fRelHeight*pParent_->get_apparent_dimensions().y);
    else
        set_height(fRelHeight*get_top_level_renderer()->get_target_dimensions().y);
}

const vector2f& uiobject::get_dimensions() const
{
    return mDimensions_;
}

vector2f uiobject::get_apparent_dimensions() const
{
    update_borders_();
    return vector2f(lBorderList_.width(), lBorderList_.height());
}

bool uiobject::is_apparent_width_defined() const
{
    return mDimensions_.x > 0.0f || (lDefinedBorderList_.left && lDefinedBorderList_.right);
}

bool uiobject::is_apparent_height_defined() const
{
    return mDimensions_.y > 0.0f || (lDefinedBorderList_.top && lDefinedBorderList_.bottom);
}

void uiobject::set_name_(const std::string& sName)
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

void uiobject::set_parent_(utils::observer_ptr<frame> pParent)
{
    if (pParent == observer_from_this())
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : Cannot call set_parent(this)." << std::endl;
        return;
    }

    if (pParent_ != pParent)
    {
        pParent_ = std::move(pParent);

        if (!bVirtual_)
            notify_borders_need_update();
    }
}

void uiobject::set_name_and_parent_(const std::string& sName, utils::observer_ptr<frame> pParent)
{
    if (pParent == observer_from_this())
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : Cannot call set_parent(this)." << std::endl;
        return;
    }

    if (pParent_ == pParent && sName == sName_)
        return;

    pParent_ = std::move(pParent);
    set_name_(sName);

    if (!bVirtual_)
        notify_borders_need_update();
}

utils::owner_ptr<uiobject> uiobject::release_from_parent()
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

const bounds2f& uiobject::get_borders() const
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
        lDefinedBorderList_ = bounds2<bool>(false, false, false, false);

        if (!bVirtual_)
        {
            update_anchors_();
            notify_borders_need_update();
            notify_renderer_need_redraw();
            get_manager().notify_object_moved();
        }
    }
}

void uiobject::set_all_points(const std::string& sObjName)
{
    if (sObjName == sName_)
    {
        gui::out << gui::error << "gui::" << lType_.back() <<
            " : Cannot call set_all_points(this)." << std::endl;
        return;
    }

    clear_all_points();

    lAnchorList_[static_cast<int>(anchor_point::TOPLEFT)].emplace(
        *this, anchor_data(anchor_point::TOPLEFT, sObjName));

    lAnchorList_[static_cast<int>(anchor_point::BOTTOMRIGHT)].emplace(
        *this, anchor_data(anchor_point::BOTTOMRIGHT, sObjName));

    lDefinedBorderList_ = bounds2<bool>(true, true, true, true);

    if (!bVirtual_)
    {
        update_anchors_();
        notify_borders_need_update();
        notify_renderer_need_redraw();
        get_manager().notify_object_moved();
    }
}

void uiobject::set_all_points(const utils::observer_ptr<uiobject>& pObj)
{
    if (pObj == observer_from_this())
    {
        gui::out << gui::error << "gui::" << lType_.back() <<
        " : Cannot call set_all_points(this)." << std::endl;
        return;
    }

    set_all_points(pObj ? pObj->get_name() : "");
}

void uiobject::set_point(const anchor_data& mAnchor)
{
    lAnchorList_[static_cast<int>(mAnchor.mPoint)].emplace(*this, mAnchor);

    switch (mAnchor.mPoint)
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
        get_manager().notify_object_moved();
    }
}

bool uiobject::depends_on(const uiobject& mObj) const
{
    for (const auto& mAnchor : lAnchorList_)
    {
        if (!mAnchor)
            continue;

        const uiobject* pParent = mAnchor->get_parent().get();
        if (pParent == &mObj)
            return true;

        if (pParent)
            return pParent->depends_on(mObj);
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

anchor& uiobject::modify_point(anchor_point mPoint)
{
    get_manager().notify_object_moved();

    if (!bVirtual_)
        notify_borders_need_update();

    auto& mAnchor = lAnchorList_[static_cast<int>(mPoint)];
    if (!mAnchor)
    {
        throw gui::exception("uiobject",
            "Cannot modify a point that does not exist. Use set_point() first.");
    }

    return *mAnchor;
}

const anchor& uiobject::get_point(anchor_point mPoint) const
{
    const auto& mAnchor = lAnchorList_[static_cast<int>(mPoint)];
    if (!mAnchor)
    {
        throw gui::exception("uiobject",
            "Cannot get a point that does not exist. Use set_point() first.");
    }

    return *mAnchor;
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

void uiobject::notify_anchored_object(utils::observer_ptr<uiobject> pObj, bool bAnchored) const
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

float uiobject::round_to_pixel(float fValue, utils::rounding_method mMethod) const
{
    float fScalingFactor = get_manager().get_interface_scaling_factor();
    return utils::round(fValue, 1.0f/fScalingFactor, mMethod);
}

vector2f uiobject::round_to_pixel(const vector2f& mPosition, utils::rounding_method mMethod) const
{
    float fScalingFactor = get_manager().get_interface_scaling_factor();
    return vector2f(utils::round(mPosition.x, 1.0f/fScalingFactor, mMethod),
                    utils::round(mPosition.y, 1.0f/fScalingFactor, mMethod));
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
        const uiobject* pObj = mAnchor.get_parent().get();
        if (pObj)
            pObj->update_borders_();

        const vector2f mAnchorPoint = mAnchor.get_point(*this);

        switch (mAnchor.mPoint)
        {
            case anchor_point::TOPLEFT :
                fTop = std::min<float>(fTop, mAnchorPoint.y);
                fLeft = std::min<float>(fLeft, mAnchorPoint.x);
                break;
            case anchor_point::TOP :
                fTop = std::min<float>(fTop, mAnchorPoint.y);
                fXCenter = mAnchorPoint.x;
                break;
            case anchor_point::TOPRIGHT :
                fTop = std::min<float>(fTop, mAnchorPoint.y);
                fRight = std::max<float>(fRight, mAnchorPoint.x);
                break;
            case anchor_point::RIGHT :
                fRight = std::max<float>(fRight, mAnchorPoint.x);
                fYCenter = mAnchorPoint.y;
                break;
            case anchor_point::BOTTOMRIGHT :
                fBottom = std::max<float>(fBottom, mAnchorPoint.y);
                fRight = std::max<float>(fRight, mAnchorPoint.x);
                break;
            case anchor_point::BOTTOM :
                fBottom = std::max<float>(fBottom, mAnchorPoint.y);
                fXCenter = mAnchorPoint.x;
                break;
            case anchor_point::BOTTOMLEFT :
                fBottom = std::max<float>(fBottom, mAnchorPoint.y);
                fLeft = std::min<float>(fLeft, mAnchorPoint.x);
                break;
            case anchor_point::LEFT :
                fLeft = std::min<float>(fLeft, mAnchorPoint.x);
                fYCenter = mAnchorPoint.y;
                break;
            case anchor_point::CENTER :
                fXCenter = mAnchorPoint.x;
                fYCenter = mAnchorPoint.y;
                break;
        }
    }
}

void uiobject::update_borders_() const
{
    // #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)
    DEBUG_LOG("  Request update for " + sLuaName_);
    if (!bUpdateBorders_)
        return;

    DEBUG_LOG("  Update anchors for " + sLuaName_);

    bool bOldReady = bReady_;
    bReady_ = true;

    if (!lAnchorList_.empty())
    {
        float fLeft = 0.0f, fRight = 0.0f, fTop = 0.0f, fBottom = 0.0f;
        float fXCenter = 0.0f, fYCenter = 0.0f;

        float fRoundedWidth = round_to_pixel(mDimensions_.x, utils::rounding_method::NEAREST_NOT_ZERO);
        float fRoundedHeight = round_to_pixel(mDimensions_.y, utils::rounding_method::NEAREST_NOT_ZERO);

        DEBUG_LOG("  Read anchors");
        read_anchors_(fLeft, fRight, fTop, fBottom, fXCenter, fYCenter);
        DEBUG_LOG("    left=" + utils::to_string(fLeft));
        DEBUG_LOG("    right=" + utils::to_string(fRight));
        DEBUG_LOG("    top=" + utils::to_string(fTop));
        DEBUG_LOG("    bottom=" + utils::to_string(fBottom));
        DEBUG_LOG("    x_center=" + utils::to_string(fXCenter));
        DEBUG_LOG("    y_center=" + utils::to_string(fYCenter));

        DEBUG_LOG("  Make borders");
        make_borders_(fTop,  fBottom, fYCenter, fRoundedHeight);
        make_borders_(fLeft, fRight,  fXCenter, fRoundedWidth);

        if (bReady_)
        {
            if (fRight < fLeft)
                fRight = fLeft+1;
            if (fBottom < fTop)
                fBottom = fTop+1;

            lBorderList_ = bounds2f(fLeft, fRight, fTop, fBottom);
        }
        else
            lBorderList_ = bounds2f::ZERO;

        bUpdateBorders_ = false;
    }
    else
    {
        lBorderList_ = bounds2f(0.0, 0.0, mDimensions_.x, mDimensions_.y);
        bReady_ = false;
    }

    DEBUG_LOG("  Final borders");
    lBorderList_.left = round_to_pixel(lBorderList_.left);
    lBorderList_.right = round_to_pixel(lBorderList_.right);
    lBorderList_.top = round_to_pixel(lBorderList_.top);
    lBorderList_.bottom = round_to_pixel(lBorderList_.bottom);

    DEBUG_LOG("    left=" + utils::to_string(lBorderList_.left));
    DEBUG_LOG("    right=" + utils::to_string(lBorderList_.right));
    DEBUG_LOG("    top=" + utils::to_string(lBorderList_.top));
    DEBUG_LOG("    bottom=" + utils::to_string(lBorderList_.bottom));

    if (bReady_ || (!bReady_ && bOldReady))
    {
        DEBUG_LOG("  Fire redraw");
        notify_renderer_need_redraw();
    }
    DEBUG_LOG("  @");
    #undef DEBUG_LOG
}

void uiobject::update_anchors_()
{
    std::vector<utils::observer_ptr<const uiobject>> lAnchorParentList;
    for (auto& mAnchor : lAnchorList_)
    {
        if (!mAnchor)
            continue;

        utils::observer_ptr<const uiobject> pObj = mAnchor->get_parent();
        if (pObj)
        {
            if (pObj->depends_on(*this))
            {
                gui::out << gui::error << "gui::" << lType_.back() << " : Cyclic anchor dependency ! "
                    << "\"" << sName_ << "\" and \"" << pObj->get_name() << "\" depend on "
                    "eachothers (directly or indirectly).\n\""
                    << anchor::get_string_point(mAnchor->mPoint) << "\" anchor removed." << std::endl;

                mAnchor.reset();
                continue;
            }

            if (utils::find(lAnchorParentList, pObj) == lAnchorParentList.end())
                lAnchorParentList.push_back(pObj);
        }
    }

    for (const auto& pParent : lPreviousAnchorParentList_)
    {
        if (utils::find(lAnchorParentList, pParent) == lAnchorParentList.end())
            pParent->notify_anchored_object(observer_from_this(), false);
    }

    for (const auto& pParent : lAnchorParentList)
    {
        if (utils::find(lPreviousAnchorParentList_, pParent) == lPreviousAnchorParentList_.end())
            pParent->notify_anchored_object(observer_from_this(), true);
    }

    lPreviousAnchorParentList_ = std::move(lAnchorParentList);
}

void uiobject::notify_borders_need_update() const
{
    if (bUpdateBorders_) return;

    bUpdateBorders_ = true;

    for (const auto& pObject : lAnchoredObjectList_)
        pObject->notify_borders_need_update();
}

void uiobject::notify_scaling_factor_updated()
{
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
    #undef DEBUG_LOG
}

sol::state& uiobject::get_lua_()
{
    return get_manager().get_lua();
}

void uiobject::remove_glue()
{
    get_lua_().globals()[sLuaName_] = sol::lua_nil;
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

const std::vector<utils::observer_ptr<uiobject>>& uiobject::get_anchored_objects() const
{
    return lAnchoredObjectList_;
}

void uiobject::notify_loaded()
{
    bLoaded_ = true;
}

utils::observer_ptr<const frame_renderer> uiobject::get_top_level_renderer() const
{
    if (!pParent_) return get_manager().observer_from_this();
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
