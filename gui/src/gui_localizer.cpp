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

namespace
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
        // TODO
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
}

localizer::localizer()
{
    try {
        // Try to set locale to system default.
        mLocale_ = std::locale("");
    } catch (const std::exception& mException) {
        // Revert to C locale.
        mLocale_ = std::locale::classic();
        gui::out << gui::error << "gui::locale : " << mException.what() << std::endl;
    }

    // Find default languages.
    auto_detect_preferred_languages();
    auto_detect_allowed_code_points();

    // Set up Lua sandbox.
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

void localizer::auto_detect_preferred_languages()
{
    set_preferred_languages(get_default_languages());
}

const std::locale& localizer::get_locale() const
{
    return mLocale_;
}

const std::vector<std::string>& localizer::get_preferred_languages() const
{
    return lLanguages_;
}

void localizer::clear_allowed_code_points()
{
    lCodePoints_.clear();
}

void localizer::add_allowed_code_points(const code_point_range& mRange)
{
    if (mRange.uiLast < mRange.uiFirst)
        throw gui::exception("gui::localizer", "code point range must have last >= first");

    code_point_range mTestRange = mRange;
    auto mIter = lCodePoints_.begin();

    do
    {
        // Find next overlapping range
        mIter = std::find_if(lCodePoints_.begin(), lCodePoints_.end(), [&](const auto& mOther)
        {
            return (mTestRange.uiFirst >= mOther.uiFirst && mTestRange.uiFirst <= mOther.uiLast) ||
                   (mTestRange.uiLast  >= mOther.uiFirst && mTestRange.uiLast  <= mOther.uiLast) ||
                   (mOther.uiFirst >= mTestRange.uiFirst && mOther.uiFirst <= mTestRange.uiLast) ||
                   (mOther.uiLast  >= mTestRange.uiFirst && mOther.uiLast  <= mTestRange.uiLast);
        });

        if (mIter != lCodePoints_.end())
        {
            // Combine the ranges
            mTestRange.uiFirst = std::min(mTestRange.uiFirst, mIter->uiFirst);
            mTestRange.uiLast = std::max(mTestRange.uiLast, mIter->uiLast);

            // Erase the overlap
            lCodePoints_.erase(mIter);
        }
    } while (mIter != lCodePoints_.end());

    // Add the new range
    lCodePoints_.push_back(mTestRange);

    // Sort by ascending code point
    std::sort(lCodePoints_.begin(), lCodePoints_.end(), [](const auto& mLeft, const auto& mRight)
    {
        return mLeft.uiFirst < mRight.uiFirst;
    });
}

void localizer::add_allowed_code_points(const std::string& sUnicodeGroup)
{
    // List from http://www.unicode.org/Public/5.2.0/ucdxml/ucd.all.flat.zip
    // Adjusted "basic latin" and "latin-1 supplement" to remove non-printable chars.
    static std::unordered_map<std::string, code_point_range> lUnicodeGroups = {
        {"basic latin", {0x0020, 0x007e}},
        {"latin-1 supplement", {0x00a0, 0x00ff}},
        {"latin extended-a", {0x0100, 0x017f}},
        {"latin extended-b", {0x0180, 0x024f}},
        {"ipa extensions", {0x0250, 0x02af}},
        {"spacing modifier letters", {0x02b0, 0x02ff}},
        {"combining diacritical marks", {0x0300, 0x036f}},
        {"greek and coptic", {0x0370, 0x03ff}},
        {"cyrillic", {0x0400, 0x04ff}},
        {"cyrillic supplement", {0x0500, 0x052f}},
        {"armenian", {0x0530, 0x058f}},
        {"hebrew", {0x0590, 0x05ff}},
        {"arabic", {0x0600, 0x06ff}},
        {"syriac", {0x0700, 0x074f}},
        {"arabic supplement", {0x0750, 0x077f}},
        {"thaana", {0x0780, 0x07bf}},
        {"nko", {0x07c0, 0x07ff}},
        {"samaritan", {0x0800, 0x083f}},
        {"devanagari", {0x0900, 0x097f}},
        {"bengali", {0x0980, 0x09ff}},
        {"gurmukhi", {0x0a00, 0x0a7f}},
        {"gujarati", {0x0a80, 0x0aff}},
        {"oriya", {0x0b00, 0x0b7f}},
        {"tamil", {0x0b80, 0x0bff}},
        {"telugu", {0x0c00, 0x0c7f}},
        {"kannada", {0x0c80, 0x0cff}},
        {"malayalam", {0x0d00, 0x0d7f}},
        {"sinhala", {0x0d80, 0x0dff}},
        {"thai", {0x0e00, 0x0e7f}},
        {"lao", {0x0e80, 0x0eff}},
        {"tibetan", {0x0f00, 0x0fff}},
        {"myanmar", {0x1000, 0x109f}},
        {"georgian", {0x10a0, 0x10ff}},
        {"hangul jamo", {0x1100, 0x11ff}},
        {"ethiopic", {0x1200, 0x137f}},
        {"ethiopic supplement", {0x1380, 0x139f}},
        {"cherokee", {0x13a0, 0x13ff}},
        {"unified canadian aboriginal syllabics", {0x1400, 0x167f}},
        {"ogham", {0x1680, 0x169f}},
        {"runic", {0x16a0, 0x16ff}},
        {"tagalog", {0x1700, 0x171f}},
        {"hanunoo", {0x1720, 0x173f}},
        {"buhid", {0x1740, 0x175f}},
        {"tagbanwa", {0x1760, 0x177f}},
        {"khmer", {0x1780, 0x17ff}},
        {"mongolian", {0x1800, 0x18af}},
        {"unified canadian aboriginal syllabics extended", {0x18b0, 0x18ff}},
        {"limbu", {0x1900, 0x194f}},
        {"tai le", {0x1950, 0x197f}},
        {"new tai lue", {0x1980, 0x19df}},
        {"khmer symbols", {0x19e0, 0x19ff}},
        {"buginese", {0x1a00, 0x1a1f}},
        {"tai tham", {0x1a20, 0x1aaf}},
        {"balinese", {0x1b00, 0x1b7f}},
        {"sundanese", {0x1b80, 0x1bbf}},
        {"lepcha", {0x1c00, 0x1c4f}},
        {"ol chiki", {0x1c50, 0x1c7f}},
        {"vedic extensions", {0x1cd0, 0x1cff}},
        {"phonetic extensions", {0x1d00, 0x1d7f}},
        {"phonetic extensions supplement", {0x1d80, 0x1dbf}},
        {"combining diacritical marks supplement", {0x1dc0, 0x1dff}},
        {"latin extended additional", {0x1e00, 0x1eff}},
        {"greek extended", {0x1f00, 0x1fff}},
        {"general punctuation", {0x2000, 0x206f}},
        {"superscripts and subscripts", {0x2070, 0x209f}},
        {"currency symbols", {0x20a0, 0x20cf}},
        {"combining diacritical marks for symbols", {0x20d0, 0x20ff}},
        {"letterlike symbols", {0x2100, 0x214f}},
        {"number forms", {0x2150, 0x218f}},
        {"arrows", {0x2190, 0x21ff}},
        {"mathematical operators", {0x2200, 0x22ff}},
        {"miscellaneous technical", {0x2300, 0x23ff}},
        {"control pictures", {0x2400, 0x243f}},
        {"optical character recognition", {0x2440, 0x245f}},
        {"enclosed alphanumerics", {0x2460, 0x24ff}},
        {"box drawing", {0x2500, 0x257f}},
        {"block elements", {0x2580, 0x259f}},
        {"geometric shapes", {0x25a0, 0x25ff}},
        {"miscellaneous symbols", {0x2600, 0x26ff}},
        {"dingbats", {0x2700, 0x27bf}},
        {"miscellaneous mathematical symbols-a", {0x27c0, 0x27ef}},
        {"supplemental arrows-a", {0x27f0, 0x27ff}},
        {"braille patterns", {0x2800, 0x28ff}},
        {"supplemental arrows-b", {0x2900, 0x297f}},
        {"miscellaneous mathematical symbols-b", {0x2980, 0x29ff}},
        {"supplemental mathematical operators", {0x2a00, 0x2aff}},
        {"miscellaneous symbols and arrows", {0x2b00, 0x2bff}},
        {"glagolitic", {0x2c00, 0x2c5f}},
        {"latin extended-c", {0x2c60, 0x2c7f}},
        {"coptic", {0x2c80, 0x2cff}},
        {"georgian supplement", {0x2d00, 0x2d2f}},
        {"tifinagh", {0x2d30, 0x2d7f}},
        {"ethiopic extended", {0x2d80, 0x2ddf}},
        {"cyrillic extended-a", {0x2de0, 0x2dff}},
        {"supplemental punctuation", {0x2e00, 0x2e7f}},
        {"cjk radicals supplement", {0x2e80, 0x2eff}},
        {"kangxi radicals", {0x2f00, 0x2fdf}},
        {"ideographic description characters", {0x2ff0, 0x2fff}},
        {"cjk symbols and punctuation", {0x3000, 0x303f}},
        {"hiragana", {0x3040, 0x309f}},
        {"katakana", {0x30a0, 0x30ff}},
        {"bopomofo", {0x3100, 0x312f}},
        {"hangul compatibility jamo", {0x3130, 0x318f}},
        {"kanbun", {0x3190, 0x319f}},
        {"bopomofo extended", {0x31a0, 0x31bf}},
        {"cjk strokes", {0x31c0, 0x31ef}},
        {"katakana phonetic extensions", {0x31f0, 0x31ff}},
        {"enclosed cjk letters and months", {0x3200, 0x32ff}},
        {"cjk compatibility", {0x3300, 0x33ff}},
        {"cjk unified ideographs extension a", {0x3400, 0x4dbf}},
        {"yijing hexagram symbols", {0x4dc0, 0x4dff}},
        {"cjk unified ideographs", {0x4e00, 0x9fff}},
        {"yi syllables", {0xa000, 0xa48f}},
        {"yi radicals", {0xa490, 0xa4cf}},
        {"lisu", {0xa4d0, 0xa4ff}},
        {"vai", {0xa500, 0xa63f}},
        {"cyrillic extended-b", {0xa640, 0xa69f}},
        {"bamum", {0xa6a0, 0xa6ff}},
        {"modifier tone letters", {0xa700, 0xa71f}},
        {"latin extended-d", {0xa720, 0xa7ff}},
        {"syloti nagri", {0xa800, 0xa82f}},
        {"common indic number forms", {0xa830, 0xa83f}},
        {"phags-pa", {0xa840, 0xa87f}},
        {"saurashtra", {0xa880, 0xa8df}},
        {"devanagari extended", {0xa8e0, 0xa8ff}},
        {"kayah li", {0xa900, 0xa92f}},
        {"rejang", {0xa930, 0xa95f}},
        {"hangul jamo extended-a", {0xa960, 0xa97f}},
        {"javanese", {0xa980, 0xa9df}},
        {"cham", {0xaa00, 0xaa5f}},
        {"myanmar extended-a", {0xaa60, 0xaa7f}},
        {"tai viet", {0xaa80, 0xaadf}},
        {"meetei mayek", {0xabc0, 0xabff}},
        {"hangul syllables", {0xac00, 0xd7af}},
        {"hangul jamo extended-b", {0xd7b0, 0xd7ff}},
        {"high surrogates", {0xd800, 0xdb7f}},
        {"high private use surrogates", {0xdb80, 0xdbff}},
        {"low surrogates", {0xdc00, 0xdfff}},
        {"private use area", {0xe000, 0xf8ff}},
        {"cjk compatibility ideographs", {0xf900, 0xfaff}},
        {"alphabetic presentation forms", {0xfb00, 0xfb4f}},
        {"arabic presentation forms-a", {0xfb50, 0xfdff}},
        {"variation selectors", {0xfe00, 0xfe0f}},
        {"vertical forms", {0xfe10, 0xfe1f}},
        {"combining half marks", {0xfe20, 0xfe2f}},
        {"cjk compatibility forms", {0xfe30, 0xfe4f}},
        {"small form variants", {0xfe50, 0xfe6f}},
        {"arabic presentation forms-b", {0xfe70, 0xfeff}},
        {"halfwidth and fullwidth forms", {0xff00, 0xffef}},
        {"specials", {0xfff0, 0xffff}},
        {"linear b syllabary", {0x10000, 0x1007f}},
        {"linear b ideograms", {0x10080, 0x100ff}},
        {"aegean numbers", {0x10100, 0x1013f}},
        {"ancient greek numbers", {0x10140, 0x1018f}},
        {"ancient symbols", {0x10190, 0x101cf}},
        {"phaistos disc", {0x101d0, 0x101ff}},
        {"lycian", {0x10280, 0x1029f}},
        {"carian", {0x102a0, 0x102df}},
        {"old italic", {0x10300, 0x1032f}},
        {"gothic", {0x10330, 0x1034f}},
        {"ugaritic", {0x10380, 0x1039f}},
        {"old persian", {0x103a0, 0x103df}},
        {"deseret", {0x10400, 0x1044f}},
        {"shavian", {0x10450, 0x1047f}},
        {"osmanya", {0x10480, 0x104af}},
        {"cypriot syllabary", {0x10800, 0x1083f}},
        {"imperial aramaic", {0x10840, 0x1085f}},
        {"phoenician", {0x10900, 0x1091f}},
        {"lydian", {0x10920, 0x1093f}},
        {"kharoshthi", {0x10a00, 0x10a5f}},
        {"old south arabian", {0x10a60, 0x10a7f}},
        {"avestan", {0x10b00, 0x10b3f}},
        {"inscriptional parthian", {0x10b40, 0x10b5f}},
        {"inscriptional pahlavi", {0x10b60, 0x10b7f}},
        {"old turkic", {0x10c00, 0x10c4f}},
        {"rumi numeral symbols", {0x10e60, 0x10e7f}},
        {"kaithi", {0x11080, 0x110cf}},
        {"cuneiform", {0x12000, 0x123ff}},
        {"cuneiform numbers and punctuation", {0x12400, 0x1247f}},
        {"egyptian hieroglyphs", {0x13000, 0x1342f}},
        {"byzantine musical symbols", {0x1d000, 0x1d0ff}},
        {"musical symbols", {0x1d100, 0x1d1ff}},
        {"ancient greek musical notation", {0x1d200, 0x1d24f}},
        {"tai xuan jing symbols", {0x1d300, 0x1d35f}},
        {"counting rod numerals", {0x1d360, 0x1d37f}},
        {"mathematical alphanumeric symbols", {0x1d400, 0x1d7ff}},
        {"mahjong tiles", {0x1f000, 0x1f02f}},
        {"domino tiles", {0x1f030, 0x1f09f}},
        {"enclosed alphanumeric supplement", {0x1f100, 0x1f1ff}},
        {"enclosed ideographic supplement", {0x1f200, 0x1f2ff}},
        {"cjk unified ideographs extension b", {0x20000, 0x2a6df}},
        {"cjk unified ideographs extension c", {0x2a700, 0x2b73f}},
        {"cjk compatibility ideographs supplement", {0x2f800, 0x2fa1f}},
        {"tags", {0xe0000, 0xe007f}},
        {"variation selectors supplement", {0xe0100, 0xe01ef}},
        {"supplementary private use area-a", {0xf0000, 0xfffff}},
        {"supplementary private use area-b", {0x100000, 0x10ffff}}
    };

    auto mIter = lUnicodeGroups.find(to_lower(sUnicodeGroup));
    if (mIter == lUnicodeGroups.end())
        throw gui::exception("gui::localizer", "unknown Unicode group '" + sUnicodeGroup + "'");

    add_allowed_code_points(mIter->second);
}

void localizer::auto_detect_allowed_code_points()
{
    // TODO: fill up that list
    static const std::vector<std::string> lLatinSupplementLanguages = {};

    clear_allowed_code_points();

    // Add basic latin (= ASCII) for all languages (required to display URLs for example).
    add_allowed_code_points("basic latin");

    // Add language-specific groups
    for (const auto& sLanguage : lLanguages_)
    {
        std::string_view sCode(sLanguage.c_str(), 2u);
        auto is_any_of = [&](const auto& lList)
        {
            return std::find(lList.begin(), lList.end(), sCode) != lList.end();
        };

        if (is_any_of(lLatinSupplementLanguages))
            add_allowed_code_points("latin-1 supplement");
        // TODO add more
    }
}

const std::vector<code_point_range>& localizer::get_allowed_code_points() const
{
    return lCodePoints_;
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
