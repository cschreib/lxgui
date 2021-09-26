#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_exception.hpp"
#include <lxgui/utils_variant.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_string.hpp>

#include <fmt/args.h>
#include <sol/state.hpp>
#include <cstring>
#include <cstdlib>
#include <functional>

namespace lxgui {
namespace gui
{

std::string to_upper(std::string sStr)
{
    for (char& cChar : sStr)
        cChar = static_cast<char>(std::toupper(cChar));

    return sStr;
}

std::string to_lower(std::string sStr)
{
    for (char& cChar : sStr)
        cChar = static_cast<char>(std::tolower(cChar));

    return sStr;
}

std::vector<std::string> get_default_languages()
{
    // First try parsing the LANGUAGE environment variable.
    // This is the best, because it lets the user specify a list of languages
    // in descending priority, so if a translation is unavailable in their
    // primary language, they may still get another match which would be
    // better for them than the default enUS (e.g., a French person could
    // prefer to fall back on a Spanish translation rather than English).
    const char* csLanguage = std::getenv("LANGUAGE");
    if (csLanguage && std::strlen(csLanguage) > 0)
    {
        std::vector<std::string> lLanguages = utils::cut(std::string{csLanguage}, ":");
        std::vector<std::string> lOutput;
        for (auto& sLanguage : lLanguages)
        {
            utils::replace(sLanguage, "_", "");
            if (sLanguage.size() == 4)
                lOutput.push_back(sLanguage);
        }

        if (!lOutput.empty())
            return lOutput;
    }

#if defined(LXGUI_PLATFORM_WINDOWS)
    // If LANGUAGE is not specified, on Windows, try OS-specific function.

#endif

    // If LANGUAGE is not specified or empty, try LANG.
    const char* csLang = std::getenv("LANG");
    if (csLang && std::strlen(csLang) > 0)
    {
        std::string sLang{csLang};
        auto uiPos1 = sLang.find_first_of(".@");
        if (uiPos1)
            sLang = sLang.substr(0, uiPos1);

        utils::replace(sLang, "_", "");
        if (sLang.size() == 4)
            return {sLang};
    }

    return {"enUS"};
}

localizer::localizer() : lLanguages_(get_default_languages())
{
    try {
        // Try to set locale to system default
        mLocale_ = std::locale("");
    } catch (const std::exception& mException) {
        gui::out << gui::error << "gui::locale : " << mException.what() << std::endl;
    }

    mLua_.open_libraries(
        sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::io,
        sol::lib::os, sol::lib::string, sol::lib::debug
    );

    // Give the translation Lua state the localize_string and format_string function, so
    // it can call it recursively as needed, but not the other functions
    // which could load more translation strings.
    mLua_.set_function("localize_string", [&](const std::string& sKey, sol::variadic_args mVArgs)
    {
        return localize(sKey, mVArgs);
    });
    mLua_.set_function("format_string", [&](const std::string& sKey, sol::variadic_args mVArgs)
    {
        return format_string(sKey, mVArgs);
    });
}

void localizer::set_locale(std::locale mLocale)
{
    if (mLocale_ == mLocale)
        return;

    mLocale_ = std::move(mLocale);
    clear_translations();
}

void localizer::set_preferred_languages(const std::vector<std::string>& lLanguages)
{
    for (const auto& sLanguage : lLanguages)
    {
        if (sLanguage.size() != 4)
            throw gui::exception("gui::localizer", "language code must have exactly 4 characters");
    }

    lLanguages_ = lLanguages;
    clear_translations();
}

const std::locale& localizer::get_locale() const
{
    return mLocale_;
}

const std::vector<std::string>& localizer::get_preferred_languages() const
{
    return lLanguages_;
}

void localizer::load_translations(const std::string& sFolderPath)
{
    // First, look for an exact match
    for (const std::string& sLanguage : lLanguages_)
    {
        std::string sLanguageFile = sFolderPath + "/" + sLanguage + ".lua";
        if (utils::file_exists(sLanguageFile))
        {
            load_translation_file(sLanguageFile);
            return;
        }
    }

    // If no exact match found, look for an approximate match (ignore region)
    const auto lFiles = utils::get_file_list(sFolderPath, false, "lua");
    for (const std::string& sLanguage : lLanguages_)
    {
        auto mIter = std::find_if(lFiles.begin(), lFiles.end(), [&](const std::string& sFile)
        {
            return sFile.size() == 8u && sFile.substr(0, 2) == sLanguage.substr(0, 2);
        });

        if (mIter == lFiles.end())
            continue;

        std::string sLanguageFile = sFolderPath + "/" + *mIter;
        load_translation_file(sLanguageFile);
        return;
    }

    // If no match found, fall back to US english
    std::string sLanguageFile = sFolderPath + "/enUS.lua";
    if (utils::file_exists(sLanguageFile))
    {
        load_translation_file(sLanguageFile);
        return;
    }
}

void localizer::load_translation_file(const std::string& sFilename) try
{
    auto mResult = mLua_.safe_script_file(sFilename);
    if (!mResult.valid())
    {
        sol::error mError = mResult;
        gui::out << gui::error << "gui::locale : " << mError.what() << std::endl;
        return;
    }

    sol::table mTable = mLua_["localize"];
    if (mTable == sol::lua_nil)
    {
        gui::out << gui::warning << "gui::locale : no 'localize' table in " << sFilename << std::endl;
        return;
    }

    mTable.for_each([&](const sol::object& mKey, const sol::object& mValue)
    {
        if (!mKey.is<std::string>()) return;
        std::string ks = mKey.as<std::string>();

        if (mValue.is<std::string>())
            lMap_.insert(std::make_pair(std::hash<std::string>{}(ks), mValue.as<std::string>()));
        else if (mValue.is<sol::protected_function>())
            lMap_.insert(std::make_pair(std::hash<std::string>{}(ks), mValue.as<sol::protected_function>()));
    });

    // Keep a copy so variables/functions remain alive
    mLua_["localize_" + std::to_string(std::hash<std::string>{}(sFilename))] = mTable;
}
catch (const sol::error& mError)
{
    gui::out << gui::error << "gui::locale : " << mError.what() << std::endl;
    return;
}

void localizer::clear_translations()
{
    lMap_.clear();
}

bool localizer::is_key_valid_(std::string_view sKey) const
{
    return !sKey.empty() && sKey.front() == '{' && sKey.back() == '}';
}

localizer::map_type::const_iterator localizer::find_key_(std::string_view sKey) const
{
    auto sSubstring = sKey.substr(1, sKey.size() - 2);
    return lMap_.find(std::hash<std::string_view>{}(sSubstring));
}

std::string localizer::format_string(std::string_view sMessage, sol::variadic_args mVArgs) const
{
    fmt::dynamic_format_arg_store<fmt::format_context> mStore;
    for (auto&& mArg : mVArgs)
    {
        lxgui::utils::variant mVariant;
        if (!mArg.is<sol::lua_nil_t>())
            mVariant = mArg;

        std::visit([&](auto& mValue)
        {
            using inner_type = std::decay_t<decltype(mValue)>;
            if constexpr (std::is_same_v<inner_type, lxgui::utils::empty>)
                mStore.push_back(static_cast<const char*>(""));
            else
                mStore.push_back(mValue);
        }, mVariant);
    }

    return fmt::vformat(mLocale_, sMessage, mStore);
}

std::string localizer::localize(std::string_view sKey, sol::variadic_args mVArgs) const
{
    if (!is_key_valid_(sKey)) return std::string{sKey};

    auto mIter = find_key_(sKey);
    if (mIter == lMap_.end()) return std::string{sKey};

    return std::visit([&](const auto& mItem)
    {
        using inner_type = std::decay_t<decltype(mItem)>;
        if constexpr (std::is_same_v<inner_type, std::string>)
        {
            return format_string(mItem, mVArgs);
        }
        else
        {
            auto mResult = mItem(mVArgs);
            if (!mResult.valid())
            {
                sol::error mError = mResult;
                gui::out << gui::error << "gui::locale : " << mError.what() << std::endl;
                return std::string{sKey};
            }

            if (mResult.begin() != mResult.end())
            {
                auto&& mFirst = *mResult.begin();
                if (mFirst.template is<std::string>())
                    return mFirst.template as<std::string>();
            }

            return std::string{sKey};
        }
    }, mIter->second);
}

}
}
