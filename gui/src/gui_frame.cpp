#include "lxgui/gui_frame.hpp"

#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_alive_checker.hpp"
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

frame::frame(manager& mManager) : event_receiver(mManager.get_event_manager()), region(mManager)
{
    lType_.push_back(CLASS_NAME);
}

frame::~frame()
{
    // Disable callbacks
    for (auto& lHandlerList : lScriptHandlerList_)
    {
        for (auto& mHandler : *lHandlerList.second)
            mHandler.bDisconnected = true;
    }

    // Children must be destroyed first
    lChildList_.clear();
    lRegionList_.clear();

    if (!bVirtual_)
    {
        // Tell the renderer to no longer render this widget
        get_top_level_renderer()->notify_rendered_frame(observer_from(this), false);
        pRenderer_ = nullptr;
    }

    // Unregister this frame from the GUI manager
    get_manager().remove_frame(observer_from(this));
}

void frame::render()
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
                if (pRegion->is_shown() && !pRegion->is_newly_created())
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
    if (!bIsTopLevel_ && pTopLevelParent_)
        sStr << " (" << pTopLevelParent_->get_name() << ")\n";
    else
        sStr << "\n";
    if (!bIsMouseEnabled_ && !bIsKeyboardEnabled_ && !bIsMouseWheelEnabled_)
        sStr << sTab << "  # Inputs      : none\n";
    else
    {
        sStr << sTab << "  # Inputs      :\n";
        sStr << sTab << "  |-###\n";
        if (bIsMouseEnabled_)
            sStr << sTab << "  |   # mouse\n";
        if (bIsKeyboardEnabled_)
            sStr << sTab << "  |   # keyboard\n";
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
    if ((sScriptName == "OnDragStart") ||
        (sScriptName == "OnDragStop") ||
        (sScriptName == "OnEnter") ||
        (sScriptName == "OnEvent") ||
        (sScriptName == "OnHide") ||
        (sScriptName == "OnKeyDown") ||
        (sScriptName == "OnKeyUp") ||
        (sScriptName == "OnLeave") ||
        (sScriptName == "OnLoad") ||
        (sScriptName == "OnMouseDown") ||
        (sScriptName == "OnMouseUp") ||
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
    uiobject::copy_from(mObj);

    const frame* pFrame = down_cast<frame>(&mObj);
    if (!pFrame)
        return;

    for (const auto& mItem : pFrame->lScriptHandlerList_)
    {
        for (const auto& mHandler : *mItem.second)
        {
            if (!mHandler.bDisconnected)
                this->add_script(mItem.first, mHandler.mCallback);
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

    this->enable_keyboard(pFrame->is_keyboard_enabled());
    this->enable_mouse(pFrame->is_mouse_enabled(), pFrame->is_world_input_allowed());
    this->enable_mouse_wheel(pFrame->is_mouse_wheel_enabled());

    this->set_movable(pFrame->is_movable());
    this->set_clamped_to_screen(pFrame->is_clamped_to_screen());
    this->set_resizable(pFrame->is_resizable());

    this->set_abs_hit_rect_insets(pFrame->get_abs_hit_rect_insets());
    this->set_rel_hit_rect_insets(pFrame->get_rel_hit_rect_insets());

    this->set_max_resize(pFrame->get_max_resize());
    this->set_min_resize(pFrame->get_min_resize());

    this->set_scale(pFrame->get_scale());

    for (const auto& pArt : pFrame->lRegionList_)
    {
        if (!pArt || pArt->is_special()) continue;

        utils::observer_ptr<layered_region> pNewArt = create_region(
            pArt->get_draw_layer(), pArt->get_object_type(), pArt->get_raw_name(),
            {pArt});

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

        utils::observer_ptr<frame> pNewChild = create_child(
            pChild->get_object_type(), pChild->get_raw_name(), {pChild});

        if (!pNewChild) continue;

        pNewChild->notify_loaded();
    }
}

void frame::create_title_region()
{
    if (pTitleRegion_)
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : \""+sName_+"\" already has a title region." << std::endl;
        return;
    }

    auto pTitleRegion = utils::make_owned<region>(get_manager());

    if (this->is_virtual())
        pTitleRegion->set_virtual();

    pTitleRegion->set_special();
    pTitleRegion->set_name_and_parent("$parentTitleRegion", observer_from(this));

    if (!get_manager().add_uiobject(pTitleRegion))
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Cannot create \"" << sName_ << "\"'s title region because another uiobject "
            "already took its name : \"" << pTitleRegion->get_name() << "\"." << std::endl;
        return;
    }

    if (!pTitleRegion->is_virtual())
    {
        pTitleRegion->create_glue();

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

frame::region_list_view frame::get_regions() const
{
    return region_list_view(lRegionList_);
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

void frame::set_abs_dimensions(float fAbsWidth, float fAbsHeight)
{
    uiobject::set_abs_dimensions(
        std::min(std::max(fAbsWidth,  fMinWidth_),  fMaxWidth_),
        std::min(std::max(fAbsHeight, fMinHeight_), fMaxHeight_)
    );
}

void frame::set_abs_width(float fAbsWidth)
{
    uiobject::set_abs_width(std::min(std::max(fAbsWidth, fMinWidth_), fMaxWidth_));
}

void frame::set_abs_height(float fAbsHeight)
{
    uiobject::set_abs_height(std::min(std::max(fAbsHeight, fMinHeight_), fMaxHeight_));
}

void frame::check_position() const
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
        float fScreenW = get_top_level_renderer()->get_target_width();
        float fScreenH = get_top_level_renderer()->get_target_height();

        if (lBorderList_.right > fScreenW)
        {
            float fWidth = lBorderList_.right - lBorderList_.left;
            if (fWidth > fScreenW)
            {
                lBorderList_.left = 0;
                lBorderList_.right = fScreenW;
            }
            else
            {
                lBorderList_.right = fScreenW;
                lBorderList_.left = fScreenW - fWidth;
            }
        }

        if (lBorderList_.left < 0)
        {
            float fWidth = lBorderList_.right - lBorderList_.left;
            if (lBorderList_.right - lBorderList_.left > fScreenW)
            {
                lBorderList_.left = 0;
                lBorderList_.right = fScreenW;
            }
            else
            {
                lBorderList_.left = 0;
                lBorderList_.right = fWidth;
            }
        }

        if (lBorderList_.bottom > fScreenH)
        {
            float fHeight = lBorderList_.bottom - lBorderList_.top;
            if (fHeight > fScreenH)
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = fScreenH;
            }
            else
            {
                lBorderList_.bottom = fScreenH;
                lBorderList_.top = fScreenH - fHeight;
            }
        }

        if (lBorderList_.top < 0)
        {
            float fHeight = lBorderList_.bottom - lBorderList_.top;
            if (fHeight > fScreenH)
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = fScreenH;
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
    layer& mLayer = lLayerList_[static_cast<uint>(mLayerID)];
    if (!mLayer.bDisabled)
    {
        mLayer.bDisabled = true;
        notify_renderer_need_redraw();
    }
}

void frame::enable_draw_layer(layer_type mLayerID)
{
    layer& mLayer = lLayerList_[static_cast<uint>(mLayerID)];
    if (!mLayer.bDisabled)
    {
        mLayer.bDisabled = false;
        notify_renderer_need_redraw();
    }
}

void frame::enable_keyboard(bool bIsKeyboardEnabled)
{
    if (!bVirtual_)
    {
        if (bIsKeyboardEnabled && !bIsKeyboardEnabled_)
        {
            register_event("KEY_PRESSED");
            register_event("KEY_RELEASED");
        }
        else if (!bIsKeyboardEnabled && bIsKeyboardEnabled_)
        {
            unregister_event("KEY_PRESSED");
            unregister_event("KEY_RELEASED");
        }
    }

    bIsKeyboardEnabled_ = bIsKeyboardEnabled;
}

void frame::enable_mouse(bool bIsMouseEnabled, bool bAllowWorldInput)
{
    if (!bVirtual_)
    {
        if (bIsMouseEnabled && !bIsMouseEnabled_)
        {
            register_event("MOUSE_MOVED");
            register_event("MOUSE_PRESSED");
            register_event("MOUSE_DOUBLE_CLICKED");
            register_event("MOUSE_RELEASED");
            register_event("MOUSE_DRAG_START");
            register_event("MOUSE_DRAG_STOP");
        }
        else if (!bIsMouseEnabled && bIsMouseEnabled_)
        {
            unregister_event("MOUSE_MOVED");
            unregister_event("MOUSE_PRESSED");
            unregister_event("MOUSE_DOUBLE_CLICKED");
            unregister_event("MOUSE_RELEASED");
            unregister_event("MOUSE_DRAG_START");
            unregister_event("MOUSE_DRAG_STOP");
        }
    }

    bAllowWorldInput_ = bAllowWorldInput;
    bIsMouseEnabled_ = bIsMouseEnabled;
}

void frame::enable_mouse_wheel(bool bIsMouseWheelEnabled)
{
    if (!bVirtual_)
    {
        if (bIsMouseWheelEnabled && !bIsMouseWheelEnabled_)
            register_event("MOUSE_WHEEL");
        else if (!bIsMouseWheelEnabled && bIsMouseWheelEnabled_)
            unregister_event("MOUSE_WHEEL");
    }

    bIsMouseWheelEnabled_ = bIsMouseWheelEnabled;
}

void frame::notify_loaded()
{
    uiobject::notify_loaded();

    if (!bVirtual_)
    {
        alive_checker mChecker(*this);
        on_script("OnLoad");
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
    const auto mIter = lScriptHandlerList_.find(sScriptName);
    if (mIter == lScriptHandlerList_.end())
        return false;

    for (const auto& mHandler : *mIter->second)
    {
        if (!mHandler.bDisconnected)
            return true;
    }

    return false;
}

utils::observer_ptr<layered_region> frame::add_region(
    utils::owner_ptr<layered_region> pRegion)
{
    if (!pRegion)
        return nullptr;

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
            sol::state& mLua = get_lua_();
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
    pRemovedRegion->set_parent(nullptr);

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

utils::observer_ptr<layered_region> frame::create_region(
    layer_type mLayer, const std::string& sClassName, const std::string& sName,
    const std::vector<utils::observer_ptr<const uiobject>>& lInheritance)
{
    auto pRegion = get_manager().create_layered_region(sClassName);
    if (!pRegion)
        return nullptr;

    if (this->is_virtual())
        pRegion->set_virtual();

    pRegion->set_draw_layer(mLayer);
    pRegion->set_name_and_parent(sName, observer_from(this));

    if (!get_manager().add_uiobject(pRegion))
        return nullptr;

    if (!pRegion->is_virtual())
        pRegion->create_glue();

    for (const auto& pObj : lInheritance)
    {
        if (!pRegion->is_object_type(pObj->get_object_type()))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                << "\"" << pRegion->get_name() << "\" (" << pRegion->get_object_type()
                << ") cannot inherit from \"" << pObj->get_name() << "\" (" << pObj->get_object_type()
                << "). Inheritance skipped." << std::endl;
            continue;
        }

        // Inherit from the other region
        pRegion->copy_from(*pObj);
    }

    return add_region(std::move(pRegion));
}

utils::observer_ptr<frame> frame::create_child(
    const std::string& sClassName, const std::string& sName,
    const std::vector<utils::observer_ptr<const uiobject>>& lInheritance)
{
    if (!get_manager().check_uiobject_name(sName))
        return nullptr;

    auto pNewFrame = get_manager().create_frame(sClassName);
    if (!pNewFrame)
        return nullptr;

    pNewFrame->set_name_and_parent(sName, observer_from(this));

    if (this->is_virtual())
        pNewFrame->set_virtual();

    if (!pNewFrame->is_virtual())
        get_top_level_renderer()->notify_rendered_frame(pNewFrame, true);

    pNewFrame->set_level(get_level() + 1);

    if (!get_manager().add_uiobject(pNewFrame))
        return nullptr;

    if (!pNewFrame->is_virtual())
        pNewFrame->create_glue();

    for (const auto& pObj : lInheritance)
    {
        if (!pNewFrame->is_object_type(pObj->get_object_type()))
        {
            gui::out << gui::warning << "gui::manager : "
                << "\"" << pNewFrame->get_name() << "\" (" << pNewFrame->get_object_type()
                << ") cannot inherit from \"" << pObj->get_name() << "\" (" << pObj->get_object_type()
                << "). Inheritance skipped." << std::endl;
            continue;
        }

        // Inherit from the other frame
        pNewFrame->copy_from(*pObj);
    }

    pNewFrame->set_newly_created();

    return add_child(std::move(pNewFrame));
}

utils::observer_ptr<frame> frame::add_child(utils::owner_ptr<frame> pChild)
{
    if (!pChild)
        return nullptr;

    if (bIsTopLevel_)
        pChild->notify_top_level_parent_(true, observer_from(this));

    if (pTopLevelParent_)
        pChild->notify_top_level_parent_(true, pTopLevelParent_);

    if (is_visible() && pChild->is_shown())
        pChild->notify_visible(!get_manager().is_loading_ui());
    else
        pChild->notify_invisible(!get_manager().is_loading_ui());

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
            sol::state& mLua = get_lua_();
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
        bNotifyRenderer = !pChild->get_renderer() && pTopLevelRenderer.get() != &get_manager();
        if (bNotifyRenderer)
        {
            pTopLevelRenderer->notify_rendered_frame(pChild, false);
            pChild->propagate_renderer_(false);
        }
    }

    pRemovedChild->set_parent(nullptr);

    if (!bVirtual_)
    {
        if (bNotifyRenderer)
        {
            get_manager().notify_rendered_frame(pChild, true);
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

frame::child_list_view frame::get_children() const
{
    return child_list_view(lChildList_);
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

vector2f frame::get_max_resize() const
{
    return vector2f(fMaxWidth_, fMaxHeight_);
}

vector2f frame::get_min_resize() const
{
    return vector2f(fMinWidth_, fMinHeight_);
}

uint frame::get_num_children() const
{
    return std::count_if(lChildList_.begin(), lChildList_.end(),
        [](const auto& pChild) { return pChild != nullptr; });
}

uint frame::get_rough_num_children() const
{
    return lChildList_.size();
}

uint frame::get_num_regions() const
{
    return std::count_if(lRegionList_.begin(), lRegionList_.end(),
        [](const auto& pRegion) { return pRegion != nullptr; });
}

uint frame::get_rough_num_regions() const
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

bool frame::is_in_frame(float fX, float fY) const
{
    if (pTitleRegion_ && pTitleRegion_->is_in_region(fX, fY))
        return true;

    bool bIsInXRange = lBorderList_.left  + lAbsHitRectInsetList_.left <= fX &&
        fX <= lBorderList_.right - lAbsHitRectInsetList_.right - 1.0f;
    bool bIsInYRange = lBorderList_.top   + lAbsHitRectInsetList_.top <= fY &&
        fY <= lBorderList_.bottom - lAbsHitRectInsetList_.bottom - 1.0f;

    return bIsInXRange && bIsInYRange;
}

bool frame::is_keyboard_enabled() const
{
    return bIsKeyboardEnabled_;
}

bool frame::is_mouse_enabled() const
{
    return bIsMouseEnabled_;
}

bool frame::is_world_input_allowed() const
{
    return bAllowWorldInput_;
}

bool frame::is_mouse_wheel_enabled() const
{
    return bIsMouseWheelEnabled_;
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
            *iter = tolower(*iter);
            if (iter != sAdjustedName.begin())
                iter = sAdjustedName.insert(iter, '_');
        }
    }

    return sAdjustedName;
}

std::string hijack_sol_error_line(std::string sOriginalMessage, const std::string& sFile, uint uiLineNbr)
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

    uint uiOffset = utils::string_to_uint(sOriginalMessage.substr(uiPos3 + 1, uiPos4 - uiPos3 - 1));
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

std::string hijack_sol_error_message(std::string sOriginalMessage, const std::string& sFile, uint uiLineNbr)
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

void frame::define_script_(const std::string& sScriptName, const std::string& sContent,
    bool bAppend, const script_info& mInfo)
{
    // Create the Lua function from the provided string
    sol::state& mLua = get_lua_();

    std::string sStr = "return function(self";

    constexpr uint uiMaxArgs = 9;
    for (uint i = 0; i < uiMaxArgs; ++i)
        sStr += ", arg" + utils::to_string(i + 1);

    sStr += ") " + sContent + " end";

    auto mResult = mLua.do_string(sStr, mInfo.sFileName);

    if (!mResult.valid())
    {
        sol::error mError = mResult;
        std::string sError = hijack_sol_error_message(mError.what(), mInfo.sFileName, mInfo.uiLineNbr);

        gui::out << gui::error << sError << std::endl;

        event mEvent("LUA_ERROR");
        mEvent.add(sError);
        get_manager().get_event_manager().fire_event(mEvent);
        return;
    }

    sol::protected_function mHandler = mResult;

    // Forward it as any other Lua function
    define_script_(sScriptName, std::move(mHandler), bAppend, mInfo);
}

void frame::define_script_(const std::string& sScriptName, sol::protected_function mHandler,
    bool bAppend, const script_info& mInfo)
{
    bool bAddEventName = sScriptName == "OnEvent";

    auto mWrappedHandler =
        [bAddEventName, mHandler = std::move(mHandler), mInfo](frame& mSelf, event* pEvent)
    {
        sol::state& mLua = mSelf.get_manager().get_lua();
        lua_State* pLua = mLua.lua_state();

        std::vector<sol::object> lArgs;

        if (pEvent)
        {
            if (bAddEventName)
            {
                // Set event name
                lArgs.emplace_back(pLua, sol::in_place, pEvent->get_name());
            }

            // Set arguments
            for (uint i = 0; i < pEvent->get_num_param(); ++i)
            {
                const utils::variant& mArg = pEvent->get(i);
                if (std::holds_alternative<utils::empty>(mArg))
                    lArgs.emplace_back(sol::lua_nil);
                else
                    lArgs.emplace_back(pLua, sol::in_place, mArg);
            }
        }

        // Get a reference to self
        sol::table mSelfLua = mLua[mSelf.get_lua_name()];
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

    define_script_(sScriptName, std::move(mWrappedHandler), bAppend, mInfo);
}

void frame::define_script_(const std::string& sScriptName, script_handler_function mHandler,
    bool bAppend, const script_info& mInfo)
{
    auto& lHandlerList = lScriptHandlerList_[sScriptName];
    if (!bAppend)
    {
        // Just disable existing scripts, it may not be safe to modify the handler list
        // if this script is being defined during a handler execution.
        // They will be deleted later, when we know it is safe.
        for (auto& mPrevHandler : *lHandlerList)
            mPrevHandler.bDisconnected = true;
    }

    if (lHandlerList == nullptr)
        lHandlerList = std::make_shared<std::list<script_handler_slot>>();

    lHandlerList->push_back({std::move(mHandler), false});

    if (!is_virtual())
    {
        bool bNeedsEventName = sScriptName == "OnEvent";

        // Register the function so it can be called directly from Lua
        std::string sAdjustedName = get_adjusted_script_name(sScriptName);

        get_lua_()[get_lua_name()][sAdjustedName] = [=](frame& mSelf, sol::variadic_args mVArgs)
        {
            event mEvent;
            bool bIsFirst = true;
            for (auto&& mArg : mVArgs)
            {
                if (bNeedsEventName && bIsFirst)
                {
                    mEvent.set_name(mArg.as<std::string>());
                }
                else
                {
                    lxgui::utils::variant mVariant;
                    if (!mArg.is<sol::lua_nil_t>())
                        mVariant = mArg;

                    mEvent.add(std::move(mVariant));
                }

                bIsFirst = false;
            }

            mSelf.on_script(sScriptName, &mEvent);
        };
    }
}

frame::script_list_view frame::get_script(const std::string& sScriptName) const
{
    auto iterH = lScriptHandlerList_.find(sScriptName);
    if (iterH == lScriptHandlerList_.end())
        throw gui::exception(lType_.back(), "no script registered for " + sScriptName);

    return script_list_view(*iterH->second);
}

void frame::remove_script(const std::string& sScriptName)
{
    auto iterH = lScriptHandlerList_.find(sScriptName);
    if (iterH == lScriptHandlerList_.end()) return;

    // Just disable existing scripts, it may not be safe to modify the handler list
    // if this script is being defined during a handler execution.
    // They will be deleted later, when we know it is safe.
    for (auto& mHandler : *iterH->second)
        mHandler.bDisconnected = true;

    if (!is_virtual())
    {
        std::string sAdjustedName = get_adjusted_script_name(sScriptName);
        get_lua_()[get_lua_name()][sAdjustedName] = sol::lua_nil;
    }
}

void frame::on_event(const event& mEvent)
{
    alive_checker mChecker(*this);

    if (has_script("OnEvent") &&
        (lRegEventList_.find(mEvent.get_name()) != lRegEventList_.end() || bHasAllEventsRegistred_))
    {
        // ADDON_LOADED should only be fired if it's this frame's addon
        if (mEvent.get_name() == "ADDON_LOADED")
        {
            if (!pAddOn_ || pAddOn_->sName != mEvent.get<std::string>(0))
                return;
        }

        event mTemp = mEvent;
        on_script("OnEvent", &mTemp);
        if (!mChecker.is_alive())
            return;
    }

    if (!get_manager().is_input_enabled())
        return;

    if (bIsMouseEnabled_ && bIsVisible_ && mEvent.get_name().find("MOUSE_") == 0u)
    {
        update_mouse_in_frame_();

        if (mEvent.get_name() == "MOUSE_DRAG_START")
        {
            if (bMouseInTitleRegion_)
                start_moving();

            if (bMouseInFrame_)
            {
                std::string sMouseButton = mEvent.get<std::string>(3);
                if (lRegDragList_.find(sMouseButton) != lRegDragList_.end())
                {
                    bMouseDraggedInFrame_ = true;
                    on_script("OnDragStart");
                    if (!mChecker.is_alive())
                        return;
                }
            }
        }
        else if (mEvent.get_name() == "MOUSE_DRAG_STOP")
        {
            stop_moving();

            if (bMouseDraggedInFrame_)
            {
                bMouseDraggedInFrame_ = false;
                on_script("OnDragStop");
                if (!mChecker.is_alive())
                    return;
            }

            if (bMouseInFrame_)
            {
                std::string sMouseButton = mEvent.get<std::string>(3);
                if (lRegDragList_.find(sMouseButton) != lRegDragList_.end())
                {
                    on_script("OnReceiveDrag");
                    if (!mChecker.is_alive())
                        return;
                }
            }
        }
        else if (mEvent.get_name() == "MOUSE_PRESSED")
        {
            if (bMouseInFrame_)
            {
                if (bIsTopLevel_)
                    raise();

                if (pTopLevelParent_)
                    pTopLevelParent_->raise();

                std::string sMouseButton = mEvent.get<std::string>(3);

                event mEvent2;
                mEvent2.add(sMouseButton);

                on_script("OnMouseDown", &mEvent2);
                if (!mChecker.is_alive())
                    return;
            }
        }
        else if (mEvent.get_name() == "MOUSE_RELEASED")
        {
            if (bMouseInFrame_)
            {
                std::string sMouseButton = mEvent.get<std::string>(3);

                event mEvent2;
                mEvent2.add(sMouseButton);

                on_script("OnMouseUp", &mEvent2);
                if (!mChecker.is_alive())
                    return;
            }
        }
        else if (mEvent.get_name() == "MOUSE_WHEEL")
        {
            if (bMouseInFrame_)
            {
                event mEvent2;
                mEvent2.add(mEvent.get(0));
                on_script("OnMouseWheel", &mEvent2);
                if (!mChecker.is_alive())
                    return;
            }
        }
    }

    if (bIsKeyboardEnabled_ && bIsVisible_)
    {
        if (mEvent.get_name() == "KEY_PRESSED")
        {
            event mKeyEvent;
            mKeyEvent.add((uint)mEvent.get<input::key>(0));
            mKeyEvent.add(mEvent.get(1));

            on_script("OnKeyDown", &mKeyEvent);
            if (!mChecker.is_alive())
                return;
        }
        else if (mEvent.get_name() == "KEY_RELEASED")
        {
            event mKeyEvent;
            mKeyEvent.add((uint)mEvent.get<input::key>(0));
            mKeyEvent.add(mEvent.get(1));

            on_script("OnKeyUp", &mKeyEvent);
            if (!mChecker.is_alive())
                return;
        }
    }
}

void frame::on_script(const std::string& sScriptName, event* pEvent)
{
    auto iterH = lScriptHandlerList_.find(sScriptName);
    if (iterH == lScriptHandlerList_.end())
        return;

    // Make a copy of the manager pointer: in case the frame is deleted, we will need this
    auto& mManager = get_manager();
    auto* pOldAddOn = mManager.get_current_addon();
    mManager.set_current_addon(get_addon());

    try
    {
        // Make a shared-ownership copy of the handler list, so that the list
        // survives even if this frame is destroyed midway during a handler.
        const auto lHandlerList = iterH->second;

        // Call the handlers
        for (const auto& mHandler : *lHandlerList)
        {
            if (!mHandler.bDisconnected)
                mHandler.mCallback(*this, pEvent);
        }
    }
    catch (const std::exception& mException)
    {
        // TODO: add file/line info
        std::string sError = mException.what();

        gui::out << gui::error << sError << std::endl;

        event mEvent("LUA_ERROR");
        mEvent.add(sError);
        mManager.get_event_manager().fire_event(mEvent);
    }

    mManager.set_current_addon(pOldAddOn);
}

void frame::register_all_events()
{
    bHasAllEventsRegistred_ = true;
    lRegEventList_.clear();
}

void frame::register_event(const std::string& sEvent)
{
    if (bHasAllEventsRegistred_)
        return;

    auto mInserted = lRegEventList_.insert(sEvent);

    if (!bVirtual_ && mInserted.second)
        event_receiver::register_event(sEvent);
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

void frame::set_abs_hit_rect_insets(float fLeft, float fRight, float fTop, float fBottom)
{
    lAbsHitRectInsetList_ = bounds2f(fLeft, fRight, fTop, fBottom);
}

void frame::set_abs_hit_rect_insets(const bounds2f& lInsets)
{
    lAbsHitRectInsetList_ = lInsets;
}

void frame::set_rel_hit_rect_insets(float fLeft, float fRight, float fTop, float fBottom)
{
    lRelHitRectInsetList_ = bounds2f(fLeft, fRight, fTop, fBottom);
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

void frame::set_max_resize(float fMaxWidth, float fMaxHeight)
{
    set_max_width(fMaxWidth);
    set_max_height(fMaxHeight);
}

void frame::set_max_resize(const vector2f& mMax)
{
    set_max_width(mMax.x);
    set_max_height(mMax.y);
}

void frame::set_min_resize(float fMinWidth, float fMinHeight)
{
    set_min_width(fMinWidth);
    set_min_height(fMinHeight);
}

void frame::set_min_resize(const vector2f& mMin)
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
        return get_manager().remove_root_frame(pSelf);
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
    if (bIsTopLevel_ == bIsTopLevel)
        return;

    bIsTopLevel_ = bIsTopLevel;

    for (auto& mChild : get_children())
    {
        mChild.notify_top_level_parent_(bIsTopLevel_, observer_from(this));
    }
}

void frame::raise()
{
    if (!bIsTopLevel_)
        return;

    int iOldLevel = iLevel_;
    iLevel_ = get_manager().get_highest_level(mStrata_) + 1;

    if (iLevel_ > iOldLevel)
    {
        if (!is_virtual())
        {
            get_top_level_renderer()->notify_frame_level_changed(
                observer_from(this), iOldLevel, iLevel_);
        }

        int iAmount = iLevel_ - iOldLevel;

        for (auto& mChild : get_children())
            mChild.add_level_(iAmount);
    }
    else
        iLevel_ = iOldLevel;
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
        get_manager().start_moving(observer_from(this));
    }
}

void frame::stop_moving()
{
    if (bIsMovable_)
        get_manager().stop_moving(*this);
}

void frame::start_sizing(const anchor_point& mPoint)
{
    if (bIsResizable_)
    {
        set_user_placed(true);
        get_manager().start_sizing(observer_from(this), mPoint);
    }
}

void frame::stop_sizing()
{
    get_manager().stop_sizing(*this);
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
        return get_manager().observer_from_this();
}

void frame::notify_visible(bool bTriggerEvents)
{
    uiobject::notify_visible(bTriggerEvents);

    for (auto& mRegion : get_regions())
    {
        if (mRegion.is_shown())
            mRegion.notify_visible(bTriggerEvents);
    }

    for (auto& mChild : get_children())
    {
        if (mChild.is_shown())
            mChild.notify_visible(bTriggerEvents);
    }

    if (bTriggerEvents)
    {
        lQueuedEventList_.push_back("OnShow");
        notify_renderer_need_redraw();
    }
}

void frame::notify_invisible(bool bTriggerEvents)
{
    uiobject::notify_invisible(bTriggerEvents);

    for (auto& mChild : get_children())
    {
        if (mChild.is_shown())
            mChild.notify_invisible(bTriggerEvents);
    }

    if (bTriggerEvents)
    {
        lQueuedEventList_.push_back("OnHide");
        notify_renderer_need_redraw();
    }
}

void frame::notify_top_level_parent_(bool bTopLevel, const utils::observer_ptr<frame>& pParent)
{
    if (bTopLevel)
        pTopLevelParent_ = pParent;
    else
        pTopLevelParent_ = nullptr;

    for (auto& mChild : get_children())
        mChild.notify_top_level_parent_(bTopLevel, pParent);
}

void frame::notify_renderer_need_redraw() const
{
    if (bVirtual_)
        return;

    get_top_level_renderer()->notify_strata_needs_redraw(mStrata_);
}

void frame::show()
{
    if (bIsShown_)
        return;

    bool bWasVisible_ = bIsVisible_;
    uiobject::show();

    if (!bWasVisible_)
    {
        get_manager().notify_hovered_frame_dirty();
        update_mouse_in_frame_();
    }
}

void frame::hide()
{
    if (!bIsShown_)
        return;

    bool bWasVisible_ = bIsVisible_;
    uiobject::hide();

    if (bWasVisible_)
    {
        get_manager().notify_hovered_frame_dirty();
        update_mouse_in_frame_();
    }
}

void frame::set_shown(bool bIsShown)
{
    if (bIsShown_ == bIsShown)
        return;

    bIsShown_ = bIsShown;

    if (!bIsShown_)
        notify_invisible(false);
}

void frame::unregister_all_events()
{
    bHasAllEventsRegistred_ = false;
    lRegEventList_.clear();
}

void frame::unregister_event(const std::string& sEvent)
{
    if (bHasAllEventsRegistred_)
        return;

    auto mIter = lRegEventList_.find(sEvent);
    if (mIter == lRegEventList_.end())
        return;

    lRegEventList_.erase(sEvent);

    if (!bVirtual_)
        event_receiver::unregister_event(sEvent);
}

void frame::set_addon(const addon* pAddOn)
{
    if (!pAddOn_)
    {
        pAddOn_ = pAddOn;
        for (auto& mChild : get_children())
            mChild.set_addon(pAddOn);
    }
    else
        gui::out << gui::warning << "gui::" << lType_.back() << " : set_addon() can only be called once." << std::endl;
}

const addon* frame::get_addon() const
{
    if (!pAddOn_ && pParent_)
        return pParent_->get_addon();
    else
        return pAddOn_;
}

void frame::notify_mouse_in_frame(bool bMouseInframe, float fX, float fY)
{
    alive_checker mChecker(*this);

    if (bMouseInframe)
    {
        if (!bMouseInFrame_)
        {
            on_script("OnEnter");
            if (!mChecker.is_alive())
                return;
        }

        bMouseInFrame_ = true;

        fMousePosX_ = fX;
        fMousePosY_ = fY;

        bMouseInTitleRegion_ = (pTitleRegion_ && pTitleRegion_->is_in_region(fX, fY));
    }
    else
    {
        if (bMouseInFrame_)
        {
            on_script("OnLeave");
            if (!mChecker.is_alive())
                return;
        }

        bMouseInTitleRegion_ = false;
        bMouseInFrame_ = false;
    }
}

void frame::update_borders_() const
{
    bool bPositionUpdated = bUpdateBorders_;
    uiobject::update_borders_();

    if (bPositionUpdated)
    {
        check_position();
        get_manager().notify_hovered_frame_dirty();
        if (pBackdrop_)
            pBackdrop_->notify_borders_updated();
    }
}

void frame::update_mouse_in_frame_()
{
    update_borders_();
    get_manager().get_hovered_frame();
}

void frame::update(float fDelta)
{
    //#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    alive_checker mChecker(*this);

    DEBUG_LOG("  ~");
    uiobject::update(fDelta);
    DEBUG_LOG("   #");

    for (const auto& sEvent : lQueuedEventList_)
    {
        DEBUG_LOG("   Event " + *iterEvent);
        on_script(sEvent);
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
                lLayerList_[static_cast<uint>(pRegion->get_draw_layer())].lRegionList.push_back(pRegion);
        }

        for (const auto& pRegion : lRegionList_)
        {
            if (pRegion && pRegion->get_object_type() == "font_string")
                lLayerList_[static_cast<uint>(pRegion->get_draw_layer())].lRegionList.push_back(pRegion);
        }

        bBuildLayerList_ = false;
    }

    if (is_visible())
    {
        DEBUG_LOG("   On update");
        event mEvent;
        mEvent.add(fDelta);
        on_script("OnUpdate", &mEvent);
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

    // Remove disabled handlers
    for (auto mIterList = lScriptHandlerList_.begin(); mIterList != lScriptHandlerList_.end(); ++mIterList)
    {
        auto& lHandlerList = *mIterList->second;
        auto mIterRemove = std::remove_if(lHandlerList.begin(), lHandlerList.end(),
            [](const auto& mHandler) { return mHandler.bDisconnected; });

        if (mIterRemove == lHandlerList.begin())
            mIterList = lScriptHandlerList_.erase(mIterList);
        else
            lHandlerList.erase(mIterRemove, lHandlerList.end());
    }

    float fNewWidth = get_apparent_width();
    float fNewHeight = get_apparent_height();
    if (fOldWidth_ != fNewWidth || fOldHeight_ != fNewHeight)
    {
        DEBUG_LOG("   On size changed");
        on_script("OnSizeChanged");
        if (!mChecker.is_alive())
            return;

        fOldWidth_  = fNewWidth;
        fOldHeight_ = fNewHeight;
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
