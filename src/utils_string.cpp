#include "lxgui/utils_string.hpp"

#include "utf8.h"

#include <sstream>

/** \cond NOT_REMOVE_FROM_DOC
 */
namespace lxgui { namespace utils {

string_view trim(string_view s, char cPattern) {
    std::size_t uiStart = s.find_first_not_of(cPattern);
    if (uiStart == s.npos)
        return {};

    s = s.substr(uiStart);

    std::size_t uiEnd = s.find_last_not_of(cPattern);
    if (uiEnd != s.npos)
        s = s.substr(0, uiEnd + 1);

    return s;
}

string_view trim(string_view s, string_view sPatterns) {
    std::size_t uiStart = s.find_first_not_of(sPatterns);
    if (uiStart == s.npos)
        return {};

    s = s.substr(uiStart);

    std::size_t uiEnd = s.find_last_not_of(sPatterns);
    if (uiEnd != s.npos)
        s = s.substr(0, uiEnd + 1);

    return s;
}

void replace(string& s, string_view sPattern, string_view sReplacement) {
    std::size_t uiPos = s.find(sPattern);

    while (uiPos != s.npos) {
        s.replace(uiPos, sPattern.length(), sReplacement);
        uiPos = s.find(sPattern, uiPos + sReplacement.length());
    }
}

std::size_t count_occurrences(string_view s, string_view sPattern) {
    std::size_t uiCount = 0;
    std::size_t uiPos   = s.find(sPattern);
    while (uiPos != s.npos) {
        ++uiCount;
        uiPos = s.find(sPattern, uiPos + 1);
    }

    return uiCount;
}

template<typename T>
std::vector<std::basic_string_view<T>>
cut_template(std::basic_string_view<T> s, std::basic_string_view<T> sDelim) {
    std::vector<std::basic_string_view<T>> lPieces;
    std::size_t                            uiPos     = s.find(sDelim);
    std::size_t                            uiLastPos = 0u;
    std::size_t                            uiCurSize = 0u;

    while (uiPos != std::basic_string_view<T>::npos) {
        uiCurSize = uiPos - uiLastPos;
        if (uiCurSize != 0)
            lPieces.push_back(s.substr(uiLastPos, uiCurSize));
        uiLastPos = uiPos + sDelim.size();
        uiPos     = s.find(sDelim, uiLastPos);
    }

    lPieces.push_back(s.substr(uiLastPos));

    return lPieces;
}

std::vector<string_view> cut(string_view s, string_view sDelim) {
    return cut_template(s, sDelim);
}

std::vector<ustring_view> cut(ustring_view s, ustring_view sDelim) {
    return cut_template(s, sDelim);
}

template<typename T>
std::vector<std::basic_string_view<T>>
cut_each_template(std::basic_string_view<T> s, std::basic_string_view<T> sDelim) {
    std::vector<std::basic_string_view<T>> lPieces;
    std::size_t                            uiPos     = s.find(sDelim);
    std::size_t                            uiLastPos = 0u;
    std::size_t                            uiCurSize = 0u;

    while (uiPos != std::basic_string_view<T>::npos) {
        uiCurSize = uiPos - uiLastPos;
        lPieces.push_back(s.substr(uiLastPos, uiCurSize));
        uiLastPos = uiPos + sDelim.size();
        uiPos     = s.find(sDelim, uiLastPos);
    }

    lPieces.push_back(s.substr(uiLastPos));

    return lPieces;
}

std::vector<string_view> cut_each(string_view s, string_view sDelim) {
    return cut_each_template(s, sDelim);
}

std::vector<ustring_view> cut_each(ustring_view s, ustring_view sDelim) {
    return cut_each_template(s, sDelim);
}

template<typename T>
std::pair<std::basic_string_view<T>, std::basic_string_view<T>>
cut_first_template(std::basic_string_view<T> s, std::basic_string_view<T> sDelim) {
    std::size_t uiPos = s.find(sDelim);
    if (uiPos == std::basic_string_view<T>::npos)
        return {};

    return {s.substr(0, uiPos), s.substr(uiPos + 1u)};
}

std::pair<string_view, string_view> cut_first(string_view s, string_view sDelim) {
    return cut_first_template(s, sDelim);
}

std::pair<ustring_view, ustring_view> cut_first(ustring_view s, ustring_view sDelim) {
    return cut_first_template(s, sDelim);
}

bool has_no_content(string_view s) {
    if (s.empty())
        return true;

    for (std::size_t i = 0; i < s.length(); ++i) {
        if (s[i] != ' ' && s[i] != '\t')
            return false;
    }

    return true;
}

bool starts_with(string_view s, string_view sPattern) {
    std::size_t n = std::min(s.size(), sPattern.size());
    for (std::size_t i = 0; i < n; ++i) {
        if (s[i] != sPattern[i])
            return false;
    }

    return true;
}

bool ends_with(string_view s, string_view sPattern) {
    std::size_t ss = s.size();
    std::size_t ps = sPattern.size();
    std::size_t n  = std::min(ss, ps);
    for (std::size_t i = 1; i <= n; ++i) {
        if (s[ss - i] != sPattern[ps - i])
            return false;
    }

    return true;
}

ustring utf8_to_unicode(string_view s) {
    return utf8::utf8to32(s);
}

string unicode_to_utf8(ustring_view s) {
    return utf8::utf32to8(s);
}

std::size_t hex_to_uint(string_view s) {
    std::size_t        i = 0;
    std::istringstream ss{std::string(s)};
    ss.imbue(std::locale::classic());
    ss >> std::hex >> i;
    return i;
}

template<typename T>
bool from_string_template(string_view s, T& v) {
    std::istringstream ss{std::string(s)};
    ss.imbue(std::locale::classic());
    ss >> v;

    if (!ss.fail()) {
        if (ss.eof())
            return true;

        std::string rem;
        ss >> rem;

        return rem.find_first_not_of(" \t") == rem.npos;
    }

    return false;
}

bool from_string(string_view s, int& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, long& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, long long& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, unsigned& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, unsigned long& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, unsigned long long& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, float& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, double& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, bool& v) {
    v = s == "true";
    return v || s == "false";
}

bool from_string(string_view s, string& v) {
    v = s;
    return true;
}

template<typename T>
bool from_string_template(ustring_view s, T& v) {
    return from_string(unicode_to_utf8(s), v);
}

bool from_string(ustring_view s, int& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, long& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, long long& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, unsigned& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, unsigned long& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, unsigned long long& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, float& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, double& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, bool& v) {
    v = s == U"true";
    return v || s == U"false";
}

bool from_string(ustring_view s, ustring& v) {
    v = s;
    return true;
}

bool is_number(string_view s) {
    std::istringstream mTemp{std::string(s)};
    mTemp.imbue(std::locale::classic());

    double dValue = 0;
    mTemp >> dValue;

    return !mTemp.fail();
}

bool is_number(ustring_view s) {
    return is_number(unicode_to_utf8(s));
}

bool is_number(char s) {
    return '0' <= s && s <= '9';
}

bool is_number(char32_t s) {
    return U'0' <= s && s <= U'9';
}

bool is_integer(string_view s) {
    std::istringstream mTemp{std::string(s)};
    mTemp.imbue(std::locale::classic());

    std::int64_t iValue = 0;
    mTemp >> iValue;

    return !mTemp.fail();
}

bool is_integer(ustring_view s) {
    return is_integer(unicode_to_utf8(s));
}

bool is_integer(char s) {
    return is_number(s);
}

bool is_integer(char32_t s) {
    return is_number(s);
}

bool is_boolean(string_view s) {
    return (s == "false") || (s == "true");
}

bool is_boolean(ustring_view s) {
    return (s == U"false") || (s == U"true");
}

bool is_whitespace(char c) {
    return c == '\n' || c == ' ' || c == '\t' || c == '\r';
}

bool is_whitespace(char32_t c) {
    return c == U'\n' || c == U' ' || c == U'\t' || c == '\r';
}

template<typename T>
string to_string_template(T mValue) {
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << mValue;
    return ss.str();
}

string to_string(int v) {
    return to_string_template(v);
}

string to_string(long v) {
    return to_string_template(v);
}

string to_string(long long v) {
    return to_string_template(v);
}

string to_string(unsigned v) {
    return to_string_template(v);
}

string to_string(unsigned long v) {
    return to_string_template(v);
}

string to_string(unsigned long long v) {
    return to_string_template(v);
}

string to_string(float v) {
    return to_string_template(v);
}

string to_string(double v) {
    return to_string_template(v);
}

string to_string(bool b) {
    return b ? "true" : "false";
}

string to_string(void* p) {
    std::ostringstream sStream;
    sStream.imbue(std::locale::classic());
    sStream << p;
    return sStream.str();
}

std::string to_string(const utils::variant& mValue) {
    return std::visit(
        [&](const auto& mInnerValue) -> std::string {
            using inner_type = std::decay_t<decltype(mInnerValue)>;
            if constexpr (std::is_same_v<inner_type, utils::empty>)
                return "<none>";
            else
                return to_string(mInnerValue);
        },
        mValue);
}
}} // namespace lxgui::utils

/** \endcond
 */
