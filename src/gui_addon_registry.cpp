#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_eventemitter.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_std.hpp>
#include <lxgui/utils_range.hpp>

#include <sol/state.hpp>

#include <fstream>

namespace
{
    // This should be incremented for each non-backward compatible change
    // to the GUI API, or when new elements are added to the API.
    const char* LXGUI_UI_VERSION = "0001";
}

namespace lxgui {
namespace gui
{

addon_registry::addon_registry(sol::state& mLua, localizer& mLocalizer,
    event_emitter& mEventEmitter, root& mRoot, virtual_root& mVirtualRoot) :
    mLua_(mLua), mLocalizer_(mLocalizer), mEventEmitter_(mEventEmitter),
    mRoot_(mRoot), mVirtualRoot_(mVirtualRoot)
{
}

void addon_registry::load_addon_toc_(const std::string& sAddOnName, const std::string& sAddOnDirectory)
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

    std::string sLine;
    while (std::getline(mFile, sLine))
    {
        utils::replace(sLine, "\r", "");
        if (sLine.empty())
            continue;

        std::string_view sLineView = sLine;

        if (sLineView.size() >= 2 && sLineView[0] == '#' && sLineView[1] == '#')
        {
            sLineView = sLineView.substr(2);
            sLineView = utils::trim(sLineView, ' ');
            auto lArgs = utils::cut_first(sLineView, ":");
            if (!lArgs.first.empty() && !lArgs.second.empty())
            {
                std::string_view sKey = utils::trim(lArgs.first, ' ');
                std::string_view sValue = utils::trim(lArgs.second, ' ');

                if (sKey == "Interface")
                {
                    mAddOn.sUIVersion = sValue;

                    if (mAddOn.sUIVersion == LXGUI_UI_VERSION)
                        mAddOn.bEnabled = true;
                    else
                    {
                        gui::out << gui::warning << "gui::manager : "
                            << "Wrong UI version for \"" << sAddOnName << "\" (got : "
                            << mAddOn.sUIVersion << ", expected : " << LXGUI_UI_VERSION
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
                        sVar = utils::trim(sVar, ' ');
                        if (!utils::has_no_content(sVar))
                            mAddOn.lSavedVariableList.push_back(std::string{sVar});
                    }
                }
            }
        }
        else
        {
            sLineView = utils::trim(sLineView, ' ');
            if (!utils::has_no_content(sLineView))
                mAddOn.lFileList.push_back(mAddOn.sDirectory + "/" + std::string{sLineView});
        }
    }

    if (mAddOn.sName.empty())
        gui::out << gui::error << "gui::manager : Missing addon name in " << sTOCFile << "." << std::endl;
    else
        lAddOns[sAddOnName] = mAddOn;
}

void addon_registry::load_addon_files_(const addon& mAddOn)
{
    mLocalizer_.load_translations(mAddOn.sDirectory);

    pCurrentAddOn_ = &mAddOn;
    for (const auto& sFile : mAddOn.lFileList)
    {
        const std::string sExtension = utils::get_file_extension(sFile);
        if (sExtension == ".lua")
        {
            try
            {
                mLua_.do_file(sFile);
            }
            catch (const sol::error& e)
            {
                std::string sError = e.what();

                gui::out << gui::error << sError << std::endl;

                mEventEmitter_.fire_event("LUA_ERROR", {sError});
            }
        }
        else
        {
            this->parse_layout_file_(sFile, mAddOn);
        }
    }

    std::string sSavedVariablesFile = "saves/interface/"+mAddOn.sMainDirectory+"/"+mAddOn.sName+".lua";
    if (utils::file_exists(sSavedVariablesFile))
    {
        try
        {
            mLua_.do_file(sSavedVariablesFile);
        }
        catch (const sol::error& e)
        {
            std::string sError = e.what();

            gui::out << gui::error << sError << std::endl;

            mEventEmitter_.fire_event("LUA_ERROR", {sError});
       }
    }

    mEventEmitter_.fire_event("ADDON_LOADED", {mAddOn.sName});
}

void addon_registry::load_addon_directory(const std::string& sDirectory)
{
    for (const auto& sSubDir : utils::get_directory_list(sDirectory))
        this->load_addon_toc_(sSubDir, sDirectory);

    std::vector<addon*> lCoreAddOnStack;
    std::vector<addon*> lAddOnStack;
    bool bCore = false;

    auto& lAddOns = lAddOnList_[sDirectory];

    std::ifstream mFile(sDirectory + "/addons.txt");
    if (mFile.is_open())
    {
        std::string sLine;
        while (std::getline(mFile, sLine))
        {
            utils::replace(sLine, "\r", "");
            if (sLine.empty())
                continue;

            std::string_view sLineView = sLine;

            if (sLineView[0] == '#')
            {
                sLineView = sLineView.substr(1);
                sLineView = utils::trim(sLineView, ' ');
                bCore = sLineView == "Core";
            }
            else
            {
                auto lArgs = utils::cut_first(sLineView, ":");
                if (!lArgs.first.empty() && !lArgs.second.empty())
                {
                    std::string_view sKey = utils::trim(lArgs.first, ' ');
                    std::string_view sValue = utils::trim(lArgs.second, ' ');
                    auto iter = lAddOns.find(std::string{sKey});
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
            this->load_addon_files_(*pAddOn);
    }

    for (auto* pAddOn : lAddOnStack)
    {
        if (pAddOn->bEnabled)
            this->load_addon_files_(*pAddOn);
    }

    pCurrentAddOn_ = nullptr;
}

std::string serialize(const std::string& sTab, const sol::object& mValue) noexcept
{
    if (mValue.is<double>())
    {
        return utils::to_string(mValue.as<double>());
    }
    else if (mValue.is<int>())
    {
        return utils::to_string(mValue.as<int>());
    }
    else if (mValue.is<std::string>())
    {
        return "\"" + utils::to_string(mValue.as<std::string>()) + "\"";
    }
    else if (mValue.is<sol::table>())
    {
        std::string sResult;
        sResult += "{";

        std::string sContent;
        sol::table mTable = mValue.as<sol::table>();
        for (const auto& mKeyValue : mTable)
        {
            sContent += sTab + "    [" + serialize("", mKeyValue.first) + "] = "
                + serialize(sTab + "    ", mKeyValue.second) + ",\n";
        }

        if (!sContent.empty())
            sResult += "\n" + sContent + sTab;

        sResult += "}";
        return sResult;
    }

    return "nil";
}

std::string serialize_global(sol::state& mLua, const std::string& sVariable) noexcept
{
    sol::object mValue = mLua.globals()[sVariable];
    return serialize("", mValue);
}

void addon_registry::save_variables() const
{
    for (const auto& mDirectory : lAddOnList_)
    {
        for (const auto& mAddOn : utils::range::value(mDirectory.second))
            save_variables_(mAddOn);
    }
}

void addon_registry::save_variables_(const addon& mAddOn) const noexcept
{
    if (!mAddOn.lSavedVariableList.empty())
    {
        if (!utils::make_directory("saves/interface/"+mAddOn.sMainDirectory))
        {
            gui::out << gui::error << "gui::addon_registry : "
                "unable to create directory 'saves/interface/" <<
                mAddOn.sMainDirectory << "'" << std::endl;
            return;
        }

        std::ofstream mFile("saves/interface/"+mAddOn.sMainDirectory+"/"+mAddOn.sName+".lua");
        for (const auto& sVariable : mAddOn.lSavedVariableList)
        {
            std::string sSerialized = serialize_global(mLua_, sVariable);
            if (!sSerialized.empty())
                mFile << sSerialized << "\n";
        }
    }
}

const addon* addon_registry::get_current_addon()
{
    return pCurrentAddOn_;
}

void addon_registry::set_current_addon(const addon* pAddOn)
{
    pCurrentAddOn_ = pAddOn;
}

}
}
