#include "lxgui/gui_frame.hpp"

#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

#include <lxgui/luapp_exception.hpp>
#include <lxgui/utils_string.hpp>
#include <sol/state.hpp>

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
    clear_links_();
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
    sStr << sTab << "  # Min width   : " << uiMinWidth_ << "\n";
    sStr << sTab << "  # Max width   : " << uiMaxWidth_ << "\n";
    sStr << sTab << "  # Min height  : " << uiMinHeight_ << "\n";
    sStr << sTab << "  # Max height  : " << uiMaxHeight_ << "\n";
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
        const quad2i& lInsets = pBackdrop_->get_background_insets();

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

    frame* pFrame = pObj->down_cast<frame>();
    if (!pFrame)
        return;

    for (const auto& mScript : pFrame->lDefinedScriptList_)
    {
        if (mScript.second.empty()) continue;

        const script_info& mInfo = pFrame->lXMLScriptInfoList_[mScript.first];
        this->define_script(
            "On"+mScript.first, mScript.second,
            mInfo.sFile, mInfo.uiLineNbr
        );
    }

    for (const auto& mHandler : pFrame->lDefinedHandlerList_)
    {
        if (mHandler.second)
            this->define_script("On"+mHandler.first, mHandler.second);
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

    for (auto* pChild : pFrame->get_children())
    {
        if (pChild->is_special()) continue;

        create_child(pChild->get_object_type(), pChild->get_raw_name(), {pChild});
    }

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

    for (auto* pArt : pFrame->get_regions())
    {
        if (pArt->is_special()) continue;

        std::unique_ptr<layered_region> pNewArt = pManager_->create_layered_region(pArt->get_object_type());
        if (!pNewArt) continue;

        pNewArt->set_parent(this);
        if (this->is_virtual())
            pNewArt->set_virtual();

        pNewArt->set_name(pArt->get_raw_name());
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
    pTitleRegion_->set_parent(this);
    pTitleRegion_->set_name(sName_+"TitleRegion");

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

void frame::set_abs_dimensions(uint uiAbsWidth, uint uiAbsHeight)
{
    uiobject::set_abs_dimensions(
        std::min(std::max(uiAbsWidth,  uiMinWidth_),  uiMaxWidth_),
        std::min(std::max(uiAbsHeight, uiMinHeight_), uiMaxHeight_)
    );
}

void frame::set_abs_width(uint uiAbsWidth)
{
    uiobject::set_abs_width(std::min(std::max(uiAbsWidth, uiMinWidth_), uiMaxWidth_));
}

void frame::set_abs_height(uint uiAbsHeight)
{
    uiobject::set_abs_height(std::min(std::max(uiAbsHeight, uiMinHeight_), uiMaxHeight_));
}

void frame::check_position() const
{
    if (lBorderList_.right - lBorderList_.left < int(uiMinWidth_))
    {
        lBorderList_.right = lBorderList_.left + uiMinWidth_;
        uiAbsWidth_ = uiMinWidth_;
    }
    else if (uint(lBorderList_.right - lBorderList_.left) > uiMaxWidth_)
    {
        lBorderList_.right = lBorderList_.left + uiMaxWidth_;
        uiAbsWidth_ = uiMaxWidth_;
    }

    if (lBorderList_.bottom - lBorderList_.top < int(uiMinHeight_))
    {
        lBorderList_.bottom = lBorderList_.top + uiMinHeight_;
        uiAbsHeight_ = uiMinHeight_;
    }
    else if (uint(lBorderList_.bottom - lBorderList_.top) > uiMaxHeight_)
    {
        lBorderList_.bottom = lBorderList_.top + uiMaxHeight_;
        uiAbsHeight_ = uiMaxHeight_;
    }

    if (bIsClampedToScreen_)
    {
        uint uiScreenW = pManager_->get_screen_width();
        uint uiScreenH = pManager_->get_screen_height();

        if (lBorderList_.right > int(uiScreenW))
        {
            if (uint(lBorderList_.right - lBorderList_.left) > uiScreenW)
            {
                lBorderList_.left = 0;
                lBorderList_.right = uiScreenW;
            }
            else
            {
                lBorderList_.right = uiScreenW;
                lBorderList_.left = uiScreenW - uiAbsWidth_;
            }
        }
        if (lBorderList_.left < 0)
        {
            if (uint(lBorderList_.right - lBorderList_.left) > uiScreenW)
            {
                lBorderList_.left = 0;
                lBorderList_.right = uiScreenW;
            }
            else
            {
                lBorderList_.left = 0;
                lBorderList_.right = uiAbsWidth_;
            }
        }

        if (lBorderList_.bottom > int(uiScreenH))
        {
            if (uint(lBorderList_.bottom - lBorderList_.top) > uiScreenH)
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = uiScreenH;
            }
            else
            {
                lBorderList_.bottom = uiScreenH;
                lBorderList_.top = uiScreenH - uiAbsHeight_;
            }
        }
        if (lBorderList_.top < 0)
        {
            if (uint(lBorderList_.bottom - lBorderList_.top) > uiScreenH)
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = uiScreenH;
            }
            else
            {
                lBorderList_.top = 0;
                lBorderList_.bottom = uiAbsHeight_;
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
            event_receiver::register_event("KEY_PRESSED");
            event_receiver::register_event("KEY_RELEASED");
        }
        else if (!bIsKeyboardEnabled && bIsKeyboardEnabled_)
        {
            event_receiver::unregister_event("KEY_PRESSED");
            event_receiver::unregister_event("KEY_RELEASED");
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
            event_receiver::register_event("MOUSE_MOVED");
            event_receiver::register_event("MOUSE_PRESSED");
            event_receiver::register_event("MOUSE_DOUBLE_CLICKED");
            event_receiver::register_event("MOUSE_RELEASED");
        }
        else if (!bIsMouseEnabled && bIsMouseEnabled_)
        {
            event_receiver::unregister_event("MOUSE_MOVED");
            event_receiver::unregister_event("MOUSE_PRESSED");
            event_receiver::unregister_event("MOUSE_DOUBLE_CLICKED");
            event_receiver::unregister_event("MOUSE_RELEASED");
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
            event_receiver::register_event("MOUSE_WHEEL");
        else if (!bIsMouseWheelEnabled && bIsMouseWheelEnabled_)
            event_receiver::unregister_event("MOUSE_WHEEL");
    }

    bIsMouseWheelEnabled_ = bIsMouseWheelEnabled;
}

void frame::notify_loaded()
{
    if (!bVirtual_)
        on("Load");

    uiobject::notify_loaded();
}

void frame::fire_build_layer_list()
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

    if (lRegionList_.find(pRegion->get_id()) != lRegionList_.end())
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Trying to add \"" << pRegion->get_name() << "\" to \"" << sName_ << "\"'s children, "
            "but it was already one of this frame's children." << std::endl;
        return nullptr;
    }

    layered_region* pAddedRegion = pRegion.get();
    lRegionList_.insert(std::move(pRegion));

    fire_build_layer_list();
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

    auto iter = lRegionList_.find(pRegion->get_id());
    if (iter == lRegionList_.end())
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Trying to remove \"" << pRegion->get_name() << "\" from \"" << sName_ << "\"'s children, "
            "but it was not one of this frame's children." << std::endl;
        return nullptr;
    }

    std::unique_ptr<layered_region> pRemovedRegion = std::move(*iter);
    lRegionList_.erase(iter);
    fire_build_layer_list();
    notify_renderer_need_redraw();
    pRemovedRegion->set_parent(nullptr);
    return pRemovedRegion;
}

layered_region* frame::create_region(layer_type mLayer, const std::string& sClassName,
    const std::string& sName, const std::vector<uiobject*>& lInheritance)
{
    std::unique_ptr<layered_region> pRegion = pManager_->create_layered_region(sClassName);
    pRegion->set_parent(this);

    pRegion->set_draw_layer(mLayer);
    pRegion->set_name(sName);

    if (!pManager_->add_uiobject(pRegion.get()))
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "An object with the name " << pRegion->get_name() << " already exists." << std::endl;
        return nullptr;
    }

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

    pNewFrame->set_parent(this);
    pNewFrame->set_name(sName);

    if (this->is_virtual())
        pNewFrame->set_virtual();

    if (!pManager_->add_uiobject(pNewFrame.get()))
    {
        gui::out << gui::error << "gui::manager : "
            << "An object with the name \"" << pNewFrame->get_name() << "\" already exists." << std::endl;
        return nullptr;
    }

    pNewFrame->create_glue();
    pNewFrame->set_level(get_level() + 1);

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

    if (!pNewFrame->is_virtual())
        get_top_level_renderer()->notify_rendered_frame(pNewFrame.get(), true);

    pNewFrame->set_newly_created();
    pNewFrame->notify_loaded();

    return add_child(std::move(pNewFrame));
}

frame* frame::add_child(std::unique_ptr<frame> pChild)
{
    if (!pChild)
        return nullptr;

    if (lChildList_.find(pChild->get_id()) != lChildList_.end())
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
        pChild->notify_visible_(!pManager_->is_loading_ui());
    else
        pChild->notify_invisible_(!pManager_->is_loading_ui());

    frame* pAddedChild = pChild.get();
    lChildList_.insert(std::move(pChild));

    if (!bVirtual_)
    {
        renderer* pOldTopLevelRenderer = pAddedChild->get_top_level_renderer();
        renderer* pNewTopLevelRenderer = get_top_level_renderer();
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

    auto iter = lChildList_.find(pChild->get_id());
    if (iter == lChildList_.end())
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Trying to remove \"" << pChild->get_name() << "\" from \"" << sName_
            << "\"'s children, but it was not one of this frame's children." << std::endl;
        return nullptr;
    }

    std::unique_ptr<frame> pRemovedChild = std::move(*iter);
    lChildList_.erase(iter);

    renderer* pTopLevelRenderer = get_top_level_renderer();
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

const quad2i& frame::get_abs_hit_rect_insets() const
{
    return lAbsHitRectInsetList_;
}

const quad2f& frame::get_rel_hit_rect_insets() const
{
    return lRelHitRectInsetList_;
}

vector2ui frame::get_max_resize() const
{
    return vector2ui(uiMaxWidth_, uiMaxHeight_);
}

vector2ui frame::get_min_resize() const
{
    return vector2ui(uiMinWidth_, uiMinHeight_);
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

bool frame::is_in_frame(int iX, int iY) const
{
    bool bInFrame = (
        (lBorderList_.left  + lAbsHitRectInsetList_.left <= iX &&
        iX <= lBorderList_.right - lAbsHitRectInsetList_.right - 1)
        &&
        (lBorderList_.top    + lAbsHitRectInsetList_.top <= iY &&
        iY <= lBorderList_.bottom - lAbsHitRectInsetList_.bottom - 1)
    );

    if (pTitleRegion_ && pTitleRegion_->is_in_region(iX, iY))
        return true;
    else
        return bInFrame;
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
    std::string sCutScriptName = sScriptName;
    sCutScriptName.erase(0, 2);

    std::map<std::string, handler>::iterator iterH = lDefinedHandlerList_.find(sCutScriptName);
    if (iterH != lDefinedHandlerList_.end())
        lDefinedHandlerList_.erase(iterH);

    std::string sAdjustedName = get_adjusted_script_name(sScriptName);

    std::string sStr;
    sStr += "function " + sLuaName_ + ":" + sAdjustedName + "() " + sContent + " end";

    // Actually register the function
    try
    {
        pManager_->get_lua().script(sStr, sol::script_default_on_error);
        lDefinedScriptList_[sCutScriptName] = sContent;
        lXMLScriptInfoList_[sCutScriptName].sFile = sFile;
        lXMLScriptInfoList_[sCutScriptName].uiLineNbr = uiLineNbr;
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
    std::string sCutScriptName = sScriptName;
    sCutScriptName.erase(0, 2);

    std::map<std::string, std::string>::iterator iter = lDefinedScriptList_.find(sCutScriptName);
    if (iter != lDefinedScriptList_.end())
        lDefinedScriptList_.erase(iter);

    lDefinedHandlerList_[sCutScriptName] = mHandler;
}

void frame::notify_script_defined(const std::string& sScriptName, bool bDefined)
{
    std::string sCutScriptName = sScriptName;
    sCutScriptName.erase(0, 2);

    std::map<std::string, script_info>::iterator iter = lXMLScriptInfoList_.find(sScriptName);
    if (iter != lXMLScriptInfoList_.end())
        lXMLScriptInfoList_.erase(iter);

    if (bDefined)
    {
        std::map<std::string, handler>::iterator iter2 = lDefinedHandlerList_.find(sScriptName);
        if (iter2 != lDefinedHandlerList_.end())
            lDefinedHandlerList_.erase(iter2);

        lDefinedScriptList_[sCutScriptName] = "";
    } else
        lDefinedScriptList_.erase(sCutScriptName);
}

void frame::on_event(const event& mEvent)
{
    if (has_script("Event") &&
        (lRegEventList_.find(mEvent.get_name()) != lRegEventList_.end() || bHasAllEventsRegistred_))
    {
        // ADDON_LOADED should only be fired if it's this frame's addon
        if (mEvent.get_name() == "ADDON_LOADED")
        {
            if (!pAddOn_ || pAddOn_->sName != mEvent.get<std::string>(0))
                return;
        }

        event mTemp = mEvent;
        on("Event", &mTemp);
    }

    if (!pManager_->is_input_enabled())
        return;

    if (bIsMouseEnabled_ && bIsVisible_)
    {
        if (mEvent.get_name() == "MOUSE_MOVED")
        {
            if (!lMouseButtonList_.empty() && !bMouseDragged_)
            {
                for (const auto& sButton : lMouseButtonList_)
                {
                    if (lRegDragList_.find(sButton) != lRegDragList_.end())
                    {
                        bMouseDragged_ = true;
                        on("DragStart");
                        break;
                    }
                }
            }
        }
        else if (mEvent.get_name() == "MOUSE_PRESSED")
        {
            if (bMouseInTitleRegion_)
                start_moving();

            if (bMouseInFrame_)
            {
                if (bIsTopLevel_)
                    raise();

                if (pTopLevelParent_)
                    pTopLevelParent_->raise();

                std::string sMouseButton = mEvent[3].get<std::string>();
                lMouseButtonList_.push_back(sMouseButton);

                event mEvent2;
                mEvent2.add(sMouseButton);

                on("MouseDown", &mEvent2);
            }
        }
        else if (mEvent.get_name() == "MOUSE_RELEASED")
        {
            if (bIsMovable_)
                stop_moving();

            std::string sMouseButton = mEvent[3].get<std::string>();

            if (bMouseInFrame_)
            {
                event mEvent2;
                mEvent2.add(sMouseButton);

                on("MouseUp", &mEvent2);
            }

            std::vector<std::string>::iterator iter = utils::find(lMouseButtonList_, sMouseButton);
            if (iter != lMouseButtonList_.end())
                lMouseButtonList_.erase(iter);

            if (bMouseDragged_)
            {
                bool bDrag = false;
                for (const auto& sButton : lMouseButtonList_)
                {
                    if (lRegDragList_.find(sButton) != lRegDragList_.end())
                    {
                        bDrag = true;
                        break;
                    }
                }

                if (!bDrag)
                {
                    bMouseDragged_ = false;
                    on("DragStop");
                }
            }
        }
        else if (mEvent.get_name() == "MOUSE_WHEEL" || mEvent.get_name() == "MOUSE_WHEEL_SMOOTH")
        {
            if (bMouseInFrame_)
            {
                event mEvent2;
                mEvent2.add(mEvent[0].get<float>());
                on("MouseWheel", &mEvent2);
            }
        }
    }

    if (bIsKeyboardEnabled_ && bIsVisible_)
    {
        if (mEvent.get_name() == "KEY_PRESSED")
        {
            event mKeyEvent;
            mKeyEvent.add(mEvent[0].get<input::key>());
            mKeyEvent.add(mEvent[1].get<std::string>());

            on("KeyDown", &mKeyEvent);
        }
        else if (mEvent.get_name() == "KEY_RELEASED")
        {
            event mKeyEvent;
            mKeyEvent.add(mEvent[0].get<input::key>());
            mKeyEvent.add(mEvent[1].get<std::string>());

            on("KeyUp", &mKeyEvent);
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

void frame::on(const std::string& sScriptName, event* pEvent)
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

    // Reset all arg* to nil
    for (uint i = 1; i < 9; ++i)
    {
        mLua["arg"+utils::to_string(i)] = sol::lua_nil;
    }

    if (pEvent)
    {
        if ((sScriptName == "KeyDown") ||
            (sScriptName == "KeyUp"))
        {
            // Set key name
            mLua["arg1"] = static_cast<uint>(pEvent->get<input::key>(0));
            mLua["arg2"] = pEvent->get<std::string>(1);
        }
        else if (sScriptName == "MouseDown")
        {
            // Set mouse button
            mLua["arg1"] = pEvent->get<std::string>(0);
        }
        else if (sScriptName == "MouseUp")
        {
            // Set mouse button
            mLua["arg1"] = pEvent->get<std::string>(0);
        }
        else if (sScriptName == "MouseWheel")
        {
            mLua["arg1"] = pEvent->get<float>(0);
        }
        else if (sScriptName == "Update")
        {
            // Set delta time
            mLua["arg1"] = pEvent->get<float>(0);
        }
        else if (sScriptName == "Event")
        {
            // Set event name
            mLua["event"] = pEvent->get_name();

            // Set arguments
            for (uint i = 0; i < pEvent->get_num_param(); ++i)
            {
                const utils::any* pArg = pEvent->get(i);
                const utils::any_type& mType = pArg->get_type();

                sol::object mObject;
                if      (mType == utils::any::VALUE_INT)    mObject = sol::make_object(mLua.lua_state(), pArg->get<int>());
                else if (mType == utils::any::VALUE_UINT)   mObject = sol::make_object(mLua.lua_state(), pArg->get<uint>());
                else if (mType == utils::any::VALUE_FLOAT)  mObject = sol::make_object(mLua.lua_state(), pArg->get<float>());
                else if (mType == utils::any::VALUE_DOUBLE) mObject = sol::make_object(mLua.lua_state(), pArg->get<double>());
                else if (mType == utils::any::VALUE_STRING) mObject = sol::make_object(mLua.lua_state(), pArg->get<std::string>());
                else if (mType == utils::any::VALUE_BOOL)   mObject = sol::make_object(mLua.lua_state(), pArg->get<bool>());
                else                                        mObject = sol::make_object(mLua.lua_state(), sol::lua_nil);

                mLua["arg"+utils::to_string(i+1)] = mObject;
            }
        }
    }

    std::string sAdjustedName = get_adjusted_script_name("On"+sScriptName);

    auto* pOldAddOn = pManager_->get_current_addon();
    pManager_->set_current_addon(pAddOn_);

    sol::table mSelf = mLua[sLuaName_];
    if (mSelf == sol::lua_nil)
    {
        std::string sError = "Lua glue object is nil";
        gui::out << gui::error << sError << std::endl;

        event mEvent("LUA_ERROR");
        mEvent.add(sError);
        pManager_->get_event_manager()->fire_event(mEvent);
        pManager_->set_current_addon(pOldAddOn);
        return;
    }

    sol::protected_function mCallback = mSelf[sAdjustedName];
    if (mCallback == sol::lua_nil)
    {
        std::string sError = "Lua callback "+sAdjustedName+" is nil";
        gui::out << gui::error << sError << std::endl;

        event mEvent("LUA_ERROR");
        mEvent.add(sError);
        pManager_->get_event_manager()->fire_event(mEvent);
        pManager_->set_current_addon(pOldAddOn);

        return;
    }

    auto mResult = mCallback(mSelf);
    if (!mResult.valid())
    {
        sol::error mError = mResult;

        // TODO: show file/line number from lXMLScriptInfoList_
        std::string sError = mError.what();

        gui::out << gui::error << sError << std::endl;

        event mEvent("LUA_ERROR");
        mEvent.add(sError);
        pManager_->get_event_manager()->fire_event(mEvent);
    }

    pManager_->set_current_addon(pOldAddOn);
}

void frame::register_all_events()
{
    bHasAllEventsRegistred_ = true;
    lRegEventList_.clear();
}

void frame::register_event(const std::string& sEvent)
{
    lRegEventList_.insert(sEvent);
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

    if (mStrata_ != mStrata && !bVirtual_ && bLoaded_)
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

void frame::set_abs_hit_rect_insets(int iLeft, int iRight, int iTop, int iBottom)
{
    lAbsHitRectInsetList_ = quad2i(iLeft, iRight, iTop, iBottom);
}

void frame::set_abs_hit_rect_insets(const quad2i& lInsets)
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

        if (!bVirtual_ && bLoaded_)
            get_top_level_renderer()->notify_frame_level_changed(this, iLevel, iLevel_);
    }
}

void frame::set_max_resize(uint uiMaxWidth, uint uiMaxHeight)
{
    set_max_height(uiMaxHeight);
    set_max_width(uiMaxWidth);
}

void frame::set_max_resize(const vector2ui& mMax)
{
    set_max_height(mMax.x);
    set_max_width(mMax.y);
}

void frame::set_min_resize(uint uiMinWidth, uint uiMinHeight)
{
    set_min_height(uiMinHeight);
    set_min_width(uiMinWidth);
}

void frame::set_min_resize(const vector2ui& mMin)
{
    set_min_height(mMin.x);
    set_min_width(mMin.y);
}

void frame::set_max_height(uint uiMaxHeight)
{
    if (uiMaxHeight >= uiMinHeight_)
        uiMaxHeight_ = uiMaxHeight;

    if (uiMaxHeight_ != uiMaxHeight)
        fire_update_dimensions();
}

void frame::set_max_width(uint uiMaxWidth)
{
    if (uiMaxWidth >= uiMinWidth_)
        uiMaxWidth_ = uiMaxWidth;

    if (uiMaxWidth_ != uiMaxWidth)
        fire_update_dimensions();
}

void frame::set_min_height(uint uiMinHeight)
{
    if (uiMinHeight <= uiMaxHeight_)
        uiMinHeight_ = uiMinHeight;

    if (uiMinHeight_ != uiMinHeight)
        fire_update_dimensions();
}

void frame::set_min_width(uint uiMinWidth)
{
    if (uiMinWidth <= uiMaxWidth_)
        uiMinWidth_ = uiMinWidth;

    if (uiMinWidth_ != uiMinWidth)
        fire_update_dimensions();
}

void frame::set_movable(bool bIsMovable)
{
    bIsMovable_ = bIsMovable;
}

void frame::set_parent(frame* pParent)
{
    if (pParent == this)
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : Cannot call set_parent(this)." << std::endl;
        return;
    }

    if (pParent == pParent_)
        return;

    pParent_ = pParent;

    fire_update_dimensions();
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
        if (!is_virtual() && bLoaded_)
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

    if (!is_virtual() && bLoaded_)
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

void frame::set_renderer(renderer* pRenderer)
{
    if (pRenderer == pRenderer_)
        return;

    get_top_level_renderer()->notify_rendered_frame(this, false);
    propagate_renderer_(false);

    pRenderer_ = pRenderer;

    get_top_level_renderer()->notify_rendered_frame(this, true);
    propagate_renderer_(true);
}

const renderer* frame::get_renderer() const
{
    return pRenderer_;
}

renderer* frame::get_top_level_renderer()
{
    if (pRenderer_)
        return pRenderer_;
    else if (pParent_)
        return pParent_->get_top_level_renderer();
    else
        return pManager_;
}

const renderer* frame::get_top_level_renderer() const
{
    if (pRenderer_)
        return pRenderer_;
    else if (pParent_)
        return pParent_->get_top_level_renderer();
    else
        return pManager_;
}

void frame::notify_visible_(bool bTriggerEvents)
{
    bIsVisible_ = true;
    for (auto* pChild : get_children())
    {
        if (pChild->is_shown())
            pChild->notify_visible_(bTriggerEvents);
    }

    if (bTriggerEvents)
    {
        lQueuedEventList_.push_back("Show");
        notify_renderer_need_redraw();
    }
}

void frame::notify_invisible_(bool bTriggerEvents)
{
    bIsVisible_ = false;
    for (auto* pChild : get_children())
    {
        if (pChild->is_shown())
            pChild->notify_invisible_(bTriggerEvents);
    }

    if (bTriggerEvents)
    {
        lQueuedEventList_.push_back("Hide");
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

    get_top_level_renderer()->fire_redraw(mStrata_);
}

void frame::show()
{
    if (bIsShown_)
        return;

    uiobject::show();

    if (!pParent_ || pParent_->is_visible())
        notify_visible_();
}

void frame::hide()
{
    if (!bIsShown_)
        return;

    uiobject::hide();

    if (bIsVisible_)
        notify_invisible_();
}

void frame::set_shown(bool bIsShown)
{
    if (bIsShown_ == bIsShown)
        return;

    bIsShown_ = bIsShown;

    if (!bIsShown_)
        notify_invisible_(false);
}

void frame::unregister_all_events()
{
    bHasAllEventsRegistred_ = false;
    lRegEventList_.clear();
}

void frame::unregister_event(const std::string& sEvent)
{
    lRegEventList_.erase(sEvent);
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

void frame::notify_mouse_in_frame(bool bMouseInframe, int iX, int iY)
{
    if (bMouseInframe)
    {
        if (!bMouseInFrame_)
            on("Enter");

        bMouseInFrame_ = true;

        iMousePosX_ = iX;
        iMousePosY_ = iY;

        bMouseInTitleRegion_ = (pTitleRegion_ && pTitleRegion_->is_in_region(iX, iY));
    }
    else
    {
        if (bMouseInFrame_)
            on("Leave");

        bMouseInTitleRegion_ = false;
        bMouseInFrame_ = false;
    }
}

void frame::update_borders_() const
{
    bool bPositionUpdated = bUpdateBorders_;
    uiobject::update_borders_();

    if (bPositionUpdated)
        check_position();
}

void frame::update(float fDelta)
{
    //#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    DEBUG_LOG("  ~");
    uiobject::update(fDelta);
    DEBUG_LOG("   #");

    for (const auto& sEvent : lQueuedEventList_)
    {
        DEBUG_LOG("   Event " + *iterEvent);
        on(sEvent);
    }

    lQueuedEventList_.clear();

    if (bBuildLayerList_)
    {
        DEBUG_LOG("   Build layers");
        // Clear layers' content
        for (auto& mLayer : utils::range::value(lLayerList_))
            mLayer.lRegionList.clear();

        // Fill layers with regions (with font_string rendered last withing the same layer)
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
        on("Update", &mEvent);
    }

    if (pTitleRegion_)
        pTitleRegion_->update(fDelta);

    // Update regions
    DEBUG_LOG("   Update regions");
    for (auto* pRegion : get_regions())
        pRegion->update(fDelta);

    // Update children
    DEBUG_LOG("   Update children");
    std::map<uint, frame*>::iterator iterChild;
    for (auto* pChild : get_children())
        pChild->update(fDelta);

    if (uiOldWidth_ != uiAbsWidth_ || uiOldHeight_ != uiAbsHeight_)
    {
        DEBUG_LOG("   On size changed");
        on("SizeChanged");

        uiOldWidth_  = uiAbsWidth_;
        uiOldHeight_ = uiAbsHeight_;
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

void frame::clear_links()
{
    frame::clear_links_();
    region::clear_links_();
}

void frame::clear_links_()
{
    if (bLinksClearedFrame_) return;
    bLinksClearedFrame_ = true;

    if (!bVirtual_)
    {
        // Tell the renderer to no longer render this widget
        get_top_level_renderer()->notify_rendered_frame(this, false);
    }

    // Unregister this frame from the GUI manager
    pManager_->remove_frame(this);
}

}
}
