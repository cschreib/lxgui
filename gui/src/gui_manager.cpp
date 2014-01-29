#include "lxgui/gui_manager.hpp"

#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/input.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <fstream>
#include <sstream>

namespace gui
{
int l_set_key_binding(lua_State* pLua);
int l_create_frame(lua_State* pLua);
int l_delete_frame(lua_State* pLua);
int l_get_locale(lua_State* pLua);
int l_log(lua_State* pLua);

manager::manager(const input::handler& mInputHandler, const std::string& sLocale,
    uint uiScreenWidth, uint uiScreenHeight, utils::refptr<manager_impl> pImpl) :
    event_receiver(nullptr), sUIVersion_("0001"),
    uiScreenWidth_(uiScreenWidth), uiScreenHeight_(uiScreenHeight),
    bClearFontsOnClose_(true), pLua_(nullptr), pLuaRegs_(nullptr), bClosed_(true),
    bLoadingUI_(false), bFirstIteration_(true), bInputEnabled_(true),
    pInputManager_(new input::manager(mInputHandler)),
    pCurrentAddOn_(nullptr), bBuildStrataList_(false), bObjectMoved_(false),
    pOveredFrame_(nullptr), bUpdateOveredFrame_(false), pFocusedFrame_(nullptr),
    pMovedObject_(nullptr), pSizedObject_(nullptr), fMouseMovementX_(0.0f),
    fMouseMovementY_(0.0f), pMovedAnchor_(nullptr), iMovementStartPositionX_(0),
    iMovementStartPositionY_(0), mConstraint_(CONSTRAINT_NONE), uiResizeStartW_(0),
    uiResizeStartH_(0), bResizeWidth_(false), bResizeHeight_(false), bResizeFromRight_(false),
    bResizeFromBottom_(false), uiFrameNumber_(0), bEnableCaching_(true),
    pRenderTarget_(nullptr), sLocale_(sLocale), pImpl_(pImpl)
{
    pEventManager_ = utils::refptr<event_manager>(new event_manager());
    event_receiver::pEventManager_ = pEventManager_.get();
    pInputManager_->register_event_manager(pEventManager_);
    register_event("KEY_PRESSED");
    pImpl_->set_parent(this);
}

manager::~manager()
{
    close_ui();

    pEventManager_ = nullptr;
    event_receiver::pEventManager_ = nullptr;
}

utils::wptr<manager_impl> manager::get_impl()
{
    return pImpl_;
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

    std::string::const_iterator iterName;
    foreach (iterName, sName)
    {
        char c = *iterName;
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

uiobject* manager::create_uiobject(const std::string& sClassName)
{
    if (sClassName == "Frame")
        return new frame(this);
    else if (sClassName == "FocusFrame")
        return new focus_frame(this);
    else
    {
        auto iterFrame = lCustomFrameList_.find(sClassName);
        if (iterFrame != lCustomFrameList_.end())
            return (*iterFrame->second)(this);

        auto iterRegion = lCustomRegionList_.find(sClassName);
        if (iterRegion != lCustomRegionList_.end())
            return (*iterRegion->second)(this);

        gui::out << gui::warning << "gui::manager : Unknown uiobject class : \"" << sClassName << "\"." << std::endl;
        return nullptr;
    }
}

frame* manager::create_frame(const std::string& sClassName)
{
    if (sClassName == "Frame")
        return new frame(this);
    else if (sClassName == "FocusFrame")
        return new focus_frame(this);
    else
    {
        auto iterFrame = lCustomFrameList_.find(sClassName);
        if (iterFrame != lCustomFrameList_.end())
            return (*iterFrame->second)(this);

        gui::out << gui::warning << "gui::manager : Unknown Frame class : \"" << sClassName << "\"." << std::endl;
        return nullptr;
    }
}

frame* manager::create_frame(const std::string& sClassName, const std::string& sName,
                            frame* pParent, const std::string& sInheritance)
{
    if (!check_uiobject_name(sName))
        return nullptr;

    frame* pNewFrame = create_frame(sClassName);
    if (!pNewFrame)
        return nullptr;

    pNewFrame->set_parent(pParent);
    pNewFrame->set_name(sName);

    if (get_uiobject_by_name(pNewFrame->get_name()))
    {
        gui::out << gui::error << "gui::manager : "
            << "An object with the name \"" << pNewFrame->get_name() << "\" already exists." << std::endl;
        delete pNewFrame;
        return nullptr;
    }

    add_uiobject(pNewFrame);
    pNewFrame->create_glue();

    if (pParent)
    {
        pParent->add_child(pNewFrame);
        pNewFrame->set_level(pParent->get_frame_level() + 1);
    }
    else
        pNewFrame->set_level(0);

    if (!utils::has_no_content(sInheritance))
    {
        std::vector<std::string> lObjects = utils::cut(sInheritance, ",");
        std::vector<std::string>::iterator iter;
        foreach (iter, lObjects)
        {
            utils::trim(*iter, ' ');
            uiobject* pObj = get_uiobject_by_name(*iter, true);
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
                        << ") cannot inherit from \"" << *iter << "\" (" << pObj->get_object_type()
                        << "). Inheritance skipped." << std::endl;
                }
            }
            else
            {
                gui::out << gui::warning << "gui::manager : "
                    << "Cannot find inherited object \"" << *iter << "\". Inheritance skipped." << std::endl;
            }
        }
    }

    pNewFrame->set_newly_created();
    pNewFrame->notify_loaded();

    return pNewFrame;
}

layered_region* manager::create_layered_region(const std::string& sClassName)
{
    auto iterRegion = lCustomRegionList_.find(sClassName);
    if (iterRegion != lCustomRegionList_.end())
        return (*iterRegion->second)(this);

    gui::out << gui::warning << "gui::manager : Unknown layered_region class : \"" << sClassName << "\"." << std::endl;
    return nullptr;
}

utils::refptr<sprite> manager::create_sprite(utils::refptr<material> pMat) const
{
    utils::refptr<sprite> pSprite = pImpl_->create_sprite(pMat);
    if (pSprite)
        return pSprite;
    else
        return utils::refptr<sprite>(new sprite(this, pMat));
}

utils::refptr<sprite> manager::create_sprite(utils::refptr<material> pMat, float fWidth, float fHeight) const
{
    utils::refptr<sprite> pSprite = pImpl_->create_sprite(pMat, fWidth, fHeight);
    if (pSprite)
        return pSprite;
    else
        return utils::refptr<sprite>(new sprite(this, pMat, fWidth, fHeight));
}

utils::refptr<sprite> manager::create_sprite(utils::refptr<material> pMat,
    float fU, float fV, float fWidth, float fHeight) const
{
    utils::refptr<sprite> pSprite = pImpl_->create_sprite(pMat, fU, fV, fWidth, fHeight);
    if (pSprite)
        return pSprite;
    else
        return utils::refptr<sprite>(new sprite(this, pMat, fU, fV, fWidth, fHeight));
}

utils::refptr<material> manager::create_material(const std::string& sFileName, filter mFilter) const
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

bool manager::add_uiobject(uiobject* pObj)
{
    std::map<std::string, uiobject*>* lNamedList;
    if (pObj->is_virtual())
    {
        if (pObj->get_parent())
        {
            uint i = 0;
            std::map<uint, uiobject*>::iterator iterObj = lObjectList_.find(i);
            while (iterObj != lObjectList_.end())
            {
                ++i;
                iterObj = lObjectList_.find(i);
            }
            lObjectList_[i] = pObj;
            pObj->set_id(i);

            return true;
        }
        else
            lNamedList = &lNamedVirtualObjectList_;
    }
    else
        lNamedList = &lNamedObjectList_;

    if (pObj)
    {
        std::map<std::string, uiobject*>::iterator iterNamedObj = lNamedList->find(pObj->get_name());
        if (iterNamedObj == lNamedList->end())
        {
            uint i = 0;
            std::map<uint, uiobject*>::iterator iterObj = lObjectList_.find(i);
            while (iterObj != lObjectList_.end())
            {
                ++i;
                iterObj = lObjectList_.find(i);
            }

            lObjectList_[i] = pObj;
            (*lNamedList)[pObj->get_name()] = pObj;
            pObj->set_id(i);

            if (!pObj->is_virtual())
            {
                if (!pObj->get_parent())
                {
                    lMainObjectList_[i] = pObj;
                    fire_build_strata_list();
                }

                frame* pFrame = dynamic_cast<frame*>(pObj);
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
    else
    {
        gui::out << gui::error << "gui::manager : Adding a null widget." << std::endl;
        return false;
    }
}

void manager::remove_uiobject(uiobject* pObj)
{
    if (!pObj)
        return;

    lObjectList_.erase(pObj->get_id());

    if (!pObj->is_virtual())
    {
        lNamedObjectList_.erase(pObj->get_name());

        if (!pObj->get_parent())
            lMainObjectList_.erase(pObj->get_id());

        frame* pFrame = dynamic_cast<frame*>(pObj);
        if (pFrame)
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

    lRemovedObjectList_.push_back(pObj);
}

void manager::remove_uiobject_(uiobject* pObj)
{
    if (!pObj)
        return;

    delete pObj;
}

void manager::notify_object_has_parent(uiobject* pObj)
{
    lMainObjectList_.erase(pObj->get_id());
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

utils::wptr<lua::state> manager::get_lua()
{
    return pLua_;
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
                            std::vector<std::string> lVariables = utils::cut(sValue, ",");
                            std::vector<std::string>::iterator iterVar;
                            foreach (iterVar, lVariables)
                            {
                                utils::trim(*iterVar, ' ');
                                if (!utils::has_no_content(*iterVar))
                                    mAddOn.lSavedVariableList.push_back(*iterVar);
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
    std::vector<std::string>::iterator iterFile;
    foreach (iterFile, pAddOn->lFileList)
    {
        if (iterFile->find(".lua") != iterFile->npos)
        {
            try { pLua_->do_file(*iterFile); }
            catch (lua::exception& e)
            {
                std::string sError = e.get_description();

                gui::out << gui::error << sError << std::endl;

                event mEvent("LUA_ERROR");
                mEvent.add(sError);
                pEventManager_->fire_event(mEvent);
            }
        }
        else if (iterFile->find(".xml") != iterFile->npos)
            this->parse_xml_file_(*iterFile, pAddOn);
    }

    std::string sSavedVariablesFile = "saves/interface/"+pAddOn->sMainDirectory+"/"+pAddOn->sName+".lua";
    if (utils::file_exists(sSavedVariablesFile))
    {
        try { pLua_->do_file(sSavedVariablesFile); }
        catch (lua::exception& e)
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
    std::vector<std::string> lDirs = utils::get_directory_list(sDirectory);

    std::vector<std::string>::iterator iter;
    foreach (iter, lDirs)
        this->load_addon_toc_(*iter, sDirectory);

    std::vector<addon*> lCoreAddOnStack;
    std::vector<addon*> lAddOnStack;
    bool bCore = false;

    std::map<std::string, addon>& lAddOns = lAddOnList_[sDirectory];

    std::ifstream mFile(sDirectory + "/addons.txt");
    if (mFile.is_open())
    {
        while (!mFile.eof())
        {
            std::string sLine; getline(mFile, sLine);
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
                    auto iter2 = lAddOns.find(sKey);
                    if (iter2 != lAddOns.end())
                    {
                        if (bCore)
                            lCoreAddOnStack.push_back(&iter2->second);
                        else
                            lAddOnStack.push_back(&iter2->second);

                        if (sValue != "1")
                            iter2->second.bEnabled = false;
                    }
                }
            }
        }
        mFile.close();
    }

    std::vector<addon*>::iterator iterAddon;
    foreach (iterAddon, lCoreAddOnStack)
    {
        if ((*iterAddon)->bEnabled)
            this->load_addon_files_(*iterAddon);
    }

    foreach (iterAddon, lAddOnStack)
    {
        if ((*iterAddon)->bEnabled)
            this->load_addon_files_(*iterAddon);
    }

    pCurrentAddOn_ = nullptr;
}

void manager::save_variables_(addon* pAddOn)
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
        std::vector<std::string>::iterator iterVariable;
        foreach (iterVariable, pAddOn->lSavedVariableList)
        {
            std::string sVariable = pLua_->serialize_global(*iterVariable);
            if (!sVariable.empty())
                mFile << sVariable << "\n";
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
        pLua_ = utils::refptr<lua::state>(new lua::state());
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

        std::vector<std::string>::iterator iterDirectory;
        foreach (iterDirectory, lGUIDirectoryList_)
            this->load_addon_directory_(*iterDirectory);

        if (bEnableCaching_)
        {
            // Get the active strata list
            std::map<uint, frame*>::iterator iterFrame;
            foreach (iterFrame, lFrameList_)
            {
                frame* pFrame = iterFrame->second;
                if (!pFrame->is_manually_rendered())
                    lStrataList_[pFrame->get_frame_strata()];
            }

            // Create their render target
            std::map<frame_strata, strata>::iterator iterStrata;
            foreach (iterStrata, lStrataList_)
                create_strata_render_target_(iterStrata->second);
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
        std::vector<std::string>::iterator iterDirectory;
        foreach (iterDirectory, lGUIDirectoryList_)
        {
            std::map<std::string, addon>::iterator iterAddOn;
            foreach (iterAddOn, lAddOnList_[*iterDirectory])
                save_variables_(&iterAddOn->second);
        }

        std::map<uint, uiobject*>::iterator iterObj;
        foreach (iterObj, lMainObjectList_)
            delete iterObj->second;

        lMainObjectList_.clear();
        lObjectList_.clear();
        lNamedObjectList_.clear();

        std::vector<uiobject*>::iterator iterRemoved;
        foreach (iterRemoved, lRemovedObjectList_)
            remove_uiobject_(*iterRemoved);

        lRemovedObjectList_.clear();

        std::map<std::string, uiobject*>::iterator iterVirtual;
        foreach (iterVirtual, lNamedVirtualObjectList_)
            delete iterVirtual->second;

        lNamedVirtualObjectList_.clear();

        lFrameList_.clear();

        std::map<frame_strata, strata>::iterator iterStrata;
        foreach (iterStrata, lStrataList_)
        {
            iterStrata->second.lLevelList.clear();
            iterStrata->second.bRedraw = true;
        }

        lAddOnList_.clear();

        lStrataList_.clear();
        bBuildStrataList_ = true;

        pLua_ = nullptr;

        pOveredFrame_ = nullptr;
        bUpdateOveredFrame_ = false;
        pFocusedFrame_ = nullptr;
        pMovedObject_ = nullptr;
        pSizedObject_ = nullptr;
        pMovedAnchor_ = nullptr;
        bObjectMoved_ = false;
        fMouseMovementX_ = 0.0f;
        fMouseMovementY_ = 0.0f;
        iMovementStartPositionX_ = 0;
        iMovementStartPositionY_ = 0;
        mConstraint_ = CONSTRAINT_NONE;
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
        if (pSprite_)
        {
            begin();
            pSprite_->render(0, 0);
            end();
        }
    }
    else
    {
        begin();
        std::map<frame_strata, strata>::const_iterator iterStrata;
        foreach (iterStrata, lStrataList_)
        {
            const strata& mStrata = iterStrata->second;
            std::map<int, level>::const_iterator iterLevel;
            foreach (iterLevel, mStrata.lLevelList)
            {
                const level& mLevel = iterLevel->second;

                std::vector<frame*>::const_iterator iterFrame;
                foreach (iterFrame, mLevel.lFrameList)
                {
                    frame* pFrame = *iterFrame;
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
        catch (utils::exception& e)
        {
            gui::out << gui::error << "gui::manager : "
                << "Unable to create render_target for GUI caching :\n" << e.get_description() << std::endl;

            bEnableCaching_ = false;
            return;
        }

        pSprite_ = create_sprite(create_material(pRenderTarget_));
    }

    if (!mStrata.pRenderTarget)
    {
        try
        {
            mStrata.pRenderTarget = create_render_target(uiScreenWidth_, uiScreenHeight_);
        }
        catch (utils::exception& e)
        {
            gui::out << gui::error << "gui::manager : "
                << "Unable to create render_target for strata " << mStrata.uiID << " :\n"
                << e.get_description() << std::endl;

            bEnableCaching_ = false;
            return;
        }

        mStrata.pSprite = create_sprite(create_material(mStrata.pRenderTarget));
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

        std::map<int, level>::const_iterator iterLevel;
        foreach (iterLevel, mStrata.lLevelList)
        {
            const level& mLevel = iterLevel->second;

            std::vector<frame*>::const_iterator iterFrame;
            foreach (iterFrame, mLevel.lFrameList)
            {
                frame* pFrame = *iterFrame;
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
    //#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    if (lRemovedObjectList_.size() != 0)
    {
        DEBUG_LOG(" Removing uiobjects...");
        std::vector<uiobject*>::iterator iterRemoved;
        foreach (iterRemoved, lRemovedObjectList_)
            remove_uiobject_(*iterRemoved);

        lRemovedObjectList_.clear();
    }

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
            case CONSTRAINT_NONE :
                pMovedAnchor_->set_abs_offset(
                    iMovementStartPositionX_ + int(fMouseMovementX_),
                    iMovementStartPositionY_ + int(fMouseMovementY_)
                );
                break;
            case CONSTRAINT_X :
                pMovedAnchor_->set_abs_offset(
                    iMovementStartPositionX_ + int(fMouseMovementX_),
                    iMovementStartPositionY_
                );
                break;
            case CONSTRAINT_Y :
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
    std::map<uint, uiobject*>::iterator iterObj;
    foreach (iterObj, lObjectList_)
    {
        if (!iterObj->second->is_virtual())
            iterObj->second->update_anchors();
    }

    DEBUG_LOG(" Update widgets...");
    // ... then update logics on main widgets from parent to children.
    foreach (iterObj, lMainObjectList_)
    {
        if (!iterObj->second->is_virtual())
            iterObj->second->update(fDelta);
    }

    if (bBuildStrataList_)
    {
        DEBUG_LOG(" Build strata...");
        std::map<frame_strata, strata>::iterator iterStrata;
        foreach (iterStrata, lStrataList_)
        {
            iterStrata->second.lLevelList.clear();
            iterStrata->second.bRedraw = true;
        }

        std::map<uint, frame*>::iterator iterFrame;
        foreach (iterFrame, lFrameList_)
        {
            frame* pFrame = iterFrame->second;
            if (!pFrame->is_manually_rendered())
            {
                strata& mStrata = lStrataList_[pFrame->get_frame_strata()];
                mStrata.lLevelList[pFrame->get_frame_level()].lFrameList.push_back(pFrame);

                mStrata.uiID = pFrame->get_frame_strata();
            }
        }
    }

    if (bEnableCaching_)
    {
        DEBUG_LOG(" Redraw strata...");
        bool bRedraw = false;
        std::map<frame_strata, strata>::iterator iterStrata;
        foreach (iterStrata, lStrataList_)
        {
            strata& mStrata = iterStrata->second;

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

            foreach (iterStrata, lStrataList_)
            {
                if (iterStrata->second.pSprite)
                    iterStrata->second.pSprite->render(0, 0);
            }

            end();
        }
    }

    if (bBuildStrataList_ || bObjectMoved_ ||
        (pInputManager_->get_mouse_raw_dx() != 0.0f) ||
        (pInputManager_->get_mouse_raw_dy() != 0.0f))
        bUpdateOveredFrame_ = true;

    if (bUpdateOveredFrame_ && bInputEnabled_)
    {
        DEBUG_LOG(" Update overed frame...");
        int iX = pInputManager_->get_mouse_x();
        int iY = pInputManager_->get_mouse_y();

        frame* pOveredFrame = nullptr;

        std::map<frame_strata, strata>::const_iterator iterStrata = lStrataList_.end();
        while (iterStrata != lStrataList_.begin() && !pOveredFrame)
        {
            --iterStrata;
            const strata& mStrata = iterStrata->second;

            std::map<int, level>::const_iterator iterLevel = mStrata.lLevelList.end();
            while (iterLevel != mStrata.lLevelList.begin() && !pOveredFrame)
            {
                --iterLevel;
                const level& mLevel = iterLevel->second;

                std::vector<frame*>::const_iterator iterFrame;
                foreach (iterFrame, mLevel.lFrameList)
                {
                    frame* pFrame = *iterFrame;
                    if (pFrame->is_mouse_enabled() && pFrame->is_visible() && pFrame->is_in_frame(iX, iY))
                    {
                        pOveredFrame = pFrame;
                        break;
                    }
                }
            }
        }

        set_overed_frame_(pOveredFrame, iX, iY);

        bUpdateOveredFrame_ = false;
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

    if (pFrame != pOveredFrame_)
    {
        if (pOveredFrame_)
            pOveredFrame_->notify_mouse_in_frame(false, iX, iY);

        pOveredFrame_ = pFrame;
    }

    if (pOveredFrame_)
        pOveredFrame_->notify_mouse_in_frame(true, iX, iY);
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
            pMovedObject_->set_abs_point(ANCHOR_TOPLEFT, "", ANCHOR_TOPLEFT, lBorders.top_left());
            pMovedAnchor_ = pMovedObject_->modify_point(ANCHOR_TOPLEFT);

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

        anchor_point mOppositePoint = ANCHOR_CENTER;
        vector2i mOffset;

        switch (mPoint)
        {
            case ANCHOR_TOPLEFT :
            case ANCHOR_TOP :
                mOppositePoint = ANCHOR_BOTTOMRIGHT;
                mOffset = lBorders.bottom_right();
                bResizeFromRight_  = false;
                bResizeFromBottom_ = false;
                break;
            case ANCHOR_TOPRIGHT :
            case ANCHOR_RIGHT :
                mOppositePoint = ANCHOR_BOTTOMLEFT;
                mOffset = lBorders.bottom_left();
                bResizeFromRight_  = true;
                bResizeFromBottom_ = false;
                break;
            case ANCHOR_BOTTOMRIGHT :
            case ANCHOR_BOTTOM :
                mOppositePoint = ANCHOR_TOPLEFT;
                mOffset = lBorders.top_left();
                bResizeFromRight_  = true;
                bResizeFromBottom_ = true;
                break;
            case ANCHOR_BOTTOMLEFT :
            case ANCHOR_LEFT :
                mOppositePoint = ANCHOR_TOPRIGHT;
                mOffset = lBorders.top_right();
                bResizeFromRight_  = false;
                bResizeFromBottom_ = true;
                break;
            case ANCHOR_CENTER :
                gui::out << gui::error << "gui::manager : "
                    << "Cannot resize \"" <<  pObj->get_name() << "\" from its center." << std::endl;
                pSizedObject_ = nullptr;
                return;
        }

        pSizedObject_->clear_all_points();
        pSizedObject_->set_abs_point(mOppositePoint, "", ANCHOR_TOPLEFT, mOffset);

        uiResizeStartW_ = pSizedObject_->get_abs_width();
        uiResizeStartH_ = pSizedObject_->get_abs_height();

        if (mPoint == ANCHOR_LEFT || mPoint == ANCHOR_RIGHT)
        {
            bResizeWidth_  = true;
            bResizeHeight_ = false;
        }
        else if (mPoint == ANCHOR_TOP || mPoint == ANCHOR_BOTTOM)
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
        std::map<frame_strata, strata>::iterator iterStrata;
        foreach (iterStrata, lStrataList_)
            iterStrata->second.bRedraw = true;
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
        bUpdateOveredFrame_ = true;

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
    return pOveredFrame_;
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

void manager::set_key_binding(uint uiKey, const std::string& sLuaString)
{
    lKeyBindingList_[uiKey][0][0] = sLuaString;
}

void manager::set_key_binding(uint uiKey, uint uiModifier, const std::string& sLuaString)
{
    lKeyBindingList_[uiKey][uiModifier][0] = sLuaString;
}

void manager::set_key_binding(uint uiKey, uint uiModifier1, uint uiModifier2, const std::string& sLuaString)
{
    lKeyBindingList_[uiKey][uiModifier1][uiModifier2] = sLuaString;
}

void manager::remove_key_binding(uint uiKey)
{
    std::map<uint, std::map<uint, std::map<uint, std::string> > >::iterator iter1 = lKeyBindingList_.find(uiKey);
    if (iter1 != lKeyBindingList_.end())
    {
        std::map<uint, std::map<uint, std::string> >::iterator iter2 = iter1->second.find(0);
        if (iter2 != iter1->second.end())
        {
            std::map<uint, std::string>::iterator iter3 = iter2->second.find(0);
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

void manager::remove_key_binding(uint uiKey, uint uiModifier)
{
    std::map<uint, std::map<uint, std::map<uint, std::string> > >::iterator iter1 = lKeyBindingList_.find(uiKey);
    if (iter1 != lKeyBindingList_.end())
    {
        std::map<uint, std::map<uint, std::string> >::iterator iter2 = iter1->second.find(uiModifier);
        if (iter2 != iter1->second.end())
        {
            std::map<uint, std::string>::iterator iter3 = iter2->second.find(0);
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

void manager::remove_key_binding(uint uiKey, uint uiModifier1, uint uiModifier2)
{
    std::map<uint, std::map<uint, std::map<uint, std::string> > >::iterator iter1 = lKeyBindingList_.find(uiKey);
    if (iter1 != lKeyBindingList_.end())
    {
        std::map<uint, std::map<uint, std::string> >::iterator iter2 = iter1->second.find(uiModifier1);
        if (iter2 != iter1->second.end())
        {
            std::map<uint, std::string>::iterator iter3 = iter2->second.find(uiModifier2);
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
        uint uiKey = mEvent.get<uint>(0);
        bool bCaptured = false;

        std::map<uint, std::map<uint, std::map<uint, std::string> > >::const_iterator iter1 = lKeyBindingList_.find(uiKey);
        std::map<uint, std::map<uint, std::string> >::const_iterator iter2;
        std::map<uint, std::string>::const_iterator iter3;

        if (iter1 != lKeyBindingList_.end())
        {
            foreach (iter2, iter1->second)
            {
                if (iter2->first == input::key::K_UNASSIGNED || !pInputManager_->key_is_down((input::key::code)iter2->first))
                    continue;

                foreach (iter3, iter2->second)
                {
                    if (iter3->first == input::key::K_UNASSIGNED || !pInputManager_->key_is_down((input::key::code)iter3->first))
                        continue;

                    try { pLua_->do_string(iter3->second); }
                    catch (lua::exception& e)
                    {
                        gui::out << gui::error << "Binded action : "+pInputManager_->get_key_name(
                                (input::key::code)uiKey, (input::key::code)iter2->first, (input::key::code)iter3->first
                            ) << " : " << e.get_description() << std::endl;
                    }

                    bCaptured = true;
                    break;
                }

                if (bCaptured)
                    break;

                iter3 = iter2->second.find(input::key::K_UNASSIGNED);
                if (iter3 != iter2->second.end())
                {
                    try { pLua_->do_string(iter3->second); }
                    catch (lua::exception& e)
                    {
                        gui::out << gui::error << "Binded action : "+pInputManager_->get_key_name(
                                (input::key::code)uiKey, (input::key::code)iter2->first
                            ) << " : " << e.get_description() << std::endl;
                    }
                }
            }

            iter2 = iter1->second.find(input::key::K_UNASSIGNED);
            if (iter2 != iter1->second.end())
            {
                iter3 = iter2->second.find(input::key::K_UNASSIGNED);
                if (iter3 != iter2->second.end())
                {
                    try { pLua_->do_string(iter3->second); }
                    catch (lua::exception& e)
                    {
                        gui::out << gui::error << "Binded action : "+pInputManager_->get_key_name((input::key::code)uiKey)
                            << e.get_description() << std::endl;
                    }
                }
            }
        }
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
    std::map<frame_strata, strata>::const_iterator iterStrata;
    foreach (iterStrata, lStrataList_)
    {
        const strata& mStrata = iterStrata->second;
        gui::out << "     - [" << mStrata.uiID << "] : "
            << utils::to_string(100.0f*float(mStrata.uiRedrawCount)/float(uiFrameNumber_), 2, 1) << "%" << std::endl;
    }
}

std::string manager::print_ui() const
{
    std::stringstream s;

    if (lAddOnList_.size() >= 1)
    {
        s << "\n\n######################## Loaded addons ########################\n" << std::endl;
        std::map<std::string, std::map<std::string, addon>>::const_iterator iterDirectory;
        foreach (iterDirectory, lAddOnList_)
        {
            s << "# Directory : " << iterDirectory->first << "\n|-###" << std::endl;
            std::map<std::string, addon>::const_iterator iterAdd;
            foreach (iterAdd, iterDirectory->second)
            {
                if (iterAdd->second.bEnabled)
                    s << "|   # " << iterAdd->first << std::endl;
            }
            s << "|-###\n#" << std::endl;
        }
    }
    if (lObjectList_.size() >= 1)
    {
        s << "\n\n######################## UIObjects ########################\n\n########################\n" << std::endl;
        std::map<uint, uiobject*>::const_iterator iterObj;
        foreach (iterObj, lObjectList_)
        {
            if (!iterObj->second->is_virtual() && !iterObj->second->get_parent())
                s << iterObj->second->serialize("") << "\n########################\n" << std::endl;
        }

        s << "\n\n#################### Virtual UIObjects ####################\n\n########################\n" << std::endl;
        foreach (iterObj, lObjectList_)
        {
            if (iterObj->second->is_virtual() && !iterObj->second->get_parent())
                s << iterObj->second->serialize("") << "\n########################\n" << std::endl;
        }
    }

    return s.str();
}

utils::wptr<event_manager> manager::get_event_manager()
{
    return pEventManager_;
}

utils::wptr<input::manager> manager::get_input_manager()
{
    return pInputManager_;
}

const std::string& manager::get_locale() const
{
    return sLocale_;
}

manager_impl::manager_impl()
{
}

manager_impl::~manager_impl()
{
}

utils::refptr<sprite> manager_impl::create_sprite(utils::refptr<material> pMat) const
{
    return nullptr;
}

utils::refptr<sprite> manager_impl::create_sprite(utils::refptr<material> pMat, float fWidth, float fHeight) const
{
    return nullptr;
}

utils::refptr<sprite> manager_impl::create_sprite(utils::refptr<material> pMat,
    float fU, float fV, float fWidth, float fHeight) const
{
    return nullptr;
}

void manager_impl::set_parent(manager* pParent)
{
    pParent_ = pParent;
}

strata::strata() : uiID(uint(-1)), bRedraw(true), uiRedrawCount(0u)
{
}

strata::~strata()
{
}
}
