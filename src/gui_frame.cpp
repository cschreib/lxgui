#include "lxgui/gui_frame.hpp"

#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_std.hpp>
#include <lxgui/utils_range.hpp>

#include <sol/state.hpp>
#include <sol/as_args.hpp>
#include <sol/variadic_args.hpp>

#include <sstream>
#include <functional>

namespace lxgui {
namespace gui
{

layer::layer() : bDisabled(false)
{
}

frame::frame(utils::control_block& mBlock, manager& mManager) :
    region(mBlock, mManager), mEventReceiver_(mManager.get_event_emitter())
{
    lType_.push_back(CLASS_NAME);
}

frame::~frame()
{
    // Disable callbacks
    lSignalList_.clear();

    // Children must be destroyed first
    lChildList_.clear();
    lRegionList_.clear();

    if (!bVirtual_)
    {
        // Tell the renderer to no longer render this widget
        get_top_level_renderer()->notify_rendered_frame(observer_from(this), false);
        pRenderer_ = nullptr;
    }

    get_manager().get_root().notify_hovered_frame_dirty();

    set_focus(false);
}

void frame::render() const
{
    if (bIsVisible_ && bReady_)
    {
        if (pBackdrop_)
            pBackdrop_->render();

        // Render child regions
        for (auto& mLayer : lLayerList_)
        {
            if (mLayer.bDisabled) continue;

            for (const auto& pRegion : mLayer.lRegionList)
            {
                if (pRegion->is_shown())
                    pRegion->render();
            }
        }
    }
}

void frame::create_glue()
{
    create_glue_(this);
}

std::string frame::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;

    sStr << region::serialize(sTab);
    if (auto pFrameRenderer = utils::dynamic_pointer_cast<frame>(pRenderer_))
    sStr << sTab << "  # Man. render : " << pFrameRenderer->get_name() << "\n";
    sStr << sTab << "  # Strata      : ";
    switch (mStrata_)
    {
        case frame_strata::PARENT :            sStr << "PARENT\n"; break;
        case frame_strata::BACKGROUND :        sStr << "BACKGROUND\n"; break;
        case frame_strata::LOW :               sStr << "LOW\n"; break;
        case frame_strata::MEDIUM :            sStr << "MEDIUM\n"; break;
        case frame_strata::HIGH :              sStr << "HIGH\n"; break;
        case frame_strata::DIALOG :            sStr << "DIALOG\n"; break;
        case frame_strata::FULLSCREEN :        sStr << "FULLSCREEN\n"; break;
        case frame_strata::FULLSCREEN_DIALOG : sStr << "FULLSCREEN_DIALOG\n"; break;
        case frame_strata::TOOLTIP :           sStr << "TOOLTIP\n"; break;
    }
    sStr << sTab << "  # Level       : " << iLevel_ << "\n";
    sStr << sTab << "  # TopLevel    : " << bIsTopLevel_;
    if (!bIsTopLevel_ && get_top_level_parent())
        sStr << " (" << get_top_level_parent()->get_name() << ")\n";
    else
        sStr << "\n";
    if (!bIsMouseClickEnabled_ && !bIsMouseMoveEnabled_ && ! !bIsMouseWheelEnabled_)
        sStr << sTab << "  # Inputs      : none\n";
    else
    {
        sStr << sTab << "  # Inputs      :\n";
        sStr << sTab << "  |-###\n";
        if (bIsMouseClickEnabled_)
            sStr << sTab << "  |   # mouse click\n";
        if (bIsMouseMoveEnabled_)
            sStr << sTab << "  |   # mouse move\n";
        if (bIsMouseWheelEnabled_)
            sStr << sTab << "  |   # mouse wheel\n";
        sStr << sTab << "  |-###\n";
    }
    sStr << sTab << "  # Movable     : " << bIsMovable_ << "\n";
    sStr << sTab << "  # Resizable   : " << bIsResizable_ << "\n";
    sStr << sTab << "  # Clamped     : " << bIsClampedToScreen_ << "\n";
    sStr << sTab << "  # HRect inset :\n";
    sStr << sTab << "  |-###\n";
    sStr << sTab << "  |   # left   : " << lAbsHitRectInsetList_.left << "\n";
    sStr << sTab << "  |   # right  : " << lAbsHitRectInsetList_.right << "\n";
    sStr << sTab << "  |   # top    : " << lAbsHitRectInsetList_.top << "\n";
    sStr << sTab << "  |   # bottom : " << lAbsHitRectInsetList_.bottom << "\n";
    sStr << sTab << "  |-###\n";
    sStr << sTab << "  # Min width   : " << fMinWidth_ << "\n";
    sStr << sTab << "  # Max width   : " << fMaxWidth_ << "\n";
    sStr << sTab << "  # Min height  : " << fMinHeight_ << "\n";
    sStr << sTab << "  # Max height  : " << fMaxHeight_ << "\n";
    sStr << sTab << "  # Scale       : " << fScale_ << "\n";
    if (pTitleRegion_)
    {
        sStr << sTab << "  # Title reg.  :\n";
        sStr << sTab << "  |-###\n";
        sStr << pTitleRegion_->serialize(sTab+"  | ");
        sStr << sTab << "  |-###\n";
    }
    if (pBackdrop_)
    {
        const bounds2f& lInsets = pBackdrop_->get_background_insets();

        sStr << sTab << "  # Backdrop    :\n";
        sStr << sTab << "  |-###\n";
        sStr << sTab << "  |   # Background : " << pBackdrop_->get_background_file() << "\n";
        sStr << sTab << "  |   # Tilling    : " << pBackdrop_->is_background_tilling() << "\n";
        if (pBackdrop_->is_background_tilling())
            sStr << sTab << "  |   # Tile size  : " << pBackdrop_->get_tile_size() << "\n";
        sStr << sTab << "  |   # BG Insets  :\n";
        sStr << sTab << "  |   |-###\n";
        sStr << sTab << "  |   |   # left   : " << lInsets.left << "\n";
        sStr << sTab << "  |   |   # right  : " << lInsets.right << "\n";
        sStr << sTab << "  |   |   # top    : " << lInsets.top << "\n";
        sStr << sTab << "  |   |   # bottom : " << lInsets.bottom << "\n";
        sStr << sTab << "  |   |-###\n";
        sStr << sTab << "  |   # Edge       : " << pBackdrop_->get_edge_file() << "\n";
        sStr << sTab << "  |   # Edge size  : " << pBackdrop_->get_edge_size() << "\n";
        sStr << sTab << "  |-###\n";
    }

    if (!lRegionList_.empty())
    {
        if (lChildList_.size() == 1)
            sStr << sTab << "  # Region : \n";
        else
            sStr << sTab << "  # Regions     : " << lRegionList_.size() << "\n";
        sStr << sTab << "  |-###\n";

        for (auto& mRegion : get_regions())
        {
            sStr << mRegion.serialize(sTab+"  | ");
            sStr << sTab << "  |-###\n";
        }
    }

    if (!lChildList_.empty())
    {
        if (lChildList_.size() == 1)
            sStr << sTab << "  # Child : \n";
        else
            sStr << sTab << "  # Children    : " << lChildList_.size() << "\n";
        sStr << sTab << "  |-###\n";

        for (const auto& mChild : get_children())
        {
            sStr << mChild.serialize(sTab+"  | ");
            sStr << sTab << "  |-###\n";
        }
    }

    return sStr.str();
}

bool frame::can_use_script(const std::string& sScriptName) const
{
    if ((sScriptName == "OnChar") ||
        (sScriptName == "OnDragStart") ||
        (sScriptName == "OnDragStop") ||
        (sScriptName == "OnDragMove") ||
        (sScriptName == "OnEnter") ||
        (sScriptName == "OnEvent") ||
        (sScriptName == "OnFocusGained") ||
        (sScriptName == "OnFocusLost") ||
        (sScriptName == "OnHide") ||
        (sScriptName == "OnKeyDown") ||
        (sScriptName == "OnKeyUp") ||
        (sScriptName == "OnLeave") ||
        (sScriptName == "OnLoad") ||
        (sScriptName == "OnMouseDown") ||
        (sScriptName == "OnMouseUp") ||
        (sScriptName == "OnDoubleClick") ||
        (sScriptName == "OnMouseWheel") ||
        (sScriptName == "OnReceiveDrag") ||
        (sScriptName == "OnShow") ||
        (sScriptName == "OnSizeChanged") ||
        (sScriptName == "OnUpdate"))
        return true;
    else
        return false;
}

void frame::copy_from(const uiobject& mObj)
{
    base::copy_from(mObj);

    const frame* pFrame = down_cast<frame>(&mObj);
    if (!pFrame)
        return;

    for (const auto& mItem : pFrame->lSignalList_)
    {
        for (const auto& mFunction : pFrame->get_script(mItem.first))
        {
            this->add_script(mItem.first, mFunction);
        }
    }

    this->set_frame_strata(pFrame->get_frame_strata());

    utils::observer_ptr<const frame> pHighParent = observer_from(this);

    for (int i = 0; i < pFrame->get_level(); ++i)
    {
        if (!pHighParent->get_parent())
            break;

        pHighParent = pHighParent->get_parent();
    }

    this->set_level(pHighParent->get_level() + pFrame->get_level());

    this->set_top_level(pFrame->is_top_level());

    this->enable_mouse_click(pFrame->is_mouse_click_enabled());
    this->enable_mouse_move(pFrame->is_mouse_move_enabled());
    this->enable_mouse_wheel(pFrame->is_mouse_wheel_enabled());

    this->set_movable(pFrame->is_movable());
    this->set_clamped_to_screen(pFrame->is_clamped_to_screen());
    this->set_resizable(pFrame->is_resizable());

    this->set_abs_hit_rect_insets(pFrame->get_abs_hit_rect_insets());
    this->set_rel_hit_rect_insets(pFrame->get_rel_hit_rect_insets());

    this->set_max_dimensions(pFrame->get_max_dimensions());
    this->set_min_dimensions(pFrame->get_min_dimensions());

    this->set_scale(pFrame->get_scale());

    for (const auto& pArt : pFrame->lRegionList_)
    {
        if (!pArt || pArt->is_special()) continue;

        uiobject_core_attributes mAttr;
        mAttr.sObjectType = pArt->get_object_type();
        mAttr.sName = pArt->get_raw_name();
        mAttr.lInheritance = {pArt};

        auto pNewArt = create_region(pArt->get_draw_layer(), std::move(mAttr));
        if (!pNewArt) continue;

        pNewArt->notify_loaded();
    }

    bBuildLayerList_ = true;

    if (pFrame->pBackdrop_)
    {
        pBackdrop_ = std::unique_ptr<backdrop>(new backdrop(*this));
        pBackdrop_->copy_from(*pFrame->pBackdrop_);
    }

    if (pFrame->pTitleRegion_)
    {
        this->create_title_region();
        if (pTitleRegion_)
            pTitleRegion_->copy_from(*pFrame->pTitleRegion_);
    }

    for (const auto& pChild : pFrame->lChildList_)
    {
        if (!pChild || pChild->is_special()) continue;

        uiobject_core_attributes mAttr;
        mAttr.sObjectType = pChild->get_object_type();
        mAttr.sName = pChild->get_raw_name();
        mAttr.lInheritance = {pChild};

        auto pNewChild = create_child(std::move(mAttr));
        if (!pNewChild) continue;

        pNewChild->notify_loaded();
    }
}

void frame::create_title_region()
{
    if (pTitleRegion_)
    {
        gui::out << gui::warning << "gui::" << lType_.back() <<
            " : \""+sName_+"\" already has a title region." << std::endl;
        return;
    }

    uiobject_core_attributes mAttr;
    mAttr.sObjectType = "Region";
    mAttr.bVirtual = is_virtual();
    mAttr.sName = "$parentTitleRegion";
    mAttr.pParent = observer_from(this);

    auto pTitleRegion = utils::static_pointer_cast<region>(
        get_manager().get_factory().create_uiobject(get_registry(), std::move(mAttr)));

    if (!pTitleRegion)
        return;

    pTitleRegion->set_special();

    if (!pTitleRegion->is_virtual())
    {
        // Add shortcut to region as entry in Lua table
        auto& mLua = get_lua_();
        mLua[get_lua_name()]["TitleRegion"] = mLua[pTitleRegion->get_lua_name()];
    }

    pTitleRegion_ = std::move(pTitleRegion);
    pTitleRegion_->notify_loaded();
}

utils::observer_ptr<const frame> frame::get_child(const std::string& sName) const
{
    for (const auto& pChild : lChildList_)
    {
        if (!pChild)
            continue;

        if (pChild->get_name() == sName)
            return pChild;

        const std::string& sRawName = pChild->get_raw_name();
        if (utils::starts_with(sRawName, "$parent") && sRawName.substr(7) == sName)
            return pChild;
    }

    return nullptr;
}

frame::region_list_view frame::get_regions()
{
    return region_list_view(lRegionList_);
}

frame::const_region_list_view frame::get_regions() const
{
    return const_region_list_view(lRegionList_);
}

utils::observer_ptr<const layered_region> frame::get_region(const std::string& sName) const
{
    for (const auto& pRegion : lRegionList_)
    {
        if (!pRegion)
            continue;

        if (pRegion->get_name() == sName)
            return pRegion;

        const std::string& sRawName = pRegion->get_raw_name();
        if (utils::starts_with(sRawName, "$parent") && sRawName.substr(7) == sName)
            return pRegion;
    }

    return nullptr;
}

void frame::set_dimensions(const vector2f& mDimensions)
{
    base::set_dimensions(vector2f(
        std::min(std::max(mDimensions.x,  fMinWidth_),  fMaxWidth_),
        std::min(std::max(mDimensions.y, fMinHeight_), fMaxHeight_)
    ));
}

void frame::set_width(float fAbsWidth)
{
    base::set_width(std::min(std::max(fAbsWidth, fMinWidth_), fMaxWidth_));
}

void frame::set_height(float fAbsHeight)
{
    base::set_height(std::min(std::max(fAbsHeight, fMinHeight_), fMaxHeight_));
}

void frame::check_position_()
{
    if (lBorderList_.right - lBorderList_.left < fMinWidth_)
    {
        lBorderList_.right = lBorderList_.left + fMinWidth_;
    }
    else if (lBorderList_.right - lBorderList_.left > fMaxWidth_)
    {
        lBorderList_.right = lBorderList_.left + fMaxWidth_;
    }

    if (lBorderList_.bottom - lBorderList_.top < fMinHeight_)
    {
        lBorderList_.bottom = lBorderList_.top + fMinHeight_;
    }
    else if (lBorderList_.bottom - lBorderList_.top > fMaxHeight_)
    {
        lBorderList_.bottom = lBorderList_.top + fMaxHeight_;
    }

    if (bIsClampedToScreen_)
    {
        vector2f mScreenDimensions = get_top_level_renderer()->get_target_dimensions();

        if (lBorderList_.right > mScreenDimensions.x)
        {
            float fWidth = lBorderList_.right - lBorderList_.left;
            if (fWidth > mScreenDimensions.x)
            {
                lBorderList_.left = 0;
                lBorderList_.right = mScreenDimensions.x;
            }
            else
            {
                lBorderList_.right = mScreenDimensions.x;
                lBorderList_.left = mScreenDimensions.x - fWidth;
            }
        }

        if (lBorderList_.left < 0)
        {
            float fWidth = lBorderList_.right - lBorderList_.left;
            if (lBorderList_.right - lBorderList_.left > mScreenDimensions.x)
            {
                lBorderList_.left = 0;
                lBorderList_.right = mScreenDimensions.x;
            }
            else
            {
                lBorderList_.left = 0;
                lBorderList_.right = fWidth;
            }
        }

        if (lBorderList_.bottom > mScreenDimensions.y)
        {
            float fHeight = lBorderList_.bottom - lBorderList_.top;
            if (fHeight > mScreenDimensions.y)
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = mScreenDimensions.y;
            }
            else
            {
                lBorderList_.bottom = mScreenDimensions.y;
                lBorderList_.top = mScreenDimensions.y - fHeight;
            }
        }

        if (lBorderList_.top < 0)
        {
            float fHeight = lBorderList_.bottom - lBorderList_.top;
            if (fHeight > mScreenDimensions.y)
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = mScreenDimensions.y;
            }
            else
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = fHeight;
            }
        }
    }
}

void frame::disable_draw_layer(layer_type mLayerID)
{
    layer& mLayer = lLayerList_[static_cast<std::size_t>(mLayerID)];
    if (!mLayer.bDisabled)
    {
        mLayer.bDisabled = true;
        notify_renderer_need_redraw();
    }
}

void frame::enable_draw_layer(layer_type mLayerID)
{
    layer& mLayer = lLayerList_[static_cast<std::size_t>(mLayerID)];
    if (!mLayer.bDisabled)
    {
        mLayer.bDisabled = false;
        notify_renderer_need_redraw();
    }
}

void frame::enable_mouse(bool bIsMouseEnabled)
{
    enable_mouse_click(bIsMouseEnabled);
    enable_mouse_move(bIsMouseEnabled);
}

void frame::enable_mouse_click(bool bIsMouseEnabled)
{
    bIsMouseClickEnabled_ = bIsMouseEnabled;
}

void frame::enable_mouse_move(bool bIsMouseEnabled)
{
    bIsMouseMoveEnabled_ = bIsMouseEnabled;
}

void frame::enable_mouse_wheel(bool bIsMouseWheelEnabled)
{
    bIsMouseWheelEnabled_ = bIsMouseWheelEnabled;
}

void frame::enable_key_capture(const std::string& sKey, bool bIsCaptureEnabled)
{
    if (bIsCaptureEnabled)
        lRegKeyList_.erase(sKey);
    else
        lRegKeyList_.insert(sKey);
}

void frame::notify_loaded()
{
    base::notify_loaded();

    if (!bVirtual_)
    {
        alive_checker mChecker(*this);
        fire_script("OnLoad");
        if (!mChecker.is_alive())
            return;
    }
}

void frame::notify_layers_need_update()
{
    bBuildLayerList_ = true;
}

bool frame::has_script(const std::string& sScriptName) const
{
    const auto mIter = lSignalList_.find(sScriptName);
    if (mIter == lSignalList_.end())
        return false;

    return !mIter->second.empty();
}

utils::observer_ptr<layered_region> frame::add_region(
    utils::owner_ptr<layered_region> pRegion)
{
    if (!pRegion)
        return nullptr;

    pRegion->set_parent_(observer_from(this));

    utils::observer_ptr<layered_region> pAddedRegion = pRegion;
    lRegionList_.push_back(std::move(pRegion));

    notify_layers_need_update();
    notify_renderer_need_redraw();

    if (!bVirtual_)
    {
        // Add shortcut to region as entry in Lua table
        std::string sRawName = pAddedRegion->get_raw_name();
        if (utils::starts_with(sRawName, "$parent"))
        {
            sRawName.erase(0, std::string("$parent").size());
            auto& mLua = get_lua_();
            mLua[get_lua_name()][sRawName] = mLua[pAddedRegion->get_lua_name()];
        }
    }

    return pAddedRegion;
}

utils::owner_ptr<layered_region> frame::remove_region(
    const utils::observer_ptr<layered_region>& pRegion)
{
    if (!pRegion)
        return nullptr;

    layered_region* pRawPointer = pRegion.get();
    auto mIter = utils::find_if(lRegionList_, [&](auto& pObj)
    {
        return pObj.get() == pRawPointer;
    });

    if (mIter == lRegionList_.end())
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Trying to remove \"" << pRegion->get_name() << "\" from \"" << sName_ << "\"'s children, "
            "but it was not one of this frame's children." << std::endl;
        return nullptr;
    }

    // NB: the iterator is not removed yet; it will be removed later in update().
    auto pRemovedRegion = std::move(*mIter);

    notify_layers_need_update();
    notify_renderer_need_redraw();
    pRemovedRegion->set_parent_(nullptr);

    if (!bVirtual_)
    {
        // Remove shortcut to region
        std::string sRawName = pRemovedRegion->get_raw_name();
        if (utils::starts_with(sRawName, "$parent"))
        {
            sRawName.erase(0, std::string("$parent").size());
            sol::state& mLua = get_lua_();
            mLua[get_lua_name()][sRawName] = sol::lua_nil;
        }
    }

    return pRemovedRegion;
}

utils::observer_ptr<layered_region> frame::create_region(layer_type mLayer,
    uiobject_core_attributes mAttr)
{
    mAttr.bVirtual = is_virtual();
    mAttr.pParent = observer_from(this);

    auto pRegion = get_manager().get_factory().create_layered_region(get_registry(), mAttr);

    if (!pRegion)
        return nullptr;

    pRegion->set_draw_layer(mLayer);

    return add_region(std::move(pRegion));
}

utils::observer_ptr<frame> frame::create_child(uiobject_core_attributes mAttr)
{
    mAttr.bVirtual = is_virtual();
    mAttr.pParent = observer_from(this);

    auto pNewFrame = get_manager().get_factory().create_frame(
        get_registry(), get_top_level_renderer().get(), mAttr);

    if (!pNewFrame)
        return nullptr;

    pNewFrame->set_level(get_level() + 1);

    return add_child(std::move(pNewFrame));
}

utils::observer_ptr<frame> frame::add_child(utils::owner_ptr<frame> pChild)
{
    if (!pChild)
        return nullptr;

    pChild->set_parent_(observer_from(this));

    if (is_visible() && pChild->is_shown())
        pChild->notify_visible();
    else
        pChild->notify_invisible();

    utils::observer_ptr<frame> pAddedChild = pChild;
    lChildList_.push_back(std::move(pChild));

    if (!bVirtual_)
    {
        utils::observer_ptr<frame_renderer> pOldTopLevelRenderer = pAddedChild->get_top_level_renderer();
        utils::observer_ptr<frame_renderer> pNewTopLevelRenderer = get_top_level_renderer();
        if (pOldTopLevelRenderer != pNewTopLevelRenderer)
        {
            pOldTopLevelRenderer->notify_rendered_frame(pAddedChild, false);
            pNewTopLevelRenderer->notify_rendered_frame(pAddedChild, true);
        }

        // Add shortcut to child as entry in Lua table
        std::string sRawName = pAddedChild->get_raw_name();
        if (utils::starts_with(sRawName, "$parent"))
        {
            sRawName.erase(0, std::string("$parent").size());
            auto& mLua = get_lua_();
            mLua[get_lua_name()][sRawName] = mLua[pAddedChild->get_lua_name()];
        }
    }

    return pAddedChild;
}

utils::owner_ptr<frame> frame::remove_child(const utils::observer_ptr<frame>& pChild)
{
    if (!pChild)
        return nullptr;

    frame* pRawPointer = pChild.get();
    auto mIter = utils::find_if(lChildList_, [&](auto& pObj)
    {
        return pObj.get() == pRawPointer;
    });

    if (mIter == lChildList_.end())
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Trying to remove \"" << pChild->get_name() << "\" from \"" << sName_
            << "\"'s children, but it was not one of this frame's children." << std::endl;
        return nullptr;
    }

    // NB: the iterator is not removed yet; it will be removed later in update().
    auto pRemovedChild = std::move(*mIter);

    bool bNotifyRenderer = false;
    if (!bVirtual_)
    {
        utils::observer_ptr<frame_renderer> pTopLevelRenderer = get_top_level_renderer();
        bNotifyRenderer = !pChild->get_renderer() && pTopLevelRenderer.get() != &get_manager().get_root();
        if (bNotifyRenderer)
        {
            pTopLevelRenderer->notify_rendered_frame(pChild, false);
            pChild->propagate_renderer_(false);
        }
    }

    pRemovedChild->set_parent_(nullptr);

    if (!bVirtual_)
    {
        if (bNotifyRenderer)
        {
            get_manager().get_root().notify_rendered_frame(pChild, true);
            pChild->propagate_renderer_(true);
        }

        // Remove shortcut to child
        std::string sRawName = pRemovedChild->get_raw_name();
        if (utils::starts_with(sRawName, "$parent"))
        {
            sRawName.erase(0, std::string("$parent").size());
            sol::state& mLua = get_lua_();
            mLua[get_lua_name()][sRawName] = sol::lua_nil;
        }
    }

    return pRemovedChild;
}

frame::child_list_view frame::get_children()
{
    return child_list_view(lChildList_);
}

frame::const_child_list_view frame::get_children() const
{
    return const_child_list_view(lChildList_);
}

float frame::get_effective_alpha() const
{
    if (pParent_)
        return fAlpha_*pParent_->get_effective_alpha();
    else
        return fAlpha_;
}

float frame::get_effective_scale() const
{
    if (pParent_)
        return fScale_*pParent_->get_effective_scale();
    else
        return fScale_;
}

int frame::get_level() const
{
    return iLevel_;
}

frame_strata frame::get_frame_strata() const
{
    return mStrata_;
}

utils::observer_ptr<const frame> frame::get_top_level_parent() const
{
    auto pFrame = observer_from(this);
    do
    {
        if (pFrame->is_top_level())
            return pFrame;

        pFrame = pFrame->get_parent();
    }
    while (pFrame);

    return nullptr;
}

const backdrop* frame::get_backdrop() const
{
    return pBackdrop_.get();
}

backdrop* frame::get_backdrop()
{
    return pBackdrop_.get();
}

backdrop& frame::get_or_create_backdrop()
{
    if (!pBackdrop_)
        pBackdrop_ = std::unique_ptr<backdrop>(new backdrop(*this));

    return *pBackdrop_;
}

const std::string& frame::get_frame_type() const
{
    return lType_.back();
}

const bounds2f& frame::get_abs_hit_rect_insets() const
{
    return lAbsHitRectInsetList_;
}

const bounds2f& frame::get_rel_hit_rect_insets() const
{
    return lRelHitRectInsetList_;
}

vector2f frame::get_max_dimensions() const
{
    return vector2f(fMaxWidth_, fMaxHeight_);
}

vector2f frame::get_min_dimensions() const
{
    return vector2f(fMinWidth_, fMinHeight_);
}

std::size_t frame::get_num_children() const
{
    return std::count_if(lChildList_.begin(), lChildList_.end(),
        [](const auto& pChild) { return pChild != nullptr; });
}

std::size_t frame::get_rough_num_children() const
{
    return lChildList_.size();
}

std::size_t frame::get_num_regions() const
{
    return std::count_if(lRegionList_.begin(), lRegionList_.end(),
        [](const auto& pRegion) { return pRegion != nullptr; });
}

std::size_t frame::get_rough_num_regions() const
{
    return lRegionList_.size();
}

float frame::get_scale() const
{
    return fScale_;
}

bool frame::is_clamped_to_screen() const
{
    return bIsClampedToScreen_;
}

bool frame::is_in_region(const vector2f& mPosition) const
{
    if (pTitleRegion_ && pTitleRegion_->is_in_region(mPosition))
        return true;

    bool bIsInXRange = lBorderList_.left  + lAbsHitRectInsetList_.left <= mPosition.x &&
        mPosition.x <= lBorderList_.right - lAbsHitRectInsetList_.right - 1.0f;
    bool bIsInYRange = lBorderList_.top   + lAbsHitRectInsetList_.top <= mPosition.y &&
        mPosition.y <= lBorderList_.bottom - lAbsHitRectInsetList_.bottom - 1.0f;

    return bIsInXRange && bIsInYRange;
}

utils::observer_ptr<const frame> frame::find_topmost_frame(
    const std::function<bool(const frame&)>& mPredicate) const
{
    if (mPredicate(*this))
        return observer_from(this);

    return nullptr;
}

bool frame::is_mouse_click_enabled() const
{
    return bIsMouseClickEnabled_;
}

bool frame::is_mouse_move_enabled() const
{
    return bIsMouseMoveEnabled_;
}

bool frame::is_mouse_wheel_enabled() const
{
    return bIsMouseWheelEnabled_;
}

bool frame::is_registered_for_drag(const std::string& sButton) const
{
    return lRegDragList_.find(sButton) != lRegDragList_.end();
}

bool frame::is_key_capture_enabled(const std::string& sKey) const
{
    return lRegKeyList_.find(sKey) != lRegKeyList_.end();
}

bool frame::is_movable() const
{
    return bIsMovable_;
}

bool frame::is_resizable() const
{
    return bIsResizable_;
}

bool frame::is_top_level() const
{
    return bIsTopLevel_;
}

bool frame::is_user_placed() const
{
    return bIsUserPlaced_;
}

std::string frame::get_adjusted_script_name(const std::string& sScriptName)
{
    std::string sAdjustedName = sScriptName;
    for (auto iter = sAdjustedName.begin(); iter != sAdjustedName.end(); ++iter)
    {
        if ('A' <= *iter && *iter <= 'Z')
        {
            *iter = std::tolower(*iter);
            if (iter != sAdjustedName.begin())
                iter = sAdjustedName.insert(iter, '_');
        }
    }

    return sAdjustedName;
}

std::string hijack_sol_error_line(std::string sOriginalMessage, const std::string& sFile, std::size_t uiLineNbr)
{
    auto uiPos1 = sOriginalMessage.find("[string \"" + sFile);
    if (uiPos1 == std::string::npos)
        return sOriginalMessage;

    auto uiPos2 = sOriginalMessage.find_first_of('"', uiPos1 + 9);
    if (uiPos2 == std::string::npos)
        return sOriginalMessage;

    sOriginalMessage.erase(uiPos1, uiPos2 - uiPos1 + 2);
    sOriginalMessage.insert(uiPos1, sFile);

    auto uiPos3 = sOriginalMessage.find_first_of(':', uiPos1 + sFile.size());
    if (uiPos3 == std::string::npos)
        return sOriginalMessage;

    auto uiPos4 = sOriginalMessage.find_first_of(":>", uiPos3 + 1);
    if (uiPos4 == std::string::npos)
        return sOriginalMessage;

    std::size_t uiOffset = 0;
    if (!utils::from_string(sOriginalMessage.substr(uiPos3 + 1, uiPos4 - uiPos3 - 1), uiOffset))
        return sOriginalMessage;

    sOriginalMessage.erase(uiPos3 + 1, uiPos4 - uiPos3 - 1);
    sOriginalMessage.insert(uiPos3 + 1, utils::to_string(uiLineNbr + uiOffset - 1));
    uiPos4 = sOriginalMessage.find_first_of(':', uiPos3 + 1);

    auto uiPos5 = sOriginalMessage.find("[string \"" + sFile, uiPos4);
    if (uiPos5 == std::string::npos)
        return sOriginalMessage;

    std::string sMessage = sOriginalMessage.substr(uiPos4 + 1);
    sOriginalMessage.erase(uiPos4 + 1);
    sOriginalMessage += hijack_sol_error_line(sMessage, sFile, uiLineNbr);

    return sOriginalMessage;
}

std::string hijack_sol_error_message(std::string sOriginalMessage, const std::string& sFile, std::size_t uiLineNbr)
{
    std::string sNewError;
    for (auto sLine : utils::cut(sOriginalMessage, "\n"))
    {
        if (!sNewError.empty())
            sNewError += '\n';

        sNewError += hijack_sol_error_line(sLine, sFile, uiLineNbr);
    }

    return sNewError;
}

utils::connection frame::define_script_(const std::string& sScriptName,
    const std::string& sContent, bool bAppend, const script_info& mInfo)
{
    // Create the Lua function from the provided string
    sol::state& mLua = get_lua_();

    std::string sStr = "return function(self";

    constexpr std::size_t uiMaxArgs = 9;
    for (std::size_t i = 0; i < uiMaxArgs; ++i)
        sStr += ", arg" + utils::to_string(i + 1);

    sStr += ") " + sContent + " end";

    auto mResult = mLua.do_string(sStr, mInfo.sFileName);

    if (!mResult.valid())
    {
        sol::error mError = mResult;
        std::string sError = hijack_sol_error_message(mError.what(), mInfo.sFileName, mInfo.uiLineNbr);

        gui::out << gui::error << sError << std::endl;

        get_manager().get_event_emitter().fire_event("LUA_ERROR", {sError});
        return {};
    }

    sol::protected_function mHandler = mResult;

    // Forward it as any other Lua function
    return define_script_(sScriptName, std::move(mHandler), bAppend, mInfo);
}

utils::connection frame::define_script_(const std::string& sScriptName,
    sol::protected_function mHandler, bool bAppend, const script_info& mInfo)
{
    auto mWrappedHandler =
        [mHandler = std::move(mHandler), mInfo](frame& mSelf, const event_data& mArgs)
    {
        sol::state& mLua = mSelf.get_manager().get_lua();
        lua_State* pLua = mLua.lua_state();

        std::vector<sol::object> lArgs;

        // Set arguments
        for (std::size_t i = 0; i < mArgs.get_num_param(); ++i)
        {
            const utils::variant& mArg = mArgs.get(i);
            if (std::holds_alternative<utils::empty>(mArg))
                lArgs.emplace_back(sol::lua_nil);
            else
                lArgs.emplace_back(pLua, sol::in_place, mArg);
        }

        // Get a reference to self
        sol::object mSelfLua = mLua[mSelf.get_lua_name()];
        if (mSelfLua == sol::lua_nil)
            throw gui::exception("Lua glue object is nil");

        // Call the function
        auto mResult = mHandler(mSelfLua, sol::as_args(lArgs));
        // WARNING: after this point, the frame (mSelf) may be deleted.
        // Do not use any member variable or member function directly.

        // Handle errors
        if (!mResult.valid())
        {
            sol::error mError = mResult;
            std::string sError = hijack_sol_error_message(mError.what(),
                mInfo.sFileName, mInfo.uiLineNbr);

            throw gui::exception(sError);
        }
    };

    return define_script_(sScriptName, std::move(mWrappedHandler), bAppend, mInfo);
}

utils::connection frame::define_script_(const std::string& sScriptName,
    script_function mHandler, bool bAppend, const script_info& mInfo)
{
    if (!is_virtual())
    {
        // Register the function so it can be called directly from Lua
        std::string sAdjustedName = get_adjusted_script_name(sScriptName);

        get_lua_()[get_lua_name()][sAdjustedName].set_function(
            [=](frame& mSelf, sol::variadic_args mVArgs)
            {
                event_data mData;
                for (auto&& mArg : mVArgs)
                {
                    lxgui::utils::variant mVariant;
                    if (!mArg.is<sol::lua_nil_t>())
                        mVariant = mArg;

                    mData.add(std::move(mVariant));
                }

                mSelf.fire_script(sScriptName, mData);
            }
        );
    }

    auto& lHandlerList = lSignalList_[sScriptName];
    if (!bAppend)
    {
        // Just disable existing scripts, it may not be safe to modify the handler list
        // if this script is being defined during a handler execution.
        // They will be deleted later, when we know it is safe.
        lHandlerList.disconnect_all();
    }

    // TODO: add file/line info if the handler comes from C++
    // https://github.com/cschreib/lxgui/issues/96
    return lHandlerList.connect(std::move(mHandler));
}

script_list_view frame::get_script(const std::string& sScriptName) const
{
    auto iterH = lSignalList_.find(sScriptName);
    if (iterH == lSignalList_.end())
        throw gui::exception(lType_.back(), "no script registered for " + sScriptName);

    return iterH->second.slots();
}

void frame::remove_script(const std::string& sScriptName)
{
    auto iterH = lSignalList_.find(sScriptName);
    if (iterH == lSignalList_.end()) return;

    // Just disable existing scripts, it may not be safe to modify the handler list
    // if this script is being defined during a handler execution.
    // They will be deleted later, when we know it is safe.
    iterH->second.disconnect_all();

    if (!is_virtual())
    {
        std::string sAdjustedName = get_adjusted_script_name(sScriptName);
        get_lua_()[get_lua_name()][sAdjustedName] = sol::lua_nil;
    }
}

void frame::on_event_(std::string_view sEventName, const event_data& mEvent)
{
    alive_checker mChecker(*this);

    if (has_script("OnEvent"))
    {
        // ADDON_LOADED should only be fired if it's this frame's addon
        if (sEventName == "ADDON_LOADED")
        {
            if (!pAddOn_ || pAddOn_->sName != mEvent.get<std::string>(0))
                return;
        }

        event_data mData;
        mData.add(std::string(sEventName));
        for (std::size_t i = 0; i < mEvent.get_num_param(); ++i)
            mData.add(mEvent.get(i));

        fire_script("OnEvent", mData);
        if (!mChecker.is_alive())
            return;
    }
}

void frame::fire_script(const std::string& sScriptName, const event_data& mData)
{
    if (!is_loaded())
        return;

    auto iterH = lSignalList_.find(sScriptName);
    if (iterH == lSignalList_.end())
        return;

    // Make a copy of useful pointers: in case the frame is deleted, we will need this
    auto& mEventEmitter = get_manager().get_event_emitter();
    auto& mAddonRegistry = *get_manager().get_addon_registry();
    auto* pOldAddOn = mAddonRegistry.get_current_addon();
    mAddonRegistry.set_current_addon(get_addon());

    try
    {
        // Call the handlers
        iterH->second(*this, mData);
    }
    catch (const std::exception& mException)
    {
        std::string sError = mException.what();

        gui::out << gui::error << sError << std::endl;

        mEventEmitter.fire_event("LUA_ERROR", {sError});
    }

    mAddonRegistry.set_current_addon(pOldAddOn);
}

void frame::register_event(const std::string& sEventName)
{
    if (bVirtual_)
        return;

    mEventReceiver_.register_event(sEventName,
        [=](const event_data& mEvent)
        {
            return on_event_(sEventName, mEvent);
        }
    );
}

void frame::unregister_event(const std::string& sEventName)
{
    if (bVirtual_)
        return;

    mEventReceiver_.unregister_event(sEventName);
}

void frame::register_for_drag(const std::vector<std::string>& lButtonList)
{
    lRegDragList_.clear();
    for (const auto& sButton : lButtonList)
        lRegDragList_.insert(sButton);
}

void frame::set_clamped_to_screen(bool bIsClampedToScreen)
{
    bIsClampedToScreen_ = bIsClampedToScreen;
}

void frame::set_frame_strata(frame_strata mStrata)
{
    if (mStrata == frame_strata::PARENT)
    {
        if (!bVirtual_)
        {
            if (pParent_)
                mStrata = pParent_->get_frame_strata();
            else
                mStrata = frame_strata::MEDIUM;
        }
    }

    std::swap(mStrata_, mStrata);

    if (mStrata_ != mStrata && !bVirtual_)
    {
        get_top_level_renderer()->notify_frame_strata_changed(
            observer_from(this), mStrata, mStrata_);
    }
}

void frame::set_frame_strata(const std::string& sStrata)
{
    frame_strata mStrata;

    if (sStrata == "BACKGROUND")
        mStrata = frame_strata::BACKGROUND;
    else if (sStrata == "LOW")
        mStrata = frame_strata::LOW;
    else if (sStrata == "MEDIUM")
        mStrata = frame_strata::MEDIUM;
    else if (sStrata == "HIGH")
        mStrata = frame_strata::HIGH;
    else if (sStrata == "DIALOG")
        mStrata = frame_strata::DIALOG;
    else if (sStrata == "FULLSCREEN")
        mStrata = frame_strata::FULLSCREEN;
    else if (sStrata == "FULLSCREEN_DIALOG")
        mStrata = frame_strata::FULLSCREEN_DIALOG;
    else if (sStrata == "TOOLTIP")
        mStrata = frame_strata::TOOLTIP;
    else if (sStrata == "PARENT")
    {
        if (bVirtual_)
            mStrata = frame_strata::PARENT;
        else
        {
            if (pParent_)
                mStrata = pParent_->get_frame_strata();
            else
                mStrata = frame_strata::MEDIUM;
        }
    }
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : Unknown strata : \""+sStrata+"\"." << std::endl;
        return;
    }

    set_frame_strata(mStrata);
}

void frame::set_backdrop(std::unique_ptr<backdrop> pBackdrop)
{
    pBackdrop_ = std::move(pBackdrop);
    notify_renderer_need_redraw();
}

void frame::set_abs_hit_rect_insets(const bounds2f& lInsets)
{
    lAbsHitRectInsetList_ = lInsets;
}

void frame::set_rel_hit_rect_insets(const bounds2f& lInsets)
{
    lRelHitRectInsetList_ = lInsets;
}

void frame::set_level(int iLevel)
{
    if (iLevel != iLevel_)
    {
        std::swap(iLevel, iLevel_);

        if (!bVirtual_)
        {
            get_top_level_renderer()->notify_frame_level_changed(
                observer_from(this), iLevel, iLevel_);
        }
    }
}

void frame::set_max_dimensions(const vector2f& mMax)
{
    set_max_width(mMax.x);
    set_max_height(mMax.y);
}

void frame::set_min_dimensions(const vector2f& mMin)
{
    set_min_width(mMin.x);
    set_min_height(mMin.y);
}

void frame::set_max_height(float fMaxHeight)
{
    if (fMaxHeight < 0.0f) fMaxHeight = std::numeric_limits<float>::infinity();

    if (fMaxHeight >= fMinHeight_)
        fMaxHeight_ = fMaxHeight;

    if (fMaxHeight_ != fMaxHeight && !bVirtual_)
        notify_borders_need_update();
}

void frame::set_max_width(float fMaxWidth)
{
    if (fMaxWidth < 0.0f) fMaxWidth = std::numeric_limits<float>::infinity();

    if (fMaxWidth >= fMinWidth_)
        fMaxWidth_ = fMaxWidth;

    if (fMaxWidth_ != fMaxWidth && !bVirtual_)
        notify_borders_need_update();
}

void frame::set_min_height(float fMinHeight)
{
    if (fMinHeight <= fMaxHeight_)
        fMinHeight_ = fMinHeight;

    if (fMinHeight_ != fMinHeight && !bVirtual_)
        notify_borders_need_update();
}

void frame::set_min_width(float fMinWidth)
{
    if (fMinWidth <= fMaxWidth_)
        fMinWidth_ = fMinWidth;

    if (fMinWidth_ != fMinWidth && !bVirtual_)
        notify_borders_need_update();
}

void frame::set_movable(bool bIsMovable)
{
    bIsMovable_ = bIsMovable;
}

utils::owner_ptr<uiobject> frame::release_from_parent()
{
    utils::observer_ptr<frame> pSelf = observer_from(this);
    if (pParent_)
        return pParent_->remove_child(pSelf);
    else
        return get_manager().get_root().remove_root_frame(pSelf);
}

void frame::set_resizable(bool bIsResizable)
{
    bIsResizable_ = bIsResizable;
}

void frame::set_scale(float fScale)
{
    fScale_ = fScale;
    if (fScale_ != fScale)
        notify_renderer_need_redraw();
}

void frame::set_top_level(bool bIsTopLevel)
{
    bIsTopLevel_ = bIsTopLevel;
}

void frame::raise()
{
    if (!bIsTopLevel_)
        return;

    int iOldLevel = iLevel_;
    auto pTopLevelRenderer = get_top_level_renderer();
    iLevel_ = pTopLevelRenderer->get_highest_level(mStrata_) + 1;

    if (iLevel_ > iOldLevel)
    {
        if (!is_virtual())
        {
            pTopLevelRenderer->notify_frame_level_changed(
                observer_from(this), iOldLevel, iLevel_);
        }

        int iAmount = iLevel_ - iOldLevel;

        for (auto& mChild : get_children())
            mChild.add_level_(iAmount);
    }
    else
        iLevel_ = iOldLevel;
}

void frame::enable_auto_focus(bool bEnable)
{
    bAutoFocus_ = bEnable;
}

bool frame::is_auto_focus_enabled() const
{
    return bAutoFocus_;
}

void frame::set_focus(bool bFocus)
{
    auto& mRoot = get_manager().get_root();
    if (bFocus)
        mRoot.request_focus(observer_from(this));
    else
        mRoot.release_focus(*this);
}

bool frame::has_focus() const
{
    return bFocus_;
}

void frame::notify_focus(bool bFocus)
{
    if (bFocus_ == bFocus)
        return;

    bFocus_ = bFocus;

    if (bFocus_)
        fire_script("OnFocusGained");
    else
        fire_script("OnFocusLost");
}

void frame::add_level_(int iAmount)
{
    int iOldLevel = iLevel_;
    iLevel_ += iAmount;

    if (!is_virtual())
    {
        get_top_level_renderer()->notify_frame_level_changed(
            observer_from(this), iOldLevel, iLevel_);
    }

    for (auto& mChild : get_children())
        mChild.add_level_(iAmount);
}

void frame::set_user_placed(bool bIsUserPlaced)
{
    bIsUserPlaced_ = bIsUserPlaced;
}

void frame::start_moving()
{
    if (bIsMovable_)
    {
        set_user_placed(true);
        get_manager().get_root().start_moving(observer_from(this));
    }
}

void frame::stop_moving()
{
    if (get_manager().get_root().is_moving(*this))
        get_manager().get_root().stop_moving();
}

void frame::start_sizing(const anchor_point& mPoint)
{
    if (bIsResizable_)
    {
        set_user_placed(true);
        get_manager().get_root().start_sizing(observer_from(this), mPoint);
    }
}

void frame::stop_sizing()
{
    if (get_manager().get_root().is_sizing(*this))
        get_manager().get_root().stop_sizing();
}

void frame::propagate_renderer_(bool bRendered)
{
    auto pTopLevelRenderer = get_top_level_renderer();
    for (const auto& pChild : lChildList_)
    {
        if (!pChild)
            continue;

        if (!pChild->get_renderer())
            pTopLevelRenderer->notify_rendered_frame(pChild, bRendered);

        pChild->propagate_renderer_(bRendered);
    }
}

void frame::set_renderer(utils::observer_ptr<frame_renderer> pRenderer)
{
    if (pRenderer == pRenderer_)
        return;

    get_top_level_renderer()->notify_rendered_frame(observer_from(this), false);

    propagate_renderer_(false);

    pRenderer_ = std::move(pRenderer);

    get_top_level_renderer()->notify_rendered_frame(observer_from(this), true);

    propagate_renderer_(true);
}

utils::observer_ptr<const frame_renderer> frame::get_top_level_renderer() const
{
    if (pRenderer_)
        return pRenderer_;
    else if (pParent_)
        return pParent_->get_top_level_renderer();
    else
        return get_manager().get_root().observer_from_this();
}

void frame::notify_visible()
{
    alive_checker mChecker(*this);

    if (bAutoFocus_)
    {
        set_focus(true);
        if (!mChecker.is_alive())
            return;
    }

    base::notify_visible();

    for (auto& mRegion : get_regions())
    {
        if (mRegion.is_shown())
        {
            mRegion.notify_visible();
            if (!mChecker.is_alive())
                return;
        }
    }

    for (auto& mChild : get_children())
    {
        if (mChild.is_shown())
        {
            mChild.notify_visible();
            if (!mChecker.is_alive())
                return;
        }
    }

    lQueuedEventList_.push_back("OnShow");
    notify_renderer_need_redraw();
}

void frame::notify_invisible()
{
    alive_checker mChecker(*this);

    set_focus(false);
    if (!mChecker.is_alive())
        return;

    base::notify_invisible();

    for (auto& mChild : get_children())
    {
        if (mChild.is_shown())
        {
            mChild.notify_invisible();
            if (!mChecker.is_alive())
                return;
        }
    }

    lQueuedEventList_.push_back("OnHide");
    notify_renderer_need_redraw();
}

void frame::notify_renderer_need_redraw()
{
    if (bVirtual_)
        return;

    get_top_level_renderer()->notify_strata_needs_redraw(mStrata_);
}

void frame::notify_scaling_factor_updated()
{
    base::notify_scaling_factor_updated();

    if (pTitleRegion_)
        pTitleRegion_->notify_scaling_factor_updated();

    for (auto& mChild : get_children())
        mChild.notify_scaling_factor_updated();

    for (auto& mRegion : get_regions())
        mRegion.notify_scaling_factor_updated();
}

void frame::show()
{
    if (bIsShown_)
        return;

    bool bWasVisible_ = bIsVisible_;
    base::show();

    if (!bWasVisible_)
        get_manager().get_root().notify_hovered_frame_dirty();
}

void frame::hide()
{
    if (!bIsShown_)
        return;

    bool bWasVisible_ = bIsVisible_;
    base::hide();

    if (bWasVisible_)
        get_manager().get_root().notify_hovered_frame_dirty();
}

void frame::notify_mouse_in_frame(bool bMouseInframe, const vector2f& mPosition)
{
    alive_checker mChecker(*this);

    if (bMouseInframe)
    {
        if (!bMouseInFrame_)
        {
            fire_script("OnEnter");
            if (!mChecker.is_alive())
                return;
        }

        bMouseInFrame_ = true;
    }
    else
    {
        if (bMouseInFrame_)
        {
            fire_script("OnLeave");
            if (!mChecker.is_alive())
                return;
        }

        bMouseInFrame_ = false;
    }
}

void frame::update_borders_()
{
    const bool bOldReady = bReady_;
    const auto lOldBorderList = lBorderList_;

    base::update_borders_();

    check_position_();

    if (lBorderList_ != lOldBorderList || bReady_ != bOldReady)
    {
        get_manager().get_root().notify_hovered_frame_dirty();
        if (pBackdrop_)
            pBackdrop_->notify_borders_updated();
    }
}

void frame::update(float fDelta)
{
    //#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    alive_checker mChecker(*this);

    DEBUG_LOG("  ~");
    base::update(fDelta);
    DEBUG_LOG("   #");

    for (const auto& sEvent : lQueuedEventList_)
    {
        DEBUG_LOG("   Event " + *iterEvent);
        fire_script(sEvent);
        if (!mChecker.is_alive())
            return;
    }

    lQueuedEventList_.clear();

    if (bBuildLayerList_)
    {
        DEBUG_LOG("   Build layers");
        // Clear layers' content
        for (auto& mLayer : lLayerList_)
            mLayer.lRegionList.clear();

        // Fill layers with regions (with font_string rendered last within the same layer)
        for (const auto& pRegion : lRegionList_)
        {
            if (pRegion && pRegion->get_object_type() != "font_string")
                lLayerList_[static_cast<std::size_t>(pRegion->get_draw_layer())].lRegionList.push_back(pRegion);
        }

        for (const auto& pRegion : lRegionList_)
        {
            if (pRegion && pRegion->get_object_type() == "font_string")
                lLayerList_[static_cast<std::size_t>(pRegion->get_draw_layer())].lRegionList.push_back(pRegion);
        }

        bBuildLayerList_ = false;
    }

    if (is_visible())
    {
        DEBUG_LOG("   On update");
        event_data mData;
        mData.add(fDelta);
        fire_script("OnUpdate", mData);
        if (!mChecker.is_alive())
            return;
    }

    if (pTitleRegion_)
        pTitleRegion_->update(fDelta);

    // Update regions
    DEBUG_LOG("   Update regions");
    for (auto& mRegion : get_regions())
        mRegion.update(fDelta);

    // Remove deleted regions
    {
        auto mIterRemove = std::remove_if(lRegionList_.begin(), lRegionList_.end(), [](auto& pObj) {
            return pObj == nullptr;
        });

        lRegionList_.erase(mIterRemove, lRegionList_.end());
    }

    // Update children
    DEBUG_LOG("   Update children");
    for (auto& mChild : get_children())
    {
        mChild.update(fDelta);
        if (!mChecker.is_alive())
            return;
    }

    // Remove deleted children
    {
        auto mIterRemove = std::remove_if(lChildList_.begin(), lChildList_.end(), [](auto& pObj) {
            return pObj == nullptr;
        });

        lChildList_.erase(mIterRemove, lChildList_.end());
    }

    // Remove empty handlers
    for (auto mIterList = lSignalList_.begin(); mIterList != lSignalList_.end();)
    {
        if (mIterList->second.empty())
            mIterList = lSignalList_.erase(mIterList);
        else
            ++mIterList;
    }

    vector2f mNewSize = get_apparent_dimensions();
    if (mOldSize_ != mNewSize)
    {
        DEBUG_LOG("   On size changed");
        fire_script("OnSizeChanged");
        if (!mChecker.is_alive())
            return;

        mOldSize_ = mNewSize;
    }

    DEBUG_LOG("   .");
}

layer_type layer::get_layer_type(const std::string& sLayer)
{
    if (sLayer == "ARTWORK")
        return layer_type::ARTWORK;
    else if (sLayer == "BACKGROUND")
        return layer_type::BACKGROUND;
    else if (sLayer == "BORDER")
        return layer_type::BORDER;
    else if (sLayer == "HIGHLIGHT")
        return layer_type::HIGHLIGHT;
    else if (sLayer == "OVERLAY")
        return layer_type::OVERLAY;
    else
    {
        gui::out << gui::warning << "layer : Unknown layer type : \""
            << sLayer << "\". Using \"ARTWORK\"." << std::endl;

        return layer_type::ARTWORK;
    }
}
}
}
