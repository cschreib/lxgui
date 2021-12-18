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
* For the Lua/XML API, please go to the
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
    pInputManager_->register_event_manager(this);

    mScreenDimensions_ = pInputManager_->get_window_dimensions();

    set_interface_scaling_factor(1.0f);

    pLocalizer_ = std::unique_ptr<localizer>(new localizer());

    // NB: cannot call register_event() here, as observable_from_this()
    // is not yet fully initialised! This is done in create_lua() instead.
}

manager::~manager()
{
    close_ui();
}

vector2f manager::get_target_dimensions() const
{
    return vector2f(mScreenDimensions_)/get_interface_scaling_factor();
}

void manager::set_interface_scaling_factor(float fScalingFactor)
{
    float fFullScalingFactor = fScalingFactor*pInputManager_->get_interface_scaling_factor_hint();

    if (fFullScalingFactor == fScalingFactor_) return;

    fBaseScalingFactor_ = fScalingFactor;
    fScalingFactor_ = fFullScalingFactor;

    pInputManager_->set_interface_scaling_factor(fScalingFactor_);

    for (auto& mFrame : get_root_frames())
        mFrame.notify_scaling_factor_updated();

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

utils::observer_ptr<frame> manager::create_root_frame_(
    const std::string& sClassName, const std::string& sName,
    bool bVirtual, const std::vector<utils::observer_ptr<const uiobject>>& lInheritance)
{
    if (!check_uiobject_name(sName))
        return nullptr;

    auto pNewFrame = create_frame(sClassName);
    if (!pNewFrame)
        return nullptr;

    pNewFrame->set_name_(sName);

    if (bVirtual)
        pNewFrame->set_virtual();

    if (!pNewFrame->is_virtual())
        notify_rendered_frame(pNewFrame, true);

    if (!add_uiobject(pNewFrame))
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

    return add_root_frame(std::move(pNewFrame));
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

    std::unordered_map<std::string, utils::observer_ptr<uiobject>>* lNamedList = nullptr;
    if (pObj->is_virtual())
    {
        if (pObj->get_parent())
        {
            // Virtual children are not recorded in the named list, as they
            // cannot be inherited from directly, and won't appear in the UI.
            return true;
        }

        lNamedList = &lNamedVirtualObjectList_;
    }
    else
        lNamedList = &lNamedObjectList_;

    auto iterNamedObj = lNamedList->find(pObj->get_name());
    if (iterNamedObj != lNamedList->end())
    {
        gui::out << gui::warning << "gui::manager : "
            << "A" << std::string(pObj->is_virtual() ? " virtual" : "") << " widget with the name \""
            << pObj->get_name() << "\" already exists." << std::endl;
        return false;
    }

    (*lNamedList)[pObj->get_name()] = pObj;

    return true;
}

utils::observer_ptr<frame> manager::add_root_frame(utils::owner_ptr<frame> pFrame)
{
    utils::observer_ptr<frame> pAddedFrame = pFrame;
    lRootFrameList_.push_back(std::move(pFrame));

    if (!pAddedFrame->is_virtual())
    {
        utils::observer_ptr<frame_renderer> pOldTopLevelRenderer = pAddedFrame->get_top_level_renderer();
        if (pOldTopLevelRenderer.get() != this)
        {
            pOldTopLevelRenderer->notify_rendered_frame(pAddedFrame, false);
            notify_rendered_frame(pAddedFrame, true);
        }
    }

    return pAddedFrame;
}

void manager::remove_uiobject(const utils::observer_ptr<uiobject>& pObj)
{
    uiobject* pObjRaw = pObj.get();
    if (!pObjRaw) return;

    if (!pObjRaw->is_virtual())
        lNamedObjectList_.erase(pObjRaw->get_name());
    else
        lNamedVirtualObjectList_.erase(pObjRaw->get_name());

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

utils::owner_ptr<frame> manager::remove_root_frame(
    const utils::observer_ptr<frame>& pFrame)
{
    frame* pFrameRaw = pFrame.get();
    if (!pFrameRaw)
        return nullptr;

    auto mIter = utils::find_if(lRootFrameList_, [&](const auto& pObj)
    {
        return pObj.get() == pFrameRaw;
    });

    if (mIter == lRootFrameList_.end())
        return nullptr;

    // NB: the iterator is not removed yet; it will be removed later in update().
    return std::move(*mIter);
}

manager::root_frame_list_view manager::get_root_frames()
{
    return root_frame_list_view(lRootFrameList_);
}

manager::const_root_frame_list_view manager::get_root_frames() const
{
    return const_root_frame_list_view(lRootFrameList_);
}

std::vector<utils::observer_ptr<const uiobject>> manager::get_virtual_uiobject_list(const std::string& sNames) const
{
    std::vector<utils::observer_ptr<const uiobject>> lInheritance;
    if (!utils::has_no_content(sNames))
    {
        for (auto sParent : utils::cut(sNames, ","))
        {
            utils::trim(sParent, ' ');
            utils::observer_ptr<const uiobject> pObj = get_uiobject_by_name(sParent, true);
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

            lInheritance.push_back(std::move(pObj));
        }
    }

    return lInheritance;
}

utils::observer_ptr<const uiobject> manager::get_uiobject_by_name(
    const std::string& sName, bool bVirtual) const
{
    if (bVirtual)
    {
        auto iter = lNamedVirtualObjectList_.find(sName);
        if (iter != lNamedVirtualObjectList_.end())
            return iter->second;
        else
            return nullptr;
    }
    else
    {
        auto iter = lNamedObjectList_.find(sName);
        if (iter != lNamedObjectList_.end())
            return iter->second;
        else
            return nullptr;
    }
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

        lRootFrameList_.clear();

        lNamedObjectList_.clear();
        lNamedVirtualObjectList_.clear();

        clear_strata_list_();

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

        uiFrameNumber_ = 0;

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
        mView = vector2f(mScreenDimensions_)/fScalingFactor_;

    pRenderer_->set_view(matrix4f::view(mView));
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
        pRenderer_->render_quad(mScreenQuad_);
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
            pRenderTarget_->set_dimensions(mScreenDimensions_);
        else
            pRenderTarget_ = pRenderer_->create_render_target(mScreenDimensions_);
    }
    catch (const utils::exception& e)
    {
        gui::out << gui::error << "gui::manager : "
            << "Unable to create render_target for GUI caching :\n" << e.get_description() << std::endl;

        bEnableCaching_ = false;
        return;
    }

    vector2f mScaledDimensions = vector2f(mScreenDimensions_)/get_interface_scaling_factor();

    mScreenQuad_.mat = pRenderer_->create_material(pRenderTarget_);
    mScreenQuad_.v[0].pos = vector2f::ZERO;
    mScreenQuad_.v[1].pos = vector2f(mScaledDimensions.x, 0);
    mScreenQuad_.v[2].pos = mScaledDimensions;
    mScreenQuad_.v[3].pos = vector2f(0, mScaledDimensions.y);

    mScreenQuad_.v[0].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(0, 0), true);
    mScreenQuad_.v[1].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(1, 0), true);
    mScreenQuad_.v[2].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(1, 1), true);
    mScreenQuad_.v[3].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(0, 1), true);
}

void manager::create_strata_cache_render_target_(strata& mStrata)
{
    if (mStrata.pRenderTarget)
        mStrata.pRenderTarget->set_dimensions(mScreenDimensions_);
    else
        mStrata.pRenderTarget = pRenderer_->create_render_target(mScreenDimensions_);

    vector2f mScaledDimensions = vector2f(mScreenDimensions_)/get_interface_scaling_factor();

    mStrata.mQuad.mat = pRenderer_->create_material(mStrata.pRenderTarget);
    mStrata.mQuad.v[0].pos = vector2f::ZERO;
    mStrata.mQuad.v[1].pos = vector2f(mScaledDimensions.x, 0);
    mStrata.mQuad.v[2].pos = mScaledDimensions;
    mStrata.mQuad.v[3].pos = vector2f(0, mScaledDimensions.y);

    mStrata.mQuad.v[0].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(0, 0), true);
    mStrata.mQuad.v[1].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(1, 0), true);
    mStrata.mQuad.v[2].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(1, 1), true);
    mStrata.mQuad.v[3].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(0, 1), true);
}

bool manager::is_loading_ui() const
{
    return bLoadingUI_;
}

bool manager::is_loaded() const
{
    return !bClosed_;
}

template<typename T>
void remove_null(T& lList)
{
    auto mIterRemove = std::remove_if(lList.begin(), lList.end(), [](auto& pObj)
    {
        return pObj == nullptr;
    });

    lList.erase(mIterRemove, lList.end());
}

void manager::update(float fDelta)
{
    bUpdating_ = true;

    DEBUG_LOG(" Input...");
    pInputManager_->update(fDelta);

    DEBUG_LOG(" Update widgets...");
    // ... then update logics on main widgets from parent to children.
    for (auto& mFrame : get_root_frames())
    {
        if (!mFrame.is_virtual())
            mFrame.update(fDelta);
    }

    // Removed destroyed frames
    remove_null(lRootFrameList_);

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
                    pRenderer_->render_quad(mStrata.mQuad);
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
        pInputManager_->get_mouse_delta() != vector2f::ZERO)
    {
        bUpdateHoveredFrame_ = true;
    }

    update_hovered_frame_();

    bObjectMoved_ = false;
    reset_strata_list_changed_flag_();

    if (bFirstIteration_)
    {
        DEBUG_LOG(" Entering world...");
        fire_event(event("ENTERING_WORLD"));
        bFirstIteration_ = false;
    }

    ++uiFrameNumber_;
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
    const auto mMousePos = pInputManager_->get_mouse_position();

    utils::observer_ptr<frame> pHoveredFrame = find_hovered_frame_(mMousePos);
    set_hovered_frame_(std::move(pHoveredFrame), mMousePos);

    bUpdateHoveredFrame_ = false;
}

const utils::observer_ptr<frame>& manager::get_hovered_frame()
{
    update_hovered_frame_();
    return pHoveredFrame_;
}

void manager::notify_hovered_frame_dirty() const
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

int manager::get_highest_level(frame_strata mFrameStrata) const
{
    auto& mStrata = lStrataList_[(uint)mFrameStrata];
    if (!mStrata.lLevelList.empty())
        return mStrata.lLevelList.rbegin()->first;

    return 0;
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
        // Update internal window size
        mScreenDimensions_ = vector2ui(mEvent.get<uint>(0), mEvent.get<uint>(1));

        // Update the scaling factor
        set_interface_scaling_factor(fBaseScalingFactor_);

        // Notify all frames anchored to the window edges
        for (auto& mFrame : get_root_frames())
        {
            if (!mFrame.is_virtual())
            {
                mFrame.notify_borders_need_update();
                mFrame.notify_renderer_need_redraw();
            }
        }

        notify_object_moved();

        pRenderer_->notify_window_resized(mScreenDimensions_);

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

void manager::print_statistics()
{
    gui::out << "GUI Statistics :" << std::endl;
    gui::out << "    strata redraw percent :" << std::endl;
    for (uint uiStrata = 0u; uiStrata < lStrataList_.size(); ++uiStrata)
    {
        const float fRedrawFraction = float(lStrataList_[uiStrata].uiRedrawCount)/float(uiFrameNumber_);
        gui::out << "     - [" << uiStrata << "] : "
            << utils::to_string(100.0f*fRedrawFraction) << "%" << std::endl;
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
    for (const auto& mFrame : get_root_frames())
    {
        s << mFrame.serialize("") << "\n########################\n" << std::endl;
    }

    s << "\n\n#################### Virtual UIObjects ####################\n\n########################\n" << std::endl;
    for (const auto& pObject : utils::range::value(lNamedVirtualObjectList_))
    {
        if (pObject)
            s << pObject->serialize("") << "\n########################\n" << std::endl;
    }

    return s.str();
}

}
}
