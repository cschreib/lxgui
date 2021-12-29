#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_eventmanager.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_std.hpp>
#include <lxgui/utils_range.hpp>

#include <sol/state.hpp>

#include <fstream>
#include <sstream>

namespace lxgui {
namespace gui
{

addon_registry::addon_registry(sol::state& mLua, localizer& mLocalizer,
    event_manager& mEventManager, uiroot& mRoot, virtual_uiroot& mVirtualRoot) :
    mLua_(mLua), mLocalizer_(mLocalizer), mEventManager_(mEventManager),
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

                event mEvent("LUA_ERROR");
                mEvent.add(sError);
                mEventManager_.fire_event(mEvent);
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

            event mEvent("LUA_ERROR");
            mEvent.add(sError);
            mEventManager_.fire_event(mEvent);
       }
    }

    event mEvent("ADDON_LOADED");
    mEvent.add(mAddOn.sName);
    mEventManager_.fire_event(mEvent);
}

void addon_registry::load_addon_directory(const std::string& sDirectory)
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
