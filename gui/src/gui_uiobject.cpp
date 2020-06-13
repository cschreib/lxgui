#include "lxgui/gui_uiobject.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_string.hpp>
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
    clear_links();

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
    if (bManuallyRendered_)
    sStr << sTab << "  # Man. render : " << (pRenderer_ ? pRenderer_->get_name() : "none") << "\n";
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
                sStr << mAnchor->serialize(sTab);
            sStr << sTab << "  |-###\n";
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
    sStr << sTab << "  # Abs width   : " << uiAbsWidth_ << "\n";
    sStr << sTab << "  # Abs height  : " << uiAbsHeight_ << "\n";
    sStr << sTab << "  # Rel width   : " << fRelWidth_ << "\n";
    sStr << sTab << "  # Rel height  : " << fRelHeight_ << "\n";

    return sStr.str();
}

void uiobject::copy_from(uiobject* pObj)
{
    if (pObj)
    {
        // Copy marked Lua member variables
        if (!pObj->lCopyList_.empty())
        {
            if (is_virtual())
            {
                // The current object is virtual too,
                // add the Lua member variables to its copy list.
                for (const auto& mMember : pObj->lCopyList_)
                    lCopyList_.push_back(mMember);
            }

            lua::state* pLua = pManager_->get_lua();
            pLua->get_global(pObj->get_lua_name());
            if (pLua->get_type() != lua::type::NIL)
            {
                pLua->get_global(sLuaName_);
                if (pLua->get_type() != lua::type::NIL)
                {
                    for (const auto& mMember : pObj->lCopyList_)
                    {
                        pLua->get_field(mMember, -2);
                        pLua->set_field(mMember);
                    }
                }
                pLua->pop();
            }
            pLua->pop();
        }

        bInherits_ = true;
        pInheritance_ = pObj;

        // Inherit properties
        this->set_alpha(pObj->get_alpha());
        this->set_shown(pObj->is_shown());
        if (pObj->is_width_absolute())
            this->set_abs_width(pObj->get_abs_width());
        else
            this->set_rel_width(pObj->get_rel_width());
        if (pObj->is_height_absolute())
            this->set_abs_height(pObj->get_abs_height());
        else
            this->set_rel_height(pObj->get_rel_height());

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

const std::vector<std::string>& uiobject::get_object_typeList() const
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
    bIsShown_ = true;
}

void uiobject::hide()
{
    bIsShown_ = false;
}

void uiobject::set_shown(bool bIsShown)
{
    bIsShown_ = bIsShown;
}

bool uiobject::is_shown() const
{
    return bIsShown_;
}

bool uiobject::is_visible() const
{
    return bIsVisible_;
}

void uiobject::set_abs_dimensions(uint uiAbsWidth, uint uiAbsHeight)
{
    if (uiAbsWidth_  != uiAbsWidth  || !bIsWidthAbs_ ||
        uiAbsHeight_ != uiAbsHeight || !bIsHeightAbs_)
    {
        pManager_->notify_object_moved();
        bIsWidthAbs_ = true;
        bIsHeightAbs_ = true;
        uiAbsWidth_ = uiAbsWidth;
        uiAbsHeight_ = uiAbsHeight;
        fire_update_dimensions();
        notify_renderer_need_redraw();
    }
}

void uiobject::set_abs_width(uint uiAbsWidth)
{
    if (uiAbsWidth_ != uiAbsWidth || !bIsWidthAbs_)
    {
        pManager_->notify_object_moved();
        bIsWidthAbs_ = true;
        uiAbsWidth_ = uiAbsWidth;
        fire_update_dimensions();
        notify_renderer_need_redraw();
    }
}

void uiobject::set_abs_height(uint uiAbsHeight)
{
    if (uiAbsHeight_ != uiAbsHeight || !bIsWidthAbs_)
    {
        pManager_->notify_object_moved();
        bIsHeightAbs_ = true;
        uiAbsHeight_ = uiAbsHeight;
        fire_update_dimensions();
        notify_renderer_need_redraw();
    }
}

bool uiobject::is_width_absolute() const
{
    return bIsWidthAbs_;
}

bool uiobject::is_height_absolute() const
{
    return bIsHeightAbs_;
}

void uiobject::set_rel_dimensions(float fRelWidth, float fRelHeight)
{
    if (fRelWidth_ != fRelWidth || bIsWidthAbs_ || fRelHeight_ != fRelHeight || bIsHeightAbs_)
    {
        pManager_->notify_object_moved();
        bIsWidthAbs_ = false;
        bIsHeightAbs_ = false;
        fRelWidth_ = fRelWidth;
        fRelHeight_ = fRelHeight;
        fire_update_dimensions();
        notify_renderer_need_redraw();
    }
}

void uiobject::set_rel_width(float fRelWidth)
{
    if (fRelWidth_ != fRelWidth || bIsWidthAbs_)
    {
        pManager_->notify_object_moved();
        bIsWidthAbs_ = false;
        fRelWidth_ = fRelWidth;
        fire_update_dimensions();
        notify_renderer_need_redraw();
    }
}

void uiobject::set_rel_height(float fRelHeight)
{
    if (fRelHeight_ != fRelHeight || bIsHeightAbs_)
    {
        pManager_->notify_object_moved();
        bIsHeightAbs_ = false;
        fRelHeight_ = fRelHeight;
        fire_update_dimensions();
        notify_renderer_need_redraw();
    }
}

uint uiobject::get_abs_width() const
{
    return uiAbsWidth_;
}

uint uiobject::get_apparent_width() const
{
    update_borders_();
    return lBorderList_.width();
}

uint uiobject::get_abs_height() const
{
    return uiAbsHeight_;
}

uint uiobject::get_apparent_height() const
{
    update_borders_();
    return lBorderList_.height();
}

float uiobject::get_rel_width() const
{
    return fRelWidth_;
}

float uiobject::get_rel_height() const
{
    return fRelHeight_;
}

void uiobject::set_parent(uiobject* pParent)
{
    if (pParent == this)
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : Cannot call set_parent(this)." << std::endl;
        return;
    }

    if (pParent_ != pParent)
    {
        pParent_ = pParent;
        fire_update_dimensions();
    }
}

const uiobject* uiobject::get_parent() const
{
    return pParent_;
}

uiobject* uiobject::get_parent()
{
    return pParent_;
}

uiobject* uiobject::get_base()
{
    return pInheritance_;
}

vector2<int> uiobject::get_center() const
{
    update_borders_();
    return lBorderList_.center();
}

int uiobject::get_left() const
{
    update_borders_();
    return lBorderList_.left;
}

int uiobject::get_right() const
{
    update_borders_();
    return lBorderList_.right;
}

int uiobject::get_top() const
{
    update_borders_();
    return lBorderList_.top;
}

int uiobject::get_bottom() const
{
    update_borders_();
    return lBorderList_.bottom;
}

const quad2i& uiobject::get_borders() const
{
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

        bUpdateAnchors_ = true;
        fire_update_borders();
        notify_renderer_need_redraw();
        pManager_->notify_object_moved();
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

        bUpdateAnchors_ = true;
        fire_update_borders();
        notify_renderer_need_redraw();
        pManager_->notify_object_moved();
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

void uiobject::set_abs_point(anchor_point mPoint, const std::string& sParentName, anchor_point mRelativePoint, int iX, int iY)
{
    set_abs_point(mPoint, sParentName, mRelativePoint, vector2i(iX, iY));
}

void uiobject::set_abs_point(anchor_point mPoint, const std::string& sParentName, anchor_point mRelativePoint, const vector2i& mOffset)
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

    bUpdateAnchors_ = true;
    fire_update_borders();
    notify_renderer_need_redraw();
    pManager_->notify_object_moved();
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

    bUpdateAnchors_ = true;
    fire_update_borders();
    notify_renderer_need_redraw();
    pManager_->notify_object_moved();
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

    bUpdateAnchors_ = true;
    fire_update_borders();
    notify_renderer_need_redraw();
    pManager_->notify_object_moved();
}

bool uiobject::depends_on(uiobject* pObj) const
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

    fire_update_borders();

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

const std::array<utils::optional<anchor>,9>& uiobject::get_point_list() const
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

void uiobject::update_dimensions_() const
{
    if (pParent_)
    {
        if (bIsHeightAbs_)
            fRelHeight_ = float(uiAbsHeight_)/float(pParent_->get_apparent_height());
        else
            uiAbsHeight_ = fRelHeight_*pParent_->get_apparent_height();

        if (bIsWidthAbs_)
            fRelWidth_ = float(uiAbsWidth_)/float(pParent_->get_apparent_width());
        else
            uiAbsWidth_ = fRelWidth_*pParent_->get_apparent_width();
    }
    else
    {
        if (bIsHeightAbs_)
            fRelHeight_ = float(uiAbsHeight_)/float(pManager_->get_screen_height());
        else
            uiAbsHeight_ = fRelHeight_*pManager_->get_screen_height();

        if (bIsWidthAbs_)
            fRelWidth_ = float(uiAbsWidth_)/float(pManager_->get_screen_width());
        else
            uiAbsWidth_ = fRelWidth_*pManager_->get_screen_width();
    }
}

void uiobject::make_borders_(float& iMin, float& iMax, float iCenter, float iSize) const
{
    if (math::isinf(iMin) && math::isinf(iMax))
    {
        if (!math::isinf(iSize) && iSize != 0 && !math::isinf(iCenter))
        {
            iMin = iCenter - iSize/2.0f;
            iMax = iCenter + iSize/2.0f;
        }
        else
            bReady_ = false;
    }
    else if (math::isinf(iMax))
    {
        if (!math::isinf(iSize) && iSize != 0)
            iMax = iMin + iSize;
        else if (!math::isinf(iCenter))
            iMax = iMin + 2.0f*(iCenter - iMin);
        else
            bReady_ = false;
    }
    else if (math::isinf(iMin))
    {
        if (!math::isinf(iSize) && iSize != 0)
            iMin = iMax - iSize;
        else if (!math::isinf(iCenter))
            iMin = iMax - 2.0f*(iMax - iCenter);
        else
            bReady_ = false;
    }
}

void uiobject::read_anchors_(float& iLeft, float& iRight, float& iTop,
    float& iBottom, float& iXCenter, float& iYCenter) const
{
    iLeft   = +std::numeric_limits<float>::infinity();
    iRight  = -std::numeric_limits<float>::infinity();
    iTop    = +std::numeric_limits<float>::infinity();
    iBottom = -std::numeric_limits<float>::infinity();

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
                iTop = std::min<float>(iTop, mAnchor.get_abs_y());
                iLeft = std::min<float>(iLeft, mAnchor.get_abs_x());
                break;
            case anchor_point::TOP :
                iTop = std::min<float>(iTop, mAnchor.get_abs_y());
                iXCenter = mAnchor.get_abs_x();
                break;
            case anchor_point::TOPRIGHT :
                iTop = std::min<float>(iTop, mAnchor.get_abs_y());
                iRight = std::max<float>(iRight, mAnchor.get_abs_x());
                break;
            case anchor_point::RIGHT :
                iRight = std::max<float>(iRight, mAnchor.get_abs_x());
                iYCenter = mAnchor.get_abs_y();
                break;
            case anchor_point::BOTTOMRIGHT :
                iBottom = std::max<float>(iBottom, mAnchor.get_abs_y());
                iRight = std::max<float>(iRight, mAnchor.get_abs_x());
                break;
            case anchor_point::BOTTOM :
                iBottom = std::max<float>(iBottom, mAnchor.get_abs_y());
                iXCenter = mAnchor.get_abs_x();
                break;
            case anchor_point::BOTTOMLEFT :
                iBottom = std::max<float>(iBottom, mAnchor.get_abs_y());
                iLeft = std::min<float>(iLeft, mAnchor.get_abs_x());
                break;
            case anchor_point::LEFT :
                iLeft = std::min<float>(iLeft, mAnchor.get_abs_x());
                iYCenter = mAnchor.get_abs_y();
                break;
            case anchor_point::CENTER :
                iXCenter = mAnchor.get_abs_x();
                iYCenter = mAnchor.get_abs_y();
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

    if (bUpdateDimensions_)
    {
        DEBUG_LOG("  Update dimensions");
        update_dimensions_();
        bUpdateDimensions_ = false;
    }

    if (get_num_point() > 0u)
    {
        float fLeft = 0.0f, fRight = 0.0f, fTop = 0.0f, fBottom = 0.0f;
        float fXCenter = 0.0f, fYCenter = 0.0f;

        DEBUG_LOG("  Read anchors");
        read_anchors_(fLeft, fRight, fTop, fBottom, fXCenter, fYCenter);

        DEBUG_LOG("  Make borders");
        make_borders_(fTop,  fBottom, fYCenter, uiAbsHeight_);
        make_borders_(fLeft, fRight,  fXCenter, uiAbsWidth_);

        if (bReady_)
        {
            int iLeft = fLeft, iRight = fRight, iTop = fTop, iBottom = fBottom;

            if (iRight < iLeft)
                iRight = iLeft+1;
            if (iBottom < iTop)
                iBottom = iTop+1;

            lBorderList_ = quad2i(iLeft, iRight, iTop, iBottom);

            bIsWidthAbs_ = true;
            uiAbsWidth_  = iRight - iLeft;

            bIsHeightAbs_ = true;
            uiAbsHeight_  = iBottom - iTop;

            DEBUG_LOG("  Update dimentions");
            update_dimensions_();
        }
        else
            lBorderList_ = quad2i::ZERO;

        bUpdateBorders_ = false;
    }
    else
        bReady_ = false;

    if (bReady_ || (!bReady_ && bOldReady))
    {
        DEBUG_LOG("  Fire redraw");
        notify_renderer_need_redraw();
    }
    DEBUG_LOG("  @");
}

void uiobject::update_anchors()
{
    if (!bUpdateAnchors_)
        return;

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
                gui::out << gui::error << "gui::" << lType_.back() << " : Cyclic anchor dependency !"
                    << "\"" << sName_ << "\" and \"" << pObj->get_name() << "\" depend on"
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
    bUpdateAnchors_ = false;
}

void uiobject::fire_update_borders() const
{
    bUpdateBorders_ = true;

    for (auto* pObject : lAnchoredObjectList_)
        pObject->fire_update_borders();
}

void uiobject::fire_update_dimensions() const
{
    fire_update_borders();
    bUpdateDimensions_ = true;
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

void uiobject::push_on_lua(lua::state* pLua) const
{
    pLua->push_global(sLuaName_);
}

void uiobject::remove_glue()
{
    lua::state* pLua = pManager_->get_lua();
    pLua->push_nil();
    pLua->set_global(sLuaName_);
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

void uiobject::set_manually_rendered(bool bManuallyRendered, uiobject* pRenderer)
{
    uiobject* pOldrenderer = pRenderer_;

    if (pOldrenderer && pOldrenderer != pRenderer)
        pOldrenderer->notify_manually_rendered_object_(this, false);

    notify_renderer_need_redraw();

    bManuallyRendered_ = bManuallyRendered;
    if (bManuallyRendered_)
        pRenderer_ = pRenderer;
    else
        pRenderer_ = nullptr;

    if (pRenderer_ && pRenderer_ != pOldrenderer && bManuallyRendered_)
        pRenderer_->notify_manually_rendered_object_(this, true);

    notify_renderer_need_redraw();
}

bool uiobject::is_manually_rendered() const
{
    return bManuallyRendered_;
}

void uiobject::set_newly_created()
{
    bNewlyCreated_ = true;
}

bool uiobject::is_newly_created() const
{
    return bNewlyCreated_;
}

void uiobject::notify_renderer_need_redraw() const
{
}

void uiobject::fire_redraw() const
{
}

void uiobject::notify_manually_rendered_object_(uiobject* pObject, bool bManuallyRendered)
{
}

void uiobject::mark_for_copy(const std::string& sVariable)
{
    if (utils::find(lCopyList_, sVariable) == lCopyList_.end())
        lCopyList_.push_back(sVariable);
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "\"" << sName_ << "." << sVariable << "\" has already been marked for copy. Ignoring." << std::endl;
    }
}

const std::vector<uiobject*>& uiobject::get_anchored_objects() const
{
    return lAnchoredObjectList_;
}

void uiobject::clear_links()
{
    // Tell the renderer to no longer render this widget
    notify_renderer_need_redraw();
    if (bManuallyRendered_ && pRenderer_)
        pRenderer_->notify_manually_rendered_object_(this, false);

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
            vector2i mOffset = pAnchor->get_abs_offset();

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

        pObj->update_anchors();
    }

    // Unregister this object from the GUI manager
    pManager_->remove_uiobject(this);
}

void uiobject::notify_loaded()
{
    bLoaded_ = true;
}
}
}
