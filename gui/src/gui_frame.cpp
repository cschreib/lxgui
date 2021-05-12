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

#include <lxgui/luapp_exception.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_std.hpp>
#include <lxgui/utils_range.hpp>

#include <sol/state.hpp>
#include <sol/as_args.hpp>

#include <sstream>
#include <functional>

namespace lxgui {
namespace gui
{
int l_xml_error(lua_State*);

layer::layer() : bDisabled(false)
{
}

frame::frame(manager* pManager) : event_receiver(pManager->get_event_manager()), region(pManager)
{
    lType_.push_back(CLASS_NAME);
}

frame::~frame()
{
    // Children must be destroyed first
    lChildList_.clear();
    lRegionList_.clear();

    if (!bVirtual_)
    {
        // Tell the renderer to no longer render this widget
        get_top_level_renderer()->notify_rendered_frame(this, false);
        pRenderer_ = nullptr;
    }

    // Unregister this frame from the GUI manager
    pManager_->remove_frame(this);
}

void frame::render()
{
    if (bIsVisible_ && bReady_)
    {
        if (pBackdrop_)
            pBackdrop_->render();

        // Render child regions
        for (auto& mLayer : utils::range::value(lLayerList_))
        {
            if (mLayer.bDisabled) continue;

            for (auto* pRegion : mLayer.lRegionList)
            {
                if (pRegion->is_shown() && !pRegion->is_newly_created())
                    pRegion->render();
            }
        }
    }
}

void frame::create_glue()
{
    create_glue_<lua_frame>();
}

std::string frame::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;

    sStr << region::serialize(sTab);
    if (pRenderer_ && pRenderer_ != pManager_)
    sStr << sTab << "  # Man. render : " << dynamic_cast<frame*>(pRenderer_)->get_name() << "\n";
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
        const quad2f& lInsets = pBackdrop_->get_background_insets();

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

        for (auto* pRegion : get_regions())
        {
            sStr << pRegion->serialize(sTab+"  | ");
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

        for (const auto* pChild : get_children())
        {
            sStr << pChild->serialize(sTab+"  | ");
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

void frame::copy_from(uiobject* pObj)
{
    uiobject::copy_from(pObj);

    frame* pFrame = down_cast<frame>(pObj);
    if (!pFrame)
        return;

    for (const auto& mScript : pFrame->lDefinedScriptList_)
    {
        if (mScript.second.empty()) continue;

        const script_info& mInfo = pFrame->lXMLScriptInfoList_[mScript.first];
        this->define_script(
            mScript.first, mScript.second,
            mInfo.sFile, mInfo.uiLineNbr
        );
    }

    for (const auto& mHandler : pFrame->lDefinedHandlerList_)
    {
        if (mHandler.second)
            this->define_script(mHandler.first, mHandler.second);
    }

    this->set_frame_strata(pFrame->get_frame_strata());

    frame* pHighParent = this;
    for (int i = 0; i < pFrame->get_level(); ++i)
    {
        if (pHighParent->get_parent())
            pHighParent = pHighParent->get_parent();
        else
            break;
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

    for (auto* pArt : pFrame->get_regions())
    {
        if (pArt->is_special()) continue;

        std::unique_ptr<layered_region> pNewArt = pManager_->create_layered_region(pArt->get_object_type());
        if (!pNewArt) continue;

        if (this->is_virtual())
            pNewArt->set_virtual();

        pNewArt->set_name_and_parent(pArt->get_raw_name(), this);
        if (!pManager_->add_uiobject(pNewArt.get()))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                << "Trying to add \"" << pArt->get_name() << "\" to \"" << sName_
                << "\", but its name was already taken : \"" << pNewArt->get_name()
                << "\". Skipped." << std::endl;

            continue;
        }

        if (!pNewArt->is_virtual())
            pNewArt->create_glue();

        pNewArt->set_draw_layer(pArt->get_draw_layer());

        auto* pAddedArt = this->add_region(std::move(pNewArt));
        pAddedArt->copy_from(pArt);
        pAddedArt->notify_loaded();
    }

    bBuildLayerList_ = true;

    if (pFrame->pBackdrop_)
    {
        pBackdrop_ = std::unique_ptr<backdrop>(new backdrop(this));
        pBackdrop_->copy_from(*pFrame->pBackdrop_);
    }

    if (pFrame->pTitleRegion_)
    {
        this->create_title_region();
        if (pTitleRegion_)
            pTitleRegion_->copy_from(pFrame->pTitleRegion_.get());
    }

    for (auto* pChild : pFrame->get_children())
    {
        if (pChild->is_special()) continue;

        frame* pNewChild = create_child(pChild->get_object_type(), pChild->get_raw_name(), {pChild});
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

    pTitleRegion_ = std::unique_ptr<region>(new region(pManager_));
    if (this->is_virtual())
        pTitleRegion_->set_virtual();
    pTitleRegion_->set_special();
    pTitleRegion_->set_name_and_parent(sName_+"TitleRegion", this);

    if (!pManager_->add_uiobject(pTitleRegion_.get()))
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Cannot create \"" << sName_ << "\"'s title region because another uiobject "
            "already took its name : \"" << pTitleRegion_->get_name() << "\"." << std::endl;

        pTitleRegion_ = nullptr;
        return;
    }

    if (!pTitleRegion_->is_virtual())
        pTitleRegion_->create_glue();

    pTitleRegion_->notify_loaded();
}

frame* frame::get_child(const std::string& sName)
{
    for (auto* pChild : get_children())
    {
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

layered_region* frame::get_region(const std::string& sName)
{
    for (auto* pRegion : get_regions())
    {
        if (pRegion->get_name() == sName)
            return pRegion;
    }

    for (auto* pRegion : get_regions())
    {
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
        fAbsWidth_ = fMinWidth_;
    }
    else if (lBorderList_.right - lBorderList_.left > fMaxWidth_)
    {
        lBorderList_.right = lBorderList_.left + fMaxWidth_;
        fAbsWidth_ = fMaxWidth_;
    }

    if (lBorderList_.bottom - lBorderList_.top < fMinHeight_)
    {
        lBorderList_.bottom = lBorderList_.top + fMinHeight_;
        fAbsHeight_ = fMinHeight_;
    }
    else if (lBorderList_.bottom - lBorderList_.top > fMaxHeight_)
    {
        lBorderList_.bottom = lBorderList_.top + fMaxHeight_;
        fAbsHeight_ = fMaxHeight_;
    }

    if (bIsClampedToScreen_)
    {
        float fScreenW = get_top_level_renderer()->get_target_width();
        float fScreenH = get_top_level_renderer()->get_target_height();

        if (lBorderList_.right > fScreenW)
        {
            if (lBorderList_.right - lBorderList_.left > fScreenW)
            {
                lBorderList_.left = 0;
                lBorderList_.right = fScreenW;
            }
            else
            {
                lBorderList_.right = fScreenW;
                lBorderList_.left = fScreenW - fAbsWidth_;
            }
        }
        if (lBorderList_.left < 0)
        {
            if (lBorderList_.right - lBorderList_.left > fScreenW)
            {
                lBorderList_.left = 0;
                lBorderList_.right = fScreenW;
            }
            else
            {
                lBorderList_.left = 0;
                lBorderList_.right = fAbsWidth_;
            }
        }

        if (lBorderList_.bottom > fScreenH)
        {
            if (lBorderList_.bottom - lBorderList_.top > fScreenH)
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = fScreenH;
            }
            else
            {
                lBorderList_.bottom = fScreenH;
                lBorderList_.top = fScreenH - fAbsHeight_;
            }
        }
        if (lBorderList_.top < 0)
        {
            if (lBorderList_.bottom - lBorderList_.top > fScreenH)
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = fScreenH;
            }
            else
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = fAbsHeight_;
            }
        }
    }
}

void frame::disable_draw_layer(layer_type mLayerID)
{
    layer& mLayer = lLayerList_[mLayerID];
    if (!mLayer.bDisabled)
    {
        mLayer.bDisabled = true;
        notify_renderer_need_redraw();
    }
}

void frame::enable_draw_layer(layer_type mLayerID)
{
    layer& mLayer = lLayerList_[mLayerID];
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
        alive_checker mChecker(this);
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
    return lDefinedScriptList_.find(sScriptName) != lDefinedScriptList_.end() ||
           lDefinedHandlerList_.find(sScriptName) != lDefinedHandlerList_.end();
}

layered_region* frame::add_region(std::unique_ptr<layered_region> pRegion)
{
    if (!pRegion)
        return nullptr;

    auto mIter = utils::find_if(lRegionList_, [&](auto& pObj) {
        return pObj->get_id() == pRegion->get_id();
    });

    if (mIter != lRegionList_.end())
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Trying to add \"" << pRegion->get_name() << "\" to \"" << sName_ << "\"'s children, "
            "but it was already one of this frame's children." << std::endl;
        return nullptr;
    }

    layered_region* pAddedRegion = pRegion.get();
    lRegionList_.push_back(std::move(pRegion));

    notify_layers_need_update();
    notify_renderer_need_redraw();

    if (!bVirtual_)
    {
        std::string sRawName = pAddedRegion->get_raw_name();
        if (utils::starts_with(sRawName, "$parent"))
        {
            sRawName.erase(0, std::string("$parent").size());
            pManager_->get_lua().script(get_lua_name()+"."+sRawName+"="+pAddedRegion->get_lua_name());
        }
    }

    return pAddedRegion;
}

std::unique_ptr<layered_region> frame::remove_region(layered_region* pRegion)
{
    if (!pRegion)
        return nullptr;

    auto mIter = utils::find_if(lRegionList_, [&](auto& pObj) {
        return pObj->get_id() == pRegion->get_id();
    });

    if (mIter == lRegionList_.end())
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Trying to remove \"" << pRegion->get_name() << "\" from \"" << sName_ << "\"'s children, "
            "but it was not one of this frame's children." << std::endl;
        return nullptr;
    }

    // NB: the iterator is not removed yet; it will be removed later in update().
    std::unique_ptr<layered_region> pRemovedRegion = std::move(*mIter);

    notify_layers_need_update();
    notify_renderer_need_redraw();
    pRemovedRegion->set_parent(nullptr);

    return pRemovedRegion;
}

layered_region* frame::create_region(layer_type mLayer, const std::string& sClassName,
    const std::string& sName, const std::vector<uiobject*>& lInheritance)
{
    std::unique_ptr<layered_region> pRegion = pManager_->create_layered_region(sClassName);
    if (!pRegion)
        return nullptr;

    pRegion->set_draw_layer(mLayer);
    pRegion->set_name_and_parent(sName, this);

    if (!pManager_->add_uiobject(pRegion.get()))
        return nullptr;

    pRegion->create_glue();

    for (auto* pObj : lInheritance)
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
        pRegion->copy_from(pObj);
    }

    return add_region(std::move(pRegion));
}

frame* frame::create_child(const std::string& sClassName, const std::string& sName,
    const std::vector<uiobject*>& lInheritance)
{
    if (!pManager_->check_uiobject_name(sName))
        return nullptr;

    std::unique_ptr<frame> pNewFrame = pManager_->create_frame(sClassName);
    if (!pNewFrame)
        return nullptr;

    pNewFrame->set_name_and_parent(sName, this);

    if (this->is_virtual())
        pNewFrame->set_virtual();

    if (!pNewFrame->is_virtual())
        get_top_level_renderer()->notify_rendered_frame(pNewFrame.get(), true);

    pNewFrame->set_level(get_level() + 1);

    if (!pManager_->add_uiobject(pNewFrame.get()))
        return nullptr;

    pNewFrame->create_glue();

    for (auto* pObj : lInheritance)
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
        pNewFrame->copy_from(pObj);
    }

    pNewFrame->set_newly_created();

    return add_child(std::move(pNewFrame));
}

frame* frame::add_child(std::unique_ptr<frame> pChild)
{
    if (!pChild)
        return nullptr;

    auto mIter = utils::find_if(lChildList_, [&](auto& pObj) {
        return pObj && pObj->get_id() == pChild->get_id();
    });

    if (mIter != lChildList_.end())
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Trying to add \"" << pChild->get_name() << "\" to \"" << sName_
            << "\"'s children, but it was already one of this frame's children." << std::endl;
        return nullptr;
    }

    if (bIsTopLevel_)
        pChild->notify_top_level_parent_(true, this);

    if (pTopLevelParent_)
        pChild->notify_top_level_parent_(true, pTopLevelParent_);

    if (is_visible() && pChild->is_shown())
        pChild->notify_visible(!pManager_->is_loading_ui());
    else
        pChild->notify_invisible(!pManager_->is_loading_ui());

    frame* pAddedChild = pChild.get();
    lChildList_.push_back(std::move(pChild));

    if (!bVirtual_)
    {
        frame_renderer* pOldTopLevelRenderer = pAddedChild->get_top_level_renderer();
        frame_renderer* pNewTopLevelRenderer = get_top_level_renderer();
        if (pOldTopLevelRenderer != pNewTopLevelRenderer)
        {
            pOldTopLevelRenderer->notify_rendered_frame(pAddedChild, false);
            pNewTopLevelRenderer->notify_rendered_frame(pAddedChild, true);
        }

        std::string sRawName = pAddedChild->get_raw_name();
        if (utils::starts_with(sRawName, "$parent"))
        {
            sRawName.erase(0, std::string("$parent").size());
            pManager_->get_lua().script(get_lua_name()+"."+sRawName+" = "+pAddedChild->get_lua_name());
        }
    }

    return pAddedChild;
}

std::unique_ptr<frame> frame::remove_child(frame* pChild)
{
    if (!pChild)
        return nullptr;

    auto mIter = utils::find_if(lChildList_, [&](auto& pObj) {
        return pObj && pObj->get_id() == pChild->get_id();
    });

    if (mIter == lChildList_.end())
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Trying to remove \"" << pChild->get_name() << "\" from \"" << sName_
            << "\"'s children, but it was not one of this frame's children." << std::endl;
        return nullptr;
    }

    // NB: the iterator is not removed yet; it will be removed later in update().
    std::unique_ptr<frame> pRemovedChild = std::move(*mIter);

    frame_renderer* pTopLevelRenderer = get_top_level_renderer();
    bool bNotifyRenderer = !pChild->get_renderer() && pTopLevelRenderer != pManager_;
    if (bNotifyRenderer)
    {
        pTopLevelRenderer->notify_rendered_frame(pChild, false);
        pChild->propagate_renderer_(false);
    }

    pRemovedChild->set_parent(nullptr);

    if (bNotifyRenderer)
    {
        pManager_->notify_rendered_frame(pChild, true);
        pChild->propagate_renderer_(true);
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

const std::string& frame::get_frame_type() const
{
    return lType_.back();
}

const quad2f& frame::get_abs_hit_rect_insets() const
{
    return lAbsHitRectInsetList_;
}

const quad2f& frame::get_rel_hit_rect_insets() const
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
    return lChildList_.size();
}

uint frame::get_num_regions() const
{
    return lRegionList_.size();
}

float frame::get_scale() const
{
    return fScale_;
}

region* frame::get_title_region() const
{
    return pTitleRegion_.get();
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

void frame::define_script(const std::string& sScriptName, const std::string& sContent, const std::string& sFile, uint uiLineNbr)
{
    std::map<std::string, handler>::iterator iterH = lDefinedHandlerList_.find(sScriptName);
    if (iterH != lDefinedHandlerList_.end())
        lDefinedHandlerList_.erase(iterH);

    std::string sAdjustedName = get_adjusted_script_name(sScriptName);

    std::string sStr;
    sStr += "function " + sLuaName_ + ":" + sAdjustedName +
        "(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) " + sContent + " end";

    // Actually register the function
    try
    {
        pManager_->get_lua().script(sStr, sol::script_default_on_error);
        lDefinedScriptList_[sScriptName] = sContent;
        lXMLScriptInfoList_[sScriptName].sFile = sFile;
        lXMLScriptInfoList_[sScriptName].uiLineNbr = uiLineNbr;
    }
    catch (const sol::error& mError)
    {
        // TODO: show file/line number from lXMLScriptInfoList_
        std::string sError = mError.what();
        gui::out << gui::error << sError << std::endl;

        event mEvent("LUA_ERROR");
        mEvent.add(sError);
        pManager_->get_event_manager()->fire_event(mEvent);
    }
}

void frame::define_script(const std::string& sScriptName, handler mHandler)
{
    std::map<std::string, std::string>::iterator iter = lDefinedScriptList_.find(sScriptName);
    if (iter != lDefinedScriptList_.end())
        lDefinedScriptList_.erase(iter);

    lDefinedHandlerList_[sScriptName] = mHandler;
}

void frame::notify_script_defined(const std::string& sScriptName, bool bDefined)
{
    std::map<std::string, script_info>::iterator iter = lXMLScriptInfoList_.find(sScriptName);
    if (iter != lXMLScriptInfoList_.end())
        lXMLScriptInfoList_.erase(iter);

    if (bDefined)
    {
        std::map<std::string, handler>::iterator iter2 = lDefinedHandlerList_.find(sScriptName);
        if (iter2 != lDefinedHandlerList_.end())
            lDefinedHandlerList_.erase(iter2);

        lDefinedScriptList_[sScriptName] = "";
    } else
        lDefinedScriptList_.erase(sScriptName);
}

void frame::on_event(const event& mEvent)
{
    alive_checker mChecker(this);

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

    if (!pManager_->is_input_enabled())
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

int l_xml_error(lua_State* pLua)
{
    if (!lua_isstring(pLua, -1))
    return 0;

    lua_Debug d;

    lua_getstack(pLua, 1, &d);
    lua_getinfo(pLua, "Sl", &d);

    if (d.short_src[0] == '[')
    {
        lua_getglobal(pLua, "_xml_file_name");
        std::string sFile = lua_tostring(pLua, -1);
        lua_pop(pLua, 1);
        lua_getglobal(pLua, "_xml_line_nbr");
        uint uiLineNbr = lua_tonumber(pLua, -1);
        lua_pop(pLua, 1);

        std::string sError = sFile + ":" + utils::to_string(uiLineNbr + d.currentline - 1) + ": "
            + std::string(lua_tostring(pLua, -1));

        lua_pushstring(pLua, sError.c_str());
    }
    else
    {
        std::string sError = std::string(d.short_src) + ":" + utils::to_string(int(d.currentline)) + ": "
            + std::string(lua_tostring(pLua, -1));

        lua_pushstring(pLua, sError.c_str());
    }

    return 1;
}

void frame::on_script(const std::string& sScriptName, event* pEvent)
{
    std::map<std::string, handler>::const_iterator iterH = lDefinedHandlerList_.find(sScriptName);
    if (iterH != lDefinedHandlerList_.end())
    {
        if (iterH->second)
            iterH->second(this, pEvent);
    }

    std::map<std::string, std::string>::const_iterator iter = lDefinedScriptList_.find(sScriptName);
    if (iter == lDefinedScriptList_.end())
        return;

    sol::state& mLua = pManager_->get_lua();
    lua_State* pLua = mLua.lua_state();

    std::vector<sol::object> lArgs;

    if (pEvent)
    {
        if (sScriptName == "OnEvent")
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

    std::string sAdjustedName = get_adjusted_script_name(sScriptName);

    auto* pManager = pManager_; // make a copy in case the frame is deleted, we will need this
    auto* pOldAddOn = pManager->get_current_addon();
    pManager->set_current_addon(pAddOn_);

    sol::table mSelf = mLua[sLuaName_];
    if (mSelf == sol::lua_nil)
    {
        std::string sError = "Lua glue object is nil";
        gui::out << gui::error << sError << std::endl;

        event mEvent("LUA_ERROR");
        mEvent.add(sError);
        pManager->get_event_manager()->fire_event(mEvent);
        pManager->set_current_addon(pOldAddOn);
        return;
    }

    sol::protected_function mCallback = mSelf[sAdjustedName];
    if (mCallback == sol::lua_nil)
    {
        std::string sError = "Lua callback "+sAdjustedName+" is nil";
        gui::out << gui::error << sError << std::endl;

        event mEvent("LUA_ERROR");
        mEvent.add(sError);
        pManager->get_event_manager()->fire_event(mEvent);
        pManager->set_current_addon(pOldAddOn);

        return;
    }

    auto mResult = mCallback(mSelf, sol::as_args(lArgs));
    // WARNING: after this point, the frame (this) may be deleted.
    // Do not use any member variable or member function directly.

    if (!mResult.valid())
    {
        sol::error mError = mResult;

        // TODO: show file/line number from lXMLScriptInfoList_
        std::string sError = mError.what();

        gui::out << gui::error << sError << std::endl;

        event mEvent("LUA_ERROR");
        mEvent.add(sError);
        pManager->get_event_manager()->fire_event(mEvent);
    }

    pManager->set_current_addon(pOldAddOn);
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
        get_top_level_renderer()->notify_frame_strata_changed(this, mStrata, mStrata_);
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
    lAbsHitRectInsetList_ = quad2f(fLeft, fRight, fTop, fBottom);
}

void frame::set_abs_hit_rect_insets(const quad2f& lInsets)
{
    lAbsHitRectInsetList_ = lInsets;
}

void frame::set_rel_hit_rect_insets(float fLeft, float fRight, float fTop, float fBottom)
{
    lRelHitRectInsetList_ = quad2f(fLeft, fRight, fTop, fBottom);
}

void frame::set_rel_hit_rect_insets(const quad2f& lInsets)
{
    lRelHitRectInsetList_ = lInsets;
}

void frame::set_level(int iLevel)
{
    if (iLevel != iLevel_)
    {
        std::swap(iLevel, iLevel_);

        if (!bVirtual_)
            get_top_level_renderer()->notify_frame_level_changed(this, iLevel, iLevel_);
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

std::unique_ptr<uiobject> frame::release_from_parent()
{
    if (pParent_)
        return pParent_->remove_child(this);
    else
        return pManager_->remove_root_frame(this);
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

    for (auto* pChild : get_children())
        pChild->notify_top_level_parent_(bIsTopLevel_, this);
}

void frame::raise()
{
    if (!bIsTopLevel_)
        return;

    int iOldLevel = iLevel_;
    iLevel_ = pManager_->get_highest_level(mStrata_) + 1;

    if (iLevel_ > iOldLevel)
    {
        if (!is_virtual())
            get_top_level_renderer()->notify_frame_level_changed(this, iOldLevel, iLevel_);

        int iAmount = iLevel_ - iOldLevel;

        for (auto* pChild : get_children())
            pChild->add_level_(iAmount);
    }
    else
        iLevel_ = iOldLevel;
}

void frame::add_level_(int iAmount)
{
    int iOldLevel = iLevel_;
    iLevel_ += iAmount;

    if (!is_virtual())
        get_top_level_renderer()->notify_frame_level_changed(this, iOldLevel, iLevel_);

    for (auto* pChild : get_children())
        pChild->add_level_(iAmount);
}

void frame::set_user_placed(bool bIsUserPlaced)
{
    bIsUserPlaced_ = bIsUserPlaced;
}

void frame::start_moving()
{
    if (bIsMovable_)
        pManager_->start_moving(this);
}

void frame::stop_moving()
{
    if (bIsMovable_)
        pManager_->stop_moving(this);
}

void frame::start_sizing(const anchor_point& mPoint)
{
    if (bIsResizable_)
        pManager_->start_sizing(this, mPoint);
}

void frame::stop_sizing()
{
    pManager_->stop_sizing(this);
}

void frame::propagate_renderer_(bool bRendered)
{
    auto* pTopLevelRenderer = get_top_level_renderer();
    for (auto* pChild : get_children())
    {
        if (!pChild->get_renderer())
            pTopLevelRenderer->notify_rendered_frame(pChild, bRendered);

        pChild->propagate_renderer_(bRendered);
    }
}

void frame::set_renderer(frame_renderer* pRenderer)
{
    if (pRenderer == pRenderer_)
        return;

    get_top_level_renderer()->notify_rendered_frame(this, false);
    propagate_renderer_(false);

    pRenderer_ = pRenderer;

    get_top_level_renderer()->notify_rendered_frame(this, true);
    propagate_renderer_(true);
}

const frame_renderer* frame::get_renderer() const
{
    return pRenderer_;
}

frame_renderer* frame::get_top_level_renderer()
{
    if (pRenderer_)
        return pRenderer_;
    else if (pParent_)
        return pParent_->get_top_level_renderer();
    else
        return pManager_;
}

const frame_renderer* frame::get_top_level_renderer() const
{
    if (pRenderer_)
        return pRenderer_;
    else if (pParent_)
        return pParent_->get_top_level_renderer();
    else
        return pManager_;
}

void frame::notify_visible(bool bTriggerEvents)
{
    uiobject::notify_visible(bTriggerEvents);

    for (auto* pRegion : get_regions())
    {
        if (pRegion->is_shown())
            pRegion->notify_visible(bTriggerEvents);
    }

    for (auto* pChild : get_children())
    {
        if (pChild->is_shown())
            pChild->notify_visible(bTriggerEvents);
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

    for (auto* pChild : get_children())
    {
        if (pChild->is_shown())
            pChild->notify_invisible(bTriggerEvents);
    }

    if (bTriggerEvents)
    {
        lQueuedEventList_.push_back("OnHide");
        notify_renderer_need_redraw();
    }
}

void frame::notify_top_level_parent_(bool bTopLevel, frame* pParent)
{
    if (bTopLevel)
        pTopLevelParent_ = pParent;
    else
        pTopLevelParent_ = nullptr;

    for (auto* pChild : get_children())
        pChild->notify_top_level_parent_(bTopLevel, pParent);
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
        pManager_->notify_hovered_frame_dirty();
}

void frame::hide()
{
    if (!bIsShown_)
        return;

    bool bWasVisible_ = bIsVisible_;
    uiobject::hide();

    if (bWasVisible_)
        pManager_->notify_hovered_frame_dirty();
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

void frame::set_addon(addon* pAddOn)
{
    if (!pAddOn_)
    {
        pAddOn_ = pAddOn;
        for (auto* pChild : get_children())
            pChild->set_addon(pAddOn);
    }
    else
        gui::out << gui::warning << "gui::" << lType_.back() << " : set_addon() can only be called once." << std::endl;
}

addon* frame::get_addon() const
{
    if (!pAddOn_ && pParent_)
        return pParent_->get_addon();
    else
        return pAddOn_;
}

void frame::notify_mouse_in_frame(bool bMouseInframe, float fX, float fY)
{
    alive_checker mChecker(this);

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
        pManager_->notify_hovered_frame_dirty();
    }
}

void frame::update_mouse_in_frame_()
{
    update_borders_();
    pManager_->get_hovered_frame();
}

void frame::update(float fDelta)
{
    //#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    alive_checker mChecker(this);

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
        for (auto& mLayer : utils::range::value(lLayerList_))
            mLayer.lRegionList.clear();

        // Fill layers with regions (with font_string rendered last within the same layer)
        for (auto* pRegion : get_regions())
        {
            if (pRegion->get_object_type() != "font_string")
                lLayerList_[pRegion->get_draw_layer()].lRegionList.push_back(pRegion);
        }

        for (auto* pRegion : get_regions())
        {
            if (pRegion->get_object_type() == "font_string")
                lLayerList_[pRegion->get_draw_layer()].lRegionList.push_back(pRegion);
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
    for (auto* pRegion : get_regions())
        pRegion->update(fDelta);

    // Remove deleted regions
    {
        auto mIterRemove = std::remove_if(lRegionList_.begin(), lRegionList_.end(), [](auto& pObj) {
            return pObj == nullptr;
        });

        lRegionList_.erase(mIterRemove, lRegionList_.end());
    }

    // Update children
    DEBUG_LOG("   Update children");
    for (auto* pChild : get_children())
    {
        pChild->update(fDelta);
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

    if (fOldWidth_ != fAbsWidth_ || fOldHeight_ != fAbsHeight_)
    {
        DEBUG_LOG("   On size changed");
        on_script("OnSizeChanged");
        if (!mChecker.is_alive())
            return;

        fOldWidth_  = fAbsWidth_;
        fOldHeight_ = fAbsHeight_;
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
        gui::out << gui::warning << "layer : Uknown layer type : \""
            << sLayer << "\". Using \"ARTWORK\"." << std::endl;

        return layer_type::ARTWORK;
    }
}
}
}
