#include "lxgui/gui_manager.hpp"

#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/input.hpp"

#include <lxgui/luapp_exception.hpp>
#include <lxgui/luapp_state.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <fstream>
#include <sstream>

namespace lxgui {
namespace gui
{
int l_set_key_binding(lua_State* pLua);
int l_create_frame(lua_State* pLua);
int l_delete_frame(lua_State* pLua);
int l_get_locale(lua_State* pLua);
int l_log(lua_State* pLua);

manager::manager(std::unique_ptr<input::source_impl> pInputSource, const std::string& sLocale,
    uint uiScreenWidth, uint uiScreenHeight, std::unique_ptr<renderer_impl> pImpl) :
    event_receiver(nullptr), uiScreenWidth_(uiScreenWidth), uiScreenHeight_(uiScreenHeight),
    pInputManager_(new input::manager(std::move(pInputSource))), sLocale_(sLocale),
    pImpl_(std::move(pImpl))
{
    pEventManager_ = std::unique_ptr<event_manager>(new event_manager());
    event_receiver::set_event_manager(pEventManager_.get());
    pInputManager_->register_event_manager(pEventManager_.get());
    register_event("KEY_PRESSED");
    register_event("WINDOW_RESIZED");
    pImpl_->set_parent(this);
}

manager::~manager()
{
    close_ui();

    // Notify event receiver that event manager is about to be destroyed
    event_receiver::set_event_manager(nullptr);
}

renderer_impl* manager::get_renderer()
{
    return pImpl_.get();
}

const renderer_impl* manager::get_renderer() const
{
    return pImpl_.get();
}

uint manager::get_screen_width() const
{
    return uiScreenWidth_;
}

uint manager::get_screen_height() const
{
    return uiScreenHeight_;
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

std::unique_ptr<frame> manager::create_frame(const std::string& sClassName, const std::string& sName,
                                             const std::string& sInheritance)
{
    if (!check_uiobject_name(sName))
        return nullptr;

    std::unique_ptr<frame> pNewFrame = create_frame(sClassName);
    if (!pNewFrame)
        return nullptr;

    pNewFrame->set_name(sName);

    if (!add_uiobject(pNewFrame.get()))
    {
        gui::out << gui::error << "gui::manager : "
            << "An object with the name \"" << pNewFrame->get_name() << "\" already exists." << std::endl;
        return nullptr;
    }

    pNewFrame->create_glue();
    pNewFrame->set_level(0);

    if (!utils::has_no_content(sInheritance))
    {
        for (auto sParent : utils::cut(sInheritance, ","))
        {
            utils::trim(sParent, ' ');
            uiobject* pObj = get_uiobject_by_name(sParent, true);
            if (pObj)
            {
                if (pNewFrame->is_object_type(pObj->get_object_type()))
                {
                    // Inherit from the other frame
                    pNewFrame->copy_from(pObj);
                }
                else
                {
                    gui::out << gui::warning << "gui::manager : "
                        << "\"" << pNewFrame->get_name() << "\" (" << pNewFrame->get_object_type()
                        << ") cannot inherit from \"" << sParent << "\" (" << pObj->get_object_type()
                        << "). Inheritance skipped." << std::endl;
                }
            }
            else
            {
                gui::out << gui::warning << "gui::manager : "
                    << "Cannot find inherited object \"" << sParent << "\". Inheritance skipped." << std::endl;
            }
        }
    }

    pNewFrame->set_newly_created();
    pNewFrame->notify_loaded();

    return pNewFrame;
}

std::unique_ptr<layered_region> manager::create_layered_region(const std::string& sClassName)
{
    auto iterRegion = lCustomRegionList_.find(sClassName);
    if (iterRegion != lCustomRegionList_.end())
        return std::unique_ptr<layered_region>((*iterRegion->second)(this));

    gui::out << gui::warning << "gui::manager : Unknown layered_region class : \"" << sClassName << "\"." << std::endl;
    return nullptr;
}

sprite manager::create_sprite(utils::refptr<material> pMat) const
{
    return sprite(this, pMat);
}

sprite manager::create_sprite(utils::refptr<material> pMat, float fWidth, float fHeight) const
{
    return sprite(this, pMat, fWidth, fHeight);
}

sprite manager::create_sprite(utils::refptr<material> pMat,
    float fU, float fV, float fWidth, float fHeight) const
{
    return sprite(this, pMat, fU, fV, fWidth, fHeight);
}

utils::refptr<material> manager::create_material(const std::string& sFileName, material::filter mFilter) const
{
    return pImpl_->create_material(sFileName, mFilter);
}

utils::refptr<material> manager::create_material(const color& mColor) const
{
    return pImpl_->create_material(mColor);
}

utils::refptr<material> manager::create_material(utils::refptr<render_target> pRenderTarget) const
{
    return pImpl_->create_material(pRenderTarget);
}

utils::refptr<render_target> manager::create_render_target(uint uiWidth, uint uiHeight) const
{
    return pImpl_->create_render_target(uiWidth, uiHeight);
}

utils::refptr<font> manager::create_font(const std::string& sFontFile, uint uiSize) const
{
    return pImpl_->create_font(sFontFile, uiSize);
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
            frame* pFrame = pObj->down_cast<frame>();
            if (pFrame)
                lFrameList_[i] = pFrame;
        }

        return true;
    }
    else
    {
        gui::out << gui::warning << "gui::manager : "
            << "A " << std::string(pObj->is_virtual() ? "virtual" : "") << " widget with the name \""
            << pObj->get_name() << "\" already exists." << std::endl;
        return false;
    }
}

uiobject* manager::add_root_uiobject(std::unique_ptr<uiobject> pObj)
{
    uiobject* pAddedObj = pObj.release();
    lMainObjectList_[pAddedObj->get_id()] = pAddedObj;
    if (!pAddedObj->is_virtual())
        fire_build_strata_list();

    return pAddedObj;
}

void manager::remove_uiobject(uiobject* pObj)
{
    if (!pObj)
        return;

    lObjectList_.erase(pObj->get_id());

    if (!pObj->is_virtual())
    {
        lNamedObjectList_.erase(pObj->get_name());

        // NB: cannot use down_cast() here, as the frame destructor
        // may have already been called.
        if (pObj->is_object_type<frame>())
            lFrameList_.erase(pObj->get_id());
    }
    else
        lNamedVirtualObjectList_.erase(pObj->get_name());

    if (!pObj->is_manually_rendered())
        fire_build_strata_list();

    if (pMovedObject_ == pObj)
        stop_moving(pObj);

    if (pSizedObject_ == pObj)
        stop_sizing(pObj);
}

std::unique_ptr<uiobject> manager::remove_root_uiobject(uiobject* pObj)
{
    auto iter = lMainObjectList_.find(pObj->get_id());
    if (iter == lMainObjectList_.end())
        return nullptr;

    std::unique_ptr<uiobject> pRemovedObject(iter->second);
    lMainObjectList_.erase(iter);
    return pRemovedObject;
}

const uiobject* manager::get_uiobject(uint uiID) const
{
    std::map<uint, uiobject*>::const_iterator iter = lObjectList_.find(uiID);
    if (iter != lObjectList_.end())
        return iter->second;
    else
        return nullptr;
}

uiobject* manager::get_uiobject(uint uiID)
{
    std::map<uint, uiobject*>::iterator iter = lObjectList_.find(uiID);
    if (iter != lObjectList_.end())
        return iter->second;
    else
        return nullptr;
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

lua::state* manager::get_lua()
{
    return pLua_.get();
}

const lua::state* manager::get_lua() const
{
    return pLua_.get();
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

void manager::create_lua(std::function<void()> pLuaRegs)
{
    if (!pLua_)
    {
        pLua_ = std::unique_ptr<lua::state>(new lua::state());
        pLua_->set_print_error_function(gui_out);

        register_lua_manager_();
        pLua_->reg<lua_virtual_glue>();
        pLua_->reg<lua_uiobject>();
        pLua_->reg<lua_frame>();
        pLua_->reg<lua_focus_frame>();
        pLua_->reg<lua_layered_region>();

        pLua_->reg("set_key_binding", l_set_key_binding);
        pLua_->reg("create_frame",    l_create_frame);
        pLua_->reg("delete_frame",    l_delete_frame);
        pLua_->reg("get_locale",      l_get_locale);
        pLua_->reg("log",             l_log);

        pLuaRegs_ = pLuaRegs;
        if (pLuaRegs_)
            pLuaRegs_();
    }
}

void manager::read_files()
{
    if (bClosed_)
    {
        bLoadingUI_ = true;

        for (const auto& sDirectory : lGUIDirectoryList_)
            this->load_addon_directory_(sDirectory);

        if (bEnableCaching_)
        {
            // Get the active strata list
            for (auto* pFrame : utils::range::value(lFrameList_))
            {
                if (!pFrame->is_manually_rendered())
                    lStrataList_[pFrame->get_frame_strata()];
            }

            // Create their render target
            for (auto& mStrata : utils::range::value(lStrataList_))
                create_strata_render_target_(mStrata);
        }

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

        for (auto* pObject : utils::range::value(lMainObjectList_))
            delete pObject;

        lMainObjectList_.clear();
        lObjectList_.clear();
        lNamedObjectList_.clear();
        lNamedVirtualObjectList_.clear();

        lFrameList_.clear();

        for (auto& mStrata : utils::range::value(lStrataList_))
        {
            mStrata.lLevelList.clear();
            mStrata.bRedraw = true;
        }

        lAddOnList_.clear();

        lStrataList_.clear();
        bBuildStrataList_ = true;

        pLua_ = nullptr;

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

void manager::render_ui() const
{
    if (bEnableCaching_)
    {
        begin();
        mSprite_.render(0, 0);
        end();
    }
    else
    {
        begin();
        for (const auto& mStrata : utils::range::value(lStrataList_))
        {
            for (const auto& mLevel : utils::range::value(mStrata.lLevelList))
            {
                for (auto* pFrame : mLevel.lFrameList)
                {
                    if (!pFrame->is_newly_created())
                        pFrame->render();
                }
            }

            ++mStrata.uiRedrawCount;
        }
        end();
    }
}

void manager::render_quad(const quad& mQuad) const
{
    pImpl_->render_quad(mQuad);
}

void manager::render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const
{
    pImpl_->render_quads(mQuad, lQuadList);
}

void manager::create_strata_render_target(frame_strata mframe_strata)
{
    create_strata_render_target_(lStrataList_[mframe_strata]);
}

void manager::create_strata_render_target_(strata& mStrata)
{
    if (!pRenderTarget_)
    {
        try
        {
            pRenderTarget_ = create_render_target(uiScreenWidth_, uiScreenHeight_);
        }
        catch (const utils::exception& e)
        {
            gui::out << gui::error << "gui::manager : "
                << "Unable to create render_target for GUI caching :\n" << e.get_description() << std::endl;

            bEnableCaching_ = false;
            return;
        }

        mSprite_ = create_sprite(create_material(pRenderTarget_));
    }

    if (!mStrata.pRenderTarget)
    {
        try
        {
            mStrata.pRenderTarget = create_render_target(uiScreenWidth_, uiScreenHeight_);
        }
        catch (const utils::exception& e)
        {
            gui::out << gui::error << "gui::manager : "
                << "Unable to create render_target for strata " << static_cast<uint>(mStrata.mStrata) << " :\n"
                << e.get_description() << std::endl;

            bEnableCaching_ = false;
            return;
        }

        mStrata.mSprite = create_sprite(create_material(mStrata.pRenderTarget));
    }
}

void manager::render_strata_(strata& mStrata)
{
    if (!mStrata.pRenderTarget)
        create_strata_render_target_(mStrata);

    if (mStrata.pRenderTarget)
    {
        begin(mStrata.pRenderTarget);
        mStrata.pRenderTarget->clear(color::EMPTY);

        for (const auto& mLevel : utils::range::value(mStrata.lLevelList))
        {
            for (auto* pFrame : mLevel.lFrameList)
            {
                if (!pFrame->is_newly_created())
                    pFrame->render();
            }
        }

        end();

        ++mStrata.uiRedrawCount;
    }
}

bool manager::is_loading_ui() const
{
    return bLoadingUI_;
}

void manager::fire_build_strata_list()
{
    bBuildStrataList_ = true;
}

void manager::update(float fDelta)
{
    // #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    DEBUG_LOG(" Input...");
    pInputManager_->update(fDelta);

    if (pMovedObject_ || pSizedObject_)
    {
        DEBUG_LOG(" Moved object...");
        fMouseMovementX_ += pInputManager_->get_mouse_raw_dx();
        fMouseMovementY_ += pInputManager_->get_mouse_raw_dy();
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

        pMovedObject_->fire_update_borders();
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

    DEBUG_LOG(" Update anchors...");
    // update anchors for all widgets
    for (auto* pObject : utils::range::value(lObjectList_))
    {
        if (!pObject->is_virtual())
            pObject->update_anchors();
    }

    DEBUG_LOG(" Update widgets...");
    // ... then update logics on main widgets from parent to children.
    for (auto* pObject : utils::range::value(lMainObjectList_))
    {
        if (!pObject->is_virtual())
            pObject->update(fDelta);
    }

    if (bBuildStrataList_)
    {
        DEBUG_LOG(" Build strata...");
        for (auto& mStrata : utils::range::value(lStrataList_))
        {
            mStrata.lLevelList.clear();
            mStrata.bRedraw = true;
        }

        for (auto* pFrame : utils::range::value(lFrameList_))
        {
            if (pFrame->is_manually_rendered()) continue;

            strata& mStrata = lStrataList_[pFrame->get_frame_strata()];
            mStrata.mStrata = pFrame->get_frame_strata();
            mStrata.lLevelList[pFrame->get_level()].lFrameList.push_back(pFrame);
        }
    }

    if (bEnableCaching_)
    {
        DEBUG_LOG(" Redraw strata...");
        bool bRedraw = false;
        for (auto& mStrata : utils::range::value(lStrataList_))
        {
            if (mStrata.bRedraw)
            {
                render_strata_(mStrata);
                bRedraw = true;
            }

            mStrata.bRedraw = false;
        }

        if (bRedraw && pRenderTarget_)
        {
            begin(pRenderTarget_);
            pRenderTarget_->clear(color::EMPTY);

            for (auto& mStrata : utils::range::value(lStrataList_))
            {
                mStrata.mSprite.render(0, 0);
            }

            end();
        }
    }

    if (bBuildStrataList_ || bObjectMoved_ ||
        (pInputManager_->get_mouse_raw_dx() != 0.0f) ||
        (pInputManager_->get_mouse_raw_dy() != 0.0f))
        bUpdateHoveredFrame_ = true;

    if (bUpdateHoveredFrame_ && bInputEnabled_)
    {
        DEBUG_LOG(" Update overed frame...");
        int iX = pInputManager_->get_mouse_x();
        int iY = pInputManager_->get_mouse_y();

        frame* pHoveredFrame = nullptr;

        // Iterate through the frames in reverse order from rendering (frame on top goes first)
        for (const auto& mStrata : utils::range::reverse_value(lStrataList_))
        {
            for (const auto& mLevel : utils::range::reverse_value(mStrata.lLevelList))
            {
                for (auto* pFrame : utils::range::reverse(mLevel.lFrameList))
                {
                    if (pFrame->is_mouse_enabled() && pFrame->is_visible() && pFrame->is_in_frame(iX, iY))
                    {
                        pHoveredFrame = pFrame;
                        break;
                    }
                }

                if (pHoveredFrame) break;

            }

            if (pHoveredFrame) break;
        }

        set_overed_frame_(pHoveredFrame, iX, iY);

        bUpdateHoveredFrame_ = false;
    }

    bObjectMoved_ = false;
    bBuildStrataList_ = false;

    if (bFirstIteration_)
    {
        DEBUG_LOG(" Entering world...");
        pEventManager_->fire_event(event("ENTERING_WORLD"));
        bFirstIteration_ = false;
    }

    ++uiFrameNumber_;
    pEventManager_->frame_ended();
}

void manager::set_overed_frame_(frame* pFrame, int iX, int iY)
{
    if (pFrame && !pFrame->is_world_input_allowed())
        pInputManager_->block_input("WORLD");
    else
        pInputManager_->allow_input("WORLD");

    if (pFrame != pHoveredFrame_)
    {
        if (pHoveredFrame_)
            pHoveredFrame_->notify_mouse_in_frame(false, iX, iY);

        pHoveredFrame_ = pFrame;
    }

    if (pHoveredFrame_)
        pHoveredFrame_->notify_mouse_in_frame(true, iX, iY);
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

void manager::fire_redraw(frame_strata mStrata) const
{
    std::map<frame_strata, strata>::const_iterator iter = lStrataList_.find(mStrata);
    if (iter != lStrataList_.end())
        iter->second.bRedraw = true;
}

void manager::toggle_caching()
{
    bEnableCaching_ = !bEnableCaching_;

    if (bEnableCaching_)
    {
        for (auto& mStrata : utils::range::value(lStrataList_))
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
            pInputManager_->set_focus(true, pFocusedFrame_);
    }
    else
    {
        set_overed_frame_(nullptr);

        if (pFocusedFrame_)
            pInputManager_->set_focus(false);
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

const frame* manager::get_overed_frame() const
{
    return pHoveredFrame_;
}

void manager::request_focus(focus_frame* pFocusFrame)
{
    if (pFocusFrame == pFocusedFrame_)
        return;

    if (pFocusedFrame_)
        pFocusedFrame_->notify_focus(false);

    pFocusedFrame_ = pFocusFrame;

    if (pFocusedFrame_)
    {
        pFocusedFrame_->notify_focus(true);
        pInputManager_->set_focus(true, pFocusedFrame_);
    }
    else
        pInputManager_->set_focus(false);
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
    std::map<frame_strata, strata>::const_iterator iterStrata = lStrataList_.find(mFrameStrata);
    if (iterStrata != lStrataList_.end())
    {
        if (!iterStrata->second.lLevelList.empty())
        {
            std::map<int, level>::const_iterator iterLevel = iterStrata->second.lLevelList.end();
            --iterLevel;
            return iterLevel->first;
        }
    }

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
        uiScreenWidth_ = mEvent.get<uint>(0);
        uiScreenHeight_ = mEvent.get<uint>(1);

        for (auto* pObject : utils::range::value(lMainObjectList_))
        {
            if (!pObject->is_virtual())
                pObject->fire_update_dimensions();
        }

        pImpl_->notify_window_resized(uiScreenWidth_, uiScreenHeight_);
    }
}

void manager::begin(utils::refptr<render_target> pTarget) const
{
    pImpl_->begin(pTarget);
}

void manager::end() const
{
    pImpl_->end();
}

void manager::print_statistics()
{
    gui::out << "GUI Statistics :" << std::endl;
    gui::out << "    strata redraw percent :" << std::endl;
    for (const auto& mStrata : utils::range::value(lStrataList_))
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

void renderer_impl::set_parent(manager* pParent)
{
    pParent_ = pParent;
}

void renderer_impl::notify_window_resized(uint uiNewWidth, uint uiNewHeight)
{
}
}
}
