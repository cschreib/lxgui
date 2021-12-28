#include "lxgui/gui_manager.hpp"

#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/gui_virtual_registry.hpp"
#include "lxgui/gui_uiroot.hpp"
#include "lxgui/input.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_std.hpp>
#include <lxgui/utils_range.hpp>

#include <fstream>
#include <sstream>

/** \mainpage lxgui documentation
*
* This page allows you to browse the documentation for the C++ API of lxgui.
*
* For the Lua and layout file API, please go to the
* <a href="../lua/index.html">Lua documentation</a>.
*/

// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace lxgui {
namespace gui
{

manager::manager(std::unique_ptr<input::source> pInputSource,
    std::unique_ptr<renderer> pRenderer) :
    event_manager(),
    event_receiver(static_cast<event_manager&>(*this)),
    pInputManager_(new input::manager(std::move(pInputSource))),
    pRenderer_(std::move(pRenderer))
{
    set_interface_scaling_factor(1.0f);

    pLocalizer_ = std::make_unique<localizer>();
    pObjectRegistry_ = std::make_unique<registry>();
    pVirtualObjectRegistry_ = std::make_unique<virtual_registry>(*pObjectRegistry_);
    pRoot_ = utils::make_owned<uiroot>(*this);

    // NB: cannot call register_event() here, as observable_from_this()
    // is not yet fully initialised! This is done in create_lua() instead.
}

manager::~manager()
{
    close_ui();
}

void manager::set_interface_scaling_factor(float fScalingFactor)
{
    float fFullScalingFactor = fScalingFactor*pInputManager_->get_interface_scaling_factor_hint();

    if (fFullScalingFactor == fScalingFactor_) return;

    fBaseScalingFactor_ = fScalingFactor;
    fScalingFactor_ = fFullScalingFactor;

    pInputManager_->set_interface_scaling_factor(fScalingFactor_);
    pRoot_->notify_scaling_factor_updated();

    notify_object_moved();
}

float manager::get_interface_scaling_factor() const
{
    return fScalingFactor_;
}

void manager::add_addon_directory(const std::string& sDirectory)
{
    if (utils::find(lGUIDirectoryList_, sDirectory) == lGUIDirectoryList_.end())
        lGUIDirectoryList_.push_back(sDirectory);
}

void manager::clear_addon_directory_list()
{
    lGUIDirectoryList_.clear();
}

utils::owner_ptr<uiobject> manager::create_uiobject(const std::string& sClassName)
{
    if (sClassName == "Frame")
        return utils::make_owned<frame>(*this);
    else if (sClassName == "FocusFrame")
        return utils::make_owned<focus_frame>(*this);
    else
    {
        auto iterFrame = lCustomFrameList_.find(sClassName);
        if (iterFrame != lCustomFrameList_.end())
            return iterFrame->second(*this);

        auto iterRegion = lCustomRegionList_.find(sClassName);
        if (iterRegion != lCustomRegionList_.end())
            return iterRegion->second(*this);

        gui::out << gui::warning << "gui::manager : Unknown uiobject class : \""
            << sClassName << "\"." << std::endl;
        return nullptr;
    }
}

utils::owner_ptr<frame> manager::create_frame(const std::string& sClassName)
{
    if (sClassName == "Frame")
        return utils::make_owned<frame>(*this);
    else if (sClassName == "FocusFrame")
        return utils::make_owned<focus_frame>(*this);
    else
    {
        auto iterFrame = lCustomFrameList_.find(sClassName);
        if (iterFrame != lCustomFrameList_.end())
            return iterFrame->second(*this);

        gui::out << gui::warning << "gui::manager : Unknown Frame class : \""
            << sClassName << "\"." << std::endl;
        return nullptr;
    }
}

utils::owner_ptr<layered_region> manager::create_layered_region(const std::string& sClassName)
{
    auto iterRegion = lCustomRegionList_.find(sClassName);
    if (iterRegion != lCustomRegionList_.end())
        return iterRegion->second(*this);

    gui::out << gui::warning << "gui::manager : Unknown layered_region class : \""
        << sClassName << "\"." << std::endl;
    return nullptr;
}

bool manager::add_uiobject(utils::observer_ptr<uiobject> pObj)
{
    if (!pObj)
    {
        gui::out << gui::error << "gui::manager : Adding a null widget." << std::endl;
        return false;
    }

    registry* pRegistry = nullptr;
    if (pObj->is_virtual())
    {
        if (pObj->get_parent())
        {
            // Virtual children are not recorded in the named list, as they
            // cannot be inherited from directly, and won't appear in the UI.
            return true;
        }

        pRegistry = pVirtualObjectRegistry_.get();
    }
    else
        pRegistry = pObjectRegistry_.get();

    return pRegistry->add_uiobject(std::move(pObj));
}

void manager::remove_uiobject(const utils::observer_ptr<uiobject>& pObj)
{
    uiobject* pObjRaw = pObj.get();
    if (!pObjRaw) return;

    registry* pRegistry = nullptr;
    if (pObjRaw->is_virtual())
    {
        if (!pObjRaw->get_parent())
            pRegistry = pVirtualObjectRegistry_.get();
    }
    else
        pRegistry = pObjectRegistry_.get();

    if (pRegistry)
        pRegistry->remove_uiobject(*pObjRaw);

    if (pMovedObject_.get() == pObjRaw)
        stop_moving(*pObjRaw);

    if (pSizedObject_.get() == pObjRaw)
        stop_sizing(*pObjRaw);
}

void manager::remove_frame(const utils::observer_ptr<frame>& pObj)
{
    frame* pObjRaw = pObj.get();
    if (!pObjRaw) return;

    if (pHoveredFrame_.get() == pObjRaw)
        clear_hovered_frame_();

    if (pFocusedFrame_.get() == pObjRaw)
        clear_focussed_frame_();
}

sol::state& manager::get_lua()
{
    return *pLua_;
}

const sol::state& manager::get_lua() const
{
    return *pLua_;
}

void manager::load_addon_toc_(const std::string& sAddOnName, const std::string& sAddOnDirectory)
{
    auto& lAddOns = lAddOnList_[sAddOnDirectory];
    if (lAddOns.find(sAddOnName) != lAddOns.end())
        return;

    addon mAddOn;
    mAddOn.bEnabled = true;
    mAddOn.sMainDirectory = utils::cut(sAddOnDirectory, "/").back();
    mAddOn.sDirectory = sAddOnDirectory + "/" + sAddOnName;

    std::string sTOCFile = mAddOn.sDirectory + "/" + sAddOnName + ".toc";
    std::ifstream mFile(sTOCFile);
    if (!mFile.is_open())
        return;

    while (!mFile.eof())
    {
        std::string sLine; getline(mFile, sLine);
        if (sLine.empty())
            continue;

        utils::replace(sLine, "\r", "");
        if (sLine[0] == '#' && sLine[1] == '#')
        {
            sLine.erase(0, 2);
            utils::trim(sLine, ' ');
            std::pair<std::string, std::string> lArgs = utils::cut_first(sLine, ":");
            if (!lArgs.first.empty() && !lArgs.second.empty())
            {
                std::string sKey = lArgs.first;
                utils::trim(sKey, ' ');
                std::string sValue = lArgs.second;
                utils::trim(sValue, ' ');

                if (sKey == "Interface")
                {
                    mAddOn.sUIVersion = sValue;

                    if (mAddOn.sUIVersion == sUIVersion_)
                        mAddOn.bEnabled = true;
                    else
                    {
                        gui::out << gui::warning << "gui::manager : "
                            << "Wrong UI version for \"" << sAddOnName << "\" (got : "
                            << mAddOn.sUIVersion << ", expected : " << sUIVersion_
                            << "). AddOn disabled." << std::endl;
                        mAddOn.bEnabled = false;
                    }
                }
                else if (sKey == "Title")
                    mAddOn.sName = sValue;
                else if (sKey == "Version")
                    mAddOn.sVersion = sValue;
                else if (sKey == "Author")
                    mAddOn.sAuthor = sValue;
                else if (sKey == "SavedVariables")
                {
                    for (auto sVar : utils::cut(sValue, ","))
                    {
                        utils::trim(sVar, ' ');
                        if (!utils::has_no_content(sVar))
                            mAddOn.lSavedVariableList.push_back(std::move(sVar));
                    }
                }
            }
        }
        else
        {
            utils::trim(sLine, ' ');
            if (!utils::has_no_content(sLine))
                mAddOn.lFileList.push_back(mAddOn.sDirectory + "/" + sLine);
        }
    }

    mFile.close();

    if (mAddOn.sName.empty())
        gui::out << gui::error << "gui::manager : Missing addon name in " << sTOCFile << "." << std::endl;
    else
        lAddOns[sAddOnName] = mAddOn;
}

void manager::load_addon_files_(addon* pAddOn)
{
    pLocalizer_->load_translations(pAddOn->sDirectory);

    pCurrentAddOn_ = pAddOn;
    for (const auto& sFile : pAddOn->lFileList)
    {
        const std::string sExtension = utils::get_file_extension(sFile);
        if (sExtension == ".lua")
        {
            try
            {
                pLua_->do_file(sFile);
            }
            catch (const sol::error& e)
            {
                std::string sError = e.what();

                gui::out << gui::error << sError << std::endl;

                event mEvent("LUA_ERROR");
                mEvent.add(sError);
                fire_event(mEvent);
            }
        }
        else
        {
            this->parse_layout_file_(sFile, pAddOn);
        }
    }

    std::string sSavedVariablesFile = "saves/interface/"+pAddOn->sMainDirectory+"/"+pAddOn->sName+".lua";
    if (utils::file_exists(sSavedVariablesFile))
    {
        try
        {
            pLua_->do_file(sSavedVariablesFile);
        }
        catch (const sol::error& e)
        {
            std::string sError = e.what();

            gui::out << gui::error << sError << std::endl;

            event mEvent("LUA_ERROR");
            mEvent.add(sError);
            fire_event(mEvent);
        }
    }

    event mEvent("ADDON_LOADED");
    mEvent.add(pAddOn->sName);
    fire_event(mEvent);
}

void manager::load_addon_directory_(const std::string& sDirectory)
{
    for (auto sSubDir : utils::get_directory_list(sDirectory))
        this->load_addon_toc_(sSubDir, sDirectory);

    std::vector<addon*> lCoreAddOnStack;
    std::vector<addon*> lAddOnStack;
    bool bCore = false;

    auto& lAddOns = lAddOnList_[sDirectory];

    std::ifstream mFile(sDirectory + "/addons.txt");
    if (mFile.is_open())
    {
        while (!mFile.eof())
        {
            std::string sLine;
            getline(mFile, sLine);
            if (sLine.empty())
                continue;

            utils::replace(sLine, "\r", "");
            if (sLine[0] == '#')
            {
                sLine.erase(0, 1);
                utils::trim(sLine, ' ');
                bCore = sLine == "Core";
            }
            else
            {
                std::pair<std::string,std::string> lArgs = utils::cut_first(sLine, ":");
                if (!lArgs.first.empty() && !lArgs.second.empty())
                {
                    std::string sKey = lArgs.first;
                    utils::trim(sKey, ' ');
                    std::string sValue = lArgs.second;
                    utils::trim(sValue, ' ');
                    auto iter = lAddOns.find(sKey);
                    if (iter != lAddOns.end())
                    {
                        if (bCore)
                            lCoreAddOnStack.push_back(&iter->second);
                        else
                            lAddOnStack.push_back(&iter->second);

                        iter->second.bEnabled = sValue == "1";
                    }
                }
            }
        }
        mFile.close();
    }

    for (auto* pAddOn : lCoreAddOnStack)
    {
        if (pAddOn->bEnabled)
            this->load_addon_files_(pAddOn);
    }

    for (auto* pAddOn : lAddOnStack)
    {
        if (pAddOn->bEnabled)
            this->load_addon_files_(pAddOn);
    }

    pCurrentAddOn_ = nullptr;
}

void manager::save_variables_(const addon* pAddOn)
{
    if (!pAddOn->lSavedVariableList.empty())
    {
        if (!utils::make_directory("saves/interface/"+pAddOn->sMainDirectory))
        {
            gui::out << gui::error << "gui::manager : "
                "unable to create directory 'saves/interface/" <<
                pAddOn->sMainDirectory << "'" << std::endl;
            return;
        }

        std::ofstream mFile("saves/interface/"+pAddOn->sMainDirectory+"/"+pAddOn->sName+".lua");
        for (const auto& sVariable : pAddOn->lSavedVariableList)
        {
            std::string sSerialized = serialize_global_(sVariable);
            if (!sSerialized.empty())
                mFile << sSerialized << "\n";
        }
    }
}

void gui_out(const std::string& sMessage)
{
    gui::out << sMessage << std::endl;
}

void manager::read_files()
{
    if (bClosed_)
    {
        bLoadingUI_ = true;

        for (const auto& sDirectory : lGUIDirectoryList_)
            this->load_addon_directory_(sDirectory);

        bLoadingUI_ = false;
        bClosed_ = false;
    }
}

void manager::load_ui()
{
    create_lua(pLuaRegs_);

    read_files();
}

void manager::close_ui()
{
    if (!bClosed_)
    {
        for (const auto& sDirectory : lGUIDirectoryList_)
        {
            for (const auto& mAddOn : utils::range::value(lAddOnList_[sDirectory]))
                save_variables_(&mAddOn);
        }

        pRoot_ = utils::make_owned<uiroot>(*this);

        pObjectRegistry_ = std::make_unique<registry>();
        pVirtualObjectRegistry_ = std::make_unique<virtual_registry>(*pObjectRegistry_);

        lAddOnList_.clear();

        pLua_ = nullptr;

        pHoveredFrame_ = nullptr;
        bUpdateHoveredFrame_ = false;
        pFocusedFrame_ = nullptr;
        pMovedObject_ = nullptr;
        pSizedObject_ = nullptr;
        pMovedAnchor_ = nullptr;
        bObjectMoved_ = false;
        mMouseMovement_ = vector2f::ZERO;
        mMovementStartPosition_ = vector2f::ZERO;
        mConstraint_ = constraint::NONE;
        mResizeStart_ = vector2f::ZERO;
        bResizeWidth_ = false;
        bResizeHeight_ = false;
        bResizeFromRight_ = false;
        bResizeFromBottom_ = false;

        lKeyBindingList_.clear();

        bClosed_ = true;
        bLoadingUI_ = false;
        pCurrentAddOn_ = nullptr;

        pLocalizer_->clear_translations();

        unregister_event("KEY_PRESSED");
        unregister_event("MOUSE_MOVED");
        unregister_event("WINDOW_RESIZED");
    }
}

void manager::reload_ui()
{
    bReloadUI_ = true;
}

void manager::reload_ui_now()
{
    gui::out << "Closing UI..." << std::endl;
    close_ui();
    gui::out << "Done. Loading UI..." << std::endl;
    load_ui();
    gui::out << "Done." << std::endl;

    bReloadUI_ = false;
}

void manager::begin(std::shared_ptr<render_target> pTarget) const
{
    pRenderer_->begin(pTarget);

    vector2f mView;
    if (pTarget)
        mView = vector2f(pTarget->get_canvas_dimensions())/fScalingFactor_;
    else
        mView = pRoot_->get_target_dimensions();

    pRenderer_->set_view(matrix4f::view(mView));
}

void manager::end() const
{
    pRenderer_->end();
}

void manager::render_ui() const
{
    begin();

    pRoot_->render();

    end();
}

bool manager::is_loading_ui() const
{
    return bLoadingUI_;
}

bool manager::is_loaded() const
{
    return !bClosed_;
}

void manager::update(float fDelta)
{
    bUpdating_ = true;

    DEBUG_LOG(" Input...");
    pInputManager_->update(fDelta);

    DEBUG_LOG(" Update widgets...");
    pRoot_->update(fDelta);

    if (bObjectMoved_ || pInputManager_->get_mouse_delta() != vector2f::ZERO)
        bUpdateHoveredFrame_ = true;

    update_hovered_frame_();

    bObjectMoved_ = false;

    if (bFirstIteration_)
    {
        DEBUG_LOG(" Entering world...");
        fire_event(event("ENTERING_WORLD"));
        bFirstIteration_ = false;
    }

    frame_ended();
    bUpdating_ = false;

    if (bReloadUI_)
        reload_ui_now();
}

void manager::clear_hovered_frame_()
{
    pHoveredFrame_ = nullptr;
    pInputManager_->allow_input("WORLD");
}

void manager::set_hovered_frame_(utils::observer_ptr<frame> pFrame, const vector2f& mMousePos)
{
    if (pHoveredFrame_ && pFrame != pHoveredFrame_)
        pHoveredFrame_->notify_mouse_in_frame(false, mMousePos);

    if (pFrame)
    {
        pHoveredFrame_ = pFrame;
        pHoveredFrame_->notify_mouse_in_frame(true, mMousePos);
        if (pHoveredFrame_->is_world_input_allowed())
            pInputManager_->allow_input("WORLD");
        else
            pInputManager_->block_input("WORLD");
    }
    else
        clear_hovered_frame_();
}

void manager::start_moving(utils::observer_ptr<uiobject> pObj, anchor* pAnchor,
    constraint mConstraint, std::function<void()> mApplyConstraintFunc)
{
    pSizedObject_ = nullptr;
    pMovedObject_ = pObj;
    mMouseMovement_ = vector2f::ZERO;

    if (pMovedObject_)
    {
        mConstraint_ = mConstraint;
        mApplyConstraintFunc_ = mApplyConstraintFunc;
        if (pAnchor)
        {
            pMovedAnchor_ = pAnchor;
            mMovementStartPosition_ = pMovedAnchor_->mOffset;
        }
        else
        {
            const bounds2f lBorders = pMovedObject_->get_borders();

            pMovedObject_->clear_all_points();
            pMovedObject_->set_point(anchor_data(anchor_point::TOPLEFT, "", lBorders.top_left()));

            pMovedAnchor_ = &pMovedObject_->modify_point(anchor_point::TOPLEFT);

            mMovementStartPosition_ = lBorders.top_left();
        }
    }
}

void manager::stop_moving(const uiobject& mObj)
{
    if (pMovedObject_.get() == &mObj)
    {
        pMovedObject_ = nullptr;
        pMovedAnchor_ = nullptr;
    }
}

bool manager::is_moving(const uiobject& mObj) const
{
    return pMovedObject_.get() == &mObj;
}

void manager::start_sizing(utils::observer_ptr<uiobject> pObj, anchor_point mPoint)
{
    pMovedObject_   = nullptr;
    pSizedObject_   = pObj;
    mMouseMovement_ = vector2f::ZERO;

    if (pSizedObject_)
    {
        const bounds2f lBorders = pSizedObject_->get_borders();

        anchor_point mOppositePoint = anchor_point::CENTER;
        vector2f mOffset;

        switch (mPoint)
        {
            case anchor_point::TOPLEFT :
            case anchor_point::TOP :
                mOppositePoint = anchor_point::BOTTOMRIGHT;
                mOffset = lBorders.bottom_right();
                bResizeFromRight_  = false;
                bResizeFromBottom_ = false;
                break;
            case anchor_point::TOPRIGHT :
            case anchor_point::RIGHT :
                mOppositePoint = anchor_point::BOTTOMLEFT;
                mOffset = lBorders.bottom_left();
                bResizeFromRight_  = true;
                bResizeFromBottom_ = false;
                break;
            case anchor_point::BOTTOMRIGHT :
            case anchor_point::BOTTOM :
                mOppositePoint = anchor_point::TOPLEFT;
                mOffset = lBorders.top_left();
                bResizeFromRight_  = true;
                bResizeFromBottom_ = true;
                break;
            case anchor_point::BOTTOMLEFT :
            case anchor_point::LEFT :
                mOppositePoint = anchor_point::TOPRIGHT;
                mOffset = lBorders.top_right();
                bResizeFromRight_  = false;
                bResizeFromBottom_ = true;
                break;
            case anchor_point::CENTER :
                gui::out << gui::error << "gui::manager : "
                    << "Cannot resize \"" <<  pObj->get_name() << "\" from its center." << std::endl;
                pSizedObject_ = nullptr;
                return;
        }

        pSizedObject_->clear_all_points();
        pSizedObject_->set_point(anchor_data(mOppositePoint, "", anchor_point::TOPLEFT, mOffset));

        mResizeStart_ = pSizedObject_->get_apparent_dimensions();

        if (mPoint == anchor_point::LEFT || mPoint == anchor_point::RIGHT)
        {
            bResizeWidth_  = true;
            bResizeHeight_ = false;
        }
        else if (mPoint == anchor_point::TOP || mPoint == anchor_point::BOTTOM)
        {
            bResizeWidth_  = false;
            bResizeHeight_ = true;
        }
        else
        {
            bResizeWidth_  = true;
            bResizeHeight_ = true;
        }
    }
}

void manager::stop_sizing(const uiobject& mObj)
{
    if (pSizedObject_.get() == &mObj)
        pSizedObject_ = nullptr;
}

bool manager::is_sizing(const uiobject& mObj) const
{
    return pSizedObject_.get() == &mObj;
}

const vector2f& manager::get_movement() const
{
    return mMouseMovement_;
}

void manager::notify_object_moved()
{
    bObjectMoved_ = true;
}

void manager::enable_input(bool bEnable)
{
    if (bInputEnabled_ != bEnable)
        toggle_input();
}

void manager::toggle_input()
{
    bInputEnabled_ = !bInputEnabled_;

    if (bInputEnabled_)
    {
        bUpdateHoveredFrame_ = true;

        if (pFocusedFrame_)
            pInputManager_->set_keyboard_focus(true, pFocusedFrame_);
    }
    else
    {
        set_hovered_frame_(nullptr);

        if (pFocusedFrame_)
            pInputManager_->set_keyboard_focus(false);
    }
}

bool manager::is_input_enabled() const
{
    return bInputEnabled_;
}

void manager::update_hovered_frame_()
{
    if (!bUpdateHoveredFrame_ || !bInputEnabled_)
        return;

    DEBUG_LOG(" Update hovered frame...");
    const auto mMousePos = pInputManager_->get_mouse_position();

    utils::observer_ptr<frame> pHoveredFrame = pRoot_->find_hovered_frame(mMousePos);
    set_hovered_frame_(std::move(pHoveredFrame), mMousePos);

    bUpdateHoveredFrame_ = false;
}

const utils::observer_ptr<frame>& manager::get_hovered_frame()
{
    update_hovered_frame_();
    return pHoveredFrame_;
}

void manager::notify_hovered_frame_dirty()
{
    bUpdateHoveredFrame_ = true;
}

void manager::clear_focussed_frame_()
{
    pFocusedFrame_ = nullptr;
    pInputManager_->set_keyboard_focus(false);
}

void manager::request_focus(utils::observer_ptr<focus_frame> pFocusFrame)
{
    if (pFocusFrame == pFocusedFrame_)
        return;

    if (pFocusedFrame_)
        pFocusedFrame_->notify_focus(false);

    if (pFocusFrame)
    {
        pFocusedFrame_ = std::move(pFocusFrame);
        pFocusedFrame_->notify_focus(true);
        pInputManager_->set_keyboard_focus(true, pFocusedFrame_);
    }
    else
        clear_focussed_frame_();
}

void manager::set_key_binding(input::key mKey, const std::string& sLuaString)
{
    lKeyBindingList_[mKey][input::key::K_UNASSIGNED][input::key::K_UNASSIGNED] = sLuaString;
}

void manager::set_key_binding(input::key mKey, input::key mModifier, const std::string& sLuaString)
{
    lKeyBindingList_[mKey][mModifier][input::key::K_UNASSIGNED] = sLuaString;
}

void manager::set_key_binding(input::key mKey, input::key mModifier1, input::key mModifier2, const std::string& sLuaString)
{
    lKeyBindingList_[mKey][mModifier1][mModifier2] = sLuaString;
}

void manager::remove_key_binding(input::key mKey, input::key mModifier1, input::key mModifier2)
{
    auto iter1 = lKeyBindingList_.find(mKey);
    if (iter1 != lKeyBindingList_.end())
    {
        auto iter2 = iter1->second.find(mModifier1);
        if (iter2 != iter1->second.end())
        {
            auto iter3 = iter2->second.find(mModifier2);
            if (iter3 != iter2->second.end())
            {
                iter2->second.erase(iter3);

                if (iter2->second.empty())
                    iter1->second.erase(iter2);

                if (iter1->second.empty())
                    lKeyBindingList_.erase(iter1);
            }
        }
    }
}

const addon* manager::get_current_addon()
{
    return pCurrentAddOn_;
}

void manager::set_current_addon(const addon* pAddOn)
{
    pCurrentAddOn_ = pAddOn;
}

std::string manager::parse_file_name(const std::string& sFileName) const
{
    if (sFileName.empty())
        return sFileName;

    std::string sNewFile = sFileName;
    if (sNewFile[0] == '|')
    {
        sNewFile[0] = '/';
        if (pCurrentAddOn_)
            sNewFile = pCurrentAddOn_->sDirectory + sNewFile;
    }

    return sNewFile;
}

void manager::on_event(const event& mEvent)
{
    if (mEvent.get_name() == "KEY_PRESSED")
    {
        const input::key mKey = mEvent.get<input::key>(0);

        std::string sScript;
        std::string sKeyName;

        auto iter1 = lKeyBindingList_.find(mKey);
        if (iter1 != lKeyBindingList_.end())
        {
            for (const auto& iter2 : iter1->second)
            {
                if (iter2.first == input::key::K_UNASSIGNED ||
                    !pInputManager_->key_is_down(iter2.first))
                    continue;

                // First try to get a match with the most complicated binding with two modifiers
                for (const auto& iter3 : iter2.second)
                {
                    if (iter3.first == input::key::K_UNASSIGNED ||
                        !pInputManager_->key_is_down(iter3.first))
                        continue;

                    sScript = iter3.second;
                    sKeyName = pInputManager_->get_key_name(mKey, iter2.first, iter3.first);
                    break;
                }

                if (!sScript.empty())
                    break;

                // If none was found, try with only one modifier
                auto iter3 = iter2.second.find(input::key::K_UNASSIGNED);
                if (iter3 != iter2.second.end())
                {
                    sScript = iter3->second;
                    sKeyName = pInputManager_->get_key_name(mKey, iter2.first);
                }
            }

            if (sScript.empty())
            {
                // If no modifier was matching, try with no modifier
                auto iter2 = iter1->second.find(input::key::K_UNASSIGNED);
                if (iter2 != iter1->second.end())
                {
                    auto iter3 = iter2->second.find(input::key::K_UNASSIGNED);
                    if (iter3 != iter2->second.end())
                    {
                        sScript = iter3->second;
                        sKeyName = pInputManager_->get_key_name(mKey);
                    }
                }
            }
        }

        if (!sScript.empty())
        {
            try
            {
                pLua_->do_string(sScript);
            }
            catch (const sol::error& e)
            {
                gui::out << gui::error << "Bound action : " << sKeyName
                    << " : " << e.what() << std::endl;
            }
        }
    }
    else if (mEvent.get_name() == "WINDOW_RESIZED")
    {
        // Update the scaling factor
        set_interface_scaling_factor(fBaseScalingFactor_);

        notify_object_moved();

        pRenderer_->notify_window_resized(vector2ui(
            mEvent.get<std::uint32_t>(0), mEvent.get<std::uint32_t>(1)));
    }
    else if (mEvent.get_name() == "MOUSE_MOVED")
    {
        if (pMovedObject_ || pSizedObject_)
        {
            DEBUG_LOG(" Moved object...");
            mMouseMovement_ += vector2f(mEvent.get<float>(0), mEvent.get<float>(1));
        }

        if (pMovedObject_)
        {
            switch (mConstraint_)
            {
                case constraint::NONE :
                    pMovedAnchor_->mOffset = mMovementStartPosition_ + mMouseMovement_;
                    break;
                case constraint::X :
                    pMovedAnchor_->mOffset = mMovementStartPosition_ +
                        vector2f(mMouseMovement_.x, 0.0f);
                    break;
                case constraint::Y :
                    pMovedAnchor_->mOffset = mMovementStartPosition_ +
                        vector2f(0.0f, mMouseMovement_.y);
                    break;
                default : break;
            }

            if (mApplyConstraintFunc_)
                mApplyConstraintFunc_();

            // As a result of applying constraints, object may have been deleted,
            // so check again before use
            if (pMovedObject_)
                pMovedObject_->notify_borders_need_update();
        }
        else if (pSizedObject_)
        {
            float fWidth;
            if (bResizeFromRight_)
                fWidth = std::max(0.0f, mResizeStart_.x + mMouseMovement_.x);
            else
                fWidth = std::max(0.0f, mResizeStart_.x - mMouseMovement_.x);

            float fHeight;
            if (bResizeFromBottom_)
                fHeight = std::max(0.0f, mResizeStart_.y + mMouseMovement_.y);
            else
                fHeight = std::max(0.0f, mResizeStart_.y - mMouseMovement_.y);

            if (bResizeWidth_ && bResizeHeight_)
                pSizedObject_->set_dimensions(vector2f(fWidth, fHeight));
            else if (bResizeWidth_)
                pSizedObject_->set_width(fWidth);
            else if (bResizeHeight_)
                pSizedObject_->set_height(fHeight);
        }
    }
}

std::string manager::print_ui() const
{
    std::stringstream s;

    if (!lAddOnList_.empty())
    {
        s << "\n\n######################## Loaded addons ########################\n" << std::endl;
        for (const auto& iterDirectory : lAddOnList_)
        {
            s << "# Directory : " << iterDirectory.first << "\n|-###" << std::endl;
            for (const auto& iterAdd : iterDirectory.second)
            {
                if (iterAdd.second.bEnabled)
                    s << "|   # " << iterAdd.first << std::endl;
            }
            s << "|-###\n#" << std::endl;
        }
    }

    s << "\n\n######################## UIObjects ########################\n\n########################\n" << std::endl;
    for (const auto& mFrame : pRoot_->get_root_frames())
    {
        if (mFrame.is_virtual())
            continue;

        s << mFrame.serialize("") << "\n########################\n" << std::endl;
    }

    s << "\n\n#################### Virtual UIObjects ####################\n\n########################\n" << std::endl;
    for (const auto& mFrame : pRoot_->get_root_frames())
    {
        if (!mFrame.is_virtual())
            continue;

        s << mFrame.serialize("") << "\n########################\n" << std::endl;
    }

    return s.str();
}

}
}
