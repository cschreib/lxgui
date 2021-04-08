#include "lxgui/gui_manager.hpp"

#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/input.hpp"

#include <lxgui/luapp_exception.hpp>
#include <lxgui/luapp_state.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_std.hpp>
#include <lxgui/utils_range.hpp>

#include <sol/state.hpp>

#include <fstream>
#include <sstream>

/** \mainpage lxgui documentation
*
* This page allows you to browse the documentation for the C++ API of lxgui.
*
* For the Lua/XML API, please go to the
* <a href="../lua/index.html">Lua documentation</a>.
*/

// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace lxgui {
namespace gui
{
int l_set_key_binding(lua_State* pLua);
int l_create_frame(lua_State* pLua);
int l_delete_frame(lua_State* pLua);
int l_get_locale(lua_State* pLua);
int l_set_interface_scaling_factor(lua_State* pLua);
int l_log(lua_State* pLua);

manager::manager(std::unique_ptr<input::source> pInputSource,
    std::unique_ptr<renderer> pRenderer, const std::string& sLocale) :
    event_receiver(nullptr),
    pInputManager_(new input::manager(std::move(pInputSource))), sLocale_(sLocale),
    pRenderer_(std::move(pRenderer))
{
    pEventManager_ = std::unique_ptr<event_manager>(new event_manager());
    event_receiver::set_event_manager(pEventManager_.get());
    pInputManager_->register_event_manager(pEventManager_.get());
    register_event("KEY_PRESSED");
    register_event("MOUSE_MOVED");
    register_event("WINDOW_RESIZED");

    uiScreenWidth_ = pInputManager_->get_window_width();
    uiScreenHeight_ = pInputManager_->get_window_height();

    set_interface_scaling_factor(1.0f);
}

manager::~manager()
{
    close_ui();

    // Notify event receiver that event manager is about to be destroyed
    event_receiver::set_event_manager(nullptr);
}

renderer* manager::get_renderer()
{
    return pRenderer_.get();
}

const renderer* manager::get_renderer() const
{
    return pRenderer_.get();
}

uint manager::get_target_width() const
{
    return uiScreenWidth_/get_interface_scaling_factor();
}

uint manager::get_target_height() const
{
    return uiScreenHeight_/get_interface_scaling_factor();
}

void manager::set_interface_scaling_factor(float fScalingFactor)
{
    fScalingFactor *= pInputManager_->get_interface_scaling_factor_hint();

    if (fScalingFactor == fScalingFactor_) return;

    fBaseScalingFactor_ = fScalingFactor;
    fScalingFactor_ = fScalingFactor;

    pInputManager_->set_interface_scaling_factor(fScalingFactor_);

    for (auto* pObject : utils::range::value(lObjectList_))
        pObject->notify_scaling_factor_updated();

    if (pRenderTarget_)
        create_caching_render_target_();

    for (auto& mStrata : lStrataList_)
    {
        if (mStrata.pRenderTarget)
            create_strata_cache_render_target_(mStrata);
    }

    notify_object_moved();
}

float manager::get_interface_scaling_factor() const
{
    return fBaseScalingFactor_;
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

bool manager::check_uiobject_name(const std::string& sName) const
{
    if (utils::has_no_content(sName))
    {
        gui::out << gui::error << "gui::manager : "
            << "Cannot create a uiobject with a blank name." << std::endl;
        return false;
    }

    if (utils::is_number(sName[0]))
    {
        gui::out << gui::error << "gui::manager : "
            << "A widget's name cannot start by a number : \"" << sName
            << "\" is forbidden." << std::endl;
        return false;
    }

    size_t uiPos = sName.find("$");
    if (uiPos != sName.npos && uiPos != 0)
    {
        gui::out << gui::error << "gui::manager : "
            << "A widget's name cannot contain the character '$' except at the begining : \""
            << sName << "\" is forbidden." << std::endl;
        return false;
    }

    for (auto c : sName)
    {
        if (!isalnum(c) && c != '_' && c != '$')
        {
            gui::out << gui::error << "gui::manager : "
                << "A widget's name can only contain alphanumeric symbols, or underscores : \""
                << sName << "\" is forbidden." << std::endl;
            return false;
        }
    }

    return true;
}

std::unique_ptr<uiobject> manager::create_uiobject(const std::string& sClassName)
{
    if (sClassName == "Frame")
        return std::unique_ptr<uiobject>(new frame(this));
    else if (sClassName == "FocusFrame")
        return std::unique_ptr<uiobject>(new focus_frame(this));
    else
    {
        auto iterFrame = lCustomFrameList_.find(sClassName);
        if (iterFrame != lCustomFrameList_.end())
            return std::unique_ptr<uiobject>((*iterFrame->second)(this));

        auto iterRegion = lCustomRegionList_.find(sClassName);
        if (iterRegion != lCustomRegionList_.end())
            return std::unique_ptr<uiobject>((*iterRegion->second)(this));

        gui::out << gui::warning << "gui::manager : Unknown uiobject class : \"" << sClassName << "\"." << std::endl;
        return nullptr;
    }
}

std::unique_ptr<frame> manager::create_frame(const std::string& sClassName)
{
    if (sClassName == "Frame")
        return std::unique_ptr<frame>(new frame(this));
    else if (sClassName == "FocusFrame")
        return std::unique_ptr<frame>(new focus_frame(this));
    else
    {
        auto iterFrame = lCustomFrameList_.find(sClassName);
        if (iterFrame != lCustomFrameList_.end())
            return std::unique_ptr<frame>((*iterFrame->second)(this));

        gui::out << gui::warning << "gui::manager : Unknown Frame class : \"" << sClassName << "\"." << std::endl;
        return nullptr;
    }
}

frame* manager::create_root_frame_(const std::string& sClassName, const std::string& sName,
                                   bool bVirtual, const std::vector<uiobject*>& lInheritance)
{
    if (!check_uiobject_name(sName))
        return nullptr;

    std::unique_ptr<frame> pNewFrame = create_frame(sClassName);
    if (!pNewFrame)
        return nullptr;

    pNewFrame->set_name(sName);

    if (bVirtual)
        pNewFrame->set_virtual();

    if (!pNewFrame->is_virtual())
        notify_rendered_frame(pNewFrame.get(), true);

    if (!add_uiobject(pNewFrame.get()))
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

    return add_root_frame(std::move(pNewFrame));
}

std::unique_ptr<layered_region> manager::create_layered_region(const std::string& sClassName)
{
    auto iterRegion = lCustomRegionList_.find(sClassName);
    if (iterRegion != lCustomRegionList_.end())
        return std::unique_ptr<layered_region>((*iterRegion->second)(this));

    gui::out << gui::warning << "gui::manager : Unknown layered_region class : \"" << sClassName << "\"." << std::endl;
    return nullptr;
}

uint manager::get_new_object_id_() const
{
    uint i = 0;
    auto iterObj = lObjectList_.find(i);
    while (iterObj != lObjectList_.end())
    {
        ++i;
        iterObj = lObjectList_.find(i);
    }

    return i;
}

bool manager::add_uiobject(uiobject* pObj)
{
    if (!pObj)
    {
        gui::out << gui::error << "gui::manager : Adding a null widget." << std::endl;
        return false;
    }

    std::map<std::string, uiobject*>* lNamedList;
    if (pObj->is_virtual())
    {
        if (pObj->get_parent())
        {
            const uint i = get_new_object_id_();
            lObjectList_[i] = pObj;
            pObj->set_id(i);

            return true;
        }
        else
            lNamedList = &lNamedVirtualObjectList_;
    }
    else
        lNamedList = &lNamedObjectList_;

    std::map<std::string, uiobject*>::iterator iterNamedObj = lNamedList->find(pObj->get_name());
    if (iterNamedObj == lNamedList->end())
    {
        const uint i = get_new_object_id_();
        lObjectList_[i] = pObj;
        pObj->set_id(i);

        (*lNamedList)[pObj->get_name()] = pObj;

        if (!pObj->is_virtual())
        {
            frame* pFrame = down_cast<frame>(pObj);
            if (pFrame)
                lFrameList_[i] = pFrame;
        }

        return true;
    }
    else
    {
        gui::out << gui::warning << "gui::manager : "
            << "A" << std::string(pObj->is_virtual() ? " virtual" : "") << " widget with the name \""
            << pObj->get_name() << "\" already exists." << std::endl;
        return false;
    }
}

frame* manager::add_root_frame(std::unique_ptr<frame> pFrame)
{
    frame* pAddedFrame = pFrame.get();
    lRootFrameList_.push_back(std::move(pFrame));

    if (!pAddedFrame->is_virtual())
    {
        frame_renderer* pOldTopLevelRenderer = pAddedFrame->get_top_level_renderer();
        if (pOldTopLevelRenderer != this)
        {
            pOldTopLevelRenderer->notify_rendered_frame(pAddedFrame, false);
            notify_rendered_frame(pAddedFrame, true);
        }
    }

    return pAddedFrame;
}

void manager::remove_uiobject(uiobject* pObj)
{
    if (!pObj) return;

    lObjectList_.erase(pObj->get_id());

    if (!pObj->is_virtual())
        lNamedObjectList_.erase(pObj->get_name());
    else
        lNamedVirtualObjectList_.erase(pObj->get_name());

    if (pMovedObject_ == pObj)
        stop_moving(pObj);

    if (pSizedObject_ == pObj)
        stop_sizing(pObj);
}

void manager::remove_frame(frame* pObj)
{
    if (!pObj) return;

    if (!pObj->is_virtual())
        lFrameList_.erase(pObj->get_id());

    if (pHoveredFrame_ == pObj)
        clear_hovered_frame_();

    if (pFocusedFrame_ == pObj)
        clear_focussed_frame_();
}

std::unique_ptr<frame> manager::remove_root_frame(frame* pFrame)
{
    auto mIter = utils::find_if(lRootFrameList_, [&](auto& pObj) {
        return pObj && pObj->get_id() == pFrame->get_id();
    });

    if (mIter == lRootFrameList_.end())
        return nullptr;

    // NB: the iterator is not removed yet; it will be removed later in update().
    return std::move(*mIter);
}

manager::root_frame_list_view manager::get_root_frames() const
{
    return root_frame_list_view(lRootFrameList_);
}

const uiobject* manager::get_uiobject(uint uiID) const
{
    std::map<uint, uiobject*>::const_iterator mIter = lObjectList_.find(uiID);
    if (mIter != lObjectList_.end())
        return mIter->second;
    else
        return nullptr;
}

uiobject* manager::get_uiobject(uint uiID)
{
    std::map<uint, uiobject*>::iterator mIter = lObjectList_.find(uiID);
    if (mIter != lObjectList_.end())
        return mIter->second;
    else
        return nullptr;
}

std::vector<uiobject*> manager::get_virtual_uiobject_list(const std::string& sNames)
{
    std::vector<uiobject*> lInheritance;
    if (!utils::has_no_content(sNames))
    {
        for (auto sParent : utils::cut(sNames, ","))
        {
            utils::trim(sParent, ' ');
            uiobject* pObj = get_uiobject_by_name(sParent, true);
            if (!pObj)
            {
                bool bNonVirtual = false;
                if (get_uiobject_by_name(sParent))
                    bNonVirtual = true;

                gui::out << gui::warning << "gui::manager : "
                    << "Cannot find inherited object \"" << sParent << "\""
                    << std::string(bNonVirtual ? " (object is not virtual)" : "")
                    << ". Inheritance skipped." << std::endl;

                continue;
            }

            lInheritance.push_back(pObj);
        }
    }

    return lInheritance;
}

const uiobject* manager::get_uiobject_by_name(const std::string& sName, bool bVirtual) const
{
    if (bVirtual)
    {
        std::map<std::string, uiobject*>::const_iterator iter = lNamedVirtualObjectList_.find(sName);
        if (iter != lNamedVirtualObjectList_.end())
            return iter->second;
        else
            return nullptr;
    }
    else
    {
        std::map<std::string, uiobject*>::const_iterator iter = lNamedObjectList_.find(sName);
        if (iter != lNamedObjectList_.end())
            return iter->second;
        else
            return nullptr;
    }
}

uiobject* manager::get_uiobject_by_name(const std::string& sName, bool bVirtual)
{
    if (bVirtual)
    {
        std::map<std::string, uiobject*>::iterator iter = lNamedVirtualObjectList_.find(sName);
        if (iter != lNamedVirtualObjectList_.end())
            return iter->second;
        else
            return nullptr;
    }
    else
    {
        std::map<std::string, uiobject*>::iterator iter = lNamedObjectList_.find(sName);
        if (iter != lNamedObjectList_.end())
            return iter->second;
        else
            return nullptr;
    }
}

sol::state& manager::get_lua()
{
    return *pSol_;
}

const sol::state& manager::get_lua() const
{
    return *pSol_;
}

lua::state& manager::get_luapp()
{
    return *pLua_;
}

const lua::state& manager::get_luapp() const
{
    return *pLua_;
}

void manager::load_addon_toc_(const std::string& sAddOnName, const std::string& sAddOnDirectory)
{
    std::map<std::string, addon>& lAddOns = lAddOnList_[sAddOnDirectory];
    if (lAddOns.find(sAddOnName) == lAddOns.end())
    {
        addon mAddOn;
        mAddOn.bEnabled = true;
        mAddOn.sMainDirectory = utils::cut(sAddOnDirectory, "/").back();
        mAddOn.sDirectory = sAddOnDirectory + "/" + sAddOnName;

        std::string sTOCFile = mAddOn.sDirectory + "/" + sAddOnName + ".toc";
        std::ifstream mFile(sTOCFile);

        if (mFile.is_open())
        {
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
                    std::vector<std::string> lArgs = utils::cut(sLine, ":", 1);
                    if (lArgs.size() == 2)
                    {
                        std::string sKey = lArgs[0];
                        utils::trim(sKey, ' ');
                        std::string sValue = lArgs[1];
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
                    if (sLine.find(".lua") != sLine.npos || sLine.find(".xml") != sLine.npos)
                        mAddOn.lFileList.push_back(mAddOn.sDirectory + "/" + sLine);
                }
            }

            mFile.close();

            if (mAddOn.sName == "")
                gui::out << gui::error << "gui::manager : Missing addon name in " << sTOCFile << "." << std::endl;
            else
                lAddOns[sAddOnName] = mAddOn;
        }
    }
}

void manager::load_addon_files_(addon* pAddOn)
{
    pCurrentAddOn_ = pAddOn;
    for (const auto& sFile : pAddOn->lFileList)
    {
        if (sFile.find(".lua") != sFile.npos)
        {
            try
            {
                pLua_->do_file(sFile);
            }
            catch (const lua::exception& e)
            {
                std::string sError = e.get_description();

                gui::out << gui::error << sError << std::endl;

                event mEvent("LUA_ERROR");
                mEvent.add(sError);
                pEventManager_->fire_event(mEvent);
            }
        }
        else if (sFile.find(".xml") != sFile.npos)
            this->parse_xml_file_(sFile, pAddOn);
    }

    std::string sSavedVariablesFile = "saves/interface/"+pAddOn->sMainDirectory+"/"+pAddOn->sName+".lua";
    if (utils::file_exists(sSavedVariablesFile))
    {
        try
        {
            pLua_->do_file(sSavedVariablesFile);
        }
        catch (const lua::exception& e)
        {
            std::string sError = e.get_description();

            gui::out << gui::error << sError << std::endl;

            event mEvent("LUA_ERROR");
            mEvent.add(sError);
            pEventManager_->fire_event(mEvent);
        }
    }

    event mEvent("ADDON_LOADED");
    mEvent.add(pAddOn->sName);
    pEventManager_->fire_event(mEvent);
}

void manager::load_addon_directory_(const std::string& sDirectory)
{
    for (auto sSubDir : utils::get_directory_list(sDirectory))
        this->load_addon_toc_(sSubDir, sDirectory);

    std::vector<addon*> lCoreAddOnStack;
    std::vector<addon*> lAddOnStack;
    bool bCore = false;

    std::map<std::string, addon>& lAddOns = lAddOnList_[sDirectory];

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
                if (sLine == "Core")
                    bCore = true;
                else
                    bCore = false;
            }
            else
            {
                std::vector<std::string> lArgs = utils::cut(sLine, ":", 1);
                if (lArgs.size() == 2)
                {
                    std::string sKey = lArgs[0];
                    utils::trim(sKey, ' ');
                    std::string sValue = lArgs[1];
                    utils::trim(sValue, ' ');
                    auto iter = lAddOns.find(sKey);
                    if (iter != lAddOns.end())
                    {
                        if (bCore)
                            lCoreAddOnStack.push_back(&iter->second);
                        else
                            lAddOnStack.push_back(&iter->second);

                        if (sValue != "1")
                            iter->second.bEnabled = false;
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
            std::string sSerialized = pLua_->serialize_global(sVariable);
            if (!sSerialized.empty())
                mFile << sSerialized << "\n";
        }
    }
}

void gui_out(const std::string& sMessage)
{
    gui::out << sMessage << std::endl;
}

void manager::create_lua(std::function<void(gui::manager&)> pLuaRegs)
{
    if (pLua_) return;

    pSol_ = std::unique_ptr<sol::state>(new sol::state());
    pSol_->open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::io,
        sol::lib::os, sol::lib::string, sol::lib::debug);

    pLua_ = std::unique_ptr<lua::state>(new lua::state(pSol_->lua_state()));
    pLua_->set_print_error_function(gui_out);

    register_lua_manager_();
    pLua_->reg<lua_virtual_glue>();
    pLua_->reg<lua_uiobject>();
    pLua_->reg<lua_frame>();
    pLua_->reg<lua_focus_frame>();
    pLua_->reg<lua_layered_region>();

    pLua_->reg("set_key_binding",              l_set_key_binding);
    pLua_->reg("create_frame",                 l_create_frame);
    pLua_->reg("delete_frame",                 l_delete_frame);
    pLua_->reg("get_locale",                   l_get_locale);
    pLua_->reg("set_interface_scaling_factor", l_set_interface_scaling_factor);
    pLua_->reg("log",                          l_log);

    pLuaRegs_ = pLuaRegs;
    if (pLuaRegs_)
        pLuaRegs_(*this);
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

        lRootFrameList_.clear();

        lObjectList_.clear();
        lNamedObjectList_.clear();
        lNamedVirtualObjectList_.clear();

        lFrameList_.clear();

        clear_strata_list_();

        lAddOnList_.clear();

        pLua_ = nullptr;
        pSol_ = nullptr;

        pHoveredFrame_ = nullptr;
        bUpdateHoveredFrame_ = false;
        pFocusedFrame_ = nullptr;
        pMovedObject_ = nullptr;
        pSizedObject_ = nullptr;
        pMovedAnchor_ = nullptr;
        bObjectMoved_ = false;
        fMouseMovementX_ = 0.0f;
        fMouseMovementY_ = 0.0f;
        iMovementStartPositionX_ = 0;
        iMovementStartPositionY_ = 0;
        mConstraint_ = constraint::NONE;
        uiResizeStartW_ = 0;
        uiResizeStartH_ = 0;
        bResizeWidth_ = false;
        bResizeHeight_ = false;
        bResizeFromRight_ = false;
        bResizeFromBottom_ = false;

        uiFrameNumber_ = 0;

        lKeyBindingList_.clear();

        bClosed_ = true;
        bLoadingUI_ = false;
        pCurrentAddOn_ = nullptr;
    }
}

void manager::reload_ui()
{
    gui::out << "Closing UI..." << std::endl;
    close_ui();
    gui::out << "Done. Loading UI..." << std::endl;
    load_ui();
    gui::out << "Done." << std::endl;
}

void manager::begin(std::shared_ptr<render_target> pTarget) const
{
    pRenderer_->begin(pTarget);

    float fWidth, fHeight;
    if (pTarget)
    {
        fWidth = pTarget->get_real_width()/fScalingFactor_;
        fHeight = pTarget->get_real_height()/fScalingFactor_;
    }
    else
    {
        fWidth = uiScreenWidth_/fScalingFactor_;
        fHeight = uiScreenHeight_/fScalingFactor_;
    }

    pRenderer_->set_view(matrix4f::view(vector2f(fWidth, fHeight)));
}

void manager::end() const
{
    pRenderer_->end();
}

void manager::render_ui() const
{
    begin();

    if (bEnableCaching_)
    {
        mSprite_.render(0, 0);
    }
    else
    {
        for (const auto& mStrata : lStrataList_)
        {
            render_strata_(mStrata);
        }
    }

    end();
}

void manager::create_caching_render_target_()
{
    try
    {
        if (pRenderTarget_)
            pRenderTarget_->set_dimensions(uiScreenWidth_, uiScreenHeight_);
        else
            pRenderTarget_ = pRenderer_->create_render_target(uiScreenWidth_, uiScreenHeight_);
    }
    catch (const utils::exception& e)
    {
        gui::out << gui::error << "gui::manager : "
            << "Unable to create render_target for GUI caching :\n" << e.get_description() << std::endl;

        bEnableCaching_ = false;
        return;
    }

    mSprite_ = sprite(pRenderer_.get(), pRenderer_->create_material(pRenderTarget_));

    float fScale = 1.0/get_interface_scaling_factor();
    mSprite_.set_dimensions(mSprite_.get_width()*fScale, mSprite_.get_height()*fScale);
}

void manager::create_strata_cache_render_target_(strata& mStrata)
{
    if (mStrata.pRenderTarget)
        mStrata.pRenderTarget->set_dimensions(uiScreenWidth_, uiScreenHeight_);
    else
        mStrata.pRenderTarget = pRenderer_->create_render_target(uiScreenWidth_, uiScreenHeight_);

    mStrata.mSprite = sprite(pRenderer_.get(), pRenderer_->create_material(mStrata.pRenderTarget));

    float fScale = 1.0/fScalingFactor_;
    mStrata.mSprite.set_dimensions(mStrata.mSprite.get_width()*fScale, mStrata.mSprite.get_height()*fScale);
}

bool manager::is_loading_ui() const
{
    return bLoadingUI_;
}

void manager::update(float fDelta)
{
    DEBUG_LOG(" Input...");
    pInputManager_->update(fDelta);

    DEBUG_LOG(" Update anchors...");
    // update anchors for all widgets
    for (auto* pObject : utils::range::value(lObjectList_))
    {
        if (!pObject->is_virtual())
            pObject->update_anchors();
    }

    DEBUG_LOG(" Update widgets...");
    // ... then update logics on main widgets from parent to children.
    for (auto* pObject : get_root_frames())
    {
        if (!pObject->is_virtual())
            pObject->update(fDelta);
    }

    // Removed destroyed frames
    {
        auto mIterRemove = std::remove_if(lRootFrameList_.begin(), lRootFrameList_.end(), [](auto& pObj) {
            return pObj == nullptr;
        });

        lRootFrameList_.erase(mIterRemove, lRootFrameList_.end());
    }

    if (bEnableCaching_)
    {
        DEBUG_LOG(" Redraw strata...");

        try
        {
            bool bRedraw = has_strata_list_changed_();
            for (auto& mStrata : lStrataList_)
            {
                if (mStrata.bRedraw)
                {
                    if (!mStrata.pRenderTarget)
                        create_strata_cache_render_target_(mStrata);

                    if (mStrata.pRenderTarget)
                    {
                        begin(mStrata.pRenderTarget);
                        mStrata.pRenderTarget->clear(color::EMPTY);
                        render_strata_(mStrata);
                        end();
                    }

                    bRedraw = true;
                }

                mStrata.bRedraw = false;
            }

            if (!pRenderTarget_)
                create_caching_render_target_();

            if (bRedraw && pRenderTarget_)
            {
                begin(pRenderTarget_);
                pRenderTarget_->clear(color::EMPTY);

                for (auto& mStrata : lStrataList_)
                {
                    mStrata.mSprite.render(0, 0);
                }

                end();
            }
        }
        catch (const utils::exception& e)
        {
            gui::out << gui::error << "gui::manager : "
                << "Unable to create render_target for strata :\n"
                << e.get_description() << std::endl;

            bEnableCaching_ = false;
        }
    }

    if (has_strata_list_changed_() || bObjectMoved_ ||
        (pInputManager_->get_mouse_dx() != 0.0f) ||
        (pInputManager_->get_mouse_dy() != 0.0f))
    {
        bUpdateHoveredFrame_ = true;
    }

    update_hovered_frame_();

    bObjectMoved_ = false;
    reset_strata_list_changed_flag_();

    if (bFirstIteration_)
    {
        DEBUG_LOG(" Entering world...");
        pEventManager_->fire_event(event("ENTERING_WORLD"));
        bFirstIteration_ = false;
    }

    ++uiFrameNumber_;
    pEventManager_->frame_ended();
}

void manager::clear_hovered_frame_()
{
    pHoveredFrame_ = nullptr;
    pInputManager_->allow_input("WORLD");
}

void manager::set_hovered_frame_(frame* pFrame, int iX, int iY)
{
    if (pHoveredFrame_ && pFrame != pHoveredFrame_)
        pHoveredFrame_->notify_mouse_in_frame(false, iX, iY);

    if (pFrame)
    {
        pHoveredFrame_ = pFrame;
        pHoveredFrame_->notify_mouse_in_frame(true, iX, iY);
        if (pHoveredFrame_->is_world_input_allowed())
            pInputManager_->allow_input("WORLD");
        else
            pInputManager_->block_input("WORLD");
    }
    else
        clear_hovered_frame_();
}

void manager::start_moving(uiobject* pObj, anchor* pAnchor, constraint mConstraint, std::function<void()> pApplyConstraintFunc)
{
    pSizedObject_ = nullptr;
    pMovedObject_ = pObj;
    fMouseMovementX_ = 0.0f;
    fMouseMovementY_ = 0.0f;

    if (pMovedObject_)
    {
        mConstraint_ = mConstraint;
        pApplyConstraintFunc_ = pApplyConstraintFunc;
        if (pAnchor)
        {
            pMovedAnchor_ = pAnchor;
            iMovementStartPositionX_ = pMovedAnchor_->get_abs_offset_x();
            iMovementStartPositionY_ = pMovedAnchor_->get_abs_offset_y();
        }
        else
        {
            pMovedObject_->clear_all_points();
            const quad2i& lBorders = pMovedObject_->get_borders();
            pMovedObject_->set_abs_point(anchor_point::TOPLEFT, "", anchor_point::TOPLEFT, lBorders.top_left());
            pMovedAnchor_ = pMovedObject_->modify_point(anchor_point::TOPLEFT);

            iMovementStartPositionX_ = lBorders.left;
            iMovementStartPositionY_ = lBorders.top;
        }
    }
}

void manager::stop_moving(uiobject* pObj)
{
    if (pMovedObject_ == pObj)
    {
        pMovedObject_ = nullptr;
        pMovedAnchor_ = nullptr;
    }
}

bool manager::is_moving(uiobject* pObj) const
{
    return (pMovedObject_ == pObj);
}

void manager::start_sizing(uiobject* pObj, anchor_point mPoint)
{
    pMovedObject_    = nullptr;
    pSizedObject_    = pObj;
    fMouseMovementX_ = 0.0f;
    fMouseMovementY_ = 0.0f;

    if (pSizedObject_)
    {
        const quad2i& lBorders = pSizedObject_->get_borders();

        anchor_point mOppositePoint = anchor_point::CENTER;
        vector2i mOffset;

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
        pSizedObject_->set_abs_point(mOppositePoint, "", anchor_point::TOPLEFT, mOffset);

        uiResizeStartW_ = pSizedObject_->get_abs_width();
        uiResizeStartH_ = pSizedObject_->get_abs_height();

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

void manager::stop_sizing(uiobject* pObj)
{
    if (pSizedObject_ == pObj)
        pSizedObject_ = nullptr;
}

bool manager::is_sizing(uiobject* pObj) const
{
    return (pSizedObject_ == pObj);
}

int manager::get_movement_x() const
{
    return fMouseMovementX_;
}

int manager::get_movement_y() const
{
    return fMouseMovementY_;
}

void manager::notify_object_moved()
{
    bObjectMoved_ = true;
}

void manager::toggle_caching()
{
    bEnableCaching_ = !bEnableCaching_;

    if (bEnableCaching_)
    {
        for (auto& mStrata : lStrataList_)
            mStrata.bRedraw = true;
    }
}

void manager::enable_caching(bool bEnable)
{
    if (bEnableCaching_ != bEnable)
        toggle_caching();
}

bool manager::is_caching_enabled() const
{
    return bEnableCaching_;
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

void manager::clear_fonts_on_close(bool bClear)
{
    bClearFontsOnClose_ = bClear;
}

void manager::update_hovered_frame_()
{
    if (!bUpdateHoveredFrame_ || !bInputEnabled_)
        return;

    DEBUG_LOG(" Update hovered frame...");
    int iX = pInputManager_->get_mouse_x();
    int iY = pInputManager_->get_mouse_y();

    frame* pHoveredFrame = find_hovered_frame_(iX, iY);
    set_hovered_frame_(pHoveredFrame, iX, iY);

    bUpdateHoveredFrame_ = false;
}

frame* manager::get_hovered_frame()
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

void manager::request_focus(focus_frame* pFocusFrame)
{
    if (pFocusFrame == pFocusedFrame_)
        return;

    if (pFocusedFrame_)
        pFocusedFrame_->notify_focus(false);

    if (pFocusFrame)
    {
        pFocusedFrame_ = pFocusFrame;
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

                if (iter2->second.size() == 0)
                    iter1->second.erase(iter2);

                if (iter1->second.size() == 0)
                    lKeyBindingList_.erase(iter1);
            }
        }
    }
}

int manager::get_highest_level(frame_strata mFrameStrata) const
{
    auto& mStrata = lStrataList_[(uint)mFrameStrata];
    if (!mStrata.lLevelList.empty())
        return mStrata.lLevelList.rbegin()->first;

    return 0;
}

addon* manager::get_current_addon()
{
    return pCurrentAddOn_;
}

void manager::set_current_addon(addon* pAddOn)
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
            catch (const lua::exception& e)
            {
                gui::out << gui::error << "Bound action : " << sKeyName
                    << " : " << e.get_description() << std::endl;
            }
        }
    }
    else if (mEvent.get_name() == "WINDOW_RESIZED")
    {
        // Update internal window size
        uiScreenWidth_ = mEvent.get<uint>(0);
        uiScreenHeight_ = mEvent.get<uint>(1);

        // Update the scaling factor
        set_interface_scaling_factor(fBaseScalingFactor_);

        // Notify all frames anchored to the window edges
        for (auto* pObject : get_root_frames())
        {
            if (!pObject->is_virtual())
            {
                pObject->notify_borders_need_update();
                pObject->notify_renderer_need_redraw();
            }
        }

        notify_object_moved();

        pRenderer_->notify_window_resized(uiScreenWidth_, uiScreenHeight_);

        // Resize caching render targets
        if (pRenderTarget_)
            create_caching_render_target_();

        for (auto& mStrata : lStrataList_)
        {
            if (mStrata.pRenderTarget)
                create_strata_cache_render_target_(mStrata);
        }
    }
    else if (mEvent.get_name() == "MOUSE_MOVED")
    {
        if (pMovedObject_ || pSizedObject_)
        {
            DEBUG_LOG(" Moved object...");
            fMouseMovementX_ += mEvent.get<float>(0);
            fMouseMovementY_ += mEvent.get<float>(1);
        }

        if (pMovedObject_)
        {
            switch (mConstraint_)
            {
                case constraint::NONE :
                    pMovedAnchor_->set_abs_offset(
                        iMovementStartPositionX_ + int(fMouseMovementX_),
                        iMovementStartPositionY_ + int(fMouseMovementY_)
                    );
                    break;
                case constraint::X :
                    pMovedAnchor_->set_abs_offset(
                        iMovementStartPositionX_ + int(fMouseMovementX_),
                        iMovementStartPositionY_
                    );
                    break;
                case constraint::Y :
                    pMovedAnchor_->set_abs_offset(
                        iMovementStartPositionX_,
                        iMovementStartPositionY_ + int(fMouseMovementY_)
                    );
                    break;
                default : break;
            }

            if (pApplyConstraintFunc_)
                pApplyConstraintFunc_();

            pMovedObject_->notify_borders_need_update();
        }
        else if (pSizedObject_)
        {
            if (bResizeWidth_ && bResizeHeight_)
            {
                uint uiWidth;
                if (bResizeFromRight_)
                    uiWidth = std::max(0, int(uiResizeStartW_) + int(fMouseMovementX_));
                else
                    uiWidth = std::max(0, int(uiResizeStartW_) - int(fMouseMovementX_));

                uint uiHeight;
                if (bResizeFromBottom_)
                    uiHeight = std::max(0, int(uiResizeStartH_) + int(fMouseMovementY_));
                else
                    uiHeight = std::max(0, int(uiResizeStartH_) - int(fMouseMovementY_));

                pSizedObject_->set_abs_dimensions(uiWidth, uiHeight);
            }
            else if (bResizeWidth_)
            {
                uint uiWidth;
                if (bResizeFromRight_)
                    uiWidth = std::max(0, int(uiResizeStartW_) + int(fMouseMovementX_));
                else
                    uiWidth = std::max(0, int(uiResizeStartW_) - int(fMouseMovementX_));

                pSizedObject_->set_abs_width(uiWidth);
            }
            else if (bResizeHeight_)
            {
                uint uiHeight;
                if (bResizeFromBottom_)
                    uiHeight = std::max(0, int(uiResizeStartH_) + int(fMouseMovementY_));
                else
                    uiHeight = std::max(0, int(uiResizeStartH_) - int(fMouseMovementY_));

                pSizedObject_->set_abs_height(uiHeight);
            }
        }
    }
}

void manager::print_statistics()
{
    gui::out << "GUI Statistics :" << std::endl;
    gui::out << "    strata redraw percent :" << std::endl;
    for (const auto& mStrata : lStrataList_)
    {
        gui::out << "     - [" << static_cast<uint>(mStrata.mStrata) << "] : "
            << utils::to_string(100.0f*float(mStrata.uiRedrawCount)/float(uiFrameNumber_), 2, 1) << "%" << std::endl;
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
    if (!lObjectList_.empty())
    {
        s << "\n\n######################## UIObjects ########################\n\n########################\n" << std::endl;
        for (const auto* pObject : utils::range::value(lObjectList_))
        {
            if (!pObject->is_virtual() && !pObject->get_parent())
                s << pObject->serialize("") << "\n########################\n" << std::endl;
        }

        s << "\n\n#################### Virtual UIObjects ####################\n\n########################\n" << std::endl;
        for (const auto* pObject : utils::range::value(lObjectList_))
        {
            if (pObject->is_virtual() && !pObject->get_parent())
                s << pObject->serialize("") << "\n########################\n" << std::endl;
        }
    }

    return s.str();
}

const event_manager* manager::get_event_manager() const
{
    return pEventManager_.get();
}

event_manager* manager::get_event_manager()
{
    return pEventManager_.get();
}

const input::manager* manager::get_input_manager() const
{
    return pInputManager_.get();
}

input::manager* manager::get_input_manager()
{
    return pInputManager_.get();
}

const std::string& manager::get_locale() const
{
    return sLocale_;
}
}
}
