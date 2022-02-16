#include "lxgui/gui_localizer.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_filesystem.hpp"
#include "lxgui/utils_range.hpp"
#include "lxgui/utils_string.hpp"
#include "lxgui/utils_variant.hpp"

#include <cstdlib>
#include <cstring>
#include <fmt/args.h>
#include <functional>
#include <sol/state.hpp>

namespace lxgui::gui {

namespace {
std::string to_lower(std::string str) {
    for (char& c : str)
        c = static_cast<char>(std::tolower(c));

    return str;
}

std::string get_environment_variable(const std::string& name) {
#if defined(LXGUI_PLATFORM_WINDOWS)
    // Windows has std::getenv, but MSVC offers a safer alternative that it insists on using
    char*       sBuffer = nullptr;
    std::size_t uiSize  = 0;
    if (_dupenv_s(&sBuffer, &uiSize, sName.c_str()) != 0 || sBuffer == nullptr)
        return "";

    std::string sResult = sBuffer;
    free(sBuffer);
    return sResult;
#else
    const char* result = std::getenv(name.c_str());
    return result != nullptr ? result : "";
#endif
}

std::vector<std::string> get_default_languages() {
    // First try parsing the LANGUAGE environment variable.
    // This is the best, because it lets the user specify a list of languages
    // in descending priority, so if a translation is unavailable in their
    // primary language, they may still get another match which would be
    // better for them than the default enUS (e.g., a French person could
    // prefer to fall back on a Spanish translation rather than English).
    const std::string language_var = get_environment_variable("LANGUAGE");
    if (!language_var.empty()) {
        std::vector<std::string> output;
        for (auto language : utils::cut(language_var, ":")) {
            std::string language_normalized{language};
            utils::replace(language_normalized, "_", "");
            if (language_normalized.size() == 4)
                output.push_back(language_normalized);
        }

        if (!output.empty())
            return output;
    }

#if defined(LXGUI_PLATFORM_WINDOWS)
    // If LANGUAGE is not specified, on Windows, try OS-specific function.
    // TODO: https://github.com/cschreib/lxgui/issues/95
#endif

    // If LANGUAGE is not specified or empty, try LANG.
    std::string lang = get_environment_variable("LANG");
    if (!lang.empty()) {
        auto ui_pos1 = lang.find_first_of(".@");
        if (ui_pos1 != std::string::npos)
            lang = lang.substr(0, ui_pos1);

        utils::replace(lang, "_", "");
        if (lang.size() == 4)
            return {lang};
    }

    return {"enUS"};
}
} // namespace

localizer::localizer() {
    try {
        // Try to set locale to system default.
        m_locale_ = std::locale("");
    } catch (const std::exception& m_exception) {
        // Revert to C locale.
        m_locale_ = std::locale::classic();
        gui::out << gui::error << "gui::locale : " << m_exception.what() << std::endl;
        gui::out << gui::error << "gui::locale : reverting to default classic locale" << std::endl;
    }

    // Find default languages.
    auto_detect_preferred_languages();
    auto_detect_allowed_code_points();

    // Set up Lua sandbox.
    m_lua_.open_libraries(
        sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::io, sol::lib::os,
        sol::lib::string, sol::lib::debug);

    // Give the translation Lua state the localize_string and format_string function, so
    // it can call it recursively as needed, but not the other functions
    // which could load more translation strings.
    m_lua_.set_function(
        "localize_string", [&](const std::string& key, sol::variadic_args m_v_args) {
            return localize(key, m_v_args);
        });
    m_lua_.set_function("format_string", [&](const std::string& key, sol::variadic_args m_v_args) {
        return format_string(key, m_v_args);
    });
}

void localizer::set_locale(const std::locale& m_locale) {
    if (m_locale_ == m_locale)
        return;

    m_locale_ = m_locale;
    clear_translations();
}

void localizer::set_preferred_languages(const std::vector<std::string>& languages) {
    // TODO implement more generic input checks
    // https://github.com/cschreib/lxgui/issues/98
    // for (const auto& sLanguage : lLanguages)
    // {
    //     if (sLanguage.size() != 4)
    //         throw gui::exception("gui::localizer", "language code must have exactly 4
    //         characters");
    // }

    languages_ = languages;
    clear_translations();
}

void localizer::auto_detect_preferred_languages() {
    set_preferred_languages(get_default_languages());
}

const std::locale& localizer::get_locale() const {
    return m_locale_;
}

const std::vector<std::string>& localizer::get_preferred_languages() const {
    return languages_;
}

void localizer::clear_allowed_code_points() {
    code_points_.clear();
}

void localizer::add_allowed_code_points(const code_point_range& m_range) {
    if (m_range.ui_last < m_range.ui_first)
        throw gui::exception("gui::localizer", "code point range must have last >= first");

    code_point_range m_test_range = m_range;
    auto             m_iter       = code_points_.begin();

    do {
        // Find next overlapping range
        m_iter = std::find_if(code_points_.begin(), code_points_.end(), [&](const auto& m_other) {
            return (m_test_range.ui_first >= m_other.ui_first &&
                    m_test_range.ui_first <= m_other.ui_last) ||
                   (m_test_range.ui_last >= m_other.ui_first &&
                    m_test_range.ui_last <= m_other.ui_last) ||
                   (m_other.ui_first >= m_test_range.ui_first &&
                    m_other.ui_first <= m_test_range.ui_last) ||
                   (m_other.ui_last >= m_test_range.ui_first &&
                    m_other.ui_last <= m_test_range.ui_last);
        });

        if (m_iter != code_points_.end()) {
            // Combine the ranges
            m_test_range.ui_first = std::min(m_test_range.ui_first, m_iter->ui_first);
            m_test_range.ui_last  = std::max(m_test_range.ui_last, m_iter->ui_last);

            // Erase the overlap
            code_points_.erase(m_iter);
        }
    } while (m_iter != code_points_.end());

    // Add the new range
    code_points_.push_back(m_test_range);

    // Sort by ascending code point
    std::sort(
        code_points_.begin(), code_points_.end(),
        [](const auto& m_left, const auto& m_right) { return m_left.ui_first < m_right.ui_first; });
}

void localizer::add_allowed_code_points_for_group(const std::string& unicode_group) {
    // List from http://www.unicode.org/Public/5.2.0/ucdxml/ucd.all.flat.zip
    // Adjusted "basic latin" and "latin-1 supplement" to remove non-printable chars.
    static std::unordered_map<std::string, code_point_range> unicode_groups = {
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
        // added manually! source https://en.wikipedia.org/wiki/Mandaic_script
        {"mandaic", {0x0840, 0x085f}},
        // added manually! source https://en.wikipedia.org/wiki/Arabic_alphabet
        {"arabic extended-a", {0x08a0, 0x08ff}},
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
        // added manually! source https://en.wikipedia.org/wiki/Batak_script
        {"batak", {0x1bc0, 0x1bff}},
        {"lepcha", {0x1c00, 0x1c4f}},
        {"ol chiki", {0x1c50, 0x1c7f}},
        // added manually! source https://en.wikipedia.org/wiki/Cyrillic_script
        {"cyrillic extended-c", {0x1c80, 0x1c8f}},
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
        // added manually! source https://en.wikipedia.org/wiki/Latin_script_in_Unicode
        {"latin extended-e", {0xab30, 0xab6f}},
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
        // added manually! source https://en.wikipedia.org/wiki/Caucasian_Albanian_script
        {"caucasian albanian", {0x10530, 0x1056f}},
        // added manually! source https://en.wikipedia.org/wiki/Linear_A
        {"linear a", {0x10600, 0x1077f}},
        {"aegean numbers", {0x10100, 0x1013f}},
        {"ancient greek numbers", {0x10140, 0x1018f}},
        {"ancient symbols", {0x10190, 0x101cf}},
        {"phaistos disc", {0x101d0, 0x101ff}},
        {"lycian", {0x10280, 0x1029f}},
        {"carian", {0x102a0, 0x102df}},
        {"old italic", {0x10300, 0x1032f}},
        {"gothic", {0x10330, 0x1034f}},
        {"permic", {0x10350, 0x1037f}},
        {"ugaritic", {0x10380, 0x1039f}},
        {"old persian", {0x103a0, 0x103df}},
        {"deseret", {0x10400, 0x1044f}},
        {"shavian", {0x10450, 0x1047f}},
        {"osmanya", {0x10480, 0x104af}},
        // added manually! source https://en.wikipedia.org/wiki/Osage_script
        {"osage", {0x104b0, 0x104ff}},
        // added manually! source https://en.wikipedia.org/wiki/Elbasan_script
        {"elbasan", {0x10500, 0x1052f}},
        // added manually! source https://en.wikipedia.org/wiki/Latin_script_in_Unicode
        {"latin extended-f", {0x10780, 0x107bf}},
        {"cypriot syllabary", {0x10800, 0x1083f}},
        {"imperial aramaic", {0x10840, 0x1085f}},
        {"phoenician", {0x10900, 0x1091f}},
        {"lydian", {0x10920, 0x1093f}},
        {"meroitic", {0x10980, 0x109ff}},
        {"kharoshthi", {0x10a00, 0x10a5f}},
        {"old south arabian", {0x10a60, 0x10a7f}},
        // added manually! source https://en.wikipedia.org/wiki/Old_North_Arabian_(Unicode_block)
        {"old north arabian", {0x10a80, 0x10a9f}},
        // added manually! source https://en.wikipedia.org/wiki/Manichaean_script
        {"manichaean", {0x10ac0, 0x10aff}},
        {"avestan", {0x10b00, 0x10b3f}},
        {"inscriptional parthian", {0x10b40, 0x10b5f}},
        {"inscriptional pahlavi", {0x10b60, 0x10b7f}},
        {"old turkic", {0x10c00, 0x10c4f}},
        {"rumi numeral symbols", {0x10e60, 0x10e7f}},
        {"kaithi", {0x11080, 0x110cf}},
        // added manually! source https://en.wikipedia.org/wiki/Sorang_Sompeng_script
        {"sora sompeng", {0x110d0, 0x110ff}},
        // added manually! source https://en.wikipedia.org/wiki/Chakma_script
        {"chakma", {0x11100, 0x1114f}},
        // added manually! source https://en.wikipedia.org/wiki/Mahajani
        {"mahajani", {0x11150, 0x1117f}},
        // added manually! source https://en.wikipedia.org/wiki/Sharada_(Unicode_block)
        {"sharada", {0x11180, 0x111df}},
        // added manually! source https://en.wikipedia.org/wiki/Khojki_script
        {"khojki", {0x11200, 0x1124f}},
        // added manually! source https://en.wikipedia.org/wiki/Khudabadi_script
        {"khudawadi", {0x112b0, 0x112ff}},
        // added manually! source https://en.wikipedia.org/wiki/Grantha_script
        {"grantha", {0x11300, 0x1137f}},
        // added manually! source https://en.wikipedia.org/wiki/Tirhuta_script
        {"tirhuta", {0x11480, 0x114df}},
        // added manually! source https://en.wikipedia.org/wiki/Siddha%E1%B9%83_script
        {"siddham", {0x11580, 0x115ff}},
        // added manually! source https://en.wikipedia.org/wiki/Modi_script
        {"modi", {0x11600, 0x1165f}},
        // added manually! source https://en.wikipedia.org/wiki/Takri_script
        {"takri", {0x11680, 0x116cf}},
        // added manually! source https://en.wikipedia.org/wiki/Warang_Citi
        {"varang kshiti", {0x118a0, 0x118ff}},
        {"cuneiform", {0x12000, 0x123ff}},
        {"cuneiform numbers and punctuation", {0x12400, 0x1247f}},
        {"egyptian hieroglyphs", {0x13000, 0x1342f}},
        {"byzantine musical symbols", {0x1d000, 0x1d0ff}},
        {"musical symbols", {0x1d100, 0x1d1ff}},
        {"ancient greek musical notation", {0x1d200, 0x1d24f}},
        {"tai xuan jing symbols", {0x1d300, 0x1d35f}},
        {"counting rod numerals", {0x1d360, 0x1d37f}},
        {"mathematical alphanumeric symbols", {0x1d400, 0x1d7ff}},
        {"adlam", {0x1e800, 0x1e8df}},
        // added manually! source https://en.wikipedia.org/wiki/Mende_Kikakui_script
        {"mende", {0x1e900, 0x1e95f}},
        // added manually! source https://en.wikipedia.org/wiki/Arabic_alphabet
        {"arabic mathematical alphabetic symbols", {0x1ee00, 0x1eeff}},
        {"mahjong tiles", {0x1f000, 0x1f02f}},
        {"domino tiles", {0x1f030, 0x1f09f}},
        {"enclosed alphanumeric supplement", {0x1f100, 0x1f1ff}},
        {"enclosed ideographic supplement", {0x1f200, 0x1f2ff}},
        // added manually! source https://en.wikipedia.org/wiki/Mro_(Unicode_block)
        {"mro", {0x16a40, 0x16a6f}},
        // added manually! source https://en.wikipedia.org/wiki/Pahawh_Hmong
        {"pahawh hmong", {0x16b00, 0x16bbf}},
        // added manually! source https://en.wikipedia.org/wiki/Pollard_script
        {"pollard", {0x16f00, 0x16f9f}},
        // added manually! source https://en.wikipedia.org/wiki/Latin_script_in_Unicode
        {"latin extended-g", {0x1df00, 0x1dfff}},
        {"cjk unified ideographs extension b", {0x20000, 0x2a6df}},
        {"cjk unified ideographs extension c", {0x2a700, 0x2b73f}},
        // added manually! source https://en.wikipedia.org/wiki/Han_unification
        {"cjk unified ideographs extension d", {0x2b740, 0x2b81f}},
        // added manually! source https://en.wikipedia.org/wiki/Han_unification
        {"cjk unified ideographs extension e", {0x2b820, 0x2ceaf}},
        // added manually! source https://en.wikipedia.org/wiki/Han_unification
        {"cjk unified ideographs extension f", {0x2ceb0, 0x2ebef}},
        {"cjk compatibility ideographs supplement", {0x2f800, 0x2fa1f}},
        // added manually! source https://en.wikipedia.org/wiki/Han_unification
        {"cjk unified ideographs extension g", {0x30000, 0x3134f}},
        {"tags", {0xe0000, 0xe007f}},
        {"variation selectors supplement", {0xe0100, 0xe01ef}},
        {"supplementary private use area-a", {0xf0000, 0xfffff}},
        {"supplementary private use area-b", {0x100000, 0x10ffff}}};

    auto m_iter = unicode_groups.find(to_lower(unicode_group));
    if (m_iter == unicode_groups.end())
        throw gui::exception("gui::localizer", "unknown Unicode group '" + unicode_group + "'");

    add_allowed_code_points(m_iter->second);
}

void localizer::add_allowed_code_points_for_language(const std::string& language_code) {
    // Lists from http://unicode.org/Public/cldr/39/cldr-common-39.0.zip
    // Mapped manually to Unicode groups above with the help of
    // https://unicode-org.github.io/cldr-staging/charts/37/supplemental/scripts_and_languages.html
    static const std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>>
        scripts = {
            {{"basic latin", "latin-1 supplement", "latin extended-a", "latin extended-b",
              "latin extended-c", "latin extended-d", "latin extended-e", "latin extended-f",
              "latin extended-g", "latin extended additional"},
             {"aa",  "abr", "ace", "ach", "ada", "af",  "agq", "ain", "ak",  "akz", "ale", "aln",
              "amo", "an",  "ang", "aoz", "arn", "aro", "arp", "arw", "asa", "ast", "atj", "avk",
              "ay",  "az",  "bal", "ban", "bar", "bas", "bbc", "bbj", "bci", "bem", "bew", "bez",
              "bfd", "bi",  "bik", "bin", "bjn", "bkm", "bku", "bla", "bm",  "bmq", "bqv", "br",
              "brh", "bs",  "bss", "bto", "buc", "bug", "bum", "bvb", "byv", "bze", "bzx", "ca",
              "cad", "car", "cay", "cch", "ceb", "cgg", "ch",  "chk", "chn", "cho", "chp", "chy",
              "cic", "co",  "cps", "cr",  "crj", "crl", "crs", "cs",  "csb", "ctd", "cy",  "da",
              "dak", "dav", "de",  "del", "den", "dgr", "din", "dje", "dnj", "dsb", "dtm", "dtp",
              "dua", "dum", "dyo", "dyu", "ebu", "ee",  "efi", "egl", "eka", "en",  "enm", "eo",
              "es",  "esu", "et",  "ett", "eu",  "ewo", "ext", "fan", "ff",  "ffm", "fi",  "fil",
              "fit", "fj",  "fo",  "fon", "fr",  "frc", "frm", "fro", "frp", "frr", "frs", "fud",
              "fuq", "fur", "fuv", "fvr", "fy",  "ga",  "gaa", "gag", "gay", "gba", "gcr", "gd",
              "gil", "gl",  "gmh", "gn",  "goh", "gor", "gos", "grb", "gsw", "gub", "guc", "gur",
              "guz", "gv",  "gwi", "ha",  "hai", "haw", "hi",  "hif", "hil", "hmn", "hnn", "ho",
              "hop", "hr",  "hsb", "ht",  "hu",  "hup", "hz",  "ia",  "iba", "ibb", "id",  "ife",
              "ig",  "ii",  "ik",  "ikt", "ilo", "inh", "is",  "it",  "iu",  "izh", "jam", "jgo",
              "jmc", "jut", "jv",  "kab", "kac", "kaj", "kam", "kao", "kcg", "kck", "kde", "kea",
              "kfo", "kg",  "kge", "kgp", "kha", "khq", "ki",  "kiu", "kj",  "kjg", "kkj", "kl",
              "kln", "kmb", "kos", "kpe", "kr",  "kri", "krj", "krl", "ksb", "ksf", "ksh", "ku",
              "kut", "kvr", "kw",  "ky",  "la",  "lag", "laj", "lam", "lb",  "lbw", "lfn", "lg",
              "li",  "lij", "liv", "ljp", "lkt", "lmo", "ln",  "lol", "loz", "lt",  "ltg", "lu",
              "lua", "lui", "lun", "luo", "lut", "luy", "lv",  "lzz", "mad", "maf", "mak", "man",
              "mas", "maz", "mdh", "mdr", "mdt", "men", "mer", "mfe", "mg",  "mgh", "mgo", "mgy",
              "mh",  "mi",  "mic", "min", "mls", "moe", "moh", "mos", "mro", "ms",  "mt",  "mua",
              "mus", "mwk", "mwl", "mwv", "mxc", "myx", "na",  "nap", "naq", "nb",  "nch", "nd",
              "ndc", "nds", "ng",  "ngl", "nhe", "nhw", "nia", "nij", "niu", "njo", "nl",  "nmg",
              "nn",  "nnh", "no",  "nov", "nr",  "nsk", "nso", "nus", "nv",  "nxq", "ny",  "nym",
              "nyn", "nyo", "nzi", "oc",  "oj",  "om",  "osa", "osc", "pag", "pam", "pap", "pau",
              "pcd", "pcm", "pdc", "pdt", "pfl", "pko", "pl",  "pms", "pnt", "pon", "prg", "pro",
              "pt",  "puu", "qu",  "quc", "qug", "rap", "rar", "rcf", "rej", "rgn", "ria", "rif",
              "rm",  "rmf", "rmo", "rmu", "rn",  "rng", "ro",  "rob", "rof", "rom", "rtm", "rug",
              "rup", "rw",  "rwk", "sad", "saf", "saq", "sas", "sat", "sbp", "sc",  "scn", "sco",
              "scs", "sdc", "se",  "see", "sef", "seh", "sei", "ses", "sg",  "sga", "sgs", "shi",
              "sid", "sk",  "sl",  "sli", "sly", "sm",  "sma", "smj", "smn", "sms", "sn",  "snk",
              "so",  "sq",  "sr",  "srb", "srn", "srr", "ss",  "ssy", "st",  "stq", "su",  "suk",
              "sus", "sv",  "sw",  "swb", "swg", "sxn", "syi", "szl", "tbw", "tem", "teo", "ter",
              "tet", "tg",  "tiv", "tk",  "tkl", "tkr", "tli", "tly", "tmh", "tn",  "to",  "tog",
              "tpi", "tr",  "tru", "trv", "ts",  "tsg", "tsi", "ttj", "ttt", "tum", "tvl", "twq",
              "ty",  "tzm", "udm", "ug",  "uli", "umb", "uz",  "vai", "ve",  "vec", "vep", "vi",
              "vic", "vls", "vmf", "vmw", "vo",  "vot", "vro", "vun", "wa",  "wae", "war", "was",
              "wbp", "wls", "wo",  "xav", "xh",  "xog", "xum", "yao", "yap", "yav", "ybb", "yo",
              "yrl", "yua", "za",  "zag", "zap", "zea", "zmi", "zu",  "zun", "zza"}},
            {{"cyrillic", "cyrillic supplement", "cyrillic extended-a", "cyrillic extended-b",
              "cyrillic extended-c"},
             {"ab",  "abq", "ady", "aii", "alt", "av",  "az",  "ba",  "be",  "bg",  "bs",  "bua",
              "ce",  "chm", "cjs", "ckt", "crh", "cu",  "cv",  "dar", "dng", "evn", "gag", "gld",
              "inh", "kaa", "kbd", "kca", "kjh", "kk",  "koi", "kpy", "krc", "ku",  "kum", "kv",
              "ky",  "lbe", "lez", "lfn", "mdf", "mk",  "mn",  "mns", "mrj", "myv", "nog", "os",
              "pnt", "ro",  "rom", "ru",  "rue", "sah", "se",  "sel", "sr",  "tab", "tg",  "tk",
              "tkr", "tly", "tt",  "ttt", "tyv", "ude", "udm", "ug",  "uk",  "uz",  "xal", "yrk"}},
            {{"devanagari", "devanagari extended", "vedic extensions"},
             {"anp", "awa", "bap", "bfy", "bgc", "bhb", "bhi", "bho", "bjj", "bra", "brx",
              "btv", "doi", "dty", "gbm", "gom", "gon", "gvr", "hi",  "hif", "hne", "hoc",
              "hoj", "jml", "kfr", "kfy", "khn", "kok", "kru", "ks",  "lif", "mag", "mai",
              "mgp", "mr",  "mrd", "mtr", "mwr", "ne",  "new", "noe", "pi",  "raj", "rjs",
              "sa",  "sat", "sck", "sd",  "srx", "swv", "taj", "tdg", "tdh", "thl", "thq",
              "thr", "tkt", "unr", "unx", "wbr", "wtm", "xnr", "xsr"}},
            {{"arabic", "arabic supplement", "arabic extended-a", "arabic presentation forms-a",
              "arabic presentation forms-b", "arabic mathematical alphabetic symbols"},
             {"aeb", "ar",  "arq", "ars", "ary", "arz", "az",  "bal", "bej", "bft", "bgn", "bqi",
              "brh", "cja", "cjm", "ckb", "cop", "dcc", "doi", "dyo", "fa",  "fia", "gbz", "gjk",
              "gju", "glk", "ha",  "haz", "hnd", "hno", "id",  "inh", "khw", "kk",  "ks",  "ku",
              "kvx", "kxp", "ky",  "lah", "lki", "lrc", "luz", "mfa", "ms",  "mvy", "mzn", "pa",
              "prd", "ps",  "rmt", "sd",  "sdh", "shi", "skr", "so",  "sus", "swb", "tg",  "tk",
              "tly", "tr",  "trw", "ttt", "ug",  "ur",  "uz",  "wni", "wo",  "zdj"}},
            {{"cjk radicals supplement", "cjk strokes", "cjk symbols and punctuation",
              "cjk unified ideographs", "cjk unified ideographs extension a",
              "cjk unified ideographs extension b", "cjk unified ideographs extension c",
              "cjk unified ideographs extension d", "cjk unified ideographs extension e",
              "cjk unified ideographs extension f", "cjk unified ideographs extension g",
              "cjk compatibility", "cjk compatibility ideographs", "cjk compatibility forms",
              "cjk compatibility ideographs supplement", "kangxi radicals"},
             {"gan", "hak", "hsn", "lzh", "nan", "vi", "wuu", "yue", "za", "zh", "ko", "ja"}},
            {{"greek and coptic", "greek extended", "coptic"},
             {"bgx", "cop", "el", "grc", "pnt", "tsd"}},
            {{"bengali"},
             {"as", "bn", "bpy", "ccp", "grt", "kha", "lus", "mni", "rkt", "sat", "syl", "unr",
              "unx"}},
            {{"thai"}, {"kdt", "kxm", "lcp", "lwl", "pi", "sou", "th", "tts"}},
            {{"ethiopic", "ethiopic supplement", "ethiopic extended"},
             {"am", "byn", "gez", "om", "ti", "tig", "wal"}},
            {{"hebrew"}, {"he", "jpr", "jrb", "lad", "sam", "yi"}},
            {{"tibetan"}, {"bft", "bo", "dz", "taj", "tdg", "tsj"}},
            {{"unified canadian aboriginal syllabics"}, {"bft", "bo", "dz", "taj", "tdg", "tsj"}},
            {{"tifinagh"}, {"rif", "shi", "tzm", "zen", "zgh"}},
            {{"telugu"}, {"gon", "lmn", "te", "wbq"}},
            {{"syriac"}, {"aii", "ar", "syr", "tru"}},
            {{"myanmar"}, {"kht", "mnw", "my", "shn"}},
            {{"nko"}, {"bm", "man", "nqo"}},
            {{"buginese"}, {"bug", "mak", "mdr"}},
            {{"old italic"}, {"ett", "osc", "xum"}},
            {{"lao"}, {"hnj", "kjg", "lo"}},
            {{"georgian", "georgian supplement"}, {"ka", "lzz", "xmf"}},
            {{"sinhala"}, {"pi", "sa", "si"}},
            {{"tamil"}, {"bfq", "ta"}},
            {{"katakana", "katakana phonetic extensions"}, {"ain", "ryu"}},
            {{"cuneiform", "cuneiform numbers and punctuation"}, {"akk", "hit"}},
            {{"cham"}, {"cja", "cjm"}},
            {{"runic"}, {"de", "non"}},
            {{"kayah"}, {"eky", "kyu"}},
            {{"kannada"}, {"kn", "tcy"}},
            {{"mongolian"}, {"mn", "mnc"}},
            {{"phags-pa"}, {"mn", "zh"}},
            {{"oriya"}, {"or", "sat"}},
            {{"samaritan"}, {"sam", "smp"}},
            {{"armenian"}, {"hy"}},
            {{"javanese"}, {"jv"}},
            {{"gujarati"}, {"gu"}},
            {{"malayalam"}, {"ml"}},
            {{"avestan"}, {"ae"}},
            {{"aramaic"}, {"arc"}},
            {{"balinese"}, {"ban"}},
            {{"bamum"}, {"bax"}},
            {{"batak"}, {"bbc"}},
            {{"buhid"}, {"bku"}},
            {{"tai viet"}, {"blt"}},
            {{"chakma"}, {"ccp"}},
            {{"cherokee"}, {"chr"}},
            {{"takri"}, {"doi"}},
            {{"thaana"}, {"dv"}},
            {{"egyptian hieroglyphs"}, {"egy"}},
            {{"adlam"}, {"ff"}},
            {{"tagalog"}, {"fil"}},
            {{"gothic"}, {"got"}},
            {{"cypriot"}, {"grc"}},
            {{"linear b syllabary", "linear b ideograms"}, {"grc"}},
            {{"mahajani"}, {"hi"}},
            {{"pollard"}, {"hmd"}},
            {{"pahawh hmong"}, {"hmn"}},
            {{"hanunoo"}, {"hnn"}},
            {{"varang kshiti"}, {"hoc"}},
            {{"yi syllables", "yi radicals"}, {"ii"}},
            {{"hiragana", "katakana"}, {"ja"}},
            {{"new tai lue"}, {"khb"}},
            {{"khmer", "khmer symbols"}, {"km"}},
            {{"hangul jamo", "hangul compatibility jamo", "hangul jamo extended-a",
              "hangul jamo extended-b", "hangul syllables"},
             {"ko"}},
            {{"permic"}, {"kv"}},
            {{"linear a"}, {"lab"}},
            {{"lepcha"}, {"lep"}},
            {{"caucasian albanian"}, {"lez"}},
            {{"limbu"}, {"lif"}},
            {{"lisu"}, {"lis"}},
            {{"tirhuta"}, {"mai"}},
            {{"mende"}, {"men"}},
            {{"meetei mayek"}, {"mni"}},
            {{"modi"}, {"mr"}},
            {{"mro"}, {"mro"}},
            {{"mandaic"}, {"myz"}},
            {{"tai tham"}, {"nod"}},
            {{"osage"}, {"osa"}},
            {{"old turkic"}, {"otk"}},
            {{"gurmukhi"}, {"pa"}},
            {{"inscriptional pahlavi"}, {"pal"}},
            {{"old persian"}, {"peo"}},
            {{"phoenician"}, {"phn"}},
            {{"rejang"}, {"rej"}},
            {{"grantha", "sharada", "siddham"}, {"sa"}},
            {{"ol chiki"}, {"sat"}},
            {{"saurashtra"}, {"saz"}},
            {{"khojki", "khudawadi"}, {"sd"}},
            {{"ogham"}, {"sga"}},
            {{"osmanya"}, {"so"}},
            {{"elbasan"}, {"sq"}},
            {{"sora sompeng"}, {"srb"}},
            {{"sundanese"}, {"su"}},
            {{"syloti nagri"}, {"syl"}},
            {{"tagbanwa"}, {"tbw"}},
            {{"tai le"}, {"tdd"}},
            {{"ugaritic"}, {"uga"}},
            {{"vai"}, {"vai"}},
            {{"carian"}, {"xcr"}},
            {{"lycian"}, {"xlc"}},
            {{"lydian"}, {"xld"}},
            {{"manichaean"}, {"xmn"}},
            {{"meroitic"}, {"xmr"}},
            {{"old north arabian"}, {"xna"}},
            {{"inscriptional parthian"}, {"xpr"}},
            {{"old south arabian"}, {"xsa"}},
            {{"bopomofo", "bopomofo extended"}, {"zh"}}};

    // Add basic latin (= ASCII) for all languages (required to display URLs for example).
    add_allowed_code_points_for_group("basic latin");
    // Add "geometric shapes" to allow rendering the "missing character" glyph
    add_allowed_code_points_for_group("geometric shapes");

    for (const auto& m_script : scripts) {
        if (std::find(m_script.second.begin(), m_script.second.end(), language_code) ==
            m_script.second.end())
            continue;

        for (const auto& m_code_range : m_script.first)
            add_allowed_code_points_for_group(m_code_range);
    }
}

void localizer::auto_detect_allowed_code_points() {
    clear_allowed_code_points();

    if (languages_.empty()) {
        // If no language specified, fall back to basic latin (=ASCII)
        add_allowed_code_points_for_group("basic latin");
        // Add "geometric shapes" to allow rendering the "missing character" glyph
        add_allowed_code_points_for_group("geometric shapes");
        return;
    }

    // Add language-specific groups
    for (const auto& language : languages_) {
        // Extract the language code from the language string (first set of lower case letters)
        auto m_pos =
            std::find_if(language.begin(), language.end(), [](char c) { return std::isupper(c); });

        add_allowed_code_points_for_language(std::string(language.begin(), m_pos));
    }
}

const std::vector<code_point_range>& localizer::get_allowed_code_points() const {
    return code_points_;
}

void localizer::set_fallback_code_point(char32_t ui_code_point) {
    ui_default_code_point_ = ui_code_point;
}

char32_t localizer::get_fallback_code_point() const {
    return ui_default_code_point_;
}

void localizer::load_translations(const std::string& folder_path) {
    // First, look for an exact match
    for (const std::string& language : languages_) {
        std::string language_file = folder_path + "/" + language + ".lua";
        if (utils::file_exists(language_file)) {
            load_translation_file(language_file);
            return;
        }
    }

    // If no exact match found, look for an approximate match (ignore region)
    const auto files = utils::get_file_list(folder_path, false, "lua");
    for (const std::string& language : languages_) {
        auto m_iter = std::find_if(files.begin(), files.end(), [&](const std::string& file) {
            return file.size() == 8u && file.substr(0, 2) == language.substr(0, 2);
        });

        if (m_iter == files.end())
            continue;

        std::string language_file = folder_path + "/" + *m_iter;
        load_translation_file(language_file);
        return;
    }

    // If no match found, fall back to US english
    std::string language_file = folder_path + "/enUS.lua";
    if (utils::file_exists(language_file)) {
        load_translation_file(language_file);
        return;
    }
}

void localizer::load_translation_file(const std::string& file_name) try {
    auto m_result = m_lua_.safe_script_file(file_name);
    if (!m_result.valid()) {
        sol::error m_error = m_result;
        gui::out << gui::error << "gui::locale : " << m_error.what() << std::endl;
        return;
    }

    sol::table m_table = m_lua_["localize"];
    if (m_table == sol::lua_nil) {
        gui::out << gui::warning << "gui::locale : no 'localize' table in " << file_name
                 << std::endl;
        return;
    }

    m_table.for_each([&](const sol::object& m_key, const sol::object& m_value) {
        if (!m_key.is<std::string>())
            return;
        std::string ks = m_key.as<std::string>();

        if (m_value.is<std::string>())
            map_.insert(std::make_pair(std::hash<std::string>{}(ks), m_value.as<std::string>()));
        else if (m_value.is<sol::protected_function>())
            map_.insert(std::make_pair(
                std::hash<std::string>{}(ks), m_value.as<sol::protected_function>()));
    });

    // Keep a copy so variables/functions remain alive
    m_lua_["localize_" + std::to_string(std::hash<std::string>{}(file_name))] = m_table;
} catch (const sol::error& m_error) {
    gui::out << gui::error << "gui::locale : " << m_error.what() << std::endl;
    return;
}

void localizer::clear_translations() {
    map_.clear();
}

bool localizer::is_key_valid_(std::string_view key) const {
    return !key.empty() && key.front() == '{' && key.back() == '}';
}

localizer::map_type::const_iterator localizer::find_key_(std::string_view key) const {
    auto substring = key.substr(1, key.size() - 2);
    return map_.find(std::hash<std::string_view>{}(substring));
}

std::string localizer::format_string(std::string_view message, sol::variadic_args m_v_args) const {
    fmt::dynamic_format_arg_store<fmt::format_context> m_store;
    for (auto&& m_arg : m_v_args) {
        lxgui::utils::variant m_variant;
        if (!m_arg.is<sol::lua_nil_t>())
            m_variant = m_arg;

        std::visit(
            [&](auto& m_value) {
                using inner_type = std::decay_t<decltype(m_value)>;
                if constexpr (std::is_same_v<inner_type, lxgui::utils::empty>)
                    m_store.push_back(static_cast<const char*>(""));
                else
                    m_store.push_back(m_value);
            },
            m_variant);
    }

    return fmt::vformat(m_locale_, message, m_store);
}

std::string localizer::localize(std::string_view key, sol::variadic_args mVArgs) const {
    if (!is_key_valid_(key))
        return std::string{key};

    auto m_iter = find_key_(key);
    if (m_iter == map_.end())
        return std::string{key};

    return std::visit(
        [&](const auto& m_item) {
            using inner_type = std::decay_t<decltype(m_item)>;
            if constexpr (std::is_same_v<inner_type, std::string>) {
                return format_string(m_item, mVArgs);
            } else {
                auto m_result = m_item(mVArgs);
                if (!m_result.valid()) {
                    sol::error m_error = m_result;
                    gui::out << gui::error << "gui::locale : " << m_error.what() << std::endl;
                    return std::string{key};
                }

                if (m_result.begin() != m_result.end()) {
                    auto&& m_first = *m_result.begin();
                    if (m_first.template is<std::string>())
                        return m_first.template as<std::string>();
                }

                return std::string{key};
            }
        },
        m_iter->second);
}

} // namespace lxgui::gui
